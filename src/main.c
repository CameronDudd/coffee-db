/*
 *   Copyright (c) 2024 Cameron Dudd
 *   All rights reserved.
 */

#include <locale.h>
#include <ncurses.h>

#include "tui.h"

int main() {
  setlocale(LC_ALL, "");
  tuiMain();
  return 0;
}
