/*
*  	Protocoale de comunicatii: 
*  	Laborator 6: UDP
*	client mini-server de backup fisiere
*/

#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <string.h>

#include "helpers.h"

void usage(char*file)
{
	fprintf(stderr,"Usage: %s ip_server port_server file\n",file);
	exit(0);
}

long int calculateFileSize(char *file_name) { 
	FILE* fp = fopen(file_name, "r");
	DIE(fp == NULL, "Cannot open file");
	fseek(fp, 0L, SEEK_END);
	long int res = ftell(fp);
	fclose(fp);
	return res; 
} 

/*
*	Utilizare: ./client ip_server port_server nume_fisier_trimis
*/
int main(int argc,char**argv)
{
	if (argc!=4)
		usage(argv[0]);
	
	/* Deschidere fisier pentru citire */
	int size = calculateFileSize(argv[3]);
	int fd;
	DIE((fd=open(argv[3],O_RDONLY))==-1,"open file");

	/*Deschidere socket*/
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	DIE(sockfd < 0, "socket error");

	/*Setare struct sockaddr_in pentru a specifica unde trimit datele*/
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[2]));
	inet_aton(argv[1], &addr.sin_addr);

	char buf[BUFLEN];

	uint32_t bytesRead = 0;
	uint32_t bytesReadBuffer = 0;
	int s;
	
	while(bytesRead < size) {
  	// Read data from file in buffer
	  	memset(buf, 0, BUFLEN);
	  	bytesReadBuffer = read(fd, buf, sizeof(buf));
	  	DIE(bytesReadBuffer < 0, "reading error");
	  	bytesRead += bytesReadBuffer;
		s = sendto(sockfd, buf, BUFLEN, 0, (struct sockaddr*) &addr, sizeof(addr));
		DIE(s<0, "transmission error");
		usleep(200);
	}
	sendto(sockfd, buf, 0, 0, (struct sockaddr*) &addr, sizeof(addr));
	/*Inchidere socket*/
	close(sockfd);
	
	/*Inchidere fisier*/
	close(fd);

	return 0;
}
