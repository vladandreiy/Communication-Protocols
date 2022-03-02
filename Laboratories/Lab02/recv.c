#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "link_emulator/lib.h"

#define HOST "127.0.0.1"
#define PORT 10001


int main(int argc,char** argv){
  msg r;
  init(HOST,PORT);

  // Receive file size
  if (recv_message(&r)<0){
    perror("Receive message");
    return -1;
  }
  int size = atoi(r.payload);
  printf("[recv] Got msg with payload: <%d>\n", size);

  // Send ACK
  printf("[recv] Sending ACK\n");
  sprintf(r.payload,"%s", "ACK");
  r.len = strlen(r.payload) + 1;
  send_message(&r);


  // Create file
  int dest = open("recv_file", O_WRONLY | O_CREAT, 0777);
  if (dest < 0) {
    perror("Cannot create file");
    return -1;
  }

  // Receive file data
  int bytesWrote = 0;
  int copied;
  while(bytesWrote < size) {
    // Receive message
    if (recv_message(&r)<0){
      perror("Receive message");
      return -1;
    }
    printf("[recv] Received message with length: %d\n", r.len);

    // Write data to file
    bytesWrote += r.len;
    copied = write(dest, r.payload, r.len);
    if (copied < 0) {
      perror("Writing error");
      return -1;
    }
    printf("[recv] Wrote message to file\n");

    // Send ACK
    printf("[recv] Sending ACK\n");
    sprintf(r.payload,"%s", "ACK");
    r.len = strlen(r.payload) + 1;
    send_message(&r);
  }
  close(dest);
  return 0;
}
