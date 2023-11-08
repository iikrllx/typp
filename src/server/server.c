/*
typp-server - accepts requests from the client (typp) and processes them.
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
 * server.c - all code here, run 'launch.sh' script for launch server.
 *
*/

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define CON_LIMIT 100

void xfprintf(char *str, int line)
{
	fprintf(stderr, "typp-server: %s:%d: %s\n", __FILE__, line, str);
	exit(EXIT_FAILURE);
}

void write_data_to_file(const char *file, const char *value)
{
	int fd;

	if ((fd = open(file, O_WRONLY | O_CREAT | O_APPEND, 0644)) == -1)
		xfprintf(strerror(errno), __LINE__);

	if (write(fd, value, strlen(value)) == -1)
		xfprintf(strerror(errno), __LINE__);
}

int check_client_msg_format(char client_msg[])
{
	int j = 0;

	char *token = strtok(client_msg, " ");

	while (token != NULL) {
		j++;

		if (j == 1) {
			if (!isdigit(token[0]) || strlen(token) != 1)
				return -1;
		}

		if (j == 2) {
			for (int i = 0; token[i] != 0; i++)
				if (!isalpha(token[i]))
					return -1;
		}

		if (j == 3 || j == 4) {
			for (int i = 0; token[i] != 0; i++)
				if (!isdigit(token[i]))
					return -1;
		}

		if (j == 5) {
			if (strchr(token, ':') == NULL)
				return -1;
		}

		token = strtok(NULL, " ");
	}

	return 0;
}

void *socket_thread(void *arg)
{
	pid_t child_sort;

	int nb, fd;
	int client_sock = *((int *)arg);
	int file_size = 0, modif_size = 0;

	char *pmsg, *tmppt, *all_results;

	/* files names for writing / reading results */
	char final_res[] = "-pm.final";
	char all_res[] = "-pm.all";

	/* message from client */
	char client_msg[64] = { 0 };

	if (recv(client_sock, client_msg, sizeof(client_msg), 0) == -1)
		xfprintf(strerror(errno), __LINE__);

	tmppt = client_msg;

	if (strlen(tmppt) == 3 && strcmp(++tmppt, "pm") == 0) {

		final_res[0] = client_msg[0];
		fd = open(final_res, O_RDONLY);

		if (fd == -1)
			xfprintf(strerror(errno), __LINE__);

		/* get size of the file */
		file_size = lseek(fd, 0, SEEK_END);
		lseek(fd, 0, SEEK_SET);

		modif_size = file_size * sizeof(char) + sizeof(char);
		all_results = malloc(modif_size);

		if (all_results == NULL)
			xfprintf(strerror(errno), __LINE__);

		/* fill buffer with results */
		if ((nb = read(fd, all_results, modif_size)) <= 0)
			xfprintf(strerror(errno), __LINE__);

		/* send all values to client */
		if (send(client_sock, all_results, nb, 0) != nb)
			xfprintf(strerror(errno), __LINE__);

		free(all_results);
		close(client_sock);
		close(fd);

	} else {
		/* check language index, ru - 0 / en - 1
		   prepare file name for writing */
		if (client_msg[0] != '1')
			all_res[0] = 'c';
		else
			all_res[0] = 'w';

		pmsg = strdup(client_msg);

		/* check message client format: "<n> Username <nnn> <nn> <nn>:<nn>" */
		if (check_client_msg_format(client_msg) != -1) {
			/* message without first number (language index) */
			write_data_to_file(all_res, pmsg + 2);

			switch (child_sort = fork()) {
			case -1:
				xfprintf(strerror(errno), __LINE__);
			case 0:
				execlp("sort-values.sh", "sort-values.sh",
				       all_res, NULL);
				xfprintf(strerror(errno), __LINE__);
			default:
				wait(NULL);
			}
		}

		free(pmsg);
	}

	pthread_exit(NULL);
}

int main(void)
{
	struct sockaddr_in servaddr;
	struct sockaddr_storage servstorage;
	int serv_sock, client_sock, i = 0, opt = 1;
	pthread_t tid[CON_LIMIT];
	socklen_t addr_size;

	if ((serv_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		xfprintf(strerror(errno), __LINE__);
	memset(&servaddr, 0, sizeof(servaddr));

	/* This is completely optional, but it helps in reuse of address and port.
	   Prevents error such as: "address already in use". */
	if (setsockopt
	    (serv_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
	     sizeof(opt)) == -1)
		xfprintf(strerror(errno), __LINE__);

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(8012);

	if ((bind(serv_sock, (struct sockaddr *)&servaddr, sizeof(servaddr))) !=
	    0)
		xfprintf(strerror(errno), __LINE__);

	if ((listen(serv_sock, CON_LIMIT)) != 0)
		xfprintf(strerror(errno), __LINE__);

	while (true) {
		/* Accept call creates a new socket for the incoming connection */
		addr_size = sizeof(servstorage);
		client_sock =
		    accept(serv_sock, (struct sockaddr *)&servstorage,
			   &addr_size);

		if (pthread_create
		    (&tid[i++], NULL, socket_thread, (void *)&client_sock) != 0)
			xfprintf(strerror(errno), __LINE__);

		if (i >= CON_LIMIT) {
			i = 0;
			while (i < CON_LIMIT)
				pthread_join(tid[i++], NULL);
			i = 0;
		}
	}

	return 0;
}
