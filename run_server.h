#include "headers.h"

#define connect_format "New client %s connected from %s : %d\n"
#define exit_format "Client %s disconnected.\n"
#define allready_connected_format "Client %s already connected.\n"
#define message_format "%s:%d - %s - "
#define MAX_CLIENTS 2000
#define MAX_LEN 1500
#define MAX_TOPICS 50


/**
 * Structura pentru un mesaj primit de la un client UIDP
*/
typedef struct {
    char topic[MAX_TOPICS];
    uint8_t type;
    char payload[MAX_LEN];
} message;



/**
 * Structura pentru un mesaj trimis de server unui client TCP
*/
typedef struct {
    char ip_client_udp[MAX_TOPICS];
    int port_client_udp;
    message msg;
} udp_message;

/*
* Eliberez memoria alocata pentru toate structurile
*/
void init_all_vectors();

/*
* Verific daca un client cu acel id este deja conectat la server;
*/
bool check_if_client_exists(char *id);

/**
 * Caut id ul unui client pe baza file descriptorului
*/
char *get_id(int fd);

/**
 * Inchid conexiunea cu un client aflat pe pozitia i in vectorul de file descriptori
*/
void close_current_socket(int *i, int *num_clients);

/**
 * Adaug un nou mesaj de tip udp_message in vectorul de mesaje al topicului respectiv
*/
void add_new_message_to_topic(udp_message msg);

/**
 * Actualizez file descriptorul unui client
*/
void actualizare_fd(char *id, int fd);


/**
 * Adaug un nou client 
*/
void add_new_client(int fd, struct sockaddr_in cli_addr, int *num_clients, bool is_tcp);

/**
 * Intoare un iterator catre topicul respectiv
*/
auto get_topic_iterator(char *topic);


/**
 * Verifica cati clienti are topicul inainte sa fie adaugat in vectorul de topicuri
*/
int has_subscribers(char *topic);

/**
 * Trimitem mesajul catre toti clientii abonati la topicul respectiv
*/
int send_new_messages(udp_message msg);


/*
 * Atunci cand un client se concteaza la server, ii trimit toate mesajele pe care le are de primit in timp ce a fost deconectat,
 * cele cu sf = 1
*/
void try_to_send_messages(int fd);