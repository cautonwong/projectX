#include <leveldb/c.h>
#include <sqlite3.h>
#include <MQTTAsync.h>
#include <cjson/cJSON.h>
#include <microhttpd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <uv.h>
#include <service.h>
#include <common.h>

// Global service instance pointer for signal handler
static service_t *global_service = NULL;

// --- Callbacks for libmsx modules ---

// MQTT Callbacks
static void on_mqtt_message(service_t *svc, const char *topic, const char *payload, int len, int qos, bool retained) {
    LOG_INFO("MQTT Message received on topic '%s': %.*s", topic, len, payload);
    // TODO: Parse payload as JSON for DB command
    // Example: {"type": "sqlite_query", "sql": "SELECT * FROM users;"}
    // Example: {"type": "leveldb_get", "key": "mykey"}
}

static void on_mqtt_connection_lost(service_t *svc, const char *cause) {
    LOG_ERROR("MQTT Connection Lost: %s", cause ? cause : "Unknown");
    // TODO: Implement reconnection logic
}

static void on_mqtt_delivery_complete(service_t *svc, int token) {
    LOG_INFO("MQTT Message delivered, token: %d", token);
}

// HTTP Server Callback
static enum MHD_Result on_http_request(void *cls, struct MHD_Connection *connection,
                                       const char *url, const char *method,
                                       const char *version, const char *upload_data,
                                       size_t *upload_data_size, void **con_cls) {
    LOG_INFO("HTTP Request: %s %s", method, url);

    // Simple health check
    if (strcmp(url, "/health") == 0 && strcmp(method, "GET") == 0) {
        const char *response_text = "{\"status\":\"UP\"}";
        struct MHD_Response *response = MHD_create_response_from_buffer(strlen(response_text), (void *)response_text, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, "Content-Type", "application/json");
        int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return ret;
    }

    // TODO: Implement DB query/storage endpoints

    // Default 404 Not Found
    const char *not_found_page = "<html><body>404 Not Found</body></html>";
    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(not_found_page), (void *)not_found_page, MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response(response);
    return ret;
}

// SQLite Callbacks
static void on_sqlite_query_result(service_t *svc, const char *result, size_t len, void *user_data) {
    LOG_INFO("SQLite Query Result: %.*s", (int)len, result);
    // TODO: Send result back via MQTT or HTTP
}

// LevelDB Callbacks
static void on_leveldb_query_result(service_t *svc, const char *key, const char *value, size_t len, void *user_data) {
    LOG_INFO("LevelDB Query Result for key '%s': %.*s", key, (int)len, value);
    // TODO: Send result back via MQTT or HTTP
}

// --- Signal Handler for graceful shutdown ---
static void signal_handler(int signum) {
    if (global_service) {
        LOG_INFO("Signal %d received, initiating graceful shutdown...", signum);
        // Stop the uv loop, which will cause uv_run to return
        uv_stop(global_service->loop);
    }
}

// --- Main Service Entry Point ---
int main() {
    // 1. Create the main service object
    service_t *svc = service_create();
    if (!svc) {
        LOG_ERROR("Failed to create service. Exiting.");
        return EXIT_FAILURE;
    }
    global_service = svc; // Set global pointer for signal handler

    // 2. Configure and add MQTT module
    service_mqtt_config_t mqtt_config = {
        .broker_address = "tcp://localhost:1883",
        .client_id = "core-data-service",
        .username = NULL,
        .password = NULL,
        .on_message_cb = on_mqtt_message,
        .on_connect_lost_cb = on_mqtt_connection_lost,
        .on_delivery_complete_cb = on_mqtt_delivery_complete
    };
    if (service_add_mqtt(svc, &mqtt_config) != 0) {
        LOG_ERROR("Failed to add MQTT module. Exiting.");
        service_destroy(svc);
        return EXIT_FAILURE;
    }

    // 3. Configure and add HTTP Server module
    service_http_server_config_t http_server_config = {
        .port = 8080,
        .on_request_cb = on_http_request
    };
    if (service_add_http_server(svc, &http_server_config) != 0) {
        LOG_ERROR("Failed to add HTTP Server module. Exiting.");
        service_destroy(svc);
        return EXIT_FAILURE;
    }

    // 4. Configure and add SQLite module
    service_sqlite_config_t sqlite_config = {
        .db_path = "./core_data.db", // Persistent SQLite database file
        .on_query_cb = on_sqlite_query_result,
        .user_data = NULL
    };
    if (service_add_sqlite(svc, &sqlite_config) != 0) {
        LOG_ERROR("Failed to add SQLite module. Exiting.");
        service_destroy(svc);
        return EXIT_FAILURE;
    }

    // 5. Configure and add LevelDB module
    service_leveldb_config_t leveldb_config = {
        .db_path = "./core_leveldb", // Persistent LevelDB directory
        .on_query_cb = on_leveldb_query_result,
        .user_data = NULL
    };
    if (service_add_leveldb(svc, &leveldb_config) != 0) {
        LOG_ERROR("Failed to add LevelDB module. Exiting.");
        service_destroy(svc);
        return EXIT_FAILURE;
    }

    // 6. Register signal handlers for graceful shutdown
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // 7. Run the service
    LOG_INFO("Core Data Service started. Press Ctrl+C to stop.");
    service_run(svc);

    // 8. Clean up and exit
    LOG_INFO("Core Data Service shutting down.");
    service_destroy(svc);
    return EXIT_SUCCESS;
}