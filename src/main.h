/*
typp - practice of typing text from the keyboard.
Copyright (C) 2021 Kirill Rekhov <krekhov.dev@mail.ru>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/*
 * main.h - header file with declarations.
 *
*/

#ifndef _MAIN_H
#define _MAIN_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_NCURSES_H
#include <ncurses.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_MENU_H
#include <menu.h>
#endif

#ifdef HAVE_FORM_H
#include <form.h>
#endif

void version_header_box();
void xfprintf(char *str, int line);
void *xmalloc(size_t size);
void menu_of_two_elements(char *elements[], int highlight);

extern struct tui_elements {
	const char *main_title;
	const char *sel_lang_title;
	const char *sel_unit_title;
	const char *note_msg;
	const char *nick_msg;
	const char *menu_footer_msg;
	WINDOW *main_title_win;
	WINDOW *result_win;
	WINDOW *text_win;
	WINDOW *help_win;
	WINDOW *rating_win;
	WINDOW *menu_win;
	MENU *menu;
	ITEM **items;
	FIELD *field[2];
	FORM *form;
} tuiv;

#define COLOR_BOLD(N) (COLOR_PAIR(N) | A_BOLD)
#define BOX_WBORDER_ZERO(W) (box(W, 0, 0))

#define END_CLEAR endwin(); clear();
#define END_CLEAR_REFRESH endwin(); clear(); refresh();

#define FOOTER_MSGS \
  attron(COLOR_PAIR(5)); \
  mvprintw(LINES - 2, 4, "%s", CANCEL_MSG); \
  mvprintw(LINES - 2, (COLS - strlen(QUIT_MSG)) - 4, "%s", QUIT_MSG); \
  attroff(COLOR_PAIR(5));

#define case_EXIT \
  case KEY_F(10): \
    endwin(); \
    exit(EXIT_SUCCESS);

#define case_CLEAR_CANCEL \
  case KEY_F(3): \
    clear(); \
    return;

#define case_CANCEL \
  case KEY_F(3): \
    return;

#define program_name "typp"
#define NAME_VERSION "Typing Practice - v1.0.0"
#define QUIT_MSG "F10 Quit"
#define CANCEL_MSG "F3 Cancel"
#define HELP_MSG "F1 Help"
#define ASCII_ENTER 10
#define ASCII_SPACE 32
#define ASCII_DEL 127
#define NICKNAME_LEN 19

#endif
