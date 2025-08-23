#include "http_client_manager.h"
#include "http_server_manager.h"
#include "leveldb_manager.h"
#include "mqtt_client_manager.h"
#include "service.h"
#include "sqlite_manager.h"
#include <stdio.h>

static void on_mqtt_msg(service_t *svc, const char *topic, const char *payload,
                        int len, int qos, bool retained) {
  printf("MQTT: %.*s\n", len, payload);
}

static void on_mqtt_conn_lost(service_t *svc, const char *cause) {
  printf("MQTT Conn Lost: %s\n", cause);
}

static void on_mqtt_delivery(service_t *svc, int token) {
  printf("MQTT Delivery: %d\n", token);
}

static void on_http_response(service_t *svc, long status, const char *data,
                             size_t len, void *user_data) {
  printf("HTTP Client: %ld - %.*s\n", status, (int)len, data);
}

static enum MHD_Result
on_http_request(service_t *svc, struct MHD_Connection *connection,
                const char *url, const char *method, const char *version,
                const char *upload_data, size_t *upload_data_size,
                void **con_cls) {
  const char *response = "Hello from HTTP Server!";
  struct MHD_Response *resp = MHD_create_response_from_buffer(
      strlen(response), (void *)response, MHD_RESPMEM_PERSISTENT);
  MHD_queue_response(connection, MHD_HTTP_OK, resp);
  MHD_destroy_response(resp);
  return MHD_YES;
}

static void on_sqlite_result(service_t *svc, const char *result, size_t len,
                             void *user_data) {
  printf("SQLite: %.*s\n", (int)len, result);
}

static void on_leveldb_result(service_t *svc, const char *key,
                              const char *value, size_t len, void *user_data) {
  printf("LevelDB: %s = %.*s\n", key, (int)len, value);
}

int main() {
  service_t *svc =
      service_init("config.ini", on_mqtt_msg, on_mqtt_conn_lost,
                   on_mqtt_delivery, on_http_response, NULL, on_http_request,
                   on_sqlite_result, NULL, on_leveldb_result, NULL);
  if (!svc) {
    fprintf(stderr, "Failed to initialize service\n");
    return -1;
  }

  // Trigger operations
  mqtt_client_connect(svc->mqtt_ctx);
  mqtt_client_subscribe_async(svc->mqtt_ctx, "test/topic", 1);
  http_client_get_async(svc->http_ctx, "/test");
  sqlite_exec_async(svc->sqlite_ctx, "SELECT 'Hello'");
  leveldb_get_async(svc->leveldb_ctx, "key");

  service_run(svc);
  service_cleanup(svc);
  return 0;
}

/*
// 在 data_collection_app.c 中定义你的 API 处理器
struct MHD_Response* get_sensor_data_handler(void *app_ctx, const char *url,
const char *method, const char *body, size_t body_size) { data_collection_app_t
*app = (data_collection_app_t*)app_ctx;
    // 你的业务逻辑：从数据库读取数据，格式化成 JSON
    cJSON *json_resp = cJSON_CreateObject();
    cJSON_AddNumberToObject(json_resp, "temperature", 25); // 模拟数据
    cJSON_AddNumberToObject(json_resp, "humidity", 70);
    cJSON_AddStringToObject(json_resp, "device_id", app->device_id);
    char *response_str = cJSON_Print(json_resp);
    cJSON_Delete(json_resp);

    // 创建响应对象
    struct MHD_Response *response =
MHD_create_response_from_buffer(strlen(response_str), (void*)response_str,
MHD_RESPMEM_MUST_FREE); MHD_add_response_header(response, "Content-Type",
"application/json"); return response;
}

// 在 data_collection_app_init() 中注册这个处理器
bool data_collection_app_init(void *app_ctx_base, void *service_instance) {
    // ... 其他初始化代码 ...
    service_t *svc = (service_t*)service_instance;
    // ...
    // 注册 HTTP 路由
    if (svc->http_mhd_ctx) {
        http_server_mhd_register_route(svc->http_mhd_ctx, "/api/v1/data", "GET",
get_sensor_data_handler);
        // 可以注册更多路由
        // http_server_mhd_register_route(svc->http_mhd_ctx, "/api/v1/command",
"POST", post_command_handler);
    }
    // ...
    return true;
}


// 定义一个处理函数，接收动态路径参数
struct MHD_Response* get_device_data_handler(void *app_ctx, const char *url,
const char *method, const char *body, size_t body_size, url_param_t *params,
size_t num_params) { const char *device_id = NULL; for (size_t i = 0; i <
num_params; ++i) { if (strcmp(params[i].key, "id") == 0) { device_id =
params[i].value; break;
        }
    }

    if (!device_id) {
        // ... 返回 400 Bad Request ...
    }

    LOG_INFO("App: Received request for device ID: %s", device_id);

    // 现在你可以使用 device_id 变量来查询特定设备的数据
    // ...

    const char *response_str = "{\"status\":\"ok\", \"message\":\"Data for
device found.\"}"; struct MHD_Response *response =
MHD_create_response_from_buffer(strlen(response_str), (void*)response_str,
MHD_RESPMEM_PERSISTENT); MHD_add_response_header(response, "Content-Type",
"application/json"); return response;
}

// 在 data_collection_app_init() 中注册这个带参数的路由
bool data_collection_app_init(void *app_ctx_base, void *service_instance) {
    // ...
    if (svc->http_mhd_ctx) {
        // 注册一个带参数的路由
        http_server_mhd_register_route(svc->http_mhd_ctx, "/api/v1/data/:id",
"GET", get_device_data_handler);
    }
    // ...
    return true;
}
*/