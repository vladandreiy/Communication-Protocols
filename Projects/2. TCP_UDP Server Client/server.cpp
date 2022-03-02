#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <map>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include "helpers.h"

using namespace std;

void usage(char *file) {
    fprintf(stderr, "Usage: %s server_port\n", file);
    exit(0);
}

/*
 * Find client id in first element of the pair in vector and return its index
 */
long findById(vector<pair<string, int>> v, const string &id) {
    for (auto it = v.begin(); it != v.end(); ++it) {
        if ((*it).first == id) {
            return it - v.begin();
        }
    }
    return -1;
}

/*
 * Find file descriptor in second element of the pair in vector and return its
 * index
 */
long findByFd(vector<pair<string, int>> v, int fd) {
    for (auto it = v.begin(); it != v.end(); ++it) {
        if ((*it).second == fd) {
            return it - v.begin();
        }
    }
    return -1;
}

/*
 * Find in the topic vector the client id and return its index
 */
long findTopicById(vector<id_sf> tcp_topic_ids, const string& id) {
    for (auto it = tcp_topic_ids.begin(); it != tcp_topic_ids.end(); ++it) {
        if (it->id == id)
            return it - tcp_topic_ids.begin();
    }
    return -1;
}

/*
 * Delete a client id from a given topic in the topics map
 */
void unsubscribeClient(map<string, vector<id_sf>> &tcp_topic_ids,
                       const string &topic,
                       const string& id) {
    int c = 0;
    for (auto it = tcp_topic_ids[topic].begin();
         it != tcp_topic_ids[topic].end(); ++it) {
        if (it->id == id) {
            c = 1;
            tcp_topic_ids[topic].erase(it);
            break;
        }
    }
    if (c == 0) {
        fprintf(stderr, "Client not subscribed to topic.\n");
    }
}

/*
 * Calculate the length of the message
 */
unsigned int messageSize(tcp_message tcpMessage) {
    unsigned int size = 0;
    size += sizeof(struct sockaddr_in) + sizeof(tcpMessage.message.topic) +
            sizeof(tcpMessage.message.type) + sizeof(tcpMessage.length);
    if (tcpMessage.message.type == 0)
        size += sizeof(char) + sizeof(uint32_t);
    else if (tcpMessage.message.type == 1)
        size += sizeof(uint16_t);
    else if (tcpMessage.message.type == 2)
        size += sizeof(char) + sizeof(uint32_t) + sizeof(uint8_t);
    else if (tcpMessage.message.type == 3) {
        unsigned int len = strlen(tcpMessage.message.data);
        if (len > DATA_SIZE)
            size += DATA_SIZE;
        else
            size += len;
    }
    return size;
}

int main(int argc, char *argv[]) {
    // Disable buffering for stdout
    setvbuf(stdout, nullptr, _IONBF, BUFSIZ);
    // Verify command line arguments
    if (argc != 2) {
        usage(argv[0]);
    }
    int newsockfd, i;

    // Buffer used for reading from TCP connections or stdin
    char buffer[MAX_BUF];
    // Buffer used for reading udp messages
    char udp_buf[MAX_BUF];
    struct sockaddr_in serv_addr{}, cli_addr{};
    long n, r, index;
    socklen_t clilen;

    // Set up tcp server using command line arguments
    int tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
    DIE(tcp_sock < 0, "Socket error");
    int portno = atoi(argv[1]);
    DIE(portno == 0, "Conversion error");
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    r = bind(tcp_sock, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
    DIE(r < 0, "TCP Binding error");
    r = listen(tcp_sock, MAX_WAITING);
    DIE(r < 0, "Listening error");

    // Vector which connects client id's with their sockets
    vector<pair<string, int>> tcp_id_sock;

    // Map which connects topics with client id's and set flags
    map<string, vector<id_sf>> tcp_topic_ids;

    // Store messages - tcp_message, vector with of ids
    vector<pair<tcp_message, vector<string>>> store_messages;
    int stored_messages = 0;

    // Setting up UDP
    int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    DIE(udp_sock < 0, "Socket error");
    struct sockaddr_in udp_addr{};
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_port = htons(portno);
    udp_addr.sin_addr.s_addr = INADDR_ANY;
    int b = bind(udp_sock, (struct sockaddr *) &udp_addr, sizeof(udp_addr));
    DIE(b < 0, "UDP Binding error");
    struct sockaddr_in udp_client{};
    socklen_t udp_socklen = sizeof(udp_client);

    // File descriptors used for reading (socket that listens for TCP
    // connections, stdin and socket for UDP)
    fd_set read_fds;
    fd_set tmp_fds;
    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);
    FD_SET(tcp_sock, &read_fds);
    FD_SET(STDIN, &read_fds);
    FD_SET(udp_sock, &read_fds);
    int fdmax = max(tcp_sock, udp_sock);

    while (true) {
        // Set file descriptors used for reading
        tmp_fds = read_fds;

        // Select from tcp clients, tcp listening socket, udp, stdin
        r = select(fdmax + 1, &tmp_fds, nullptr, nullptr, nullptr);
        DIE(r < 0, "Error selecting reading file descriptor");

        for (i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &tmp_fds)) {
                if (i == STDIN) {
                    memset(buffer, 0, MAX_BUF);
                    read(STDIN, buffer, MAX_BUF - 1);
                    // Exit command
                    if (!strncmp(buffer, "exit", 4)) {
                        for (const auto &tcp_client : tcp_id_sock) {
                            close(tcp_client.second);
                        }
                        close(tcp_sock);
                        close(udp_sock);
                        return 0;
                    } else {
                        fprintf(stderr, "Unknown command\n");
                    }
                } else if (i == udp_sock) {
                    // Receive message from UDP
                    memset(udp_buf, 0, MAX_BUF);
                    r = recvfrom(udp_sock, udp_buf, MAX_BUF, 0,
                                 (struct sockaddr *) &udp_client,
                                 &udp_socklen);
                    DIE(r < 0, "Error receiving from UDP");
                    auto *udp_mess = (udp_message *) udp_buf;
                    if (!tcp_topic_ids.count(udp_mess->topic)) {
                        tcp_topic_ids[udp_mess->topic] = {};
                        continue;
                    }
                    int already_stored = 0;
                    for (const auto &client_id : tcp_topic_ids[udp_mess->topic]) {
                        index = findById(tcp_id_sock, client_id.id);
                        int client_fd = tcp_id_sock[index].second;
                        // Construct TCP message
                        tcp_message tcpMessage;
                        tcpMessage.udp_addr = udp_client;
                        tcpMessage.message = *udp_mess;
                        tcpMessage.length = messageSize(tcpMessage);
                        // If client is not connected
                        if (client_fd == 0) {
                            // If client has set flag enabled
                            if (client_id.sf == 1) {
                                // Store message for later
                                if (already_stored == 0) {
                                    store_messages.emplace_back(tcpMessage,
                                                                vector<string>{
                                                                        client_id.id});
                                }
                                else {
                                    // Message doesn't need to be stored multiple times for different clients
                                    store_messages[stored_messages].second.push_back(
                                            client_id.id);
                                }
                                already_stored = 1;
                                stored_messages++;
                            }
                            continue;
                        }
                        // If client is connected, send message now
                        n = send(client_fd, &tcpMessage,
                                 tcpMessage.length, 0);
                        DIE(n < 0, "send");
                    }
                } else if (i == tcp_sock) {
                    // TCP listening is active, meaning a new client connected
                    clilen = sizeof(cli_addr);
                    newsockfd = accept(tcp_sock, (struct sockaddr *) &cli_addr,
                                       &clilen);
                    DIE(newsockfd < 0, "accept");

                    // Disable Nagle's algorithm
                    int yes = 1;
                    int result = setsockopt(newsockfd,
                                            IPPROTO_TCP,
                                            TCP_NODELAY,
                                            (char *) &yes,
                                            sizeof(int));
                    DIE(result < 0, "Cannot disable Nagle's algorithm");

                    // Get message containing client's ID
                    memset(buffer, 0, MAX_BUF);
                    n = recv(newsockfd, buffer, sizeof(buffer), 0);
                    DIE(n < 0, "recv");


                    index = findById(tcp_id_sock, buffer);
                    // If it's a new client
                    if (index == -1) {
                        tcp_id_sock.emplace_back(buffer, newsockfd);
                    } else {
                        // If client existed but is not connected
                        if (tcp_id_sock[index].second == 0) {
                            tcp_id_sock[index].second = newsockfd;
                        }
                        // If client is already connected
                        else {
                            printf("Client %s already connected.\n",
                                   buffer);
                            shutdown(newsockfd, SHUT_RDWR);
                            close(newsockfd);
                            continue;
                        }
                    }
                    // Add the new socket
                    FD_SET(newsockfd, &read_fds);
                    if (newsockfd > fdmax) {
                        fdmax = newsockfd;
                    }
                    printf("New client %s connected from %s:%d\n",
                           buffer,
                           inet_ntoa(cli_addr.sin_addr),
                           ntohs(cli_addr.sin_port));
                    // If there are any messages saved for client, send them now
                    for (auto it = store_messages.begin();
                         it != store_messages.end(); ++it) {
                        for (auto it2 = it->second.begin();
                             it2 != it->second.end(); ++it2) {
                            if (*it2 == buffer) {
                                n = send(newsockfd, &(it->first),
                                         it->first.length, 0);
                                DIE(n < 0, "send");
                                it->second.erase(it2--);
                            }
                        }
                        if (it->second.empty())
                            store_messages.erase(it--);
                    }

                } else {
                    // Receiving data from a TCP client
                    memset(buffer, 0, MAX_BUF);
                    n = recv(i, buffer, sizeof(buffer), 0);
                    DIE(n < 0, "recv");

                    if (n == 0) {
                        // Connection closed
                        printf("Client %s disconnected.\n",
                               tcp_id_sock[findByFd(tcp_id_sock,
                                                    i)].first.c_str());
                        tcp_id_sock[findByFd(tcp_id_sock, i)].second = 0;
                        shutdown(i, SHUT_RDWR);
                        close(i);
                        FD_CLR(i, &read_fds);
                    } else {
                        char topic[50];
                        char *p;
                        index = findByFd(tcp_id_sock, i);
                        int sf;
                        // Subscribe command
                        if (!strncmp(buffer, "subscribe", 9)) {
                            sprintf(topic, "%s", buffer + 10);
                            p = strtok(topic, " ");
                            strcpy(topic, p);
                            long m = findTopicById(tcp_topic_ids[topic],
                                                  tcp_id_sock[index].first);
                            if (m == -1) {
                                p = strtok(nullptr, " ");
                                sf = atoi(p);
                                tcp_topic_ids[topic].emplace_back(
                                        tcp_id_sock[index].first, sf);
                            } else {
                                fprintf(stderr,"Client already subscribed to topic.\n");
                            }
                        }
                        // Unsubscribe command
                        else if (!strncmp(buffer, "unsubscribe", 11)) {
                            sprintf(topic, "%s", buffer + 12);
                            p = strtok(topic, " \n");
                            unsubscribeClient(tcp_topic_ids, p,
                                              tcp_id_sock[index].first);
                        }
                    }
                }
            }
        }
    }
}
