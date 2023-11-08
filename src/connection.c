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
 * connection.c - server connection.
 *
*/

#include "main.h"

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

int connect_to_server()
{
	struct sockaddr_in servaddr;
	int sock;

	/* create client socket */
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		endwin();
		xfprintf(strerror(errno), __LINE__);
	}
	memset(&servaddr, 0, sizeof(servaddr));

	/* assign IP, PORT */
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("5.63.158.181");
	servaddr.sin_port = htons(8012);

	/* connect the client socket to server socket */
	if (connect(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
		mvprintw(0, 0, "Sorry, server not respond, please try later.");
		mvprintw(1, 0, "Press any key...");
		getch();
		return -1;
	}

	return sock;
}

void
send_res_to_server(int sock, int lang, char *nickname,
		   double npm, int err, int min, int sec)
{
	char user_val[64] = { 0 };
	sprintf(user_val, "%d %s %0.0f %d %.2d:%.2d\n", lang, nickname, npm,
		err, min, sec);

	if (send(sock, user_val, sizeof(user_val), 0) == -1) {
		endwin();
		xfprintf(strerror(errno), __LINE__);
	}

	close(sock);
}
