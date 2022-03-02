#ifndef _HELPERS_H
#define _HELPERS_H 1

#include <cstdio>
#include <cstdlib>
#include <utility>
#include <string>

#define MAX_WAITING     5
#define TOPIC_SIZE      50
#define DATA_SIZE       1500
#define MAX_BUF         1600
#define MAX_TCP_BUF     3200
#define STDIN           0
#define DIE(assertion, call_description)    \
    do {                                    \
        if (assertion) {                    \
            fprintf(stderr, "(%s, %d): ",    \
                    __FILE__, __LINE__);    \
            perror(call_description);        \
            exit(EXIT_FAILURE);                \
        }                                    \
    } while(0)

typedef struct udp_message {
    char topic[TOPIC_SIZE];
    uint8_t type;
    char data[DATA_SIZE];
} udp_message;

typedef struct tcp_message {
    uint32_t length{};
    struct sockaddr_in udp_addr;
    udp_message message{};
} tcp_message;

typedef struct id_sf {
    std::string id;
    uint8_t sf;

    id_sf(std::string id, uint8_t sf) : id(std::move(id)), sf(sf) {}
} id_sf;


#endif
