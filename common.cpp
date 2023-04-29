#include "headers.h"

udp_message create_udp_message(char *ip_client_udp, int port_client_udp, message msg) {
  udp_message udp_msg;
  strcpy(udp_msg.ip_client_udp, ip_client_udp);
  udp_msg.port_client_udp = port_client_udp;
  udp_msg.msg = msg;
  return udp_msg;
}

message create_message(char *topic, uint8_t data, char *payload) {
  message msg;
  strcpy(msg.topic, topic);
  msg.type = data;
  strcpy(msg.payload, payload);
  return msg;
}

int send_all(int sockfd, void *buffer, size_t len) {
  size_t bytes_sent = 0;
  size_t bytes_remaining = len;
  char *buff = (char *)buffer;
  
  while(bytes_remaining) {
    int rc = send(sockfd, buff, bytes_remaining, 0);
    if (rc <= 0) {
      return rc;
    }
    bytes_sent += rc;
    bytes_remaining -= rc;
    buff += rc;
  }

  return bytes_sent;
}

int recv_all(int sockfd, void *buffer, size_t len) {

  size_t bytes_received = 0;
  size_t bytes_remaining = len;
  char *buff = (char *)buffer;
  
  while(bytes_remaining) {
    int rc = recv(sockfd, buff, bytes_remaining, 0);
    if (rc <= 0) {
      return rc;
    }
    bytes_received += rc;
    bytes_remaining -= rc;
    buff += rc;
  }
  return bytes_received;
}