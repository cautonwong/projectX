#include "serial_manager.h"
#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

// Static function forward declarations
static void on_poll(uv_poll_t *handle, int status, int events);
static void on_fs_write(uv_fs_t *req);

#define BAUD_RATE(b) \
  ((b) == 300      ? B300      : (b) == 600    ? B600      : \
   (b) == 1200   ? B1200     : (b) == 2400   ? B2400     : \
   (b) == 4800   ? B4800     : (b) == 9600   ? B9600     : \
   (b) == 19200  ? B19200    : (b) == 38400  ? B38400    : \
   (b) == 57600  ? B57600    : (b) == 115200 ? B115200   : B9600)

static int set_serial_attribs(int fd, const uv_serial_config_t *config) {
    struct termios tty;
    if (tcgetattr(fd, &tty) < 0) {
        LOG_ERROR("Failed to get serial attributes");
        return -1;
    }

    cfsetospeed(&tty, BAUD_RATE(config->baud_rate));
    cfsetispeed(&tty, BAUD_RATE(config->baud_rate));

    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~(0);

    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;

    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        LOG_ERROR("Failed to set serial attributes");
        return -1;
    }
    return 0;
}

int uv_serial_init(uv_loop_t *loop, uv_serial_t *serial, const char *port_name, const uv_serial_config_t *config) {
    int fd = open(port_name, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        LOG_ERROR("Failed to open serial port: %s", port_name);
        return -1;
    }

    memset(serial, 0, sizeof(*serial));
    serial->loop = loop;
    serial->fd = fd;
    serial->config = *config;
    serial->state = UV_SERIAL_STATE_OPEN;
    uv__queue_init(&serial->write_queue);

    if (set_serial_attribs(fd, config) != 0) {
        close(fd);
        return -1;
    }

    if (uv_poll_init(loop, &serial->poll_handle, fd) != 0) {
        close(fd);
        return -1;
    }
    serial->poll_handle.data = serial;

    if (uv_timer_init(loop, &serial->timer_handle) != 0) {
        uv_poll_stop(&serial->poll_handle);
        close(fd);
        return -1;
    }
    serial->timer_handle.data = serial;

    return 0;
}

void uv_serial_close(uv_serial_t *serial, uv_close_cb close_cb) {
    if (serial->state == UV_SERIAL_STATE_CLOSED) return;

    serial->state = UV_SERIAL_STATE_CLOSED;
    serial->close_cb = close_cb;

    uv_poll_stop(&serial->poll_handle);
    uv_timer_stop(&serial->timer_handle);

    uv_close((uv_handle_t*)&serial->poll_handle, NULL);
    uv_close((uv_handle_t*)&serial->timer_handle, (uv_close_cb)close_cb);
    close(serial->fd);
}

int uv_serial_read_start(uv_serial_t *serial, uv_serial_read_cb alloc_cb, uv_serial_read_cb read_cb) {
    if (serial->state == UV_SERIAL_STATE_CLOSED) return -1;
    serial->read_cb = read_cb;
    serial->state = UV_SERIAL_STATE_READING;
    return uv_poll_start(&serial->poll_handle, UV_READABLE, on_poll);
}

int uv_serial_read_stop(uv_serial_t *serial) {
    if (serial->state != UV_SERIAL_STATE_READING) return 0;
    serial->state = UV_SERIAL_STATE_OPEN;
    return uv_poll_stop(&serial->poll_handle);
}

static void process_write_queue(uv_serial_t *serial) {
    if (uv__queue_empty(&serial->write_queue)) {
        serial->state = UV_SERIAL_STATE_OPEN;
        return;
    }

    uv_serial_write_t *write_req = uv__queue_data(uv__queue_head(&serial->write_queue), uv_serial_write_t, queue);
    serial->state = UV_SERIAL_STATE_WRITING;

    uv_fs_write(serial->loop, &write_req->fs_req, serial->fd, &write_req->buf, 1, -1, on_fs_write);
}

int uv_serial_write(uv_serial_t *serial, const uv_buf_t bufs[], unsigned int nbufs, uv_serial_write_cb cb) {
    if (serial->state == UV_SERIAL_STATE_CLOSED) return -1;

    uv_serial_write_t *req = malloc(sizeof(uv_serial_write_t));
    if (!req) return -1;

    // For simplicity, this implementation only handles one buffer.
    req->buf = uv_buf_init(malloc(bufs[0].len), bufs[0].len);
    memcpy(req->buf.base, bufs[0].base, bufs[0].len);
    req->cb = cb;
    req->serial = serial;
    req->fs_req.data = req;

    uv__queue_insert_tail(&serial->write_queue, &req->queue);

    if (serial->state != UV_SERIAL_STATE_WRITING) {
        process_write_queue(serial);
    }
    return 0;
}

static void on_fs_write(uv_fs_t *req) {
    uv_serial_write_t *write_req = (uv_serial_write_t *)req->data;
    uv_serial_t *serial = write_req->serial;
    int result = req->result;

    uv_fs_req_cleanup(req);
    uv__queue_remove(&write_req->queue);

    if (write_req->cb) {
        write_req->cb(serial, result < 0 ? result : 0);
    }

    free(write_req->buf.base);
    free(write_req);

    process_write_queue(serial);
}

static void on_poll(uv_poll_t *handle, int status, int events) {
    uv_serial_t *serial = (uv_serial_t *)handle->data;
    if (status < 0) {
        if (serial->error_cb) serial->error_cb(serial, status);
        return;
    }

    if (events & UV_READABLE) {
        if (!serial->read_buf) {
            serial->read_buf = malloc(DEFAULT_BUFFER_SIZE);
            serial->read_buf_size = DEFAULT_BUFFER_SIZE;
        }
        ssize_t nread = read(serial->fd, serial->read_buf, serial->read_buf_size);
        if (nread > 0) {
            uv_buf_t buf = uv_buf_init(serial->read_buf, nread);
            serial->read_cb(serial, nread, &buf);
        } else if (nread < 0) {
            if (errno != EAGAIN && serial->error_cb) {
                serial->error_cb(serial, UV_EIO);
            }
        } else { // nread == 0 (EOF)
            if (serial->error_cb) serial->error_cb(serial, UV_EOF);
        }
    }
}