/*
 *   Copyright (c) 2024 Cameron Dudd
 *   All rights reserved.
 */

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

#include "db.h"

int main() {
  sqlite3 *conn = getCoffeeDBConnection();
  if (conn == NULL) {
    fprintf(stderr, "Failed to get coffee database connection.");
    return 1;
  }
  initializeCoffeeDB(conn);
  addMachine(conn, "name", "brand", "model", 0);
  closeConnection(conn);
  return 0;
}
