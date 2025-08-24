#include "mqtt_client_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void on_poll(uv_poll_t *handle, int status, int events) {
  mqtt_client_context_t *ctx = (mqtt_client_context_t *)handle->data;
  if (status < 0) {
    fprintf(stderr, "Poll error: %s\n", uv_strerror(status));
    return;
  }
  if (events & (UV_READABLE | UV_WRITABLE)) {
    MQTTClient_yield(); // Process MQTT events
  }
}

static void on_timer(uv_timer_t *handle) {
  MQTTClient_yield();
}

static int mqtt_message_callback(void *context, char *topicName, int topicLen,
                                  MQTTClient_message *message) {
  mqtt_client_context_t *ctx = (mqtt_client_context_t *)context;
  if (ctx->on_message_received_cb) {
    ctx->on_message_received_cb(ctx->parent_service, topicName,
                                (char *)message->payload, message->payloadlen,
                                message->qos, message->retained);
  }
  MQTTClient_freeMessage(&message);
  MQTTClient_free(topicName);
  return 1;
}

static void mqtt_connection_lost(void *context, char *cause) {
  mqtt_client_context_t *ctx = (mqtt_client_context_t *)context;
  if (ctx->on_connection_lost_cb) {
    ctx->on_connection_lost_cb(ctx->parent_service, cause);
  }
}

static void mqtt_delivery_complete(void *context, MQTTClient_deliveryToken dt) {
  mqtt_client_context_t *ctx = (mqtt_client_context_t *)context;
  if (ctx->on_delivery_complete_cb) {
    ctx->on_delivery_complete_cb(ctx->parent_service, (int)dt);
  }
}

mqtt_client_context_t *mqtt_client_init(
    uv_loop_t *loop, service_t *parent_svc, const char *broker_addr,
    const char *client_id, const char *username, const char *password,
    void (*on_msg_cb)(service_t *, const char *, const char *, int, int, bool),
    void (*on_conn_lost_cb)(service_t *, const char *),
    void (*on_del_comp_cb)(service_t *, int)) {
  mqtt_client_context_t *ctx = malloc(sizeof(mqtt_client_context_t));
  if (!ctx)
    return NULL;

  ctx->parent_service = parent_svc;
  ctx->loop = loop;
  ctx->broker_address = strdup(broker_addr);
  ctx->client_id = strdup(client_id);
  ctx->username = username ? strdup(username) : NULL;
  ctx->password = password ? strdup(password) : NULL;
  ctx->qos = 1;
  ctx->timeout_ms = 10000;

  MQTTClient_create(&ctx->client, broker_addr, client_id,
                    MQTTCLIENT_PERSISTENCE_NONE, NULL);
  MQTTClient_setCallbacks(ctx->client, ctx, mqtt_connection_lost,
                          mqtt_message_callback, mqtt_delivery_complete);

  // Initialize UV handles
  uv_poll_init(loop, &ctx->poll_handle,
               -1); // Placeholder FD, updated on connect
  ctx->poll_handle.data = ctx;
  uv_timer_init(loop, &ctx->timer_handle);
  ctx->timer_handle.data = ctx;
  uv_timer_start(&ctx->timer_handle, on_timer, 100, 100); // 100ms interval

  ctx->on_message_received_cb = on_msg_cb;
  ctx->on_connection_lost_cb = on_conn_lost_cb;
  ctx->on_delivery_complete_cb = on_del_comp_cb;

  fprintf(stderr, "MQTT client initialized for %s with client ID %s\n",
          broker_addr, client_id);
  return ctx;
}

void mqtt_client_cleanup(mqtt_client_context_t *ctx) {
  if (!ctx)
    return;

  mqtt_client_disconnect(ctx);
  if (ctx->broker_address)
    free(ctx->broker_address);
  if (ctx->client_id)
    free(ctx->client_id);
  if (ctx->username)
    free(ctx->username);
  if (ctx->password)
    free(ctx->password);
  uv_timer_stop(&ctx->timer_handle);
  uv_close((uv_handle_t *)&ctx->timer_handle, NULL);
  uv_poll_stop(&ctx->poll_handle);
  uv_close((uv_handle_t *)&ctx->poll_handle, NULL);
  MQTTClient_destroy(&ctx->client);
  free(ctx);
}

bool mqtt_client_connect(mqtt_client_context_t *ctx) {
  if (!ctx || !ctx->broker_address)
    return false;

  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
  conn_opts.keepAliveInterval = 20;
  conn_opts.cleansession = 1;
  conn_opts.username = ctx->username;
  conn_opts.password = ctx->password;
  conn_opts.connectTimeout = ctx->timeout_ms / 1000;

  MQTTClient_yield(); // Ensure no pending events
  int rc = MQTTClient_connect(ctx->client, &conn_opts);
  if (rc != MQTTCLIENT_SUCCESS) {
    fprintf(stderr, "Failed to connect to MQTT broker: %s\n",
            MQTTClient_strerror(rc));
    return false;
  }

  // NOTE: Paho C lib does not expose socket for external polling in a standard way.
  // The yield mechanism is the primary way to integrate.

  fprintf(stderr, "Connected to MQTT broker at %s\n", ctx->broker_address);
  return true;
}

int mqtt_client_publish_async(mqtt_client_context_t *ctx, const char *topic,
                              const char *payload, int qos, bool retained) {
  if (!ctx || !topic || !payload)
    return -1;

  MQTTClient_message msg = MQTTClient_message_initializer;
  msg.payload = (void *)payload;
  msg.payloadlen = strlen(payload);
  msg.qos = qos;
  msg.retained = retained;

  MQTTClient_deliveryToken token;
  int rc = MQTTClient_publishMessage(ctx->client, topic, &msg, &token);
  if (rc != MQTTCLIENT_SUCCESS) {
    fprintf(stderr, "Failed to publish: %s\n", MQTTClient_strerror(rc));
    return -1;
  }
  return (int)token;
}

int mqtt_client_subscribe_async(mqtt_client_context_t *ctx, const char *topic,
                                int qos) {
  if (!ctx || !topic)
    return -1;
  int rc = MQTTClient_subscribe(ctx->client, topic, qos);
  if (rc != MQTTCLIENT_SUCCESS) {
    fprintf(stderr, "Failed to subscribe: %s\n", MQTTClient_strerror(rc));
    return -1;
  }
  return rc;
}

int mqtt_client_unsubscribe_async(mqtt_client_context_t *ctx,
                                  const char *topic) {
  if (!ctx || !topic)
    return -1;
  int rc = MQTTClient_unsubscribe(ctx->client, topic);
  if (rc != MQTTCLIENT_SUCCESS) {
    fprintf(stderr, "Failed to unsubscribe: %s\n", MQTTClient_strerror(rc));
    return -1;
  }
  return rc;
}

bool mqtt_client_disconnect(mqtt_client_context_t *ctx) {
  if (!ctx)
    return false;
  int rc = MQTTClient_disconnect(ctx->client, ctx->timeout_ms);
  if (rc != MQTTCLIENT_SUCCESS) {
    fprintf(stderr, "Failed to disconnect: %s\n", MQTTClient_strerror(rc));
    return false;
  }
  uv_poll_stop(&ctx->poll_handle);
  fprintf(stderr, "Disconnected from MQTT broker\n");
  return true;
}