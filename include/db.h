/*
 *   Copyright (c) 2024 Cameron Dudd
 *   All rights reserved.
 */

#ifndef DB_H
#define DB_H

#include <sqlite3.h>

#define DB_NAME     "coffee-diary.db"
#define SCHEMA_NAME "coffee-db.sql"

typedef int (*dbCallbackFuncT)(void *callbackData, int argc, char **argv, char **azColName);
typedef int (*dbFuncT)(sqlite3 *conn, dbCallbackFuncT callback, void *callbackData);

sqlite3 *getCoffeeDBConnection();
void closeConnection(sqlite3 *conn);

void initializeCoffeeDB(sqlite3 *conn);

int simpleQuery(sqlite3 *conn, dbCallbackFuncT callback, void *callbackData, const char *sql);

// START SELECT
int getBrewSessions(sqlite3 *conn, dbCallbackFuncT callback, void *callbackData);
int getCafes(sqlite3 *conn, dbCallbackFuncT callback, void *callbackData);
// END SELECT

// START INSERT
int addMachine(sqlite3 *conn, const char *name, const char *brand, const char *model, const int supportsPreInfusion);
int addGrinder(sqlite3 *conn, const char *name, const char *brand, const int automatic);
int addGrinderSetting(sqlite3 *conn, const int grinderId, const char *grinderConfig);
// END INSERT

#endif  // DB_H
