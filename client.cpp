#include "headers.h"

using namespace std;

int main(int argc, char *argv[]) {

    // Parsam port-ul ca un numar
    uint16_t port;
    int rc = sscanf(argv[2], "%hu", &port);
    if (rc != 1) {
        fprintf(stderr, "Invalid port number: %s\n", argv[2]);
        return 1;
    }    

    // Obtinem un socket TCP pentru receptionarea conexiunilor
    int tcpfd = socket(AF_INET, SOCK_STREAM, 0);
    if(tcpfd < 0) {
        fprintf(stderr, "Error creating socket\n");
        return 1;
    }

    struct sockaddr_in serv_addr;
    socklen_t socket_len = sizeof(struct sockaddr_in);

    memset(&serv_addr, 0, socket_len);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    rc = inet_pton(AF_INET, argv[1], &serv_addr.sin_addr.s_addr);
    if(rc <= 0) {
        fprintf(stderr, "Error converting IP address\n");
        return 1;
    }

    // Ne conectăm la server
    rc = connect(tcpfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if(rc < 0) {
        fprintf(stderr, "Error connecting to server\n");
        return 1;
    }
    return 0;
}