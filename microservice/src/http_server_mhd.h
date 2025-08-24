#ifndef HTTP_SERVER_MHD_H
#define HTTP_SERVER_MHD_H

#include <stdbool.h>
#include <microhttpd.h>

typedef struct service service_t;
// URL 参数列表结构体
typedef struct {
    char *key;
    char *value;
} url_param_t;

// 应用程序路由处理函数类型
// 参数：应用程序上下文、URL、HTTP方法、请求体、请求体大小
// 返回值：一个 MHD_Response 对象，或者 NULL 表示错误
typedef struct MHD_Response* (*http_router_cb)(void *app_ctx, const char *url, const char *method, const char *body, size_t body_size);

// 路由结构体
// 路由结构体
typedef struct http_route_s {
    char *url_path;
    char *method;
    http_router_cb handler_cb;
    struct http_route_s *next;
} http_route_t;

// HTTP 服务器上下文
typedef struct http_server_mhd_context_s {
    struct MHD_Daemon *daemon;
    char *listen_ip;
    int listen_port;
    service_t *parent_service;
    http_route_t *routes; // 路由表链表头
} http_server_mhd_context_t;

http_server_mhd_context_t* http_server_mhd_init(service_t *parent_svc, const char *ip, int port);
void http_server_mhd_cleanup(http_server_mhd_context_t *ctx);

void http_server_mhd_register_route(http_server_mhd_context_t *ctx, const char *url, const char *method, http_router_cb handler);

#endif // HTTP_SERVER_MHD_H