#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "link_emulator/lib.h"

#define HOST "127.0.0.1"
#define PORT 10000
#define BUFF_LEN 1400


long int calculateFileSize(char file_name[]) { 
	FILE* fp = fopen(file_name, "r");
	if (fp == NULL) {
		perror("Cannot open file");
		return -1;
	}
	fseek(fp, 0L, SEEK_END);
	long int res = ftell(fp);
	fclose(fp);

	return res; 
} 

int main(int argc,char** argv){
	init(HOST,PORT);
	msg t;

	// Open file
	int file;
	file = open("dummy_file", O_RDONLY);
	if (file < 0) {
		perror("Cannot open file");
	return -1;
	}

  // Send file size
  int size = calculateFileSize("dummy_file");
  printf("[send] Sending file size...\n");
  sprintf(t.payload,"%d", size);
  t.len = strlen(t.payload)+1;
  send_message(&t);

  // Wait for ACK
  if (recv_message(&t)<0){
    perror("Receive error ...");
    return -1;
  }
  else {
  	printf("[send] Got reply with payload: %s\n", t.payload);
  }


  // Send file data
  int bytesRead = 0, bytesReadBuffer;
  char buff[BUFF_LEN];
  lseek(file, 0, SEEK_SET);

  while(bytesRead < size) {
  	// Read data from file in a buffer
  	memset(buff, 0, BUFF_LEN);
  	bytesReadBuffer = read(file, buff, sizeof(buff));
  	if (bytesReadBuffer < 0) {
  		perror("Reading error");
  		return -1;
  	}
  	bytesRead += bytesReadBuffer;

  	// Send current data buffer
  	sprintf(t.payload, "%s", buff);
  	t.len = strlen(t.payload);
  	printf("[send] Sending message with length: %d\n", t.len);
  	send_message(&t);

  	// Wait for ACK
  	if (recv_message(&t)<0){
    	perror("Receive error ...");
    	return -1;
  	}
		else {
    	printf("[send] Got ACK from Receiver\n");
  	}
  }
  close(file);

  return 0;
}
