#include "headers.h"

using namespace std;

bool my_bind(int fd, uint16_t port, bool naggle = false) {
    // Completăm in serv_addr adresa serverului, familia de adrese si portul
    // pentru conectare
    struct sockaddr_in serv_addr;
    socklen_t socket_len = sizeof(struct sockaddr_in);

    // Facem adresa socket-ului reutilizabila, ca sa nu primim eroare in caz ca rulam de 2 ori rapid
    int enable = 1;
    int rc = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    if (rc < 0) {
        fprintf(stderr, "setsockopt(SO_REUSEADDR) failed\n");
        return false;
    }

    // Dezactivam algoritmul de Nagle
    if(naggle) {
        enable = 1;
        rc = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(int));
        if (rc < 0) {
            fprintf(stderr, "setsockopt(TCP_NODELAY) failed\n");
            return false;
        }
    }

    memset(&serv_addr, 0, socket_len);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    // Asociem adresa serverului cu socketul creat folosind bind
    rc = bind(fd, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if(rc < 0) {
        fprintf(stderr, "Error binding socket\n");
        return false;
    }
    return true;
}

int main(int argc, char *argv[]) {
    
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    // Parsam port-ul ca un numar
    uint16_t port;
    int rc = sscanf(argv[1], "%hu", &port);
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

    // Obtinem un socket UDP pentru a primi mesajele de la clienti
    int udpfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(udpfd < 0) {
        fprintf(stderr, "Error creating socket\n");
        return 1;
    }

    // Asociem adresa serverului cu socketul creat folosind bind
    if (!my_bind(tcpfd, port, true) || !my_bind(udpfd, port, false)) {
        return 1;
    }

    run_all_clients(tcpfd, udpfd);

    return 0;
}