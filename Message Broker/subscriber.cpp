/*
 * Protocoale de comunicatii
 * Laborator 7 - TCP si mulplixare
 * client.c
 */

#include <iostream>
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <iomanip>
#include "common.hpp"
#include "helpers.hpp"

#define POLL_CAPACITY 15

using namespace std;

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("./%s USAGE: <SUBSCRIBER_ID> <SERVER_IP_ADDRESS> <PORT>\n", argv[0]);
        return 1;
    }

    setvbuf(stdout, nullptr, _IONBF, BUFSIZ);

    /* Parsam port-ul ca un numar */
    uint16_t port;
    int rc = sscanf(argv[3], "%hu", &port);
    DIE(rc != 1, "Given port is invalid");

    /* Obtinem un socket TCP pentru conectarea la server */

    const int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "socket");

    /* Completăm in serv_addr adresa serverului, familia de adrese si portul
     pentru conectare */
    struct sockaddr_in serv_addr;
    socklen_t socket_len = sizeof(struct sockaddr_in);

    memset(&serv_addr, 0, socket_len);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    rc = inet_pton(AF_INET, argv[2], &serv_addr.sin_addr.s_addr);
    DIE(rc < 0, "inet_pton");

    // Ne conectăm la server
    rc = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in));
    DIE(rc < 0, "connect");

    /* dezactivez algoritmul lui Nagle pentru socketul tcp */

    const int enable = 1;
    rc = setsockopt(sockfd, IPPROTO_TCP, 1, (const void*) &enable, sizeof(int));
    DIE(rc < 0, "nagle");


    tcp_message *new_tcp_message = new tcp_message;
    size_t message_size = sizeof(tcp_message);
    memset(new_tcp_message, 0, message_size);

    /* ma asigur ca id-ul e suficient de mic */
    
    if (strlen(argv[1]) > 10) {
        cout << "ID de subscriber prea mare" << endl;
        return -1;
    }

    strcpy(new_tcp_message->data, argv[1]);
    new_tcp_message -> len = strlen(new_tcp_message->data);  /* atribuim lungimii din pachet cu lumgimea bufferului din pachet */

    ssize_t bytes_sent = send_all(sockfd, new_tcp_message, sizeof(*new_tcp_message));
    DIE(bytes_sent < 0, "send_all");

    /* declaram vectorul poll de file descriptori */
   
    struct pollfd poll_fds[POLL_CAPACITY];
    poll_fds[1].fd = STDIN_FILENO;
    poll_fds[1].events = POLLIN;

    poll_fds[0].fd = sockfd;
    poll_fds[0].events = POLLIN;

    while (1) {
        int rc = poll(poll_fds, POLL_CAPACITY, 2000);
        DIE(rc < 0, "poll");
        /* primim ceva de la server*/

        if (poll_fds[0].revents & POLLIN) {

            /* aici verificam ce tip de informatie primim de la server */

            tcp_message *new_message = new tcp_message;
            size_t message_size = sizeof(tcp_message);
            memset(new_message, 0, message_size);

            size_t bytes_received = recv_all(poll_fds[0].fd, new_message, message_size);
            DIE(bytes_received < 0, "recv_all");

            if (bytes_received == 0) {
                memset(&poll_fds[0], 0, sizeof(pollfd));
                memset(&poll_fds[1], 0, sizeof(pollfd));
                break;
            }

            /* aici analizam toate situatiile in care clientul poate primi informatii de la server */
            switch (new_message->operation_type) {
                case OPERATION_SUBSCRIBE : case OPERATION_UNSUBSCRIBE:      /* daca s-a abonat/dezabonat, primeste mesajul de confirmare */
                    cout << new_message -> data << endl;
                    break;

                case NEW_UDP_PUBLICATION:
                    cout << new_message -> data << endl;                    /* afiseaza topicul cu informatiile sale */
                    break;

                case SAME_CLIENT:
                    memset(&poll_fds[0], 0, sizeof(pollfd));                /* aici trebuie sa inchidem poll-urile si sa oprim fortat */
                    memset(&poll_fds[1], 0, sizeof(pollfd));
                    break;
            }
        }

        /* userul introduce comenzi din stdin */

        if (poll_fds[1].revents == POLLIN) {

            char *buffer = new char[MSG_MAXSIZE + 1];
            bool wrong_input = false; /* flag pentru corectitudinea inputului dat de user */
            if (fgets(buffer, MSG_MAXSIZE, stdin) == NULL) {
                perror("The string couldn't be read from input!\n");
                exit(EXIT_FAILURE);
            }
            
            tcp_message *new_tcp_message = new tcp_message;
            size_t message_size = sizeof(tcp_message);
            memset(new_tcp_message, 0, message_size);

            new_tcp_message -> len = strlen(buffer) - 1;
            buffer[strlen(buffer) - 1] = '\0';
            strcpy(new_tcp_message -> data, buffer);

            if (strstr(new_tcp_message -> data, "unsubscribe") != NULL) {
                new_tcp_message -> operation_type = OPERATION_UNSUBSCRIBE;
            } else if (strstr(new_tcp_message -> data, "subscribe") != NULL) {
                new_tcp_message -> operation_type = OPERATION_SUBSCRIBE;
            } else if (strstr(new_tcp_message -> data, "exit") != NULL) {
                new_tcp_message -> operation_type = OPERATION_EXIT;
                break;
            } else {
                wrong_input = true;
            }

            if (wrong_input != true) {
                ssize_t bytes_sent = send_all(poll_fds[0].fd, new_tcp_message, message_size);
                if (new_tcp_message->operation_type == OPERATION_EXIT) {
                    break;
                }
            } else {
                cout << "The input you typed is not good!"<< endl;
                wrong_input = false;
            }
        }
    }

    close(sockfd);
    return 0;
}
