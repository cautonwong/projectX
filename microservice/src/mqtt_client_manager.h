#ifndef MQTT_CLIENT_MANAGER_H
#define MQTT_CLIENT_MANAGER_H

#include <MQTTClient.h>
#include <stdbool.h>
#include <uv.h>

// Forward declaration of service_t
#include "service.h"

// MQTT client context structure
typedef struct mqtt_client_context_s {
  service_t *parent_service; // Pointer to parent service_t for callbacks or
                             // component access
  uv_loop_t *loop;           // libuv event loop for integration
  MQTTClient client;         // Paho MQTT client handle
  char *broker_address;
  char *client_id;
  char *username;
  char *password;
  int qos;
  long timeout_ms;

  // UV handles for asynchronous integration
  uv_poll_t poll_handle;
  uv_timer_t timer_handle;

  // Callback functions to pass MQTT events to the application
  void (*on_message_received_cb)(service_t *svc, const char *topic,
                                 const char *payload, int len, int qos,
                                 bool retained);
  void (*on_connection_lost_cb)(service_t *svc, const char *cause);
  void (*on_delivery_complete_cb)(service_t *svc, int token);

} mqtt_client_context_t;

// Initialize MQTT client manager
mqtt_client_context_t *mqtt_client_init(
    uv_loop_t *loop, service_t *parent_svc, const char *broker_addr,
    const char *client_id, const char *username, const char *password,
    void (*on_msg_cb)(service_t *, const char *, const char *, int, int, bool),
    void (*on_conn_lost_cb)(service_t *, const char *),
    void (*on_del_comp_cb)(service_t *, int));

// Cleanup MQTT client manager
void mqtt_client_cleanup(mqtt_client_context_t *ctx);

// Connect to MQTT broker (asynchronous)
bool mqtt_client_connect(mqtt_client_context_t *ctx);

// Publish message (asynchronous)
int mqtt_client_publish_async(mqtt_client_context_t *ctx, const char *topic,
                              const char *payload, int qos, bool retained);

// Subscribe to topic (asynchronous)
int mqtt_client_subscribe_async(mqtt_client_context_t *ctx, const char *topic,
                                int qos);

// Unsubscribe from topic (asynchronous)
int mqtt_client_unsubscribe_async(mqtt_client_context_t *ctx,
                                  const char *topic);

// Disconnect from MQTT broker (asynchronous)
bool mqtt_client_disconnect(mqtt_client_context_t *ctx);

#endif // MQTT_CLIENT_MANAGER_H