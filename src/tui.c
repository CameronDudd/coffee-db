/*
 *   Copyright (c) 2024 Cameron Dudd
 *   All rights reserved.
 */

#include "tui.h"

#include <bits/types/sig_atomic_t.h>
#include <ncurses.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "db.h"

#define MENU_HEIGHT        3
#define MENU_MAX_COL_WIDTH 10
#define MENU_MAX_COLS      20

// CONTROLS
#define MENU_LEFT    'h'
#define MENU_DOWN    'j'
#define MENU_UP      'k'
#define MENU_RIGHT   'l'
#define MENU_REFRESH 'r'

#define MIN(A, B) ((A < B) ? A : B)
#define MAX(A, B) ((A > B) ? A : B)

typedef struct {
  char ***rows;
  int maxRows;
  int maxCols;
  int currentRow;
  int argc;
} QueryResult;

volatile sig_atomic_t _alive = 1;
const char *menu[]           = {"Brew Sessions", "Cafes"};

int _drawDbContentCallback(void *data, int argc, char **argv, char **azColName) {
  int numCols         = MIN(MENU_MAX_COLS, argc);
  QueryResult *result = (QueryResult *)data;

  if (result->currentRow >= result->maxRows) {
    return 0;
  }

  if (result->currentRow == 0) {
    for (int i = 0; i < numCols; ++i) {
      strcat(result->rows[0][i], azColName[i]);
    }
    (result->currentRow)++;
    result->argc = argc;
  }

  for (int i = 0; i < numCols; ++i) {
    if (argv[i]) {
      strcat(result->rows[result->currentRow][i], argv[i]);
    } else {
      strcat(result->rows[result->currentRow][i], "NULL");
    }
  }

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
  if (menuHighlightIdx != lastMenuHighlightIdx) {
    werase(contentW);  // clear content window

    int maxRows = LINES - MENU_HEIGHT - 2;
    int maxCols = COLS - 2;

    // allocate memory for each window row
    char ***rows = malloc(maxRows * sizeof(char **));
    for (int i = 0; i < maxRows; ++i) {
      rows[i] = malloc(maxCols * sizeof(char *));
      for (int j = 0; j < maxCols; ++j) {
        rows[i][j] = calloc(MENU_MAX_COL_WIDTH, sizeof(char));
      }
    }

    // create result object
    QueryResult result = {
        .rows       = rows,
        .maxRows    = maxRows,
        .maxCols    = maxCols,
        .currentRow = 0,
        .argc       = 0,
    };

    // get data from the database (sequential)
    const char *menuItemName = menu[menuHighlightIdx];
    if (strcmp(menuItemName, "Brew Sessions") == 0) {
      getBrewSessions(conn, _drawDbContentCallback, &result);
    } else if (strcmp(menuItemName, "Cafes") == 0) {
      getCafes(conn, _drawDbContentCallback, &result);
    }

    // iterate over results to find out the largest column width for each column
    int maxWidthLookup[MENU_MAX_COLS] = {0ul};
    for (int i = 0; i <= result.currentRow; ++i) {
      for (int j = 0; j < result.argc; ++j) {
        int cellLen       = strlen(result.rows[i][j]);
        maxWidthLookup[j] = MIN(MENU_MAX_COL_WIDTH, MAX(maxWidthLookup[j], cellLen));
      }
    }

    // iterate over results to print each row
    for (int i = 0; i <= result.currentRow; ++i) {
      char rowBuff[1024] = {'\0'};
      for (int j = 0; j <= result.argc; ++j) {
        char colBuff[MENU_MAX_COL_WIDTH * MENU_MAX_COLS] = {'\0'};
        snprintf(colBuff, maxWidthLookup[j] + 4, "| %-*s ", maxWidthLookup[j], result.rows[i][j]);
        strcat(rowBuff, colBuff);
      }
      mvwprintw(contentW, i + 1, 1, "%s", rowBuff);
    }

    box(contentW, 0, 0);  // artistic border box

    // free allocated memory
    for (int i = 0; i < maxRows; ++i) {
      for (int j = 0; j < maxCols; ++j) {
        free(rows[i][j]);
      }
      free(rows[i]);
    }
    free(rows);

    lastMenuHighlightIdx = menuHighlightIdx;  // update menu idx
    wrefresh(contentW);                       // trigger refresh
  }
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
