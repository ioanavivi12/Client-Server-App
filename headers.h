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

#define MAX_CLIENTS 100
#define MAX_LEN 1500
#define connect_format "New client %s connected from %s : %d\n"
#define exit_format "Client %s disconnected\n"
#define allready_connected_format "Client %s already connected\n"

/**
 * Structura pentru un mesaj primit de un server 
*/
typedef struct {
    char topic[50];
    uint8_t type;
    char payload[MAX_LEN];
} message;

/**
 * Structura pentru un mesaj trimis de server unui client TCP
*/
typedef struct {
    char ip_client_udp[50];
    int port_client_udp;
    message msg;
} udp_message;

int run_client(int tcpfd, char *id);
int run_all_clients(int tcpfd, int udpfd);
message create_message(char *topic, uint8_t data, char *payload);
int send_all(int sockfd, void *buffer, size_t len);
int recv_all(int sockfd, void *buffer, size_t len);
udp_message create_udp_message(char *ip_client_udp, int port_client_udp, message msg);
