#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <cmath>
#include "helpers.h"

using namespace std;

void usage(char *file) {
    fprintf(stderr, "Usage: %s client_id server_address server_port\n", file);
    exit(0);
}

/*
 * Print message to stdout from a tcpMessage received as parameter
 */
void printMessage(tcp_message *tcpMessage) {
    char print[MAX_BUF];
    uint32_t length = 0;
    length += sprintf(print + length, "%s:%d - %s - ",
                      inet_ntoa(tcpMessage->udp_addr.sin_addr),
                      ntohs(tcpMessage->udp_addr.sin_port),
                      tcpMessage->message.topic);
    if (tcpMessage->message.type == 0) {
        length += sprintf(print + length, "INT - ");
        char sign = tcpMessage->message.data[0];
        uint8_t a = tcpMessage->message.data[1];
        uint8_t b = tcpMessage->message.data[2];
        uint8_t c = tcpMessage->message.data[3];
        uint8_t d = tcpMessage->message.data[4];
        uint32_t num = (a << 24) + (b << 16) + (c << 8) + d;
        if (sign == 1)
            num = -num;
        sprintf(print + length, "%d", num);
    }
    if (tcpMessage->message.type == 1) {
        length += sprintf(print + length, "SHORT_REAL - ");
        float num;
        uint8_t a = tcpMessage->message.data[0];
        uint8_t b = tcpMessage->message.data[1];
        num = (a << 8) + b;
        num /= 100;
        if (ceilf(num) == num)
            sprintf(print + length, "%.0f", num);
        else
            sprintf(print + length, "%.2f", num);
    }
    if (tcpMessage->message.type == 2) {
        length += sprintf(print + length, "FLOAT - ");
        char sign = tcpMessage->message.data[0];
        uint8_t a = tcpMessage->message.data[1];
        uint8_t b = tcpMessage->message.data[2];
        uint8_t c = tcpMessage->message.data[3];
        uint8_t d = tcpMessage->message.data[4];
        float num = (a << 24) + (b << 16) + (c << 8) + d;
        uint8_t e = tcpMessage->message.data[5];
        num = num * pow(10, -e);
        if (sign == 1)
            num = -num;
        if (ceilf(num) == num)
            sprintf(print + length, "%.0f", num);
        else
            sprintf(print + length, "%f", num);
    }
    if (tcpMessage->message.type == 3) {
        uint32_t string_length = tcpMessage->length -
                                 (sizeof(struct sockaddr_in) +
                                  sizeof(tcpMessage->message.topic) +
                                  sizeof(tcpMessage->message.type) +
                                  sizeof(tcpMessage->length)) + 1;
        length += sprintf(print + length, "STRING - ");
        snprintf(print + length, string_length, "%s",
                           tcpMessage->message.data);
    }
    printf("%s\n", print);
}

/*
 * Reads a message from stdin and sends it to the tcp server if it's a
 * subscribe or unsubscribe command
 */
int read_stdin(char *buffer, int sockfd) {
    long n;
    char *p;
    char topic[TOPIC_SIZE];
    memset(buffer, 0, MAX_BUF);
    n = read(STDIN, buffer, MAX_BUF - 1);
    DIE(n == 0, "Cannot read from stdin");
    // Subscribe command
    if (!strncmp(buffer, "subscribe", 9)) {
        int sf;
        sprintf(topic, "%s", buffer + 10);
        p = strtok(topic, " ");
        if (strlen(topic) > 50) {
            fprintf(stderr, "Topic is too long. (>50 chars)\n");
            return 1;
        }
        p = strtok(nullptr, " ");
        if (p != nullptr) {
            sf = atoi(p);
            if (sf != 0 && sf != 1) {
                fprintf(stderr, "Flag should be 0 or 1.\n");
                return 1;
            }
        } else {
            fprintf(stderr, "No flag present\n");
            return 1;
        }
        n = send(sockfd, buffer, strlen(buffer), 0);
        DIE(n < 0, "Couldn't send subscribe message");
        printf("Subscribed to topic.\n");
    }
    // Unsubscribe command
    else if (!strncmp(buffer, "unsubscribe", 11)) {
        sprintf(topic, "%s", buffer + 12);
        if (strlen(topic) > 50) {
            fprintf(stderr, "Topic is too long. (>50 chars)\n");
            return 1;
        }
        n = send(sockfd, buffer, strlen(buffer), 0);
        DIE(n < 0, "Couldn't send unsubscribe message");
        printf("Unsubscribed from topic.\n");
    }
    // Exit command
    else if (!strncmp(buffer, "exit", 4)) {
        // Close connection
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
        return 0;
    } else {
        fprintf(stderr, "Unknown command: %s\n", buffer);
    }
    return 1;
}

int main(int argc, char *argv[]) {
    // Disable buffering for stdout
    setvbuf(stdout, nullptr, _IONBF, BUFSIZ);
    // Verifiy command line arguments
    if (argc != 4) {
        usage(argv[0]);
    }
    long n, ret, sel;
    // Buffer used for receiving messages
    char buffer[MAX_BUF];
    // Buffer used for storing messages in order to have a complete message
    // if tcp window's size is lower than the length of the message
    char tcp_buffer[MAX_TCP_BUF];
    memset(tcp_buffer, 0, MAX_TCP_BUF);
    // Current length of a message in tcp_buffer
    uint32_t curr_length = 0;

    // Connect to the tcp server using the parameters received as
    // command line arguments
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "Socket error");
    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[3]));
    ret = inet_aton(argv[2], &serv_addr.sin_addr);
    DIE(ret == 0, "Conversion error");
    ret = connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    DIE(ret < 0, "Cannot connect to server");

    // File descriptors used for reading (from tcp server and stdin)
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(STDIN, &read_fds);
    FD_SET(sockfd, &read_fds);
    int fdmax = sockfd + 1;

    // Disabling Nagle's algorithm
    int yes = 1;
    int result = setsockopt(sockfd,
                            IPPROTO_TCP,
                            TCP_NODELAY,
                            (char *) &yes,
                            sizeof(int));
    DIE(result < 0, "Cannot disable Nagle's algorithm");

    // Sending message with client_id
    memset(buffer, 0, MAX_BUF);
    sprintf(buffer, "%s", argv[1]);
    n = send(sockfd, buffer, strlen(buffer), 0);
    DIE(n < 0, "Sending client_id error");
    memset(buffer, 0, MAX_BUF);

    while (true) {
        // Select from stdin or tcp server
        FD_SET(STDIN, &read_fds);
        FD_SET(sockfd, &read_fds);
        sel = select(fdmax, &read_fds, nullptr, nullptr, nullptr);
        DIE(sel < 0, "Error selecting reading file descriptor");

        // If client received data from server
        if (FD_ISSET(sockfd, &read_fds)) {
            memset(buffer, 0, MAX_BUF);
            n = recv(sockfd, buffer, sizeof(buffer), 0);
            DIE(n < 0, "Error receiving message");
            DIE(n == 0, "connection closed");
            // Copy data into tcp_buffer
            for (uint32_t i = curr_length, j = 0;
                 j < MAX_BUF && i < MAX_TCP_BUF;
                 i++, j++) {
                tcp_buffer[i] = buffer[j];
            }
            curr_length += n;
            // Get first message in tcp_buffer
            auto *tcpMessage = (tcp_message *) tcp_buffer;
            // If there is a message in tcpMessage and if there is a number
            // of bytes in the buffer bigger than the length of the message
            while (tcpMessage->length && curr_length >= tcpMessage->length) {
                // Print the message to stdout
                printMessage(tcpMessage);
                // Delete the message that was just printed
                uint32_t old_length = tcpMessage->length;
                for (uint32_t i = 0; i + old_length < MAX_TCP_BUF; i++) {
                    tcp_buffer[i] = tcp_buffer[i + old_length];
                }
                curr_length -= old_length;
                // Get next message
                tcpMessage = (tcp_message *) tcp_buffer;
            }
        }
        // If client received data from stdin
        else if (FD_ISSET(STDIN, &read_fds)) {
            ret = read_stdin(buffer, sockfd);
            if (ret == 0)
                break;
        }
    }
    return 0;
}
