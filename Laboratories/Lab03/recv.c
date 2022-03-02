#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10001

char* checkXorSum(char* message) {
	char* xorSum = (char*)malloc(NCHECK*sizeof(char));
	memset(xorSum, 0, NCHECK);
	char substring[NCHECK];
	for(int i = 0; i < strlen(message) / (NCHECK-1); i++) {
		memset(substring, 0, sizeof(substring));
		strncpy(substring, message + (NCHECK-1) * i, NCHECK - 1);
		for(int j = 0; j < NCHECK - 1; j++) {
			xorSum[j] ^= substring[j];
		}
	}
	return xorSum;
}

int main(void)
{
	msg r;
	int i, res;
	
	printf("[RECEIVER] Starting.\n");
	init(HOST, PORT);
	char received_sum[NCHECK];
	
	for (i = 0; i < COUNT; i++) {
		/* wait for message */
		res = recv_message(&r);
		if (res < 0) {
			perror("[RECEIVER] Receive error. Exiting.\n");
			return -1;
		}
		memset(&received_sum, 0, sizeof(received_sum));
		strncpy(received_sum, r.payload, NCHECK);
		if (strncmp(checkXorSum(r.payload + NCHECK), received_sum, NCHECK) != 0) {
			printf("[RECEIVER] Corrupted message. Exiting.\n");
			return -1;
		}
		printf("[RECEIVER] Received message: %s\n", r.payload + NCHECK);
		// printf("[RECEIVER] Received check sum: %s\n", received_sum);
		// printf("[RECEIVER] Check sum calculated locally: %s\n",
		// 		checkXorSum(r.payload + NCHECK));

		/* send dummy ACK */
		res = send_message(&r);
		if (res < 0) {
			perror("[RECEIVER] Send ACK error. Exiting.\n");
			return -1;
		}
	}

	printf("[RECEIVER] Finished receiving..\n");
	return 0;
}
