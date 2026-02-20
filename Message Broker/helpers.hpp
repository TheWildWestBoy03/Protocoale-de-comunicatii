#ifndef _HELPERS_H
#define _HELPERS_H 1

#include <stdio.h>
#include <stdlib.h>


#define OPERATION_SUBSCRIBE 1
#define OPERATION_UNSUBSCRIBE 2
#define OPERATION_EXIT 3
#define OPERATION_CONNECTION 4
#define NEW_UDP_PUBLICATION 5
#define SAME_CLIENT 6

/* sizes for our messages */

#define TCP_MAXSIZE 1601
#define TOPIC_MAXSIZE 50
#define TOPIC_PAYLOAD 1500
#define UDP_MESSAGE_MAXSIZE 1551

/* data type sizes */

#define TYPE_INT 0
#define TYPE_REAL 1
#define TYPE_FLOAT 2
#define TYPE_STRING 3

/*
 * Macro de verificare a erorilor
 * Exemplu:
 * 		int fd = open (file_name , O_RDONLY);
 * 		DIE( fd == -1, "open failed");
 */

#define DIE(assertion, call_description)                                       \
  do {                                                                         \
    if (assertion) {                                                           \
      fprintf(stderr, "(%s, %d): ", __FILE__, __LINE__);                       \
      perror(call_description);                                                \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
  } while (0)

#endif
