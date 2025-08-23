#include "service.h"
#include "common.h"

// Include all manager headers
#include "mqtt_client_manager.h"
#include "http_client_manager.h"
#include "http_server_manager.h"
#include "sqlite_manager.h"
#include "leveldb_manager.h"

#include <stdlib.h>

service_t* service_create(void) {
    service_t *svc = malloc(sizeof(service_t));
    if (!svc) {
        LOG_ERROR("Failed to allocate service object");
        return NULL;
    }

    svc->loop = malloc(sizeof(uv_loop_t));
    if (!svc->loop || uv_loop_init(svc->loop) != 0) {
        LOG_ERROR("Failed to initialize event loop");
        if (svc->loop) free(svc->loop);
        free(svc);
        return NULL;
    }

    curl_global_init(CURL_GLOBAL_ALL); // Global cURL initialization

    svc->mqtt_ctx = NULL;
    svc->http_client_ctx = NULL;
    svc->http_server_ctx = NULL;
    svc->sqlite_ctx = NULL;
    svc->leveldb_ctx = NULL;
    svc->user_data = NULL;

    LOG_INFO("Service created");
    return svc;
}

void service_destroy(service_t *svc) {
    if (!svc) return;

    LOG_INFO("Destroying service");

    // Clean up all modules
    if (svc->mqtt_ctx) mqtt_client_cleanup(svc->mqtt_ctx);
    if (svc->http_client_ctx) http_client_cleanup(svc->http_client_ctx);
    if (svc->http_server_ctx) http_server_cleanup(svc->http_server_ctx);
    if (svc->sqlite_ctx) sqlite_cleanup(svc->sqlite_ctx);
    if (svc->leveldb_ctx) leveldb_cleanup(svc->leveldb_ctx);

    // Close and free the loop
    if (svc->loop) {
        uv_loop_close(svc->loop);
        free(svc->loop);
    }

    curl_global_cleanup(); // Global cURL cleanup

    free(svc);
}

void service_run(service_t *svc) {
    if (svc && svc->loop) {
        LOG_INFO("Service run");
        uv_run(svc->loop, UV_RUN_DEFAULT);
    }
}

int service_add_mqtt(service_t* svc, const service_mqtt_config_t* config) {
    if (!svc || !config) return -1;
    
    svc->mqtt_ctx = mqtt_client_init(svc->loop, svc, config->broker_address, config->client_id, 
                                     config->username, config->password, 
                                     (void (*)(service_t *, const char *, const char *, int, int, bool))config->on_message_cb, 
                                     (void (*)(service_t *, const char *))config->on_connect_lost_cb, 
                                     (void (*)(service_t *, int))config->on_delivery_complete_cb);
    if (!svc->mqtt_ctx) {
        LOG_ERROR("Failed to initialize MQTT client");
        return -1;
    }
    // Automatically connect
    if (!mqtt_client_connect(svc->mqtt_ctx)) {
        LOG_ERROR("Failed to connect MQTT client");
        return -1;
    }
    return 0;
}

int service_add_http_client(service_t* svc, const service_http_client_config_t* config) {
    if (!svc || !config) return -1;

    svc->http_client_ctx = http_client_init(svc->loop, svc, config->base_url, config->timeout_ms, 
                                            (void (*)(service_t *, long, const char *, size_t, void *))config->on_response_cb, config->user_data);
    if (!svc->http_client_ctx) {
        LOG_ERROR("Failed to initialize HTTP client");
        return -1;
    }
    return 0;
}

int service_add_http_server(service_t* svc, const service_http_server_config_t* config) {
    if (!svc || !config) return -1;

    svc->http_server_ctx = http_server_init(svc, config->port, (http_request_handler_cb)config->on_request_cb);
    if (!svc->http_server_ctx) {
        LOG_ERROR("Failed to initialize HTTP server");
        return -1;
    }
    // Automatically start the server
    if (!http_server_start(svc->http_server_ctx)) {
        LOG_ERROR("Failed to start HTTP server");
        return -1;
    }
    return 0;
}

int service_add_sqlite(service_t* svc, const service_sqlite_config_t* config) {
    if (!svc || !config) return -1;

    svc->sqlite_ctx = sqlite_init(svc->loop, svc, config->db_path, 
                                    (void (*)(service_t *, const char *, size_t, void *))config->on_query_cb, config->user_data);
    if (!svc->sqlite_ctx) {
        LOG_ERROR("Failed to initialize SQLite");
        return -1;
    }
    return 0;
}

int service_add_leveldb(service_t* svc, const service_leveldb_config_t* config) {
    if (!svc || !config) return -1;

    svc->leveldb_ctx = leveldb_init(svc->loop, svc, config->db_path, config->on_query_cb, config->user_data);
    if (!svc->leveldb_ctx) {
        LOG_ERROR("Failed to initialize LevelDB");
        return -1;
    }
    return 0;
}
