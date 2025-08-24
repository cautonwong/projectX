#include "docker_manager.h"
#include "common.h"
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char *memory;
  size_t size;
  CURLcode error_code; // Added for error handling
} MemoryStruct;

static size_t write_memory_callback(void *contents, size_t size, size_t nmemb,
                                    void *userp) {
  size_t realsize = size * nmemb;
  MemoryStruct *mem = (MemoryStruct *)userp;

  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if (ptr == NULL) {
    LOG_ERROR("Docker: Not enough memory (realloc returned NULL).");
    return 0;
  }

  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

static void docker_request_done(uv_work_t *req, int status) {
  if (status == UV_ECANCELED) {
    LOG_WARN("Docker: Async request was canceled.");
    return;
  }
  MemoryStruct *chunk = (MemoryStruct *)((void **)req->data)[1];
  docker_response_cb cb = (docker_response_cb)(((void **)req->data)[2]);
  void *user_data = ((void **)req->data)[3];

  if (cb) {
    if (chunk->error_code == CURLE_OK) {
      cb(chunk->memory, user_data);
    } else {
      // Pass NULL or an error string if there was a cURL error
      cb(NULL, user_data); // Or pass a specific error code/message
    }
  }

  if (chunk) {
    if (chunk->memory)
      free(chunk->memory);
    free(chunk);
  }
  free(req->data);
  free(req);
}

static void docker_request_work(uv_work_t *req) {
  char *url = ((char **)req->data)[0];
  MemoryStruct *chunk = (MemoryStruct *)((void **)req->data)[1];
  CURL *curl;
  CURLcode res;

  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_UNIX_SOCKET_PATH, "/var/run/docker.sock");
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_memory_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)chunk);
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      LOG_ERROR("Docker: curl_easy_perform() failed: %s",
                curl_easy_strerror(res));
      if (chunk->memory) {
        free(chunk->memory);
        chunk->memory = NULL;
      }
      chunk->size = 0;
      chunk->error_code = res; // Store cURL error code
    } else {
      chunk->error_code = CURLE_OK;
    }
    curl_easy_cleanup(curl);
  }
  free(url);
}

docker_manager_context_t *dm_docker_init(const char *socket_path) {
  docker_manager_context_t *ctx =
      (docker_manager_context_t *)malloc(sizeof(docker_manager_context_t));
  if (!ctx) {
    LOG_ERROR("Docker: Failed to allocate context.");
    return NULL;
  }
  ctx->docker_socket_path = strdup(socket_path);
  LOG_INFO("Docker: Manager initialized with socket %s.",
           ctx->docker_socket_path);
  return ctx;
}

void dm_docker_cleanup(docker_manager_context_t *ctx) {
  if (!ctx)
    return;
  free(ctx->docker_socket_path);
  free(ctx);
  LOG_INFO("Docker: Cleaned up.");
}

bool dm_docker_list_containers_async(docker_manager_context_t *ctx, bool all,
                                     docker_response_cb cb, void *user_data) {
  uv_work_t *req = (uv_work_t *)malloc(sizeof(uv_work_t));
  if (!req)
    return false;

  char *url = malloc(128);
  snprintf(url, 128, "http://v1.41/containers/json?all=%s",
           all ? "true" : "false");

  MemoryStruct *chunk = (MemoryStruct *)malloc(sizeof(MemoryStruct));
  if (!chunk) {
    free(req);
    free(url);
    return false;
  }
  chunk->memory = malloc(1);
  chunk->size = 0;
  chunk->error_code = CURLE_OK; // Initialize error code

  void **work_data = malloc(4 * sizeof(void *));
  work_data[0] = url;
  work_data[1] = chunk;
  work_data[2] = (void *)cb;
  work_data[3] = user_data;
  req->data = work_data;

  uv_queue_work(ctx->loop, req, docker_request_work, docker_request_done);
  return true;
}

bool dm_docker_run_container(docker_manager_context_t *ctx, const char *image,
                             const char *name) {
  LOG_WARN("Docker: dm_docker_run_container is a basic placeholder.");
  // In a real implementation, this would involve a POST request to Docker API
  // e.g., /containers/create and then /containers/{id}/start
  // For now, just log and return true as a success placeholder.
  return true;
}