#ifndef SERVICE_H
#define SERVICE_H

#include <uv.h>
#include <stdbool.h>
#include <microhttpd.h>

// Forward declarations for internal context types
typedef struct mqtt_client_context_s mqtt_client_context_t;
typedef struct http_client_context_s http_client_context_t;
typedef struct http_server_context_s http_server_context_t;
typedef struct sqlite_context_s sqlite_context_t;
typedef struct leveldb_context_s leveldb_context_t;

// Main service object, acts as a container for all modules
typedef struct service_s {
    uv_loop_t *loop;
    void *mqtt_ctx;
    void *http_client_ctx;
    void *http_server_ctx;
    void *sqlite_ctx;
    void *leveldb_ctx;
    void *user_data; // Generic user data pointer for the whole service
} service_t;


// --- Configuration Structs and Callbacks ---

// MQTT
typedef struct {
    const char* broker_address;
    const char* client_id;
    const char* username;
    const char* password;
    void (*on_message_cb)(service_t *svc, const char *topic, const char *payload, int len, int qos, bool retained);
    void (*on_connect_lost_cb)(service_t *svc, const char *cause);
    void (*on_delivery_complete_cb)(service_t *svc, int delivery_token);
} service_mqtt_config_t;

// HTTP Client
typedef struct {
    const char* base_url;
    long timeout_ms;
    void (*on_response_cb)(service_t *svc, long status, const char *data, size_t len, void *user_data);
    void *user_data;
} service_http_client_config_t;

// HTTP Server
typedef struct {
    uint16_t port;
    enum MHD_Result (*on_request_cb)(void *cls, struct MHD_Connection *conn, const char *url, const char *method, const char *ver, const char *upload_data, size_t *upload_size, void **con_cls);
} service_http_server_config_t;

// SQLite
typedef struct {
    const char* db_path;
    void (*on_query_cb)(service_t *svc, const char *result, size_t len, void *user_data);
    void *user_data;
} service_sqlite_config_t;

// LevelDB
typedef struct {
    const char* db_path;
    void (*on_query_cb)(service_t *svc, const char *key, const char *value, size_t len, void *user_data);
    void *user_data;
} service_leveldb_config_t;


// --- New Modular Service API ---

/**
 * @brief Creates and initializes a new service object and its event loop.
 * @return A pointer to the new service_t object, or NULL on failure.
 */
service_t* service_create(void);

/**
 * @brief Cleans up all service modules and frees the service object.
 * @param svc The service object to destroy.
 */
void service_destroy(service_t *svc);

/**
 * @brief Starts the service's main event loop. This is a blocking call.
 * @param svc The service object.
 */
void service_run(service_t *svc);

/**
 * @brief Adds and configures the MQTT client module.
 * @param svc The service object.
 * @param config Configuration for the MQTT client.
 * @return 0 on success, -1 on failure.
 */
int service_add_mqtt(service_t* svc, const service_mqtt_config_t* config);

/**
 * @brief Adds and configures the HTTP client module.
 * @param svc The service object.
 * @param config Configuration for the HTTP client.
 * @return 0 on success, -1 on failure.
 */
int service_add_http_client(service_t* svc, const service_http_client_config_t* config);

/**
 * @brief Adds and configures the HTTP server module.
 * @param svc The service object.
 * @param config Configuration for the HTTP server.
 * @return 0 on success, -1 on failure.
 */
int service_add_http_server(service_t* svc, const service_http_server_config_t* config);

/**
 * @brief Adds and configures the SQLite module.
 * @param svc The service object.
 * @param config Configuration for the SQLite module.
 * @return 0 on success, -1 on failure.
 */
int service_add_sqlite(service_t* svc, const service_sqlite_config_t* config);

/**
 * @brief Adds and configures the LevelDB module.
 * @param svc The service object.
 * @param config Configuration for the LevelDB module.
 * @return 0 on success, -1 on failure.
 */
int service_add_leveldb(service_t* svc, const service_leveldb_config_t* config);


#endif // SERVICE_H
