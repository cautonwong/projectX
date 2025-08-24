#ifndef HTTP_SERVER_MANAGER_H
#define HTTP_SERVER_MANAGER_H

#include <microhttpd.h>
#include <stdbool.h>
#include <uv.h>

// Forward declaration
typedef struct service service_t;

// HTTP server context structure
typedef struct http_server_context_s {
  service_t *parent_service; // Pointer to parent service_t
  uv_loop_t *loop;           // libuv event loop
  struct MHD_Daemon *daemon; // libmicrohttpd daemon handle
  uint16_t port;             // Server port
  char *bind_address;        // Bind address (e.g., "0.0.0.0")

  // UV handle for asynchronous integration
  uv_poll_t poll_handle;

  // Callback for handling HTTP requests
  enum MHD_Result (*on_request_cb)(service_t *svc,
                                   struct MHD_Connection *connection,
                                   const char *url, const char *method,
                                   const char *version, const char *upload_data,
                                   size_t *upload_data_size, void **con_cls);
} http_server_context_t;

// Initialize HTTP server manager
http_server_context_t *http_server_init(
    uv_loop_t *loop, service_t *parent_svc, uint16_t port,
    const char *bind_address,
    enum MHD_Result (*on_request_cb)(service_t *, struct MHD_Connection *,
                                     const char *, const char *, const char *,
                                     const char *, size_t *, void **));

// Cleanup HTTP server manager
void http_server_cleanup(http_server_context_t *ctx);

// Start the HTTP server (asynchronous)
bool http_server_start(http_server_context_t *ctx);

// Stop the HTTP server (asynchronous)
bool http_server_stop(http_server_context_t *ctx);

#endif // HTTP_SERVER_MANAGER_H


