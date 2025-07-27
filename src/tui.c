/*
 *   Copyright (c) 2024 Cameron Dudd
 *   All rights reserved.
 */

#include "tui.h"

#include <bits/types/sig_atomic_t.h>
#include <ncurses.h>
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>

#include "db.h"

#define MENU_HEIGHT    3
#define MENU_COL_WIDTH 20
#define MENU_MAX_COLS  20
#define ENDL           "|\n "

// CONTROLS
#define MENU_LEFT    'h'
#define MENU_DOWN    'j'
#define MENU_UP      'k'
#define MENU_RIGHT   'l'
#define MENU_REFRESH 'r'

#define MIN(A, B) ((A < B) ? A : B)

volatile sig_atomic_t _alive = 1;
static int _wroteHeader      = 0;
const char *menu[]           = {"Brew Sessions", "Coffee", "Cafes"};

int _drawDbContentCallback(void *buff, int argc, char **argv, char **azColName) {
  int numCols                         = MIN(MENU_MAX_COLS, argc);
  static int colStrLen[MENU_MAX_COLS] = {0};

  char *outputBuff = (char *)buff;

  if (!_wroteHeader) {
    for (int i = 0; i < numCols; ++i) {
      int colWidth                 = MIN(MENU_COL_WIDTH, (int)strlen(azColName[i]) + 1);  // FIXME(cameron): support data > column name instead of +1 for `date`
      char colBuff[MENU_COL_WIDTH] = {'\0'};
      snprintf(colBuff, sizeof(colBuff), "| %-*s ", colWidth, azColName[i]);
      strcat(outputBuff, colBuff);
      colStrLen[i] = colWidth;
    }
    strcat(outputBuff, ENDL);
    _wroteHeader = 1;
  }

  for (int i = 0; i < numCols; ++i) {
    char colBuff[20] = {'\0'};
    if (argv[i]) {
      snprintf(colBuff, sizeof(colBuff), "| %-*s ", colStrLen[i], argv[i]);
    } else {
      snprintf(colBuff, sizeof(colBuff), "| %-*s ", colStrLen[i], "NULL");
    }
    strcat(outputBuff, colBuff);
  }
  strcat(outputBuff, ENDL);

  return 0;
}

void _drawMenu(WINDOW *menuW, int menuHighlightIdx, int numMenuItems) {
  int curX = 2;
  for (int i = 0; i < numMenuItems; ++i) {
    if (i == menuHighlightIdx) {
      wattron(menuW, A_REVERSE);
      mvwprintw(menuW, 1, curX, "%s", menu[i]);
      wattroff(menuW, A_REVERSE);
    } else {
      mvwprintw(menuW, 1, curX, "%s", menu[i]);
    }
    curX += strlen(menu[i]) + 1;
  }
  box(menuW, 0, 0);
  wrefresh(menuW);
}

void _drawContent(WINDOW *contentW, sqlite3 *conn, int menuHighlightIdx) {
  static int lastMenuHighlightIdx = -1;
  char buff[1024];
  memset(buff, '\0', 1024);

  int _menuIdxChange = (menuHighlightIdx == lastMenuHighlightIdx) ? 0 : 1;
  if (_menuIdxChange) {
    _wroteHeader         = 0;
    lastMenuHighlightIdx = menuHighlightIdx;
  }

  const char *menuItemName = menu[menuHighlightIdx];
  if (_menuIdxChange) {
    if (strcmp(menuItemName, "Brew Sessions") == 0) {
      getBrewSessions(conn, _drawDbContentCallback, buff);
    } else if (strcmp(menuItemName, "Cafes") == 0) {
      getCafes(conn, _drawDbContentCallback, buff);
    } else {
      werase(contentW);
    }
  }
  mvwprintw(contentW, 1, 1, "%s", buff);
  box(contentW, 0, 0);
  wrefresh(contentW);
}

void _tuiInit() {
  // From man page
  initscr();    // ncurses init screen
  cbreak();     // ncurses immediately process key press
  noecho();     // ncurses no echo
  curs_set(0);  // ncurses hide the cursor
}

void _tuiLoop(sqlite3 *conn) {
  int menuHighlightIdx = 0;
  int numMenuItems     = sizeof(menu) / sizeof(menu[0]);

  // height, width, y, x
  WINDOW *menuW    = newwin(MENU_HEIGHT, COLS, 0, 0);
  WINDOW *contentW = newwin(LINES - MENU_HEIGHT, COLS, MENU_HEIGHT, 0);

  _drawMenu(menuW, menuHighlightIdx, numMenuItems);
  _drawContent(contentW, conn, menuHighlightIdx);

  char ch;
  while (_alive && ((ch = wgetch(menuW)) != 'q')) {
    switch (ch) {
      case MENU_LEFT:
        menuHighlightIdx = (menuHighlightIdx + numMenuItems - 1) % numMenuItems;
        break;
      case MENU_RIGHT:
        menuHighlightIdx = (menuHighlightIdx + 1) % numMenuItems;
        break;
      case MENU_DOWN:
        break;
      case MENU_UP:
        break;
      default:
        break;
    }
    _drawMenu(menuW, menuHighlightIdx, numMenuItems);
    _drawContent(contentW, conn, menuHighlightIdx);
  }

  delwin(menuW);
}

void _tuiFree() { endwin(); }

void tuiKill(int sig) {
  (void)sig;  // TODO(cameron): use sig
  _alive = 0;
}

void tuiMain(sqlite3 *conn) {
  _tuiInit();
  _tuiLoop(conn);
  _tuiFree();
}
