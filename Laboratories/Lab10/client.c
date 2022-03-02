#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

int main(int argc, char *argv[])
{
    char *message;
    char *response;
    int sockfd;

        
    // Ex 1.1: GET dummy from main server

	char *host_ip = "34.118.48.238";
	int portno = 8080;
	int ip_type = AF_INET;
	int socket_type = SOCK_STREAM;
	int flag = 0;
	
	sockfd = open_connection(host_ip, 
					portno,
					ip_type,
					socket_type,
					flag);


	char *host = "34.118.48.238";
	char *url = "http://34.118.48.238:8080/api/v1/dummy";
	message = compute_get_request(host,
						url,  
						NULL,
                        NULL, 
                        0);

	printf("'n************ 1 ************\n");
	printf("Sending message to server:\n%s\n", message);
	send_to_server(sockfd, message);

	response = receive_from_server(sockfd);
	printf("Response from server:\n%s\n", response);

    // Ex 1.2: POST dummy and print response from main server
    char *content_type = "application/x-www-form-urlencoded";
    char *body_data = {"Faceultate=obsostistor&teme=grele"};
    int body_data_fields_count = 1;

    message = compute_post_request(host, 
    							url, 
    							content_type, 
    							&body_data,
    							body_data_fields_count, NULL, 0);

	printf("\n************ 2 ************\n");
	printf("Sending message to server:\n%s\n", message);
	send_to_server(sockfd, message);

	response = receive_from_server(sockfd);
	printf("Response from server:\n%s\n", response);

    // Ex 2: Login into main server
    char *url2 = "http://34.118.48.238:8080/api/v1/auth/login";
    char *body_data2 = {"username=student&password=student"};
    message = compute_post_request(host, 
    							url2, 
    							content_type, 
    							&body_data2,
    							body_data_fields_count, NULL, 0);

	printf("\n************ 3 ************\n");
	printf("Sending message to server:\n%s\n", message);
    send_to_server(sockfd, message);

    response = receive_from_server(sockfd);
	printf("Response from server:\n%s\n", response);

    // Ex 3: GET weather key from main server
    char *cookie = "connect.sid=s%3A-K8uYl3O66k9EvTyjJu9RTdyeRukm8Ol.%2FIVeEKJs3g9LH%2F1%2BURtX%2Bj63P4IpmoznTPfeUhKtLy0";

    char *url3 = "http://34.118.48.238:8080/api/v1/weather/key";

	message = compute_get_request(host,
						url3,  
						NULL,
                        &cookie, 
                        1);

	printf("\n************ 4 ************\n");
	printf("Sending message to server:\n%s\n", message);
    send_to_server(sockfd, message);

    response = receive_from_server(sockfd);
	printf("Response from server:\n%s\n", response);


    // Ex 4: GET weather data from OpenWeather API


    // Ex 5: POST weather data for verification to main server
    // Ex 6: Logout from main server
    char *url4 = "http://34.118.48.238:8080/api/v1/auth/logout";
    	message = compute_get_request(host,
						url4,  
						NULL,
                        NULL, 
                        0);

	printf("\n************ 5 ************\n");
	printf("Sending message to server:\n%s\n", message);
    send_to_server(sockfd, message);

    response = receive_from_server(sockfd);
	printf("Response from server:\n%s\n", response);

    // BONUS: make the main server return "Already logged in!"

    // free the allocated data at the end!

    return 0;
}
