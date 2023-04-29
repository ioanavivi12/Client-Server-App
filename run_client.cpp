#include "headers.h"

char exit_from_server[50] = "exit";
char subscribe[50] = "subscribe";
char unsubscribe[50] = "unsubscribe";

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
          else if(strncmp(buffer, "subscribe", 9) == 0) {
            char *topic = strtok(buffer, " ");
            topic = strtok(NULL, " ");
            int sf = atoi(strtok(NULL, " "));
            message msg = create_message(subscribe, sf, topic);
            send_all(tcpfd, &msg, sizeof(message));
            printf("Subscribed to topic.\n");
          }
        }
        else if(poll_fds[i].fd == tcpfd) {
          continue;
        }
      }
    }
  }
  return 0;

}