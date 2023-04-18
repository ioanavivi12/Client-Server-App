#include "headers.h"

int run_all_clients(int tcpfd, int udpfd) {
    listen(tcpfd, MAX_CLIENTS);
    listen(udpfd, MAX_CLIENTS);
    struct pollfd fds[MAX_CLIENTS + 3];
    
    fds[0].fd = tcpfd;
    fds[0].events = POLLIN;

    fds[1].fd = udpfd;
    fds[1].events = POLLIN;
    
    fds[2].fd = STDIN_FILENO;
    fds[2].events = POLLIN;
    
}