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
 * common.c - these functions need for another modules in this project.
 *
*/

#include "main.h"

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

void xfprintf(char *str, int line)
{
	fprintf(stderr, "%s: %s:%d: %s\n", program_name, __FILE__, line, str);
	exit(EXIT_FAILURE);
}

void *xmalloc(size_t size)
{
	void *p;
	if ((p = malloc(size)) == NULL) {
		endwin();
		xfprintf(strerror(errno), __LINE__);
	}

	return p;
}

void version_header_box()
{
	refresh();
	tuiv.main_title = NAME_VERSION;
	tuiv.main_title_win = newwin(4, COLS, 1, 0);
	BOX_WBORDER_ZERO(tuiv.main_title_win);

	wattron(tuiv.main_title_win, COLOR_BOLD(1));
	mvwprintw(tuiv.main_title_win, 1, (COLS - strlen(tuiv.main_title)) / 2, "%s", tuiv.main_title);
	wattroff(tuiv.main_title_win, COLOR_BOLD(1));
	wrefresh(tuiv.main_title_win);
}

void menu_of_two_elements(char *elements[], int highlight)
{
	for (int i = 0; i < 2; i++) {
		if (i == highlight)
			attron(A_UNDERLINE | A_STANDOUT | COLOR_BOLD(6));
		mvprintw(i + 9, (COLS - strlen(elements[i]) - 4) / 2, "  %s  ", elements[i]);
		attroff(A_UNDERLINE | A_STANDOUT | COLOR_BOLD(6));
	}
}
