#ifndef __COMMON_H__
#define __COMMON_H__

#include <stddef.h>
#include <stdint.h>
#include <netinet/in.h>

#include "helpers.hpp"
int send_all(int sockfd, void *buff, size_t len);
int recv_all(int sockfd, void *buff, size_t len);

/* Dimensiunea maxima a mesajului */
#define MSG_MAXSIZE 1024

struct udp_request_connection {
  char topic[TOPIC_MAXSIZE];
  int data_type;
  char payload[TOPIC_PAYLOAD];
};

struct tcp_message{
  uint16_t len; /* the payload length */
  uint16_t operation_type;  /* for checking if the user wants to subscribe, unsubscribe or just to exit */
  char data[TCP_MAXSIZE];  /* the client's payload */
};

#endif
