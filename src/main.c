/*
 *   Copyright (c) 2024 Cameron Dudd
 *   All rights reserved.
 */

#include <signal.h>

#include "db.h"
#include "tui.h"

int main() {
  signal(SIGINT, tuiKill);

  sqlite3 *conn = getCoffeeDBConnection();
  tuiMain(conn);
  closeConnection(conn);
  return 0;
}
