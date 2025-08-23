#ifndef HTTP_CLIENT_MANAGER_H
#define HTTP_CLIENT_MANAGER_H

#include <curl/curl.h>
#include <stdbool.h>
#include <uv.h>

// Forward declaration
#include "service.h"

// HTTP client context structure
typedef struct http_client_context_s {
  service_t *parent_service; // Pointer to parent service_t
  uv_loop_t *loop;           // libuv event loop
  CURL *curl;                // libcurl handle
  CURLM *curl_multi;         // libcurl multi handle
  char *base_url;            // Base URL for requests
  long timeout_ms;           // Timeout in milliseconds

  // UV handles for asynchronous integration
  uv_poll_t poll_handle;
  uv_timer_t timer_handle;

  // Callback for HTTP response
  void (*on_response_cb)(service_t *svc, long status, const char *data,
                         size_t len, void *user_data);
  void *user_data; // User-provided data for callbacks

} http_client_context_t;

// Initialize HTTP client manager
http_client_context_t *http_client_init(
    uv_loop_t *loop, service_t *parent_svc, const char *base_url,
    long timeout_ms,
    void (*on_response_cb)(service_t *, long, const char *, size_t, void *),
    void *user_data);

// Cleanup HTTP client manager
void http_client_cleanup(http_client_context_t *ctx);

// Perform HTTP GET request (asynchronous)
int http_client_get_async(http_client_context_t *ctx, const char *path);

// Perform HTTP POST request (asynchronous)
int http_client_post_async(http_client_context_t *ctx, const char *path,
                           const char *data);

#endif // HTTP_CLIENT_MANAGER_H