#include "leveldb_manager.h"
#include <leveldb/c.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

typedef struct {
  leveldb_context_t *ctx;
  uv_work_t req;
  char *key;
  char *value;
  size_t len;
} leveldb_work_data_t;

static void work_cb(uv_work_t *req) {
  leveldb_work_data_t *data = (leveldb_work_data_t *)req->data;
  char *err = NULL;
  data->value = leveldb_get(data->ctx->db, NULL, data->key, strlen(data->key),
                            &data->len, &err);
  if (err) {
    fprintf(stderr, "LevelDB error: %s\n", err);
    free(err);
  }
}

static void after_work_cb(uv_work_t *req, int status) {
  leveldb_work_data_t *data = (leveldb_work_data_t *)req->data;
  if (status == 0 && data->ctx->on_query_result_cb && data->value) {
    data->ctx->on_query_result_cb(data->ctx->parent_service, data->key,
                                  data->value, data->len, data->ctx->user_data);
  }
  if (data->value)
    free(data->value);
  if (data->key)
    free(data->key);
  free(data);
  free(req);
}

leveldb_context_t *
leveldb_init(uv_loop_t *loop, service_t *parent_svc, const char *db_path,
             void (*on_query_result_cb)(service_t *, const char *, const char *,
                                        size_t, void *),
             void *user_data) {
  leveldb_context_t *ctx = malloc(sizeof(leveldb_context_t));
  if (!ctx)
    return NULL;

  ctx->parent_service = parent_svc;
  ctx->loop = loop;
  ctx->db_path = strdup(db_path);
  ctx->on_query_result_cb = on_query_result_cb;
  ctx->user_data = user_data;

  leveldb_options_t *options = leveldb_options_create();
  leveldb_options_set_create_if_missing(options, 1);
  ctx->db = leveldb_open(options, db_path, NULL);
  leveldb_options_destroy(options);
  if (!ctx->db) {
    fprintf(stderr, "Failed to open LevelDB at %s\n", db_path);
    free(ctx->db_path);
    free(ctx);
    return NULL;
  }

  fprintf(stderr, "LevelDB initialized for %s\n", db_path);
  return ctx;
}

void leveldb_cleanup(leveldb_context_t *ctx) {
  if (!ctx)
    return;
  if (ctx->db)
    leveldb_close(ctx->db);
  if (ctx->db_path)
    free(ctx->db_path);
  free(ctx);
}

int leveldb_get_async(leveldb_context_t *ctx, const char *key) {
  if (!ctx || !key)
    return -1;
  leveldb_work_data_t *data = malloc(sizeof(leveldb_work_data_t));
  if (!data)
    return -1;
  uv_work_t *req = malloc(sizeof(uv_work_t));
  if (!req) {
    free(data);
    return -1;
  }

  data->ctx = ctx;
  data->req = *req;
  data->key = strdup(key);
  data->value = NULL;
  data->len = 0;
  req->data = data;

  uv_queue_work(ctx->loop, req, work_cb, after_work_cb);
  return 0;
}