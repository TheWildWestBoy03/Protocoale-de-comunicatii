#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <vector>

#include "utils.hpp"
#include "requests.hpp"

#define BUFLEN 4096
#define LINELEN 1000


using namespace std;

char* post_request(const char *host, const char *url, const char *content_type, std::string body_data, std::vector<std::string> cookies,
                                                             int cookies_num, char *token) {
    char *message = new char[BUFLEN];
    char *request_line = new char[LINELEN];

    char *buffer = new char[LINELEN];

    memset(message, 0, BUFLEN);
    memset(request_line, 0, LINELEN);    
    memset(buffer, 0, LINELEN);

    sprintf(request_line, "POST %s HTTP/1.1", url);
    message = prepare_line(message, request_line);

    sprintf(request_line, "Host: %s", host);
    message = prepare_line(message, request_line);

    if (token != NULL) {
        if (strlen(token) != 0) {
            sprintf(request_line, "Authorization: Bearer %s", token);
            message = prepare_line(message, request_line);
        }
    }

    sprintf(request_line, "Connection: keep-alive");
    message = prepare_line(message, request_line);

    int len = body_data.size();

    sprintf(request_line, "Content-Length: %d", len);
    message = prepare_line(message, request_line);

    sprintf(request_line, "Content-Type: %s", content_type);
    message = prepare_line(message, request_line);

    if (cookies.size() != 0) {
        sprintf(request_line, "Cookie: ");
        for (int i = 0; i < cookies_num; i++) {
            strcat(request_line, cookies.at(i).c_str());
        }

        strcat(request_line, cookies.at(cookies_num - 1).c_str());
        message = prepare_line(message, request_line);
    }

    sprintf(request_line, "\r\n");
    strcat(request_line, body_data.c_str());
    message = prepare_line(message, request_line);

    delete[] request_line;
    delete[] buffer;

    return message;
}

char *compute_get_request(const char *host, const char *url, char *query_params,
                            char **cookies, int cookies_count, char *token)
{
    int i;
    char *message = (char *)calloc(BUFLEN, sizeof(char));
    char *line = (char *)calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    prepare_line(message, line);

    // Step 2: add the host

    sprintf(line, "Host: %s", host);
    prepare_line(message, line);

    if (token != NULL) {
        if (strlen(token) != 0) {
            sprintf(line, "Authorization: Bearer %s", token);
            message = prepare_line(message, line);
        }
    }


    // Step 3 (optional): add headers and/or cookies, according to the protocol format
    sprintf(line, "User-Agent: Mozilla/5.0");
    prepare_line(message, line);

    sprintf(line, "Connection: keep-alive");
    prepare_line(message, line);

    if (cookies != NULL) {
        sprintf(line, "Cookie: ");

        for(i = 0; i < cookies_count - 1; i++){
            sprintf(line, "%s%s; ", line, cookies[i]);
        }

        sprintf(line, "%s%s", line, cookies[cookies_count - 1]);
        prepare_line(message, line);
    }

    // Step 4: add final new line
    prepare_line(message, "");
    return message;
}

char *compute_delete_request(const char *host, const char *url, char *query_params,
                            char **cookies, int cookies_count, char *token) {
    int i = 0;
    char *message = (char *) calloc(BUFLEN, sizeof(char));
    char *line = (char *) calloc(BUFLEN, sizeof(char));

    if (query_params != NULL) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }

    prepare_line(message, line);

    sprintf(line, "Host: %s", host);
    prepare_line(message, line);

    if (token != NULL) {
        if (strlen(token) != 0) {
            sprintf(line, "Authorization: Bearer %s", token);
            message = prepare_line(message, line);
        }
    }

    sprintf(line, "User-Agent: Mozilla/5.0");
    prepare_line(message, line);

    sprintf(line, "Connection: keep-alive");
    prepare_line(message, line);


    if (cookies != NULL) {
        sprintf(line, "Cookie: ");

        for(i = 0; i < cookies_count - 1; i++){
            sprintf(line, "%s%s; ", line, cookies[i]);
        }

        sprintf(line, "%s%s", line, cookies[cookies_count - 1]);
        prepare_line(message, line);
    }

    prepare_line(message, "");
    return message;
}
