/*
*  	Protocoale de comunicatii: 
*  	Laborator 6: UDP
*	mini-server de backup fisiere
*/

#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>

#include "helpers.h"


void usage(char*file)
{
	fprintf(stderr,"Usage: %s server_port file\n",file);
	exit(0);
}

/*
*	Utilizare: ./server server_port nume_fisier
*/
int main(int argc,char**argv)
{
	if (argc!=3)
		usage(argv[0]);

	struct sockaddr_in my_sockaddr, from_station;
	char buf[BUFLEN];

	/* Deschidere fisier pentru scriere */
	int fd;
	DIE((fd=open(argv[2],O_WRONLY|O_CREAT|O_TRUNC,0644))==-1,"open file");

	/*Deschidere socket*/
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	DIE(sockfd < 0, "socket error");

	/*Setare struct sockaddr_in pentru a asculta pe portul respectiv */
	my_sockaddr.sin_family = AF_INET;
	my_sockaddr.sin_port = htons(atoi(argv[1]));
	my_sockaddr.sin_addr.s_addr = INADDR_ANY;

	/* Legare proprietati de socket */
	int b = bind(sockfd, (struct sockaddr*)&my_sockaddr, sizeof(my_sockaddr));
	DIE(b<0, "binding error");

	socklen_t socklen = sizeof(from_station);

	uint32_t wrote = 0;
	int r;
	for( ; ; ) {
		usleep(100);
		r = recvfrom(sockfd, buf, BUFLEN, 0, (struct sockaddr*) &from_station, &socklen);
		DIE(r<0, "receiving error");
		if(r==0)
			break;
		wrote = write(fd, buf, sizeof(buf));
	}

	/*Inchidere socket*/	
	close(sockfd);
	
	/*Inchidere fisier*/
	close(fd);

	return 0;
}
