#include "headers.h"

char exit_from_server[MAX_TOPICS] = "exit";
char subscribe[MAX_TOPICS] = "subscribe";
char unsubscribe[MAX_TOPICS] = "unsubscribe";

void print_udp_message(udp_message msg) {
     printf(message_format, msg.ip_client_udp, msg.port_client_udp, 
            msg.msg.topic);
      
      switch (msg.msg.type) {
        case 0:{
          printf("INT - ");
          int8_t data_int8 = *((int8_t *)msg.msg.payload);
          uint32_t data_int32 = ntohl(*((uint32_t *)(msg.msg.payload + 1)));
          if(data_int8 == 0)
            printf("%d\n", data_int32);
          else
            printf("-%d\n", data_int32);
          break;
        }
        case 1: {
          printf("SHORT_REAL - ");
          uint16_t data = ntohs(*((uint16_t *)msg.msg.payload));
          printf("%.2f\n", (float)data / 100);
          break;
        }
        case 2: {
          printf("FLOAT - ");
          int8_t data_int8 = *((int8_t *)msg.msg.payload);
          uint32_t data_int32 = ntohl(*((uint32_t *)(msg.msg.payload + 1)));
          uint8_t data_int8_2 = *((uint8_t *)(msg.msg.payload + 5));
          if(data_int8 == 0)
            printf("%f\n", (float)(data_int32 / (pow(10, data_int8_2))));
          else
            printf("-%f\n", (float)(data_int32 / (pow(10, data_int8_2))));
          break;
        }
        case 3: {
          printf("STRING - ");
          printf("%s\n", msg.msg.payload);
          break;
        }
        default:
          break;
      }
}

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

        char buffer[sizeof(udp_message)];
        if(poll_fds[i].fd == STDIN_FILENO) {
          fgets(buffer, MAX_LEN, stdin);

          if(strncmp(buffer, "exit", 4) == 0) {
            message msg = create_message(exit_from_server, 0, id);
            send_all(tcpfd, &msg, sizeof(message));
            close(tcpfd);
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
          else if(strncmp(buffer, "unsubscribe", 11) == 0) {
            char *topic = strtok(buffer, " ");
            topic = strtok(NULL, " ");

            message msg = create_message(unsubscribe, 0, topic);
            send_all(tcpfd, &msg, sizeof(message));
            
            printf("Unsubscribed from topic.\n");
          }
        }
        else if(poll_fds[i].fd == tcpfd) {
           rc = recv_all(tcpfd, buffer, sizeof(udp_message));
            if(rc < 0) {
              fprintf(stderr, "Error receiving message from server\n");
              return 0;
            }
            else if(rc == 0) {
              // serverul s-a inchis
              return 0;
            }
            else {
              udp_message msg = *((udp_message *)buffer);
              print_udp_message(msg);
            }
        }
      }
    }
  }
  return 0;

}