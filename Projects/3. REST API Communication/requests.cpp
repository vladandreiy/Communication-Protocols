#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <vector>
#include <string>
#include <netinet/in.h>
#include "helpers.hpp"
#include "requests.hpp"

using namespace std;

#define HEADER_TERMINATOR_SIZE (sizeof(HEADER_TERMINATOR) - 1)
#define CONTENT_LENGTH "Content-Length: "
#define CONTENT_LENGTH_SIZE (sizeof(CONTENT_LENGTH) - 1)
#define CONTENT_TYPE "Content-Type: "

char *compute_get_request(char *url, char *query_params,
                          const vector<string> &headers,
                          const vector<string> &cookies) {
    char *message = static_cast<char *>(calloc(MAX_BUFF, sizeof(char)));
    char *line = static_cast<char *>(calloc(MAX_LINE, sizeof(char)));

    // Write the method name, URL, request params (if any) and protocol type
    if (query_params != nullptr) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }
    compute_message(message, line);

    // Add optional headers
    memset(line, 0, MAX_LINE);
    for (const auto & header : headers) {
        sprintf(line, "%s", header.c_str());
        compute_message(message, line);
    }

    // Add cookies
    memset(line, 0, MAX_LINE);
    uint32_t length = 0;
    if (!cookies.empty()) {
        length += sprintf(line, "%s", "Cookie:");
        for (size_t i = 0; i < cookies.size(); i++) {
            length += sprintf(line + length, " %s", cookies[i].c_str());
            if (i != cookies.size() - 1)
                length += sprintf(line + length, ";");
        }
        compute_message(message, line);
    }

    // Add final new line
    compute_message(message, "");
    return message;
}

char *compute_post_request(char *host, char *url, char *content_type,
                           const vector<string> &headers,
                           const vector<string> &body_data,
                           const vector<string> &cookies) {
    char *message = static_cast<char *>(calloc(MAX_BUFF, sizeof(char)));
    char *line = static_cast<char *>(calloc(MAX_LINE, sizeof(char)));
    char *body_data_buffer = static_cast<char *>(calloc(MAX_LINE, sizeof(char)));

    // Write the method name, URL and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);

    // Add the host
    memset(line, 0, MAX_LINE);
    if (host != nullptr) {
        sprintf(line, "Host: %s", host);
    }
    compute_message(message, line);

    // Add necessary headers (Content-Type and Content-Length are mandatory)
    // in order to write Content-Length you must first compute the message size
    memset(line, 0, MAX_LINE);
    if (content_type != nullptr) {
        sprintf(line, "%s%s", CONTENT_TYPE, content_type);
        compute_message(message, line);
    }
    uint32_t content_length = 0;
    for (const auto & body : body_data) {
        sprintf(line, "%s", body.c_str());
        content_length += strlen(line);
        compute_message(body_data_buffer, line);
    }
    sprintf(line, "%s%d", CONTENT_LENGTH, content_length);
    compute_message(message, line);

    // Add optional headers
    memset(line, 0, MAX_LINE);
    for (const auto & header : headers) {
        sprintf(line, "%s", header.c_str());
        compute_message(message, line);
    }

    // Add cookies
    memset(line, 0, MAX_LINE);
    if (!cookies.empty()) {
        sprintf(line, "Cookie:");
        for (size_t i = 0; i < cookies.size(); i++) {
            sprintf(line, " %s", cookies[i].c_str());
            if (i != cookies.size() - 1)
                sprintf(line, ";");
        }
        compute_message(message, line);
    }

    // Add new line at end of header
    compute_message(message, "");

    // Add the actual payload data
    memset(line, 0, MAX_LINE);
    compute_message(message, body_data_buffer);

    free(line);
    return message;
}

char *compute_delete_request(char *url, char *query_params,
                             const vector<string> &headers,
                             const vector<string> &cookies) {
    char *message = static_cast<char *>(calloc(MAX_BUFF, sizeof(char)));
    char *line = static_cast<char *>(calloc(MAX_LINE, sizeof(char)));

    // Write the method name, URL, request params (if any) and protocol type
    if (query_params != nullptr) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }
    compute_message(message, line);

    // Add optional headers
    memset(line, 0, MAX_LINE);
    for (const auto & header : headers) {
        sprintf(line, "%s", header.c_str());
        compute_message(message, line);
    }

    // Add cookies
    memset(line, 0, MAX_LINE);
    uint32_t length = 0;
    if (!cookies.empty()) {
        length += sprintf(line, "%s", "Cookie:");
        for (size_t i = 0; i < cookies.size(); i++) {
            length += sprintf(line + length, " %s", cookies[i].c_str());
            if (i != cookies.size() - 1)
                length += sprintf(line + length, ";");
        }
        compute_message(message, line);
    }

    // Add final new line
    compute_message(message, "");
    return message;
}