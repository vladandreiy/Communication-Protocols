#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <sstream>
#include <iostream>
#include "helpers.hpp"
#include "requests.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;
using namespace std;

vector<string> get_cookies(char *message) {
    stringstream stringStream(message);
    string line;
    vector<string> cookies;
    while (getline(stringStream, line)) {
        if (!strncmp(line.c_str(), "Set-Cookie:", 11)) {
            string cookie = line.substr(12, line.find(';') - 12);
            cookies.emplace_back(cookie);
            break;
        }
    }
    return cookies;
}

string get_token(char *message) {
    stringstream stringStream(message);
    string line;
    string token;
    while (getline(stringStream, line)) {
        if (!strncmp(line.c_str(), "\r", 1)) {
            getline(stringStream, line);
            json j = json::parse(line);
            token = j["token"].dump();
            token.erase(0, 1);
            token.erase(token.size() - 1);
            break;
        }
    }
    return token;
}

int http_error_code(char *message) {
    string error;
    if (!strncmp(message, "HTTP/1.1", 8)) {
        stringstream stringStream(message + 9);
        getline(stringStream, error);
        if (error[0] == '4')
            fprintf(stderr, "Client Error: %s\n", error.c_str());
        else if (error[0] == '5')
            fprintf(stderr, "Server Error: %s\n", error.c_str());
        if (error[0] == '4' || error[0] == '5') {
            json j = json::parse(basic_extract_json_response(message));
            fprintf(stderr, "%s\n", j.dump(4).c_str());
            return -1;
        }
    }
    if (!error.empty() &&
        (error[0] == '1' || error[0] == '2' || error[0] == '3'))
        return 0;
    else
        return -1;
}

int main() {
    setvbuf(stdout, nullptr, _IONBF, BUFSIZ);
    char *HOST_IP = (char *) "34.118.48.238";
    char *HOST = (char *) "34.118.48.238:8080";
    char *URL_REGISTER = (char *) "http://34.118.48.238:8080/api/v1/tema/auth/register";
    char *URL_LOGIN = (char *) "http://34.118.48.238:8080/api/v1/tema/auth/login";
    char *URL_LIBRARY = (char *) "http://34.118.48.238:8080/api/v1/tema/library/access";
    char *URL_BOOKS = (char *) "http://34.118.48.238:8080/api/v1/tema/library/books";
    char *URL_LOGOUT = (char *) "http://34.118.48.238:8080/api/v1/tema/auth/logout";
    char *CONTENT_JSON = (char *) "application/json";
    char *message, *response;
    char url[MAX_BUFF];

    // Connection info
    int portNo = 8080;
    int ipType = AF_INET;
    int socket_type = SOCK_STREAM;
    int flag = 0;
    int sockFd;

    // Vector to store cookies before creating a message, in this case
    // only the login session cookie
    vector<string> cookies;
    // Vector to store JWT token received from server
    string token;
    // Vector to store body data before creating a message
    vector<string> bodyData;
    // Store JWT token with its header (Authentication: Bearer)
    string authHeader;
    // Store optional headers, in this case only the JWT token
    vector<string> optionalHeaders;
    string command;
    string line;

    while (true) {
        // JSON variable to either store data that will be sent to server or
        // to print data received from server
        json j;
        // Open connection to server
        sockFd = open_connection(HOST_IP,
                                 portNo,
                                 ipType,
                                 socket_type,
                                 flag);
        getline(cin, command);
        if (command == "exit") {
            close_connection(sockFd);
            break;
        } else if (command == "register") {
            bodyData.clear();
            printf("username=");
            getline(cin, line);
            j["username"] = line;
            printf("password=");
            getline(cin, line);
            j["password"] = line;
            bodyData.emplace_back(j.dump());
            message = compute_post_request(HOST,
                                           URL_REGISTER,
                                           CONTENT_JSON,
                                           vector<string>(),
                                           bodyData,
                                           vector<string>());
            send_to_server(sockFd, message);
            response = receive_from_server(sockFd);
            if (!http_error_code(response))
                printf("Registration successful!\n");
        } else if (command == "login") {
            bodyData.clear();
            printf("username=");
            getline(cin, line);
            j["username"] = line;
            printf("password=");
            getline(cin, line);
            j["password"] = line;
            bodyData.emplace_back(j.dump());
            message = compute_post_request(HOST,
                                           URL_LOGIN,
                                           CONTENT_JSON,
                                           vector<string>(),
                                           bodyData,
                                           vector<string>());
            send_to_server(sockFd, message);
            response = receive_from_server(sockFd);
            if (!http_error_code(response))
                printf("Login successful!\n");
            cookies = get_cookies(response);
        } else if (command == "enter_library") {
            message = compute_get_request(
                    URL_LIBRARY,
                    nullptr,
                    vector<string>(),
                    cookies);
            send_to_server(sockFd, message);
            response = receive_from_server(sockFd);
            if (!http_error_code(response))
                printf("Entered library!\n");
            token = get_token(response);
            authHeader = "Authorization: Bearer " + token;
            optionalHeaders.clear();
            optionalHeaders.emplace_back(authHeader);
        } else if (command == "get_books") {
            message = compute_get_request(
                    URL_BOOKS,
                    CONTENT_JSON,
                    optionalHeaders,
                    vector<string>());
            send_to_server(sockFd, message);
            response = receive_from_server(sockFd);
            if (!http_error_code(response)) {
                if (basic_extract_json_response(response) == nullptr) {
                    printf("No books in the library\n");
                    continue;
                }
                string json_response(basic_extract_json_response(response));
                json_response = "[" + json_response;
                j = json::parse(json_response);
                printf("%s\n", j.dump(4).c_str());
            }
        } else if (command == "get_book") {
            printf("id=");
            getline(cin, line);
            int id = atoi(line.c_str());
            memset(url, 0, 100);
            strcpy(url, URL_BOOKS);
            strcat(url, "/");
            strcat(url, to_string(id).c_str());
            message = compute_get_request(
                    url,
                    CONTENT_JSON,
                    optionalHeaders,
                    vector<string>());
            send_to_server(sockFd, message);
            response = receive_from_server(sockFd);
            if (!http_error_code(response)) {
                string json_response(basic_extract_json_response(response));
                json_response.pop_back();
                j = json::parse(json_response);
                printf("%s\n", j.dump(4).c_str());
            }
        } else if (command == "add_book") {
            bodyData.clear();
            printf("title=");
            getline(cin, line);
            j["title"] = line;
            printf("author=");
            getline(cin, line);
            j["author"] = line;
            printf("genre=");
            getline(cin, line);
            j["genre"] = line.c_str();
            printf("publisher=");
            getline(cin, line);
            j["publisher"] = line.c_str();
            printf("page_count=");
            getline(cin, line);
            j["page_count"] = atoi(line.c_str());
            bodyData.push_back(j.dump());
            message = compute_post_request(HOST,
                                           URL_BOOKS,
                                           CONTENT_JSON,
                                           optionalHeaders,
                                           bodyData,
                                           vector<string>());
            send_to_server(sockFd, message);
            response = receive_from_server(sockFd);
            if (!http_error_code(response)) {
                printf("Added book successfully!\n");
            }
        } else if (command == "delete_book") {
            printf("id=");
            getline(cin, line);
            memset(url, 0, 100);
            strcpy(url, URL_BOOKS);
            strcat(url, "/");
            int id = atoi(line.c_str());
            strcat(url, to_string(id).c_str());
            message = compute_delete_request(
                    url,
                    nullptr,
                    optionalHeaders,
                    vector<string>());
            send_to_server(sockFd, message);
            response = receive_from_server(sockFd);
            if (!http_error_code(response)) {
                printf("Deleted book successfully!\n");
            }
        } else if (command == "logout") {
            message = compute_get_request(
                    URL_LOGOUT,
                    CONTENT_JSON,
                    vector<string>(),
                    cookies);
            send_to_server(sockFd, message);
            response = receive_from_server(sockFd);
            if (!http_error_code(response)) {
                printf("Logged out!\n");
            }
        } else {
            fprintf(stderr, "Unknown command: %s\n", command.c_str());
        }
        close_connection(sockFd);
    }
    return 0;
}
