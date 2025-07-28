/*
 *   Copyright (c) 2024 Cameron Dudd
 *   All rights reserved.
 */

#include "db.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

sqlite3 *getCoffeeDBConnection() {
  sqlite3 *conn = NULL;
  fprintf(stdout, "Attempting to connect to %s\n", DB_NAME);
  int rc = sqlite3_open(DB_NAME, &conn);
  if (rc == SQLITE_OK) {
    fprintf(stdout, "Opened coffee database connection successfully.\n");
    return conn;
  }
  fprintf(stderr, "Couldn't open coffee database connection: %s\n", sqlite3_errmsg(conn));
  return NULL;
}

void closeConnection(sqlite3 *conn) {
  int rc = sqlite3_close_v2(conn);
  if (rc == SQLITE_OK) {
    fprintf(stdout, "Closed connection successfully.\n");
    return;
  }
  fprintf(stderr, "Couldn't close connection.\n");
}

static sqlite3_stmt *_prepare(sqlite3 *conn, const char *sql) {
  sqlite3_stmt *stmt;
  int rc;
  rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, 0);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(conn));
    return NULL;
  }
  return stmt;
}

static int _commit(sqlite3_stmt *stmt) {
  int rc = sqlite3_step(stmt);
  if (rc != SQLITE_DONE) {
    fprintf(stderr, "Failed to step\n");
    return rc;
  }

  rc = sqlite3_finalize(stmt);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to finalize\n");
    return rc;
  }

  return SQLITE_OK;
}

void initializeCoffeeDB(sqlite3 *conn) {
  struct stat statbuf;
  int rc;

  rc = stat(SCHEMA_NAME, &statbuf);
  if (rc == -1) {
    fprintf(stderr, "Failed to get stat on %s\n", SCHEMA_NAME);
    return;
  }

  fprintf(stdout, "Attempting to open schema file: %s\n", SCHEMA_NAME);

  FILE *fp;
  fp = fopen(SCHEMA_NAME, "r");
  if (fp == NULL) {
    fprintf(stderr, "Failed to open schema file.\n");
    return;
  }
  fprintf(stdout, "Opened schema file successfully.\n");

  size_t fileSize = sizeof(char) * statbuf.st_size;
  char *buff      = malloc(fileSize + 1);
  if (buff == NULL) {
    fclose(fp);
    return;
  }

  long readSize = fread(buff, sizeof(char), statbuf.st_size, fp);
  fclose(fp);
  if (readSize != statbuf.st_size) {
    fprintf(stderr, "Failed to read entire schema file.\n");
    free(buff);
    return;
  }
  buff[fileSize] = '\0';

  char *errMsg = NULL;
  rc           = sqlite3_exec(conn, buff, 0, 0, &errMsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "%s\n", errMsg);
    sqlite3_free(errMsg);
  }

  free(buff);
}

int simpleQuery(sqlite3 *conn, dbCallbackFuncT callback, void *callbackData, const char *sql) {
  char *errMsg = NULL;
  int rc       = sqlite3_exec(conn, sql, callback, callbackData, &errMsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to get brew_sessions %s\n", errMsg);
    sqlite3_free(errMsg);
    return 1;
  }
  return 0;
}

// START SELECT
// TODO (cameron): INNER JOIN foreign keys
int getBrewSessions(sqlite3 *conn, dbCallbackFuncT callback, void *callbackData) {
  const char *sql =
      "SELECT dose_grams, yield_grams, brew_time, rating, date, "
      "cafe_location_id, notes FROM brew_sessions;";
  return simpleQuery(conn, callback, callbackData, sql);
}

int getCafes(sqlite3 *conn, dbCallbackFuncT callback, void *callbackData) {
  const char *sql = "SELECT c.name, cl.lat, cl.lon FROM cafes c INNER JOIN cafe_locations cl ON c.id = cl.cafe_id;";
  return simpleQuery(conn, callback, callbackData, sql);
}
// END SELECT

// START INSERT
int addMachine(sqlite3 *conn, const char *name, const char *brand, const char *model, const int supportsPreInfusion) {
  const char *sql =
      "INSERT INTO machines (name, brand, model, "
      "supports_pre_infusion) VALUES (?, ?, ?, ?);";
  sqlite3_stmt *stmt = _prepare(conn, sql);
  if (stmt == NULL) {
    fprintf(stderr, "Failed to prepare stmt for %s\n", sql);
    return 1;
  }
  sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 2, brand, -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 3, model, -1, SQLITE_STATIC);
  sqlite3_bind_int(stmt, 4, (supportsPreInfusion <= 0) ? 0 : 1);
  int rc = _commit(stmt);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to commit %s\n", sql);
    return 1;
  }
  return 0;
}

int addGrinder(sqlite3 *conn, const char *name, const char *brand, const int automatic) {
  const char *sql    = "INSERT INTO grinders (name, brand, automatic) VALUES (?, ?, ?);";
  sqlite3_stmt *stmt = _prepare(conn, sql);
  if (stmt == NULL) {
    fprintf(stderr, "Failed to prepare stmt for %s\n", sql);
    return 1;
  }
  sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 2, brand, -1, SQLITE_STATIC);
  sqlite3_bind_int(stmt, 3, (automatic <= 0) ? 0 : 1);
  int rc = _commit(stmt);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to commit %s\n", sql);
    return 1;
  }
  return 0;
}

int addGrinderSetting(sqlite3 *conn, const int grinderId, const char *grinderConfig) {
  const char *sql =
      "INSERT INTO grinder_settings (grinder_id, grinder_config) "
      "VALUES (?, ?);";
  sqlite3_stmt *stmt = _prepare(conn, sql);
  if (stmt == NULL) {
    fprintf(stderr, "Failed to prepare stmt for %s\n", sql);
    return 1;
  }
  sqlite3_bind_int(stmt, 1, grinderId);
  sqlite3_bind_text(stmt, 2, grinderConfig, -1, SQLITE_STATIC);
  int rc = _commit(stmt);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to commit %s\n", sql);
    return 1;
  }
  return 0;
}
// END INSERT
