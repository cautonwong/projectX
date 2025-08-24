#ifndef SQLITE_MANAGER_H
#define SQLITE_MANAGER_H

#include <sqlite3.h>
#include <uv.h>

// Forward declaration
#include "service.h"

// SQLite context structure
typedef struct sqlite_context_s {
  service_t *parent_service; // Pointer to parent service_t
  uv_loop_t *loop;           // libuv event loop
  sqlite3 *db;               // SQLite database handle
  char *db_path;             // Database file path

  // Callback for query results
  void (*on_query_result_cb)(service_t *svc, const char *result, size_t len,
                             void *user_data);
  void *user_data; // User-provided data for callbacks

} sqlite_context_t;

// Initialize SQLite manager
sqlite_context_t *sqlite_init(
    uv_loop_t *loop, service_t *parent_svc, const char *db_path,
    void (*on_query_result_cb)(service_t *, const char *, size_t, void *),
    void *user_data);

// Cleanup SQLite manager
void sqlite_cleanup(sqlite_context_t *ctx);

// Execute SQL query (asynchronous)
int sqlite_exec_async(sqlite_context_t *ctx, const char *sql);

#endif // SQLITE_MANAGER_H