#include "headers.h"

char exit_from_server[50] = "exit";

int run_client(int tcpfd, char *id) {
  struct pollfd poll_fds[2];

  poll_fds[0].fd = STDIN_FILENO;
  poll_fds[0].events = POLLIN;

  poll_fds[1].fd = tcpfd;
  poll_fds[1].events = POLLIN;

  while(1) {
    int rc = poll(poll_fds, 2, 0);
    if(rc < 0) {
      fprintf(stderr, "Error in poll\n");
      return 0;
    }

    for(int i = 0; i < 2; i++) {
      if(poll_fds[i].revents & POLLIN) {
        char buffer[MAX_LEN];
        if(poll_fds[i].fd == STDIN_FILENO) {
          fgets(buffer, MAX_LEN, stdin);

          if(strncmp(buffer, "exit", 4) == 0) {
            message msg = create_message(exit_from_server, 0, id);
            send_all(tcpfd, &msg, sizeof(message));
            return 0;
          }
        }
        else if(poll_fds[i].fd == tcpfd) {
          int rc = recv(tcpfd, buffer, MAX_LEN, 0);
          if(rc < 0) {
            fprintf(stderr, "Error receiving message\n");
            return 0;
          }
          else if(rc == 0) {
            return 0;
          }
          printf("%s\n", buffer);
        }
      }
    }
  }
  return 0;

}