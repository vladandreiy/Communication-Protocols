#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10000

char* checkXorSum(char* message) {
	char* xorSum = (char*)malloc(NCHECK * sizeof(char));
	memset(xorSum, 0, NCHECK);
	char substring[NCHECK];
	for (int i = 0; i < strlen(message) / (NCHECK - 1); i++) {
		memset(substring, 0, sizeof(substring));
		strncpy(substring, message + (NCHECK - 1) * i, NCHECK - 1);
		for (int j = 0; j < NCHECK - 1; j++) {
			xorSum[j] ^= substring[j];
		}
	}
	return xorSum;
}

int main(int argc, char *argv[])
{
	msg t;
	int i, res;

	printf("[SENDER] Starting.\n");
	init(HOST, PORT);

	char msg_buff[MSGSIZE - NCHECK];
	int msg_count = 1000000;
	int winSize = atoi(argv[1]) * 1000 / (MSGSIZE * 8);
	char *sum;

	for (i = 0; i < winSize; i++) {
		/* cleanup msg */
		memset(&t, 0, sizeof(msg));

		/* building message */
		sprintf(msg_buff, "%d", msg_count);
		sum = checkXorSum(msg_buff);
		sprintf(t.payload, "%s", sum);
		sprintf(t.payload + NCHECK, "%s", msg_buff);
		msg_count++;

		/* send msg */
		printf("[SENDER] Sending message: %s\n", t.payload + NCHECK);
		res = send_message(&t);
		if (res < 0) {
			perror("[SENDER] Send error. Exiting.\n");
			return -1;
		}
	}

	for (i = 0; i < COUNT - winSize; i++) {
		/* wait for ACK */
		res = recv_message(&t);
		if (res < 0) {
			perror("[SENDER] Receive error. Exiting.\n");
			return -1;
		}
		/* cleanup msg */
		memset(&t, 0, sizeof(msg));

		/* building message */
		sprintf(msg_buff, "%d", msg_count);
		sum = checkXorSum(msg_buff);
		sprintf(t.payload, "%s", sum);
		sprintf(t.payload + NCHECK, "%s", msg_buff);
		msg_count++;

		/* send msg */
		printf("[SENDER] Sending message: %s\n", t.payload + NCHECK);
		res = send_message(&t);
		if (res < 0) {
			perror("[SENDER] Send error. Exiting.\n");
			return -1;
		}
	}

	for (i = 0; i < winSize; i++) {
		/* wait for ACK */
		res = recv_message(&t);
		if (res < 0) {
			perror("[SENDER] Receive error. Exiting.\n");
			return -1;
		}
	}

	printf("[SENDER] Job done, all sent.\n");

	return 0;
}
