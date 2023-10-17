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
 * main.c - most of the functionality is here.
 *
*/

#include "main.h"

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#ifdef HAVE_MATH_H
#include <math.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_TIME_H
#include <time.h>
#endif

#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif

#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif

#ifdef HAVE_ERROR_H
#include <error.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

void npm_menu();
void send_res_to_server(int sock, int lang, char *nickname,
			double npm, int err, int min, int sec);

struct tui_elements tuiv;
static char *langs[2] = { "Russian", "English" };

static int lang_highlight = 0, newlcount;

static void rating_info()
{
	tuiv.rating_win = newwin(22, 80, (LINES - 24) / 2, (COLS - 80) / 2);
	BOX_WBORDER_ZERO(tuiv.rating_win);
	mvwaddstr(tuiv.rating_win, 0, (80 - strlen("Help")) / 2, "Help");
	mvwaddstr(tuiv.rating_win, 1, 1, "WPM rating:");
	mvwaddstr(tuiv.rating_win, 2, 2, "(slow) less 24");
	mvwaddstr(tuiv.rating_win, 3, 2, "(fine) more or equal 24 and less 32");
	mvwaddstr(tuiv.rating_win, 4, 2,
		  "(middle) more or equal 32 and less 52");
	mvwaddstr(tuiv.rating_win, 5, 2, "(well) more or equal 52 and less 70");
	mvwaddstr(tuiv.rating_win, 6, 2,
		  "(pro) more or equal 70 and less or equal 80");
	mvwaddstr(tuiv.rating_win, 7, 2, "(best) more 80");
	mvwaddstr(tuiv.rating_win, 10, 1, "CPM rating:");
	mvwaddstr(tuiv.rating_win, 11, 2, "(slow) less 120");
	mvwaddstr(tuiv.rating_win, 12, 2,
		  "(fine) more or equal 120 and less 160");
	mvwaddstr(tuiv.rating_win, 13, 2,
		  "(middle) more or equal 160 and less 260");
	mvwaddstr(tuiv.rating_win, 14, 2,
		  "(well) more or equal 260 and less 350");
	mvwaddstr(tuiv.rating_win, 15, 2,
		  "(pro) more or equal 350 and less or equal 400");
	mvwaddstr(tuiv.rating_win, 16, 2, "(best) more 400");
	FOOTER_MSGS;
	wrefresh(tuiv.rating_win);

	while (true) {
		switch (getch()) {
			case_EXIT;
			case_CLEAR_CANCEL;
		}
	}
}

static void help_info()
{
	tuiv.help_win = newwin(22, 80, (LINES - 24) / 2, (COLS - 80) / 2);
	BOX_WBORDER_ZERO(tuiv.help_win);
	refresh();
	mvwaddstr(tuiv.help_win, 0, (80 - strlen("Help")) / 2, "Help");
	mvwaddstr(tuiv.help_win, 1, 1,
		  "This free software, and you are welcome to redistribute in under terms of");
	mvwaddstr(tuiv.help_win, 2, 1,
		  "MIT License. This software is intended for the practice of typing text from");
	mvwaddstr(tuiv.help_win, 3, 1, "the keyboard.");
	mvwaddstr(tuiv.help_win, 5, 1,
		  "From the available texts English and Russian.");
	mvwaddstr(tuiv.help_win, 6, 1,
		  "For correct display Russian characters recommended use UTF-8 charset.");
	mvwaddstr(tuiv.help_win, 7, 1,
		  "WPM (words per minute) is used to calculate the speed of English texts.");
	mvwaddstr(tuiv.help_win, 8, 1,
		  "CPM (characters per minute) is used to calculate the speed of Russian texts.");
	mvwaddstr(tuiv.help_win, 10, 1,
		  "You can use the keys of keyboard to navigate ('Up' / 'Down') in menus.");
	mvwaddstr(tuiv.help_win, 11, 1,
		  "It is recommended to use a terminal size of at least 80x24.");
	mvwaddstr(tuiv.help_win, 13, 1,
		  "The countdown time starts from the first entered character.");
	mvwaddstr(tuiv.help_win, 14, 1,
		  "After full entering the text, the results will appear, along with the rating.");
	mvwaddstr(tuiv.help_win, 15, 1,
		  "To see the description of ratings, press");
	wattron(tuiv.help_win, COLOR_PAIR(5) | A_UNDERLINE);
	mvwaddstr(tuiv.help_win, 15, 42, "Enter");
	wattroff(tuiv.help_win, COLOR_PAIR(5) | A_UNDERLINE);
	mvwaddstr(tuiv.help_win, 16, 1,
		  "You can share your result after full entering text to compete (client-server).");
	mvwaddstr(tuiv.help_win, 17, 1,
		  "Press 'F5' in the main menu to see shared results in pivot table.");
	mvwaddstr(tuiv.help_win, 19, 1, NAME_VERSION);
	mvwaddstr(tuiv.help_win, 20, 1,
		  "Typing Practice written by Kirill Rekhov <mgrainmi@gmail.com>");
	FOOTER_MSGS;
	wrefresh(tuiv.help_win);

	while (true) {
		switch (getch()) {
			case_EXIT;

			case_CLEAR_CANCEL;

		case ASCII_ENTER:
			delwin(tuiv.help_win);
			rating_info();
			return;
		}
	}
}

static char *trim_whitespaces(char *str)
{
	char *end;

	/* trim leading space */
	while (isspace(*str))
		str++;

	/* all spaces? */
	if (*str == 0)
		return str;

	/* trim trailing space */
	end = str + strnlen(str, NICKNAME_LEN) - 1;

	while (end > str && isspace(*end))
		end--;

	/* write new null terminator */
	*(end + 1) = '\0';

	return str;
}

static void get_user_nickname(char **nickname)
{
	int ch;

	/* initialize the fields */
	tuiv.field[0] =
	    new_field(1, NICKNAME_LEN - 1, 10, (COLS - (NICKNAME_LEN - 1)) / 2,
		      0, 0);
	tuiv.field[1] = NULL;

	/* set field options */
	set_field_back(tuiv.field[0], A_UNDERLINE);	/* print a line for the option   */
	field_opts_off(tuiv.field[0], O_AUTOSKIP);	/* don't go to next field when this */

	/* create the form and post it */
	curs_set(1);
	tuiv.form = new_form(tuiv.field);
	post_form(tuiv.form);
	refresh();

	attron(COLOR_BOLD(1));
	tuiv.nick_msg =
	    "Write your nickname, use alphabetic characters and press Enter.";
	mvprintw(7, (COLS - strlen(tuiv.nick_msg)) / 2, tuiv.nick_msg);
	tuiv.nick_msg =
	    "After submitting your result will be in the 'Results Table'.";
	mvprintw(8, (COLS - strlen(tuiv.nick_msg)) / 2, tuiv.nick_msg);
	attroff(COLOR_BOLD(1));
	FOOTER_MSGS;
	pos_form_cursor(tuiv.form);

	while ((ch = getch())) {
		switch (ch) {
		case KEY_BACKSPACE:
		case ASCII_DEL:
			form_driver(tuiv.form, REQ_DEL_PREV);
			continue;

		case ASCII_ENTER:
			curs_set(0);
			form_driver(tuiv.form, REQ_NEXT_FIELD);
			form_driver(tuiv.form, REQ_PREV_FIELD);
			strcpy(*nickname,
			       trim_whitespaces(field_buffer
						(tuiv.field[0], 0)));

			if (*nickname[0] == '\0' || isdigit(*nickname[0])) {
				curs_set(1);
				mvprintw(0, 0, "Please, input your nickname.");
				pos_form_cursor(tuiv.form);
				continue;
			}

			break;

			case_EXIT;

		case KEY_F(3):
			free(*nickname);
			*nickname = NULL;
			break;

		default:
			if (isalpha(ch))
				form_driver(tuiv.form, ch);
			continue;
		}

		break;
	}

	/* free form, field memory */
	unpost_form(tuiv.form);
	free_form(tuiv.form);
	free_field(tuiv.field[0]);
}

static char *get_wpm_rating(int wpm)
{
	if (wpm < 24)
		return "slow";
	if (wpm >= 24 && wpm < 32)
		return "fine";
	if (wpm >= 32 && wpm < 52)
		return "middle";
	if (wpm >= 52 && wpm < 70)
		return "well";
	if (wpm >= 70 && wpm < 80)
		return "pro";
	if (wpm >= 80)
		return "best";
	return NULL;
}

static char *get_cpm_rating(int cpm)
{
	if (cpm < 120)
		return "slow";
	if (cpm >= 120 && cpm < 160)
		return "fine";
	if (cpm >= 160 && cpm < 260)
		return "middle";
	if (cpm >= 260 && cpm < 350)
		return "well";
	if (cpm >= 350 && cpm < 400)
		return "pro";
	if (cpm >= 400)
		return "best";
	return NULL;
}

static void
display_input_result(int errcount, int scount, int sscount,
		     int wcount, double sec_diff)
{
	int min, sec, client_sock;
	char *rating, *nickname;
	double time_in_min, npm;

	/* convert */
	time_in_min = sec_diff / 60.0f;
	min = sec_diff / 60;
	sec = sec_diff - (min * 60);

	/* speed count - wpm or cpm / get user rating */
	if (lang_highlight) {
		npm = round((scount / 5 - errcount) / time_in_min);
		rating = get_wpm_rating(npm);
	} else {
		npm = round(scount / time_in_min);
		rating = get_cpm_rating(npm);
	}

	if (rating == NULL) {
		endwin();
		xfprintf("Pointer 'rating' equal NULL", __LINE__);
	}

	tuiv.result_win = newwin(20, 35, 1, 1);
	BOX_WBORDER_ZERO(tuiv.result_win);

	refresh();
	wattron(tuiv.result_win, A_UNDERLINE | COLOR_BOLD(7));
	mvwaddstr(tuiv.result_win, 2, (35 - strlen("Result")) / 2, "Result");
	wattroff(tuiv.result_win, A_UNDERLINE | COLOR_BOLD(7));

	mvwaddstr(tuiv.result_win, 5, 2, "Rating:");
	wattron(tuiv.result_win, COLOR_BOLD(1));
	mvwaddstr(tuiv.result_win, 5, 10, rating);
	wattroff(tuiv.result_win, COLOR_BOLD(1));

	mvwprintw(tuiv.result_win, 7, 2, "Time(m:s) %16.2d:%.2d", min, sec);
	mvwprintw(tuiv.result_win, 8, 2, "Errors: %21d", errcount);

	(lang_highlight)
	    ? mvwprintw(tuiv.result_win, 10, 2, "WPM: %24.0f", npm)
	    : mvwprintw(tuiv.result_win, 10, 2, "CPM: %24.0f", npm);

	mvwprintw(tuiv.result_win, 12, 2, "Text: %s", langs[lang_highlight]);
	mvwprintw(tuiv.result_win, 13, 2, "Words: %22d", wcount);
	mvwprintw(tuiv.result_win, 14, 2, "Text lines: %17d", newlcount);
	mvwprintw(tuiv.result_win, 15, 2, "Total characters: %11d", scount);
	mvwprintw(tuiv.result_win, 16, 2, "Characters less spaces: %5d",
		  sscount);
	attron(A_UNDERLINE | A_STANDOUT | COLOR_PAIR(6));
	mvprintw(19, 3, "Press Enter to share result");
	attroff(A_UNDERLINE | A_STANDOUT | COLOR_PAIR(6));
	FOOTER_MSGS;
	wrefresh(tuiv.result_win);

	while (true) {
		switch (getch()) {
			case_EXIT;

			case_CANCEL;

		case ASCII_ENTER:
			nickname = xmalloc(NICKNAME_LEN * sizeof(char));
			get_user_nickname(&nickname);

			if (nickname != NULL) {
				if ((client_sock = connect_to_server()) == -1) {
					close(client_sock);
					clear();
					return;
				}

				send_res_to_server(client_sock, lang_highlight,
						   nickname, npm, errcount, min,
						   sec);
				mvprintw(0, 0, "%s - data sent successfully.",
					 nickname);
				mvprintw(1, 0,
					 "Your result will be displayed in the 'Results Table' along with your nickname.");
				mvprintw(2, 0, "Press any key...");

				free(nickname);
				getch();
			}

			return;
		}
	}
}

static void input_text(wchar_t *main_text, size_t wlent, WINDOW * text_win)
{
	wchar_t error_char[2];	/* in case of discrepancy */
	wint_t cuser;		/* user input character */
	bool err_bool = false;	/* boolean for error counting */
	time_t start_t, end_t;	/* need for user input time */

	double sec_diff;
	int scount, xcount = 1, ycount = 1;
	int errcount = 0, sscount = 0, wcount = 0, wc = 0;

	curs_set(1);		/* set cursor normal station */

	/* if the user enters Russian characters, they will be displayed incorrectly
	   for this is used wide characters. so that there is a correct comparison,
	   display in terminal, etc. */
	while (main_text[wc] != '\0') {
		wmove(text_win, ycount, xcount + 1);
		wget_wch(text_win, &cuser);

		if (wc == 0)
			time(&start_t);

		switch (cuser) {
			case_EXIT;

			case_CANCEL;

		default:
			if (main_text[wc] == cuser) {
				xcount++;
				if (cuser == ASCII_ENTER) {
					xcount = 1;	/* back to first character of line */
					ycount++;	/* go to the next line */
				} else {
					wattron(text_win, COLOR_BOLD(3));
					mvwaddnwstr(text_win, ycount, xcount,
						    (wchar_t *)&cuser, 1);
					wattroff(text_win, COLOR_BOLD(3));
				}

				wc++;
				wmove(text_win, ycount, xcount + 1);
				err_bool = false;
			} else {
				if (main_text[wc] == ASCII_ENTER) {
					wc++;
					xcount = 1;
					ycount++;
					if (cuser == ASCII_SPACE)
						continue;
				}

				if (err_bool == false) {
					errcount++;
					err_bool = true;
				}

				swprintf(error_char, 2, L"%lc", main_text[wc]);
				wattron(text_win, COLOR_BOLD(4));
				mvwaddnwstr(text_win, ycount, xcount + 1,
					    error_char, 1);
				wattroff(text_win, COLOR_BOLD(4));
			}
		}

		refresh();
		wrefresh(text_win);
	}

	/* total characters counter, don't count new lines */
	scount = wlent - (newlcount - 1);

	for (int i = 0; i <= wlent; i++) {
		/* words counter */
		if (isspace(main_text[i]) || main_text[i] == '\0')
			wcount++;

		/* characters counter, without spaces */
		if (!(isspace(main_text[i])))
			sscount++;
	}

	/* wait enter from user */
	while (true) {
		if ((getch()) == ASCII_ENTER) {
			time(&end_t);
			curs_set(0);
			clear();
			break;
		}
	}

	sec_diff = difftime(end_t, start_t);	/* return double number (diff seconds) */
	display_input_result(errcount, scount, sscount, wcount, sec_diff);
}

static void
display_split_text(wchar_t *main_text, size_t wlent, WINDOW * text_win)
{
	wchar_t *line, *state, *tmppt;

	tmppt = xmalloc(wlent * sizeof(wchar_t) + sizeof(wchar_t));
	wcscpy(tmppt, main_text);

	line = wcstok(tmppt, L"\n", &state);
	for (newlcount = 1; line != NULL; newlcount++) {
		wattron(tuiv.text_win, A_BOLD);
		mvwaddwstr(tuiv.text_win, newlcount, 2, line);
		wattroff(tuiv.text_win, A_BOLD);
		line = wcstok(NULL, L"\n", &state);
	}

	newlcount--;
	free(tmppt);

	refresh();
	wrefresh(tuiv.text_win);
}

static void
getrnd_text(wchar_t **main_text, size_t tsize, char *fname, int offsets[])
{
	char fpath[30] = "/usr/local/share/typp/";
	int j = sizeof(wchar_t), i;
	FILE *stream;
	wint_t c;

	strcat(fpath, fname);
	if ((stream = fopen(fpath, "r")) == NULL) {
		endwin();
		xfprintf(strerror(errno), __LINE__);
	}

	srand(time(NULL));
	fseek(stream, offsets[rand() % 11], SEEK_SET);

	for (i = 0; (c = fgetwc(stream)) != '#'; i++) {
		(*main_text)[i] = c;
		j += sizeof(wchar_t);

		if (j >= tsize) {
			*main_text = realloc(*main_text, j);
			if (main_text == NULL) {
				endwin();
				xfprintf(strerror(errno), __LINE__);
			}
		}
	}

	(*main_text)[i] = '\0';

	fclose(stream);
}

static void lets_start_type()
{
	/* arrays of number bytes where texts started, needed to
	   read files (eng.typp / rus.typp) from the desired position */
	int ru_offsets[] = {
		0, 1979, 4088, 5729,
		7563, 9597, 11677, 13789,
		15999, 18145, 20059
	};

	int en_offsets[] = {
		0, 787, 1410, 2140,
		2825, 3654, 4385, 5140,
		5960, 6755, 7509
	};

	size_t tsize = 512;
	wchar_t *main_text;

	main_text = xmalloc(tsize * sizeof(wchar_t) + sizeof(wchar_t));

	(lang_highlight)
	    ? getrnd_text(&main_text, tsize, "eng.typp", en_offsets)
	    : getrnd_text(&main_text, tsize, "rus.typp", ru_offsets);

	tuiv.note_msg = "Let's start typing...";
	tuiv.text_win = newwin(20, 80, (LINES - 21) / 2, (COLS - 80) / 2);
	BOX_WBORDER_ZERO(tuiv.text_win);
	keypad(tuiv.text_win, TRUE);

	attron(COLOR_BOLD(1));
	mvprintw((LINES - 24) / 2, (COLS - strlen(tuiv.note_msg)) / 2,
		 "%s", tuiv.note_msg);
	attroff(COLOR_BOLD(1));
	FOOTER_MSGS;

	display_split_text(main_text, wcslen(main_text), tuiv.text_win);
	input_text(main_text, wcslen(main_text), tuiv.text_win);

	free(main_text);
	clear();
}

/* If the user changed the terminal, the program will exit
   The user should re-enter it to update the LINES / COLS values
   If do not use this handler, the interface will be not correct */
static void resize_term_handler()
{
	endwin();
	printf("%s: The terminal size has changed, \
please re-enter the program\n", program_name);
	exit(EXIT_SUCCESS);
}

int main(void)
{
	setlocale(LC_ALL, "");

	if (signal(SIGWINCH, resize_term_handler) == SIG_ERR)
		xfprintf("It is impossible to process the signal SIGWINCH",
			 __LINE__);

	/* ignore SIGINT signal (CTRL + C)
	   the program has a way out key 'F10 Quit' */
	if (signal(SIGINT, SIG_IGN) == SIG_ERR)
		xfprintf("It is impossible to process the signal SIGINT",
			 __LINE__);

	if (!initscr()) {
		error(0, 0, "Error initialising ncurses");
		exit(EXIT_FAILURE);
	}

	while (true) {
		noecho();
		curs_set(0);
		keypad(stdscr, TRUE);

		/* check 80 columns by 24 rows terminal size */
		if (LINES < 24 || COLS < 80) {
			endwin();
			xfprintf("Please, use terminal size not less 80x24",
				 __LINE__);
		}

		if (has_colors()) {
			start_color();
			init_pair(1, COLOR_CYAN, COLOR_BLACK);
			init_pair(2, COLOR_MAGENTA, COLOR_BLACK);
			init_pair(3, COLOR_GREEN, COLOR_BLACK);
			init_pair(4, COLOR_RED, COLOR_BLACK);
			init_pair(5, COLOR_BLUE, COLOR_CYAN);
			init_pair(6, COLOR_WHITE, COLOR_BLUE);
			init_pair(7, COLOR_YELLOW, COLOR_BLACK);
		}

		version_header_box();
		tuiv.sel_lang_title = "Please, select language for text:";
		attron(COLOR_BOLD(7));
		mvprintw(7, (COLS - strlen(tuiv.sel_lang_title)) / 2,
			 "%s", tuiv.sel_lang_title);
		attroff(COLOR_BOLD(7));

		attron(COLOR_PAIR(5));
		mvprintw(LINES - 2, 4, "%s", HELP_MSG);
		mvprintw(LINES - 2, (COLS - strlen(RES_MSG)) / 2, "%s",
			 RES_MSG);
		mvprintw(LINES - 2, (COLS - strlen(QUIT_MSG)) - 4, "%s",
			 QUIT_MSG);
		attroff(COLOR_PAIR(5));

		menu_of_two_elements(langs, lang_highlight);

		switch (getch()) {
		case KEY_UP:
			lang_highlight--;
			if (lang_highlight == -1)
				lang_highlight = 0;
			break;

		case KEY_DOWN:
			lang_highlight++;
			if (lang_highlight == 2)
				lang_highlight = 1;
			break;

		case KEY_F(1):
			clear();
			help_info();
			break;

		case KEY_F(5):
			clear();
			npm_menu();
			continue;

			case_EXIT;

		case ASCII_ENTER:
			clear();
			lets_start_type();
			continue;
		}
	}

	return EXIT_SUCCESS;
}
