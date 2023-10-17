/*
typp - practice of typing text from the keyboard.
Copyright (C) 2021 Kirill Rekhov <mgrainmi@gmail.com>

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
 * remote_results.c - get text from remote server, 'F5 Results Table'.
 *
*/

#include "main.h"

#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

static void
display_users_results(char **align_res, int n_lines, int npm_highlight)
{
	char *table_rows[5] = { "Place", "Nickname", NULL, "Errors", "Time" };
	char lang_str[8], table_title[48];

	/* create items */
	tuiv.items = (ITEM **) calloc(n_lines + 1, sizeof(ITEM *));
	for (int i = 0; i < n_lines; i++)
		tuiv.items[i] = new_item(align_res[i], NULL);
	tuiv.items[n_lines] = (ITEM *) NULL;

	/* create menu and window */
	tuiv.menu = new_menu((ITEM **) tuiv.items);
	tuiv.menu_win = newwin(21, 80, (LINES - 24) / 2, (COLS - 80) / 2);

	/* set main window and sub window */
	set_menu_win(tuiv.menu, tuiv.menu_win);
	set_menu_sub(tuiv.menu, derwin(tuiv.menu_win, 5, 79, 5, 1));
	set_menu_format(tuiv.menu, 5, 1);
	set_menu_mark(tuiv.menu, " ");

	BOX_WBORDER_ZERO(tuiv.menu_win);
	keypad(tuiv.menu_win, TRUE);

	/* header line */
	mvwaddch(tuiv.menu_win, 2, 0, ACS_LTEE);
	mvwaddch(tuiv.menu_win, 4, 0, ACS_LTEE);
	mvwhline(tuiv.menu_win, 2, 1, ACS_HLINE, 78);
	mvwhline(tuiv.menu_win, 4, 1, ACS_HLINE, 78);
	mvwaddch(tuiv.menu_win, 2, 79, ACS_RTEE);
	mvwaddch(tuiv.menu_win, 4, 79, ACS_RTEE);

	post_menu(tuiv.menu);
	version_header_box();
	FOOTER_MSGS;

	if (npm_highlight)
		table_rows[2] = "WPM";
	else
		table_rows[2] = "CPM";

	/* table title */
	(npm_highlight)
	    ? strcpy(lang_str, "English")
	    : strcpy(lang_str, "Russian");

	wattron(tuiv.menu_win, COLOR_BOLD(2));
	sprintf(table_title, "Users Pivot Table, %s texts results - %s",
		lang_str, table_rows[2]);
	mvwprintw(tuiv.menu_win, 1, (80 - strlen(table_title)) / 2, "%s",
		  table_title);
	wattroff(tuiv.menu_win, COLOR_BOLD(2));

	/* table rows */
	wattron(tuiv.menu_win, A_BOLD);
	mvwprintw(tuiv.menu_win, 3, 2, "%-7s%-22s%s%9s%7s", table_rows[0],
		  table_rows[1], table_rows[2], table_rows[3], table_rows[4]);
	wattroff(tuiv.menu_win, A_BOLD);

	/* table footer */
	tuiv.menu_footer_msg =
	    "Use 'Up' or 'PgUp' and 'Down' or 'PgDn' to scroll down or up a page of items.";
	mvwprintw(tuiv.menu_win, 19, 2, "%s", tuiv.menu_footer_msg);;
	refresh();

	while (true) {
		switch (wgetch(tuiv.menu_win)) {
		case KEY_DOWN:
			menu_driver(tuiv.menu, REQ_DOWN_ITEM);
			break;

		case KEY_UP:
			menu_driver(tuiv.menu, REQ_UP_ITEM);
			break;

		case KEY_NPAGE:
			menu_driver(tuiv.menu, REQ_SCR_DPAGE);
			break;

		case KEY_PPAGE:
			menu_driver(tuiv.menu, REQ_SCR_UPAGE);
			break;

			case_EXIT;

		case KEY_F(3):
			/* free memory and clear window */
			unpost_menu(tuiv.menu);
			free_menu(tuiv.menu);
			for (int i = 0; i < n_lines; i++)
				free_item(tuiv.items[i]);
			free(tuiv.items);

			clear();
			return;
		}

		wrefresh(tuiv.menu_win);
	}
}

static int recv_message_len(char cpm_or_wpm[3])
{
	char buf[256];
	int client_sock, msg_len, len = 0;

	if ((client_sock = connect_to_server()) == -1) {
		close(client_sock);
		clear();
		return -1;
	}

	/* 3 bytes sent - 'wpm' or 'cpm' */
	if (send(client_sock, cpm_or_wpm, strlen(cpm_or_wpm), 0) != 3) {
		endwin();
		xfprintf("send() condition did not pass", __LINE__);
	}

	while (true) {
		memset(buf, 0, sizeof(buf));

		if ((msg_len = recv(client_sock, buf, sizeof(buf), 0)) <= 0) {
			if (msg_len == -1) {
				endwin();
				xfprintf(strerror(errno), __LINE__);
			}

			break;
		}

		len += msg_len;
	}

	close(client_sock);
	return len;
}

static void get_server_results(int npm_highlight)
{
	char **align_res, *serv_res, *token;
	char tmp_indent[32], cpm_or_wpm[4];
	int client_sock, total_msg_len = 0;
	int i, j = 0, k = 0, n = 1;
	bool first_num = true;
	size_t line_size = 50;

	/* fill array with "wpm" or "cpm" */
	(npm_highlight)
	    ? strcpy(cpm_or_wpm, "wpm")
	    : strcpy(cpm_or_wpm, "cpm");

	/* get length of the message from server
	   allocate memory with accurate size */
	if ((total_msg_len = recv_message_len(cpm_or_wpm)) == -1)
		return;

	serv_res = xmalloc(total_msg_len * sizeof(char));

	/* get all results from server */
	if ((client_sock = connect_to_server()) == -1) {
		close(client_sock);
		clear();
		return;
	}

	/* 3 bytes sent - 'wpm' or 'cpm' */
	if (send(client_sock, cpm_or_wpm, strlen(cpm_or_wpm), 0) != 3) {
		endwin();
		xfprintf("send() condition did not pass", __LINE__);
	}

	if (recv(client_sock, serv_res, total_msg_len, 0) == -1) {
		endwin();
		xfprintf(strerror(errno), __LINE__);
	}

	/* last new line replaced '\0' */
	serv_res[total_msg_len -= 1] = '\0';
	close(client_sock);

	/* count new lines */
	for (int i = 0; serv_res[i] != '\0'; i++)
		if (serv_res[i] == '\n')
			n++;

	align_res = xmalloc((n + 1) * sizeof(char *));

	for (i = 0; i < n; i++)
		align_res[i] = xmalloc(line_size * sizeof(char));
	align_res[i] = (char *)NULL;

	/* result alignment */
	token = strtok(serv_res, " \n");

	while (token != NULL) {

		if (isalpha(token[0])) {
			sprintf(tmp_indent, "%-19s", token);
		} else {
			if (strchr(token, ':')) {
				sprintf(tmp_indent, "%11s", token);
				first_num = true;
			} else if (first_num) {
				sprintf(tmp_indent, "%-7s", token);
				first_num = false;
			} else {
				sprintf(tmp_indent, "%6s", token);
			}
		}

		for (int i = 0; tmp_indent[i] != '\0'; i++) {
			align_res[j][k] = tmp_indent[i];
			k++;
		}

		if (first_num) {
			align_res[j][k] = '\0';
			j++;
			k = 0;
		}

		token = strtok(NULL, " \n");
	}

	display_users_results(align_res, n, npm_highlight);

	/* free memory and clear window */
	for (int i = 0; i < n; i++)
		free(align_res[i]);
	free(align_res);
	free(serv_res);

	clear();
}

void npm_menu()
{
	char *npm_arr[2] = { "Russian texts (CPM)", "English texts (WPM)" };
	int npm_highlight = 0;

	while (true) {
		tuiv.sel_unit_title = "Please, select top users table:";
		attron(COLOR_BOLD(7));
		mvprintw(7, (COLS - strlen(tuiv.sel_unit_title)) / 2,
			 "%s", tuiv.sel_unit_title);
		attroff(COLOR_BOLD(7));
		FOOTER_MSGS;

		version_header_box();
		menu_of_two_elements(npm_arr, npm_highlight);

		switch (getch()) {
		case KEY_UP:
			npm_highlight--;
			if (npm_highlight == -1)
				npm_highlight = 0;
			break;

		case KEY_DOWN:
			npm_highlight++;
			if (npm_highlight == 2)
				npm_highlight = 1;
			break;

			case_EXIT;

			case_CLEAR_CANCEL;

		case ASCII_ENTER:
			clear();
			get_server_results(npm_highlight);
			continue;
		}
	}
}
