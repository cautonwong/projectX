#ifndef HTTP_SERVER_MANAGER_H
#define HTTP_SERVER_MANAGER_H

#include <microhttpd.h>
#include <stdbool.h>
#include "service.h"

// Forward declaration
typedef struct service_s service_t;

// Application-defined request handler callback
typedef enum MHD_Result (*http_request_handler_cb)(void *cls, struct MHD_Connection *connection, 
                                                         const char *url, const char *method, 
                                                         const char *version, const char *upload_data, 
                                                         size_t *upload_data_size, void **con_cls);

// HTTP server context structure
typedef struct http_server_context_s {
    service_t *parent_service;
    struct MHD_Daemon *daemon;
    uint16_t port;
    http_request_handler_cb handler_cb;
    uv_poll_t poll_handle; // Added for libuv integration
} http_server_context_t;

// Public API
http_server_context_t* http_server_init(service_t *parent_svc, uint16_t port, http_request_handler_cb handler);
void http_server_cleanup(http_server_context_t *ctx);

bool http_server_start(http_server_context_t *ctx);
bool http_server_stop(http_server_context_t *ctx);

#endif // HTTP_SERVER_MANAGER_H