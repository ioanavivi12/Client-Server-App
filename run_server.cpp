#include "headers.h"

bool check_if_client_exists(std::map<char *, int> client_id, char *id, int fd) {
    for(auto it = client_id.begin(); it != client_id.end(); it++) {
        if(strcmp(it->first, id) == 0) {
            if(it->second == fd) {
                continue;
            }
            else
                return true;
        }
    }
    return false;
}

int run_all_clients(int tcpfd, int udpfd) {
    listen(tcpfd, MAX_CLIENTS);
    listen(udpfd, MAX_CLIENTS);
    std::vector<struct pollfd> fds;
    std::vector<struct sockaddr_in> client_addr;
    std::map<char *, int> client_id;
   
    // pentru fiecare topic pastram mesajele primite
    std::vector<std::vector<char *>> messages_from_topics;

    //mapam id ul fiecarui client cu un vector de 4 elemente (cele 4 topicuri)
    std::map<char *, std::vector<std::pair <int, int>>> client_topics;
    
    fds.push_back({tcpfd, POLLIN, 0});
    client_addr.push_back({});

    fds.push_back({udpfd, POLLIN, 0});
    client_addr.push_back({});

    fds.push_back({STDIN_FILENO, POLLIN, 0});
    client_addr.push_back({});

    int num_clients = 3, rc;

    while(1) {
        rc = poll(fds.data(), num_clients, 0);
        if (rc < 0) {
            fprintf(stderr, "Error in poll\n");
            return 0;
        }

        for(int i = 0; i < num_clients; i++) {
            if(fds[i].revents & POLLIN) {

                // verificam daca doreste sa se conecteze un client TCP nou
                if(fds[i].fd == tcpfd) {
                    struct sockaddr_in cli_addr;
                    socklen_t cli_len = sizeof(cli_addr);

                    int new_client = accept(tcpfd, (struct sockaddr *)&cli_addr, &cli_len);
                    if(new_client < 0) {
                        fprintf(stderr, "Error accepting new TCP client\n");
                        return 0;
                    }

                    fds.push_back({new_client, POLLIN, 0});
                    client_addr.push_back(*(struct sockaddr_in *)&cli_addr);
                    num_clients++;
                }
                // verificam daca doreste sa se conecteze un client UDP nou
                else if(fds[i].fd == udpfd) {
                    struct sockaddr_in cli_addr;
                    socklen_t cli_len = sizeof(cli_addr);

                    int new_client = accept(udpfd, (struct sockaddr *)&cli_addr, &cli_len);
                    if(new_client < 0) {
                        fprintf(stderr, "Error accepting new client\n");
                        return 0;
                    }

                    fds.push_back({new_client, POLLIN, 0});
                    client_addr.push_back(*(struct sockaddr_in *)&cli_addr);
                    num_clients++;
                }
                // verificam daca a primit un mesaj de la STDIN
                else if(fds[i].fd == STDIN_FILENO) {
                    char buffer[MAX_LEN];
                    fgets(buffer, MAX_LEN, stdin);
                    
                    if(strncmp(buffer, "exit", 4) == 0) {
                        // inchidem toate conexiunile
                        for(int j = 0; j < num_clients; j++) {
                            close(fds[j].fd);
                        }
                        fds.clear();
                        client_addr.clear();
                        return 0;
                    }
                }
                else {
                    message msg;
                    memset(&msg, 0, sizeof(message));

                    int rc = recv_all(fds[i].fd, &msg, sizeof(message));
                    if(rc < 0) {
                        fprintf(stderr, "Error receiving message\n");
                        return 0;
                    }
                    else if(rc == 0) {
                        // inchidem conexiunea
                        close(fds[i].fd);
                        // scoatem clientul din lista de clienti
                        fds.erase(fds.begin() + i);
                        client_addr.erase(client_addr.begin() + i); 
                        i--;
                        num_clients--;
                        continue;
                    }

                    if(strncmp(msg.topic, "connect", 7) == 0) {
                        bool found = check_if_client_exists(client_id, msg.payload, fds[i].fd);
                        if(!found) {
                            printf(connect_format, msg.payload, inet_ntoa(client_addr[i].sin_addr), 
                                                                ntohs(client_addr[i].sin_port));
                            client_id[msg.payload] = fds[i].fd;
                        }
                        else {
                            printf(allready_connected_format, msg.payload);
                            // inchidem conexiunea
                            close(fds[i].fd);
                            // scoatem clientul din lista de clienti
                            fds.erase(fds.begin() + i);
                            client_addr.erase(client_addr.begin() + i); 
                            i--;
                            num_clients--;
                            continue;
                        }
                    }
                    else if(strncmp(msg.topic, "exit", 4) == 0) 
                        printf(exit_format, msg.payload);
                    else if(strncmp(msg.topic, "subscribe", 9) == 0) {
                        // TODO: Trebuie sa iau caut in lista mea de topic in care ar exista unul cu mesajul din msg.payload
                        // si sa mapez clientului sf ul pentru topicul respectiv
                        int8_t sf = msg.type;
                        char *topic = msg.payload;

                    }
            
                }
            }

        }
    }
    
}