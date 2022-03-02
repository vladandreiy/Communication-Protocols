#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

#define HEADER_TERMINATOR_SIZE (sizeof(HEADER_TERMINATOR) - 1)
#define CONTENT_LENGTH "Content-Length: "
#define CONTENT_LENGTH_SIZE (sizeof(CONTENT_LENGTH) - 1)
#define CONTENT_TYPE "Content-Type: "

char *compute_get_request(char *host, char *url, char *query_params,
                          char **cookies, int cookies_count)
{
	char *message = calloc(BUFLEN, sizeof(char));
	char *line = calloc(LINELEN, sizeof(char));

	// Step 1: write the method name, URL, request params (if any) and protocol type
	if (query_params != NULL) {
		sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
	} else {
		sprintf(line, "GET %s HTTP/1.1", url);
	}

	compute_message(message, line);

	// Step 2: add the host
	memset(line, 0, LINELEN);
	if (host != NULL) {
		sprintf(line, "Host: %s", host);
	}
	compute_message(message, line);

	// Step 3 (optional): add headers and/or cookies, according to the protocol format
	memset(line, 0, LINELEN);
	uint32_t length = 0;
	if (cookies_count != 0) {
		length  += sprintf(line, "%s", "Cookie:");
		for (int i = 0; i < cookies_count; i++) {
			length += sprintf(line + length, " %s", cookies[i]);
			if (i != cookies_count - 1)
				length += sprintf(line + length, ";");
		}
		compute_message(message, line);
	}

	// Step 4: add final new line
	compute_message(message, "");
	return message;
}

char *compute_post_request(char *host, char *url, char* content_type, char **body_data,
                           int body_data_fields_count, char **cookies, int cookies_count)
{
	char *message = calloc(BUFLEN, sizeof(char));
	char *line = calloc(LINELEN, sizeof(char));
	char *body_data_buffer = calloc(LINELEN, sizeof(char));

	// Step 1: write the method name, URL and protocol type
	sprintf(line, "POST %s HTTP/1.1", url);
	compute_message(message, line);

	// Step 2: add the host
	memset(line, 0, LINELEN);
	if (host != NULL) {
		sprintf(line, "Host: %s", host);
	}
	compute_message(message, line);

	/* Step 3: add necessary headers (Content-Type and Content-Length are mandatory)
	        in order to write Content-Length you must first compute the message size
	*/

	memset(line, 0, LINELEN);
	if(content_type != NULL) {
		sprintf(line, "%s%s", CONTENT_TYPE, content_type);
		compute_message(message, line);
	}

	uint32_t content_length = 0;
	for(int i = 0; i < body_data_fields_count; i++) {
		sprintf(line, "%s", body_data[i]);
		content_length += strlen(line);
		compute_message(body_data_buffer, line);
	}
	sprintf(line, "%s%d", CONTENT_LENGTH, content_length);
	compute_message(message, line);

	// Step 4 (optional): add cookies
	memset(line, 0, LINELEN);
	if (cookies != NULL) {
		sprintf(line, "Cookie:");
		for (int i = 0; i < cookies_count; i++) {
			sprintf(line, " %s", cookies[i]);
			if (i != cookies_count - 1)
				sprintf(line, ";");
		}
		compute_message(message, line);
	}

	// Step 5: add new line at end of header
	compute_message(message, "");

	// Step 6: add the actual payload data
	memset(line, 0, LINELEN);
	compute_message(message, body_data_buffer);

	free(line);
	return message;
}
