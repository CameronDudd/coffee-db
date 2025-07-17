/*
 *   Copyright (c) 2024 Cameron Dudd
 *   All rights reserved.
 */

#ifndef DB_H
#define DB_H

#include <sqlite3.h>

#define DB_NAME "coffee-diary.db"
#define SCHEMA_NAME "coffee-diary-schema.sql"

sqlite3 *getCoffeeDBConnection();
void closeConnection(sqlite3 *conn);

void initializeCoffeeDB(sqlite3 *conn);

int addMachine(sqlite3 *db, const char *name, const char *brand,
               const char *model, int supportsPreInfusion);

#endif // DB_H
