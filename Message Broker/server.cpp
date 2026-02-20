/*
 * Protocoale de comunicatii
 * Laborator 7 - TCP
 * Echo Server
 * server.c
 */
#include <iostream>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <map>
#include <unordered_map>
#include <vector>
#include <iomanip>

#include "common.hpp"
#include "helpers.hpp"

#define MAX_CONNECTIONS 32

using namespace std;

/* map used for storing sockets as keys and ids as values */
unordered_map <int, string> socket_id_map;

/* map used for storing sockets as values and ids as keys */
unordered_map <string, int> id_socket_map;

/* map used for storing topics each tcp client has subscribed so far */
unordered_map <string, vector<string>> id_topics_map;

/* map used to store the clients for each topic */
unordered_map <string, vector <string>> topics_id_map;


/* function which prints a given map */

void print_map(map<string, int> id_socket_map) {
    map<string, int>::iterator it;

    for (it = id_socket_map.begin(); it != id_socket_map.end(); it++) {
        cout << it -> first << " " << it -> second << endl;
    }

    cout << endl;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("%s USAGE: <PORT>", argv[0]);
        return 1;
    }

    /* dezactivez buffering-ul */
    setvbuf(stdout, nullptr, _IONBF, BUFSIZ);

    /* obtin portul din linia de comanda */
    uint16_t port;
    int rc = sscanf(argv[1], "%hu", &port);
    DIE(rc != 1, "Given port is invalid");

    int max_connections = 32;

    /* aici imi definesc si initializez o conexiune tip TCP */

    const int tcp_listening_fd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(tcp_listening_fd < 0, "socket");

    sockaddr_in tcp_server_address;
    socklen_t socket_len = sizeof(sockaddr_in);


    memset(&tcp_server_address, 0, socket_len);
    tcp_server_address.sin_family = AF_INET;
    rc = inet_aton("127.0.0.1", &tcp_server_address.sin_addr);
    tcp_server_address.sin_port = htons(port);
    DIE(rc < 0, "inet_pton");

    rc = bind(tcp_listening_fd, (struct sockaddr *)&tcp_server_address, sizeof(tcp_server_address));
    DIE(rc < 0, "bind");

    rc = listen(tcp_listening_fd, MAX_CONNECTIONS);
    DIE(rc < 0, "listen");

    /* dezactivez algoritmul lui Nagle pentru socketul tcp */

    const int enable = 1;
    rc = setsockopt(tcp_listening_fd, IPPROTO_TCP, 1, (const void*) &enable, sizeof(int));
    DIE(rc < 0, "nagle");

    /* aici construiesc un socket tip UDP si initializez adresa de retea */

    const int udp_listening_fd = socket(AF_INET, SOCK_DGRAM, 0);
    DIE(udp_listening_fd < 0, "socket");

    sockaddr_in udp_server_addr;
    socklen_t udp_socket_len = sizeof(sockaddr_in);

    rc = bind(udp_listening_fd, (struct sockaddr *)&tcp_server_address, sizeof(tcp_server_address));
    DIE(rc < 0, "bind");

    /* aici imi definesc un poll, cu care sa retin toti file descriptorii de citire */

    struct pollfd *pollfds = new pollfd[MAX_CONNECTIONS];

    pollfds[2].fd = STDIN_FILENO;
    pollfds[2].events = POLLIN;
    pollfds[0].fd = tcp_listening_fd;
    pollfds[0].events = POLLIN;
    pollfds[1].fd = udp_listening_fd;
    pollfds[1].events = POLLIN;

    int current_number_of_clients = 3, read_exit = 0;    /* read_exit ---> variabila utila ca sa iesim din bucla infinita */

    while (1) {
        int rc = poll(pollfds, current_number_of_clients, -1);
        DIE(rc < 0, "poll");

        for (int i = 0; i < current_number_of_clients; i++) {
            if (pollfds[i].revents & POLLIN) {

                /* daca primesc cerere de conectare din partea unui client TCP */

                if (pollfds[i].fd == tcp_listening_fd) {
                    struct sockaddr_in tcp_client_address;
                    socklen_t cli_len = sizeof(tcp_client_address);

                    memset(&tcp_client_address, 0, cli_len);

                    int newsockfd =
                            accept(tcp_listening_fd, (struct sockaddr *) &tcp_client_address, &cli_len);
                    DIE(newsockfd < 0, "accept");

                    /* trebuie sa obtinem id-ul clientului */
                    tcp_message *tcp_request = new tcp_message;
                    memset(tcp_request, 0, sizeof(tcp_message));

                    int bytes_received = recv_all(newsockfd, tcp_request, sizeof(*tcp_request));
                    DIE(bytes_received < 0, "recv_all");

                    /* daca id_ul asociat clientului se afla in structura id -> socket si valoarea este != -1*/
                    /**
                     * daca valoarea este -1, atunci clientul cu id-ul obtinut din linia de comanda a fost sters
                     * iar valoarea din map inlocuita cu -1
                    */
                    if (id_socket_map.find(tcp_request->data) != id_socket_map.end() && id_socket_map[tcp_request->data] != -1) {  /*gasim id-ul printre clientii salvati deja */
                       
                        /* inchidem socketul */
                        int rc = close(newsockfd);
                        DIE(rc < 0, "close");

                        /* afisam mesajul corespunzator existentei unui client cu acelasi id */
                        cout << "Client " << tcp_request->data << " already connected." << endl;
                    } else {

                        /* salvam noul socket in vectorul de descriptori de fisier */
                        pollfds[current_number_of_clients].fd = newsockfd;
                        pollfds[current_number_of_clients].events = POLLIN;
                        current_number_of_clients++;

                        if (current_number_of_clients > max_connections / 2) {
                            /* alocam spatiu in plus pentru clienti noi */
                            pollfds = new pollfd[max_connections * 2];
                            max_connections *= 2;
                            if (pollfds == NULL) {
                                break;
                            }
                        }
                        /* adaugam noul client in bazele de date ale serverului */

                        if (id_socket_map.find(tcp_request->data) != id_socket_map.end() &&
                                                     id_socket_map[tcp_request -> data] == -1) {
                            id_socket_map[tcp_request->data] = newsockfd;
                        } else {
                            id_socket_map.insert(make_pair(tcp_request->data, newsockfd));
                        }
                        socket_id_map.insert(make_pair(newsockfd, tcp_request->data));

                        /* afisam noul client din server */

                        cout << "New client "<< tcp_request -> data << " connected from " 
                            << inet_ntoa(tcp_client_address.sin_addr) << ":" <<  ntohs(tcp_client_address.sin_port) << endl;
                    }
                    break;
                }

                else if (pollfds[i].fd == STDIN_FILENO) {
                    char *user_message = (char *) malloc(MSG_MAXSIZE + 1);
                    scanf("%s", user_message);

                   /* verific daca mesajul este cel pentru iesire */
                    if (!strncmp(user_message, "exit", 4)) {

                        read_exit = 1;   /* semnalez serverului ca e timpul sa se inchida */
                        for (int j = 0; j < current_number_of_clients; j++) {
                            if (j != i) {
                                int rc = close(pollfds[j].fd);           /* inchid toti socketii */
                                DIE(rc < 0, "close");
                            }
                        }
                        break;
                    }
                } 
                else if (pollfds[i].fd == udp_listening_fd) {
                    
                    /* bufferul in care vrem sa primim informatia de la clientul udp */
                    char buffer[UDP_MESSAGE_MAXSIZE];

                    /* ne initializam adresa clientului udp */
                    memset(buffer, 0, sizeof(buffer));
                    sockaddr_in client_addr{};
                    socklen_t client_size = sizeof(sockaddr_in);
                    memset(&client_addr, 0, client_size);

                    ssize_t bytes_received = recvfrom(pollfds[i].fd, buffer, sizeof(buffer), 0,
                                     (sockaddr*) &client_addr, &udp_socket_len);

                    /* obtin ip-ul si portul clientului */
                    in_port_t port = client_addr.sin_port;
                    port = ntohs(port);
                    in_addr ip = client_addr.sin_addr;
                    char *ip_addr = inet_ntoa(ip);

                    /* pun capatul de string unde este necesar */

                    buffer[bytes_received] = '\0';
                    DIE(bytes_received < 0, "recvfrom");
                    tcp_message *new_tcp_message = new tcp_message;
                    memset(new_tcp_message, 0, sizeof(tcp_message));

                    /* construiesc noua structura de tcp */
                    new_tcp_message->len = strlen(buffer);
                    new_tcp_message->operation_type = NEW_UDP_PUBLICATION;

                    char topic_name[TOPIC_MAXSIZE];    /* numele topicului */
                    char payload[TOPIC_PAYLOAD];    /* payload-ul aferent topicului */
                    int type;                       /* tipul de date aferent topicului */
                    int current_socket;                 /* socketul catre care vrem sa trimitem pachetul tcp*/
                    float float_value;                  /* variabila ajutatoare pentru cazul cu TYPE_FLOAT */
                    string value_message;               /* string ajutator in care retinem valorile numerice din payload */
                    stringstream ss;                    /* stream in care sa putem opera cu setprecision(), pentru precizie */
                    stringstream port_ss;             /* stream cu care putem opera cu porturi */

                    memset(topic_name, 0, TOPIC_MAXSIZE);       /* initializez topicul cu valori nule */
                    memcpy(topic_name, (udp_request_connection*) buffer, TOPIC_MAXSIZE);        
                    topic_name[TOPIC_MAXSIZE] = '\0';                
                    type = buffer[TOPIC_MAXSIZE];                       /* imi iau type-ul de date */
                    memcpy(payload, (buffer + TOPIC_MAXSIZE + 1), TOPIC_PAYLOAD);         

                    char message[TCP_MAXSIZE];                      /* mesajul pe care il construim si trimitem catre client */
                    message[0] = '\0';                                          

                    /* prelucram datele in functie de ce tip de date avem in vizor */

                    strcat(message, ip_addr);
                    strcat(message, ":");

                    port_ss << port;
                    value_message = port_ss.str();

                    strcat(message, value_message.c_str());
                    strcat(message, " - ");
                    
                    switch(type) {
                        case TYPE_INT:                                      /* tip de valoare int */
                            uint8_t sign;
                            uint32_t value;

                            memcpy(&sign, payload, sizeof(uint8_t));      
                            memcpy(&value, payload + sizeof(uint8_t), sizeof(uint32_t));    

                            value = ntohl(value); 
                            strcat(message, topic_name);  
                        
                            if (sign == 1) {
                                strcat(message, " - INT - -");
                            } else {
                                strcat(message, " - INT - ");
                            }
                            
                            value_message = to_string(value);
                            strcat(message, value_message.c_str());
                            break;  

                        case TYPE_STRING:                           /* tip de valoare string */
                            strcat(message, topic_name);
                            strcat(message, " - STRING - ");
                            strcat(message, payload);
                            break;

                        case TYPE_FLOAT:                            /* tip de valoare float */
                            uint8_t sign_byte;
                            uint32_t concat_value;
                            uint8_t power;

                            /* imi salvez structura numarului real, cu mantisa si exponent */
                            memcpy(&sign_byte, payload, sizeof(uint8_t));

                            memcpy(&concat_value, payload + sizeof(uint8_t), sizeof(uint32_t));  /* mantisa */

                            memcpy(&power, payload + sizeof(uint8_t) + sizeof(uint32_t), sizeof(uint8_t));  /* exponent */

                            float_value = ntohl(concat_value);

                            /* impart valoarea la puterea data in structura numarului real */
                            for (int i = 0; i < power; i++) {
                                float_value /= 10;
                            }

                            /* e numar cu semn */

                            if (sign_byte == 1) {
                                float_value *= -1;
                            }

                            strcat(message, topic_name);
                            strcat(message, " - FLOAT - ");

                            ss << fixed << setprecision(power) << float_value;
                            value_message = ss.str();
                            strcat(message, value_message.c_str());
                            break;


                        case TYPE_REAL:                     /* tip de valoare reala*/
                            uint16_t short_value;

                            memcpy(&short_value, payload, sizeof(uint16_t));
                            short_value = ntohs(short_value); // Convertim din network byte order la host byte order.

                            strcat(message, topic_name);
                            strcat(message, " - SHORT_REAL - ");

                            ss << fixed << setprecision(2) << (float) short_value / 100.00;
                            value_message = ss.str();
                            strcat(message, value_message.c_str());
                            break;
                    }

                    strcpy(new_tcp_message->data, message);
                    new_tcp_message->len = strlen(message);

                    /* trimitem informatia catre clientii abonati la topicul respectiv */

                    if (topics_id_map.find(topic_name) != topics_id_map.end()) {
                        vector<string> clients = topics_id_map[topic_name];  /* clientii abonati la topicul curent */
                        vector<string> ::iterator it = clients.begin();

                        for (it = clients.begin(); it != clients.end(); it++) {
                            string client_id = *it;                 /* salvez id-ul clientului curent */
                            if (id_socket_map[client_id] > 4) {
                                current_socket = id_socket_map[client_id];      /* socketul catre care trimit date */
                                ssize_t bytes_sent = send_all(current_socket, new_tcp_message, sizeof(tcp_message));
                                DIE(bytes_sent < 0, "send_all");
                            }
                        }
                    }
                } else {
                    tcp_message *new_tcp_request = new tcp_message;
                    uint16_t tcp_connection_size = sizeof(tcp_message);
                    memset(new_tcp_request, 0, tcp_connection_size);
                    ssize_t bytes_received = recv_all(pollfds[i].fd, new_tcp_request, tcp_connection_size);
                    DIE(bytes_received < 0, "recv_all");

                    /* incepem sa construim nou mesaj pe care il vom transmite clientului */

                    tcp_message *response = new tcp_message;
                    ssize_t response_size = sizeof(tcp_message);
                    memset(response, 0, response_size);

                    char *buffer = new char[TOPIC_MAXSIZE];
                    char *topic = new char[TOPIC_MAXSIZE];
                    strcpy(buffer, new_tcp_request->data);

                    /* in functie de situatie salvam topicul sau il stergem din memorie */
                    if (new_tcp_request->operation_type == OPERATION_SUBSCRIBE) {
                        strcpy(topic, buffer + strlen("subscribe") + 1);
                    } else if (new_tcp_request -> operation_type == OPERATION_UNSUBSCRIBE) {
                        strcpy(topic, buffer + strlen("unsubscribe") + 1);
                    } else {
                        delete[] topic;
                    }

                    /* clientul care executa actiunea */
                    string action_client_id;

                    bool subscribed = false; /* flag util in procesul de dezabonare */
                    bool subscribed_before = false; /* flag folosit pentru abonare*/

                    switch (new_tcp_request->operation_type) {
                        case OPERATION_SUBSCRIBE: /* se gestioneaza operatia de abonare de la client */
                            action_client_id = socket_id_map[pollfds[i].fd];        /* extragem id-ul clientului curent */

                            if (topics_id_map.find(topic) != topics_id_map.end()) {    /*verific daca topicul este salvat in sistem */
                                vector<string> topic_clients = topics_id_map[topic];
                                vector<string> :: iterator it;

                                for (it = topic_clients.begin(); it != topic_clients.end(); it++) {
                                    if (*it == action_client_id) {
                                        subscribed_before = true;
                                    }
                                }
                                if (it == topic_clients.end()) {
                                    topic_clients.push_back(action_client_id);
                                    topics_id_map[topic] = topic_clients;
                                }

                            } else {                                /* imi creez un vector nou cu id-uri de clienti*/
                                vector<string> new_topic_clients;
                                new_topic_clients.push_back(action_client_id);
                                topics_id_map.insert(make_pair(topic, new_topic_clients));
                            }

                            /* verific daca clientul este abonat la macar un topic */

                            if (id_topics_map.find(action_client_id) != id_topics_map.end()) {
                                vector <string> client_topics = id_topics_map[action_client_id];
                                client_topics.push_back(topic);
                                id_topics_map[action_client_id] = client_topics;
                            } else {                        /* creez un vector nou de topicuri */
                                vector<string> new_client_topics;
                                new_client_topics.push_back(action_client_id);
                                topics_id_map.insert(make_pair(topic, new_client_topics));
                            }

                            if (subscribed_before == false) {
                                strcat(response->data, "Subscribed to topic ");         /* construim mesajul */
                                strcat(response->data, topic);
                                response->operation_type = OPERATION_SUBSCRIBE;         /* setam flagul tipului de operatie */
                            }

                            break;

                        case OPERATION_UNSUBSCRIBE:             /* gestionez operatia de unsubscribe la topic */
                            action_client_id = socket_id_map[pollfds[i].fd];

                            /* verific daca acest topic a fost salvat anterior */

                            if (topics_id_map.find(topic) != topics_id_map.end()) {
                                vector<string> topic_clients = topics_id_map[topic];
                                vector<string> :: iterator it;

                                for (it = topic_clients.begin(); it != topic_clients.end(); it++) {
                                    if (*it == action_client_id) {
                                        subscribed = true;
                                        break;
                                    }
                                }
                                if (it != topic_clients.end()) {
                                    topic_clients.erase(it);
                                    topics_id_map[topic] = topic_clients;
                                }
                            } 

                            /* verific daca clientul a mai avut topicuri urmarite */

                            if (id_topics_map.find(action_client_id) != id_topics_map.end()) {
                                vector<string> client_topics = id_topics_map[action_client_id];
                                vector<string> :: iterator it;

                                for (it = client_topics.begin(); it != client_topics.end(); it++) {
                                    if (*it == topic) {
                                        subscribed = true;
                                        break;
                                    }
                                }
                                if (it != client_topics.end()) {
                                    client_topics.erase(it);
                                    id_topics_map[action_client_id] = client_topics;
                                }
                            }

                            if (subscribed) {
                                strcat(response->data, "Unsubscribed from topic ");         /* construiesc mesajul */
                                strcat(response->data, topic);
                                response->operation_type = OPERATION_UNSUBSCRIBE;           /* setez flagul tipului de operatie */
                            }
                            break;

                        case OPERATION_EXIT:
                            response->operation_type = OPERATION_EXIT;
                            break;

                        case OPERATION_CONNECTION:
                            response->operation_type = OPERATION_CONNECTION;
                            break;
                    }
                    if (response->operation_type != OPERATION_EXIT && response->operation_type != 0) {
                        ssize_t bytes_sent = send_all(pollfds[i].fd, response, sizeof(tcp_message));
                        DIE(bytes_sent < 0, "send_all");        
                    }
                    
                    if (response->operation_type == OPERATION_EXIT || bytes_received == 0) {
                        int pollfd_to_delete = pollfds[i].fd;
                        close(pollfd_to_delete);

                        for (int j = i; j < current_number_of_clients - 1; j++) {
                            pollfds[i] = pollfds[i+1];
                        }
                        
                        current_number_of_clients --;
                       
                        /* inchidem socketul clientului pe care il inchidem */

                        cout << "Client " << socket_id_map[pollfds[i].fd] << " disconnected." << endl;
                       
                        id_socket_map[socket_id_map[pollfds[i].fd]] = -1;
                        socket_id_map.erase(pollfds[i].fd);
                    }
                }

            }
        }

        if (read_exit == 1) {
            break;
        }
    }
}
