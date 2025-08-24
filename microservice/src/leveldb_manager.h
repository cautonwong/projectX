#ifndef LEVELDB_MANAGER_H
#define LEVELDB_MANAGER_H

#include <leveldb/c.h>
#include <uv.h>

// Forward declaration
#include "service.h"

// LevelDB context structure
typedef struct leveldb_context_s {
  service_t *parent_service; // Pointer to parent service_t
  uv_loop_t *loop;           // libuv event loop
  leveldb_t *db;             // LevelDB database handle
  char *db_path;             // Database directory path

  // Callback for query results
  void (*on_query_result_cb)(service_t *svc, const char *key, const char *value,
                             size_t len, void *user_data);
  void *user_data; // User-provided data for callbacks

} leveldb_context_t;

// Initialize LevelDB manager
leveldb_context_t *
leveldb_init(uv_loop_t *loop, service_t *parent_svc, const char *db_path,
             void (*on_query_result_cb)(service_t *, const char *, const char *,
                                        size_t, void *),
             void *user_data);

// Cleanup LevelDB manager
void leveldb_cleanup(leveldb_context_t *ctx);

// Get value by key (asynchronous)
int leveldb_get_async(leveldb_context_t *ctx, const char *key);

#endif // LEVELDB_MANAGER_H