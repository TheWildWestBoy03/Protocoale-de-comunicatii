#include "common.hpp"

#include <cstdio>
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>

using namespace std;

int recv_all(int sockfd, void *buffer, size_t len) {

    size_t bytes_received = 0;
    size_t bytes_remaining = len;
    char *buff = (char *)buffer;
    while (bytes_remaining) {

        size_t bytes = recv(sockfd, buff, bytes_remaining, 0);

        if (bytes <= 0) {
            return bytes; // return the number of bytes received or a negative value in case of error
        }

        bytes_received += bytes;
        buff += bytes;
        bytes_remaining -= bytes;
    }

    return bytes_received;
}


int send_all(int sockfd, void *buffer, size_t len) {

    size_t bytes_sent = 0;
    size_t bytes_remaining = len;
    char *buff = (char *)buffer;

    while (bytes_remaining) {

        int bytes = send(sockfd, buff, bytes_remaining, 0);

        if (bytes < 0) {
            return bytes; // return the error code
        }

        bytes_sent += bytes;
        buff += bytes;
        bytes_remaining -= bytes;
    }

    return bytes_sent;
}