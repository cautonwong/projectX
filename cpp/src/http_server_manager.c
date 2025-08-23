#include "http_server_manager.h"
#include "common.h"
#include <string.h>

// Forward declaration for the new uv_poll callback
static void on_http_server_poll(uv_poll_t *handle, int status, int events);

static enum MHD_Result request_handler_wrapper(void *cls, struct MHD_Connection *connection,
                                 const char *url, const char *method,
                                 const char *version, const char *upload_data,
                                 size_t *upload_data_size, void **con_cls) {
    http_server_context_t *ctx = (http_server_context_t *)cls;

    if (ctx->handler_cb) {
        return ctx->handler_cb(cls, connection, url, method, version, upload_data, upload_data_size, con_cls);
    }

    // Default 404 Not Found response
    const char *page = "<html><body>404 Not Found</body></html>";
    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(page), (void *)page, MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response(response);
    return ret;
}

http_server_context_t* http_server_init(service_t *parent_svc, uint16_t port, http_request_handler_cb handler) {
    http_server_context_t *ctx = malloc(sizeof(http_server_context_t));
    if (!ctx) {
        LOG_ERROR("Failed to allocate http_server_context_t");
        return NULL;
    }
    memset(ctx, 0, sizeof(*ctx)); // Use *ctx for memset
    ctx->parent_service = parent_svc;
    ctx->port = port;
    ctx->handler_cb = handler;

    // Initialize uv_poll_t with a placeholder FD, will be updated in http_server_start
    uv_poll_init(parent_svc->loop, &ctx->poll_handle, -1); 
    ctx->poll_handle.data = ctx;

    return ctx;
}

bool http_server_start(http_server_context_t *ctx) {
    if (!ctx) return false;
    
    // Use MHD_USE_EPOLL_LINUX_ONLY for better integration with libuv's event loop
    // MHD_USE_INTERNAL_POLLING_THREAD is removed as libuv will handle polling
    ctx->daemon = MHD_start_daemon(MHD_USE_EPOLL_LINUX_ONLY, ctx->port, NULL, NULL,
                                   &request_handler_wrapper, ctx, MHD_OPTION_END);
    if (!ctx->daemon) {
        LOG_ERROR("Failed to start HTTP server on port %u", ctx->port);
        return false;
    }

    // Get the file descriptor from libmicrohttpd and register it with uv_poll
    int sockfd;
    if (MHD_get_fdset(ctx->daemon, NULL, NULL, NULL, &sockfd) == MHD_YES) {
        // Re-initialize uv_poll_t with the correct FD
        uv_poll_init(ctx->parent_service->loop, &ctx->poll_handle, sockfd);
        ctx->poll_handle.data = ctx;
        uv_poll_start(&ctx->poll_handle, UV_READABLE | UV_WRITABLE, on_http_server_poll);
    } else {
        LOG_ERROR("Failed to get MHD socket FD");
        MHD_stop_daemon(ctx->daemon);
        ctx->daemon = NULL;
        return false;
    }

    LOG_INFO("HTTP server started on port %u", ctx->port);
    return true;
}

bool http_server_stop(http_server_context_t *ctx) {
    if (ctx && ctx->daemon) {
        uv_poll_stop(&ctx->poll_handle); // Stop polling before stopping daemon
        MHD_stop_daemon(ctx->daemon);
        ctx->daemon = NULL;
        LOG_INFO("HTTP server stopped");
    }
    return true;
}

void http_server_cleanup(http_server_context_t *ctx) {
    if (ctx) {
        http_server_stop(ctx);
        // Close the uv_poll handle
        uv_close((uv_handle_t*)&ctx->poll_handle, NULL);
        free(ctx);
    }
}

// New uv_poll callback to drive MHD
static void on_http_server_poll(uv_poll_t *handle, int status, int events) {
    http_server_context_t *ctx = (http_server_context_t *)handle->data;
    if (status < 0) {
        LOG_ERROR("HTTP Server Poll error: %s", uv_strerror(status));
        return;
    }
    // Call MHD_run to process events. MHD_run will process any pending events
    // on the file descriptor(s) managed by MHD.
    MHD_run(ctx->daemon);
}