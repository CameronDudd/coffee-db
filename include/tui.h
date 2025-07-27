/*
 *   Copyright (c) 2024 Cameron Dudd
 *   All rights reserved.
 */

#ifndef TUI_H
#define TUI_H

#include <sqlite3.h>

void tuiKill(int sig);
void tuiMain(sqlite3* conn);

#endif  // TUI_H
