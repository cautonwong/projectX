#ifndef DOCKER_MANAGER_H
#define DOCKER_MANAGER_H

#include <stdbool.h>
#include <uv.h>

typedef void (*docker_response_cb)(char* response_json, void* user_data);

typedef struct docker_manager_context_s {
    char *docker_socket_path;
    uv_loop_t *loop;
    // libcurl 句柄等
} docker_manager_context_t;

docker_manager_context_t* dm_docker_init(const char *socket_path);
void dm_docker_cleanup(docker_manager_context_t *ctx);
bool dm_docker_list_containers_async(docker_manager_context_t *ctx, bool all, docker_response_cb cb, void *user_data);
bool dm_docker_run_container(docker_manager_context_t *ctx, const char *image, const char *name);

#endif // DOCKER_MANAGER_H