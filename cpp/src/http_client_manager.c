#include "http_client_manager.h"
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t write_callback(void *contents, size_t size, size_t nmemb,
                             void *userp) {
  size_t realsize = size * nmemb;
  char **response = (char **)userp;
  char *ptr = realloc(*response, realsize + 1);
  if (!ptr) {
    fprintf(stderr, "Failed to realloc response data\n");
    return 0;
  }
  *response = ptr;
  memcpy(*response, contents, realsize);
  (*response)[realsize] = 0;
  return realsize;
}

static void on_poll(uv_poll_t *handle, int status, int events) {
  http_client_context_t *ctx = (http_client_context_t *)handle->data;
  if (status < 0) {
    fprintf(stderr, "Poll error: %s\n", uv_strerror(status));
    return;
  }
  if (events & (UV_READABLE | UV_WRITABLE)) {
    int running_handles;
    curl_multi_socket_action(ctx->curl_multi, CURL_SOCKET_TIMEOUT, 0,
                             &running_handles);
    CURLMsg *msg;
    int msgs_left;
    while ((msg = curl_multi_info_read(ctx->curl_multi, &msgs_left))) {
      if (msg->msg == CURLMSG_DONE) {
        char *response = NULL;
        long status;
        curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &status);
        curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &response);
        if (ctx->on_response_cb) {
          ctx->on_response_cb(ctx->parent_service, status, response,
                              strlen(response), ctx->user_data);
        }
        free(response);
        curl_multi_remove_handle(ctx->curl_multi, msg->easy_handle);
        curl_easy_cleanup(msg->easy_handle);
      }
    }
  }
}

static void on_timer(uv_timer_t *handle) {
  http_client_context_t *ctx = (http_client_context_t *)handle->data;
  int running_handles;
  curl_multi_socket_action(ctx->curl_multi, CURL_SOCKET_TIMEOUT, 0,
                           &running_handles);
}

http_client_context_t *http_client_init(
    uv_loop_t *loop, service_t *parent_svc, const char *base_url,
    long timeout_ms,
    void (*on_response_cb)(service_t *, long, const char *, size_t, void *),
    void *user_data) {
  http_client_context_t *ctx = malloc(sizeof(http_client_context_t));
  if (!ctx)
    return NULL;

  ctx->parent_service = parent_svc;
  ctx->loop = loop;
  ctx->base_url = strdup(base_url);
  ctx->timeout_ms = timeout_ms;
  ctx->curl = NULL;
  ctx->curl_multi = curl_multi_init();
  ctx->user_data = user_data;
  ctx->on_response_cb = on_response_cb;

  uv_poll_init(loop, &ctx->poll_handle, -1); // Placeholder FD
  ctx->poll_handle.data = ctx;
  uv_timer_init(loop, &ctx->timer_handle);
  ctx->timer_handle.data = ctx;
  uv_timer_start(&ctx->timer_handle, on_timer, 100, 100); // 100ms interval

  fprintf(stderr, "HTTP client initialized for %s\n", base_url);
  return ctx;
}

void http_client_cleanup(http_client_context_t *ctx) {
  if (!ctx)
    return;
  uv_timer_stop(&ctx->timer_handle);
  uv_close((uv_handle_t *)&ctx->timer_handle, NULL);
  uv_poll_stop(&ctx->poll_handle);
  uv_close((uv_handle_t *)&ctx->poll_handle, NULL);
  if (ctx->curl_multi)
    curl_multi_cleanup(ctx->curl_multi);
  if (ctx->base_url)
    free(ctx->base_url);
  free(ctx);
}

int http_client_get_async(http_client_context_t *ctx, const char *path) {
  if (!ctx || !path)
    return -1;
  char *url = malloc(strlen(ctx->base_url) + strlen(path) + 2);
  sprintf(url, "%s%s", ctx->base_url, path);

  CURL *curl = curl_easy_init();
  if (!curl) {
    free(url);
    return -1;
  }
  char *response = NULL;
  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, ctx->timeout_ms);
  curl_easy_setopt(curl, CURLOPT_PRIVATE,
                   response); // Store response for callback
  curl_multi_add_handle(ctx->curl_multi, curl);

  int sockfd;
  curl_multi_socket_action(ctx->curl_multi, CURL_SOCKET_TIMEOUT, 0, &sockfd);
  uv_poll_start(&ctx->poll_handle, UV_READABLE | UV_WRITABLE, on_poll);

  free(url);
  return 0;
}

int http_client_post_async(http_client_context_t *ctx, const char *path,
                           const char *data) {
  if (!ctx || !path || !data)
    return -1;
  char *url = malloc(strlen(ctx->base_url) + strlen(path) + 2);
  sprintf(url, "%s%s", ctx->base_url, path);

  CURL *curl = curl_easy_init();
  if (!curl) {
    free(url);
    return -1;
  }
  char *response = NULL;
  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, ctx->timeout_ms);
  curl_easy_setopt(curl, CURLOPT_PRIVATE, response);
  curl_multi_add_handle(ctx->curl_multi, curl);

  int sockfd;
  curl_multi_socket_action(ctx->curl_multi, CURL_SOCKET_TIMEOUT, 0, &sockfd);
  uv_poll_start(&ctx->poll_handle, UV_READABLE | UV_WRITABLE, on_poll);

  free(url);
  return 0;
}