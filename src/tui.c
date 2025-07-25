/*
 *   Copyright (c) 2024 Cameron Dudd
 *   All rights reserved.
 */

#include "tui.h"

#include <ncurses.h>
#include <string.h>

#define MENU_HEIGHT 3

#define MENU_LEFT  'h'
#define MENU_DOWN  'j'
#define MENU_UP    'k'
#define MENU_RIGHT 'l'

const char *menu[] = {"Sessions", "Coffee", "Cafe"};

void _drawMenu(WINDOW *menuW, int menuHighlightIdx, int numMenuItems) {
  int curX = 2;
  box(menuW, 0, 0);
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
  wrefresh(menuW);
}

void _drawContent(WINDOW *contentW) {
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

void _tuiLoop() {
  int menuHighlightIdx = 0;
  int numMenuItems     = sizeof(menu) / sizeof(menu[0]);

  // height, width, y, x
  WINDOW *menuW    = newwin(MENU_HEIGHT, COLS, 0, 0);
  WINDOW *contentW = newwin(LINES - MENU_HEIGHT, COLS, MENU_HEIGHT, 0);

  _drawMenu(menuW, menuHighlightIdx, numMenuItems);
  _drawContent(contentW);

  char ch;
  while ((ch = wgetch(menuW)) != 'q') {
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
    _drawContent(contentW);
  }

  delwin(menuW);
}

void _tuiFree() { endwin(); }

void tuiMain() {
  _tuiInit();
  _tuiLoop();
  _tuiFree();
}
