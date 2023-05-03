#pragma once
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
#include <bits/stdc++.h>
#include <netinet/tcp.h>
#include "run_server.h"


#define DIE(assertion, call_description)                                       \
  do {                                                                         \
    if (assertion) {                                                           \
      fprintf(stderr, "(%s, %d): ", __FILE__, __LINE__);                       \
      perror(call_description);                                                \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
  } while (0)

int run_client(int tcpfd, char *id);
int run_server(int tcpfd, int udpfd);
message create_message(char *topic, uint8_t data, char *payload);
int send_all(int sockfd, void *buffer, size_t len);
int recv_all(int sockfd, void *buffer, size_t len);
udp_message create_udp_message(char *ip_client_udp, int port_client_udp, message msg);
