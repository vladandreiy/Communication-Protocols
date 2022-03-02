// Protocoale de comunicatii
// Laborator 9 - DNS
// dns.c

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

int usage(char* name)
{
	printf("Usage:\n\t%s -n <NAME>\n\t%s -a <IP>\n", name, name);
	return 1;
}

// Receives a name and prints IP addresses
void get_ip(char* name)
{
	int ret;
	struct addrinfo hints, *result, *p;

	// TODO: set hints
	hints.ai_flags = 0;
	hints.ai_family = AF_INET;
	hints.ai_socktype = 0;
	hints.ai_protocol = 0;
	hints.ai_addrlen = 0;
	hints.ai_addr = NULL;
	hints.ai_canonname = NULL;
	hints.ai_next = NULL;

	// TODO: get addresses
	ret = getaddrinfo(name, NULL, &hints, &result);

	if(ret < 0) {
		gai_strerror(ret);
		return;
	}

	// TODO: iterate through addresses and print them
	for(p = result; p != NULL; p = p->ai_next) {
		char temp[INET_ADDRSTRLEN];
		struct sockaddr_in *temp_struct = (struct sockaddr_in*)p->ai_addr;
		if(inet_ntop(AF_INET, &(temp_struct->sin_addr), temp, sizeof(temp)) == NULL) {
			perror("inet_ntop");
			return;

		}
		printf("%s\n", temp);
	}

	// TODO: free allocated data
	free(result);
}

// Receives an address and prints the associated name and service
void get_name(char* ip)
{
	int ret;
	struct sockaddr_in addr;
	char host[1024];
	char service[20];

	// TODO: fill in address data
	addr.sin_family = AF_INET;
	addr.sin_port = 53;
	if(inet_pton(AF_INET, ip, &(addr.sin_addr)) == NULL) {
		perror("inet_pton");
		return;
	}

	// TODO: get name and service
	ret = getnameinfo(((struct sockaddr*)&addr), sizeof(addr), host,
		sizeof(host), service, sizeof(service), 0);

	if(ret < 0) {
		gai_strerror(ret);
		return;
	}
	// TODO: print name and service
	printf("host: %s\tservice: %s\n", host, service);
}

int main(int argc, char **argv)
{
	if (argc < 3) {
		return usage(argv[0]);
	}

	if (strncmp(argv[1], "-n", 2) == 0) {
		get_ip(argv[2]);
	} else if (strncmp(argv[1], "-a", 2) == 0) {
		get_name(argv[2]);
	} else {
		return usage(argv[0]);
	}

	return 0;
}
