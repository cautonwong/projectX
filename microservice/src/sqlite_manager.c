#include "sqlite_manager.h"
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

typedef struct {
  sqlite_context_t *ctx;
  uv_work_t req;
  char *sql;
  char *result;
  size_t len;
} sqlite_work_data_t;

static void work_cb(uv_work_t *req) {
  sqlite_work_data_t *data = (sqlite_work_data_t *)req->data;
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(data->ctx->db, data->sql, -1, &stmt, NULL) ==
      SQLITE_OK) {
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      const unsigned char *text = sqlite3_column_text(stmt, 0);
      data->len = strlen((const char *)text);
      data->result = malloc(data->len + 1);
      if (data->result)
        memcpy(data->result, text, data->len + 1);
    }
    sqlite3_finalize(stmt);
  }
}

static void after_work_cb(uv_work_t *req, int status) {
  sqlite_work_data_t *data = (sqlite_work_data_t *)req->data;
  if (status == 0 && data->ctx->on_query_result_cb && data->result) {
    data->ctx->on_query_result_cb(data->ctx->parent_service, data->result,
                                  data->len, data->ctx->user_data);
  }
  if (data->result)
    free(data->result);
  if (data->sql)
    free(data->sql);
  free(data);
  free(req);
}

sqlite_context_t *sqlite_init(
    uv_loop_t *loop, service_t *parent_svc, const char *db_path,
    void (*on_query_result_cb)(service_t *, const char *, size_t, void *),
    void *user_data) {
  sqlite_context_t *ctx = malloc(sizeof(sqlite_context_t));
  if (!ctx)
    return NULL;

  ctx->parent_service = parent_svc;
  ctx->loop = loop;
  ctx->db_path = strdup(db_path);
  ctx->on_query_result_cb = on_query_result_cb;
  ctx->user_data = user_data;

  if (sqlite3_open(db_path, &ctx->db) != SQLITE_OK) {
    fprintf(stderr, "Failed to open SQLite: %s\n", sqlite3_errmsg(ctx->db));
    free(ctx->db_path);
    free(ctx);
    return NULL;
  }

  fprintf(stderr, "SQLite initialized for %s\n", db_path);
  return ctx;
}

void sqlite_cleanup(sqlite_context_t *ctx) {
  if (!ctx)
    return;
  if (ctx->db)
    sqlite3_close(ctx->db);
  if (ctx->db_path)
    free(ctx->db_path);
  free(ctx);
}

int sqlite_exec_async(sqlite_context_t *ctx, const char *sql) {
  if (!ctx || !sql)
    return -1;
  sqlite_work_data_t *data = malloc(sizeof(sqlite_work_data_t));
  if (!data)
    return -1;
  uv_work_t *req = malloc(sizeof(uv_work_t));
  if (!req) {
    free(data);
    return -1;
  }

  data->ctx = ctx;
  data->req = *req;
  data->sql = strdup(sql);
  data->result = NULL;
  data->len = 0;
  req->data = data;

  uv_queue_work(ctx->loop, req, work_cb, after_work_cb);
  return 0;
}