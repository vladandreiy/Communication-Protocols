#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "helpers.h"

#define STDIN 0

void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_address server_port\n", file);
	exit(0);
}

int main(int argc, char *argv[])
{
	int sockfd, n, ret;
	struct sockaddr_in serv_addr;
	char buffer[BUFLEN];

	if (argc < 3) {
		usage(argv[0]);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[2]));
	ret = inet_aton(argv[1], &serv_addr.sin_addr);
	DIE(ret == 0, "inet_aton");

	fd_set read_fds;	// multimea de citire folosita in select()
	int sel = sockfd + 1;

	ret = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "connect");

	FD_ZERO(&read_fds);
	FD_SET(STDIN, &read_fds);
	FD_SET(sockfd, &read_fds);

	int fdmax = sockfd +1;

	int i;

	while (1) {
		FD_SET(STDIN, &read_fds);
		FD_SET(sockfd, &read_fds);

		sel = select(fdmax, &read_fds, NULL, NULL, NULL);
		DIE(sel < 0, "select");

		if(FD_ISSET(sockfd, &read_fds)) {
			memset(buffer, 0, BUFLEN);
			n = recv(sockfd, buffer, sizeof(buffer), 0);
			printf ("Server: %s\n", buffer);
			DIE(n < 0, "recv");
		}
		else if(FD_ISSET(STDIN, &read_fds)) {
			memset(buffer, 0, BUFLEN);
			read(STDIN, buffer, BUFLEN - 1);
			n = send(sockfd, buffer, strlen(buffer), 0);
			DIE(n < 0, "send");
		}
	}

	close(sockfd);

	return 0;
}
