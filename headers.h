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
int run_client(int tcpfd);
int run_all_clients(int tcpfd, int udpfd);