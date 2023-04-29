#include "headers.h"

using namespace std;

char connect_to_server[50] = "connect";

int main(int argc, char *argv[]) {

    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    char id[MAX_LEN];
    strncpy(id, argv[1], strlen(argv[1]) + 1);
    // Parsam port-ul ca un numar
    uint16_t port;
    int rc = sscanf(argv[3], "%hu", &port);
    if (rc != 1) {
        fprintf(stderr, "Invalid port number: %s\n", argv[3]);
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
    rc = inet_pton(AF_INET, argv[2], &serv_addr.sin_addr.s_addr);
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

    // Trimitem numele clientului la server
    //message msg = create_message(connect_to_server, 0, argv[1]);
    message msg = create_message(connect_to_server, 0, argv[1]);
    
    rc = send_all(tcpfd, &msg, sizeof(message));
    if(rc < 0) {
        fprintf(stderr, "Error sending to server\n");
        return 1;
    }

    run_client(tcpfd, argv[1]);

    close(tcpfd);
    return 0;
}