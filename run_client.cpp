#include "headers.h"

int run_client(int tcpfd) {
  struct pollfd poll_fds[2];

  poll_fds[0].fd = STDIN_FILENO;
  poll_fds[0].events = POLLIN;

  poll_fds[1].fd = tcpfd;
  poll_fds[1].events = POLLIN;
}