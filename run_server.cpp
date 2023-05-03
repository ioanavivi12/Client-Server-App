#include "headers.h"

// pentru fiecare client pastram fd ul
std::vector<struct pollfd> fds;

// pentru ficare client pastram adresa si portul la care este conectat
std::vector<struct sockaddr_in> client_addr;

// pentru fiecare client mapam id ul cu fd ul
std::map<char *, int> client_id;
 // pentru fiecare topic pastram mesajele primite
std::map<std::pair <char*, int>, std::vector<char *>> topics;

/* mapam id ul fiecarui client cu topicurile la care este abonat 
(pentru fiecare topic pastram pozitia in vectorul de topicuri daca are sf = 1 si -1 daca are sf = 0)
*/
std::map<char *, std::vector<std::pair <char *, int>>> client_topics;

void init_all_vectors() {
    fds.clear();
    client_addr.clear();

    for(auto it = client_id.begin(); it != client_id.end(); it++) {
        free(it->first);
    }
    client_id.clear();

    // for(auto it = topics.begin(); it != topics.end(); it++) {
    //     free(it->first.first);
    // }
    topics.clear();

    // for(auto it = client_topics.begin(); it != client_topics.end(); it++) {
    //     free(it->first.first);
    // }
    client_topics.clear();
}

bool check_if_client_exists(char *id) {
    for(auto it = client_id.begin(); it != client_id.end(); it++) {
        if(strcmp(it->first, id) == 0 && it->second != -1) {
            return true;
        }
    }
    return false;
}

char *get_id(int fd) {
    for(auto it = client_id.begin(); it != client_id.end(); it++) {
        if(it->second == fd) {
            return it->first;
        }
    }
    return NULL;
}

void close_current_socket(int *i, int *num_clients) {
    close(fds[*i].fd);
    fds.erase(fds.begin() + *i);
    client_addr.erase(client_addr.begin() + *i);
    (*num_clients)--;
    (*i)--;
}

void add_new_message_to_topic(udp_message msg) {
    // cautam in lista de chei daca exista topicul
    bool found = false;
    char *payload = (char *)malloc(sizeof(udp_message));
    memcpy(payload, &msg, sizeof(udp_message));

    for(auto it = topics.begin(); it != topics.end(); it++) {
        if(strcmp(it->first.first, msg.msg.topic) == 0 ){
            found = true;

            it->second.push_back(payload);
            break;
        }
    }
    // daca nu exista topicul, il adaugam
    if(!found) {
        std::vector<char *> v;
        // alocam memorie pentru payload
        v.push_back(payload);
        topics.insert({{((udp_message *)payload)->msg.topic, msg.msg.type}, v});
    }
}

void actualizare_fd(char *id, int fd) {
    for(auto it = client_id.begin(); it != client_id.end(); it++) {
        if(strcmp(it->first, id) == 0) {
            it->second = fd;
            return;
        }
    }
    client_id.insert({id, fd});
}

int send_new_messages(udp_message msg) {
    // cautam toti clientii abonati la topicul respectiv
    for(auto it = client_topics.begin(); it != client_topics.end(); it++) {
        if(client_id[it->first] == -1) {
            continue;
        }

        for(auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
            // cautam topicul in lista de topicuri
            if(strcmp(it2->first, msg.msg.topic) == 0) {
                if(it2->second != -1) 
                    it2->second++;

                // daca am gasit topicul, trimitem mesajul
                int rc = send_all(client_id[it->first], &msg, sizeof(udp_message));
                DIE(rc < 0, "sendto");
            }
        }
    }
    return 0;
}

void add_new_client(int fd, struct sockaddr_in cli_addr, int *num_clients, bool is_tcp) {
    fds.push_back({fd, POLLIN, 0});
    client_addr.push_back(cli_addr);
    (*num_clients)++;
}

auto get_topic_iterator(char *topic) {
    for(auto it = topics.begin(); it != topics.end(); it++) {
        if(strcmp(it->first.first, topic) == 0) {
            return it;
        }
    }
    return topics.end();
}

void try_to_send_messages(int fd) {
    // cautam clientul dupa fd
    char *id = get_id(fd);
    if(id == NULL) {
        return;
    }

    // cautam topicul in lista de topicuri
    for(auto it2 = client_topics[id].begin(); it2 != client_topics[id].end(); it2++) {
        if(it2->second == -1) {
            // sf = 0
            continue;
        }
        auto topic = get_topic_iterator(it2->first);
        if(it2->second != (int)topic->second.size()) {
            // nu s-a trimis tot
            for(; it2->second < (int)topic->second.size(); it2->second++) {
                int rc = send_all(fd, topic->second[it2->second], sizeof(udp_message));
                DIE(rc < 0, "send");
            }
        }

    }
}

int run_server(int tcpfd, int udpfd) {
    int rc = listen(tcpfd, MAX_CLIENTS);
    if(rc < 0) {
        return -1;
    }

    init_all_vectors();
    
    fds.push_back({tcpfd, POLLIN, 0});
    fds.push_back({udpfd, POLLIN, 0});
    fds.push_back({STDIN_FILENO, POLLIN, 0});

    client_addr.push_back({});
    client_addr.push_back({});
    client_addr.push_back({});

    int num_clients = 3;

    while(1) {
        rc = poll(fds.data(), num_clients, 0);
        DIE(rc < 0, "Error poll");

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
                    add_new_client(new_client, *(struct sockaddr_in *)&cli_addr, &num_clients, true);
                }
                else if(fds[i].fd == udpfd) {
                    // primim mesaj de la un client UDP
                    message msg;
                    memset(&msg, 0, sizeof(message));
                    struct sockaddr_in cli_addr;
                    socklen_t cli_len = sizeof(cli_addr);
                    int rc = recvfrom(udpfd, &msg, sizeof(message), 0, (struct sockaddr *)&cli_addr, &cli_len);
                    if(rc == 0) 
                        continue;

                    udp_message udp_msg;
                    memset(&udp_msg, 0, sizeof(udp_message));
                    strncpy(udp_msg.ip_client_udp, inet_ntoa(cli_addr.sin_addr), 16);
                    udp_msg.port_client_udp = ntohs(cli_addr.sin_port);
                    memcpy(&udp_msg.msg, &msg, sizeof(message));

                    // adaugam mesajul in lista de topicuri si il trimitem la toti clientii abonati la topicul respectiv
                    add_new_message_to_topic(udp_msg);
                    rc = send_new_messages(udp_msg);
                    if(rc < 0) {
                        fprintf(stderr, "Error sending message\n");
                        return 0;
                    }

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
                        
                        init_all_vectors();
                        num_clients = 0;
                        return 0;
                    }
                }
                else {
                    // primim mesaj de la un client TCP
                    message msg;
                    memset(&msg, 0, sizeof(message));

                    int rc = recv_all(fds[i].fd, &msg, sizeof(message));
                    if(rc < 0) {
                        fprintf(stderr, "Error receiving message\n");
                        return 0;
                    }
                    else if(rc == 0) {
                        client_id[get_id(fds[i].fd)] = -1;
                        close_current_socket(&i, &num_clients);
                        continue;
                    }

                    if(strncmp(msg.topic, "connect", 7) == 0) {
                        bool found = check_if_client_exists(msg.payload);
                        if(!found) {
                            printf(connect_format, msg.payload, inet_ntoa(client_addr[i].sin_addr), 
                                                                ntohs(client_addr[i].sin_port));
                            // adaugam id ul clientului si socketul la care este conectat
                            char *id = (char *)malloc(strlen(msg.payload) + 1);
                            if(id == NULL) {
                                fprintf(stderr, "Error allocating memory\n");
                                return 0;
                            }
                            strcpy(id, msg.payload);
                            actualizare_fd(id, fds[i].fd);
                            try_to_send_messages(fds[i].fd);
                        }
                        else {
                            printf(allready_connected_format, msg.payload);
                            close_current_socket(&i, &num_clients);
                            continue;
                        }
                    }
                    else if(strncmp(msg.topic, "exit", 4) == 0) {
                        printf(exit_format, msg.payload);
                    }
                    else if(strncmp(msg.topic, "subscribe", 9) == 0) {
                        int8_t sf = msg.type;
                        char *topic = (char *)malloc(strlen(msg.payload) + 1);
                        if(topic == NULL) {
                            fprintf(stderr, "Error allocating memory\n");
                            return 0;
                        }
                        strcpy(topic, msg.payload);
                        // adaugam clientului topicul la care este abonat
                        if(sf == 1) { 
                            if(get_topic_iterator(topic) != topics.end()) 
                                client_topics[get_id(fds[i].fd)].push_back({topic, get_topic_iterator(topic)->second.size()});
                            else 
                                client_topics[get_id(fds[i].fd)].push_back({topic, 0});
                        }
                        else {
                           
                            client_topics[get_id(fds[i].fd)].push_back({topic, -1});
                        }
                    }
                    else if(strncmp(msg.topic, "unsubscribe", 11) == 0){
                        char *topic = msg.payload;
                        // stergem topicul la care este abonat clientul
                        for(auto it = client_topics[get_id(fds[i].fd)].begin(); it != client_topics[get_id(fds[i].fd)].end(); it++) {
                            if(strcmp(it->first, topic) == 0) {
                                client_topics[get_id(fds[i].fd)].erase(it);
                                break;
                            }
                        }
                    }
                }
            }

        }
    }
    
}