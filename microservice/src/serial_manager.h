#ifndef SERIAL_MANAGER_H
#define SERIAL_MANAGER_H

#include <uv.h>
#define uv__queue_data(q, type, link)                                          \
  ((type *) ((char *) (q) - offsetof(type, link)))

#define uv__queue_init(q)                                                       \
  do {                                                                        \
    (q)->next = (q);                                                          \
    (q)->prev = (q);                                                          \
  }                                                                           \
  while (0)

#define uv__queue_empty(q)                                                      \
  ((const uv__queue *) (q) == (const uv__queue *) (q)->next)

#define uv__queue_insert_tail(h, q)                                             \
  do {                                                                        \
    (q)->next = (h);                                                          \
    (q)->prev = (h)->prev;                                                    \
    (q)->prev->next = (q);                                                    \
    (h)->prev = (q);
  }                                                                           \
  while (0)

#define uv__queue_head(h)                                                       \
  ((h)->next)

#define uv__queue_remove(q)                                                     \
  do {                                                                        \
    (q)->prev->next = (q)->next;                                              \
    (q)->next->prev = (q)->prev;
  }                                                                           \
  while (0)

struct uv__queue {
  struct uv__queue* next;
  struct uv__queue* prev;
};

// To use the queue macros from libuv, we need to include the internal header
// or replicate the necessary definitions. For simplicity, we include it here.
// In a real-world project, you might copy the queue macros to avoid this.


#define MAX_WRITE_QUEUE 16
#define DEFAULT_BUFFER_SIZE 1024

// Forward declaration
typedef struct uv_serial_s uv_serial_t;

/**
 * Custom callback type for serial read operations.
 * This avoids misusing uv_read_cb and provides a clear, type-safe interface.
 */
typedef void (*uv_serial_read_cb)(uv_serial_t *serial, ssize_t nread, const uv_buf_t *buf);

/**
 * Custom callback type for serial write operations.
 */
typedef void (*uv_serial_write_cb)(uv_serial_t *serial, int status);

/**
 * Custom callback for error notifications.
 */
typedef void (*uv_serial_error_cb)(uv_serial_t *serial, int status);


// Serial configuration structure
typedef struct {
    int baud_rate;
    int data_bits;
    int stop_bits;
    char parity;
    int timeout_ms;
    int flow_control; // 0: None, 1: RTS/CTS, 2: XON/XOFF
} uv_serial_config_t;

// Serial statistics structure
typedef struct {
    size_t bytes_sent;
    size_t bytes_received;
    size_t errors;
} uv_serial_stats_t;

// Serial state enum
typedef enum {
    UV_SERIAL_STATE_CLOSED = 0,
    UV_SERIAL_STATE_OPEN,
    UV_SERIAL_STATE_READING,
    UV_SERIAL_STATE_WRITING
} uv_serial_state_t;

// Write request structure
typedef struct uv_serial_write_s {
    uv_fs_t fs_req;
    uv_buf_t buf;
    uv_serial_write_cb cb;
    uv_serial_t *serial;
    struct uv__queue queue;
} uv_serial_write_t;

// The main serial handle structure
struct uv_serial_s {
    uv_loop_t *loop;
    uv_poll_t poll_handle;
    uv_timer_t timer_handle;
    int fd;
    uv_serial_state_t state;
    uv_serial_config_t config;
    uv_serial_stats_t stats;
    char *read_buf;
    size_t read_buf_size;
    uv_serial_read_cb read_cb;
    uv_serial_error_cb error_cb;
    uv_close_cb close_cb;
    struct uv__queue write_queue;
    void *data; // User data
};

// Public API
int uv_serial_init(uv_loop_t *loop, uv_serial_t *serial, const char *port_name, const uv_serial_config_t *config);
void uv_serial_close(uv_serial_t *serial, uv_close_cb close_cb);

int uv_serial_read_start(uv_serial_t *serial, uv_serial_read_cb alloc_cb, uv_serial_read_cb read_cb);
int uv_serial_read_stop(uv_serial_t *serial);

int uv_serial_write(uv_serial_t *serial, const uv_buf_t bufs[], unsigned int nbufs, uv_serial_write_cb cb);

#endif // SERIAL_MANAGER_H