# Tema2-PCom
## Realizat de Dabelea Ioana-Viviana, grupa 323CB
---
### Cuprins
1. [Introducere](#introducere)
2. [Structura proiectului](#structura-proiectului)
3. [Implementare Server](#implementare-server)
4. [Implementare Client](#implementare-client)
5. [Testare](#testare)
6. [Bibliografie](#bibliografie)

---
### Introducere
Proiectul implementeaza un server care primeste de la niste clienti UDP mesaje pentru un anumit topic si le trimite inapoi clientilor TCP abonati la acel topic. Serverul si clientii TCP sunt implementati in limbajul C++, iar clientii UDP in limbajul Python.

### Structura proiectului
Proiectul este structurat in mai multe fisiere sursa:
* `server.cpp` - implementarea serverului(crearea socketilor de UDP si TCP)
* `client.cpp` - implementarea clientului TCP
* `common.cpp` - implementarea functiilor folosite pentru a primi si a trimite mesaje 
* `run_server.cpp` - implementarea functionalitatii serverului (primirea de mesaje, parsarea, trimiterea catre clientii TCP)
* `run_client.cpp` - implementarea functionalitatii clientului TCP (conectarea la server, abonarea la topicuri, primirea de mesaje)

Pe langa aceste fisiere, proiectul mai contine si doua fisiere header, `headers.h` si `run_server.h`, care contin declaratiile functiilor folosite in implementare si doua structuri de date.

```
typedef struct {
    char topic[MAX_TOPICS];
    uint8_t type;
    char payload[MAX_LEN];
} message;
```
Structura `message` este folosita pentru a retine informatiile dintr-un mesaj primit de la un client UDP. Aceasta contine topicul, tipul si payloadul mesajului. Acesta este singurul tip de mesaj care poate fi primit de server.

```
typedef struct {
    char ip_client_udp[MAX_TOPICS];
    int port_client_udp;
    message msg;
} udp_message;
```
Strctura `udp_message` este folosita pentru a retine informatiile dintr-un mesaj primit de la un client UDP. Aceasta contine ip-ul si portul clientului UDP, precum si mesajul primit de la acesta. Aceasta structura este folosita pentru a retine mesajele primite de server, inainte de a fi trimise catre clientii TCP abonati la topicul respectiv. Acesta este singurul tip de mesaj care poate fi trimis de server.

### Implementare Server
Serverul este implementat in fisierul `server.cpp`. Acesta creeaza socketii de UDP si TCP, folosind functia `create_socket`. Pentru socketul de UDP si TCP, se seteaza optiunea `SO_REUSEADDR`. Pentru a dezactiva algoritmului Naglem pentru socketul de TCP, se seteaza optiunea `TCP_NODELAY`. 

Pentru ca serverul sa poata sa primeasca mesaje atat de la STDIN, UDP si toti clientii TCP, am implementat un vector alocat dinamic (STL) de file descriptori(`fd`). Am folosit functia `poll` pentru a verifica daca unul dintre socketii din vectorul `fd` este gata de citire. Daca socketul este gata de citire, se verifica daca este socketul de UDP, de TCP sau STDIN. 

Daca este socketul de **UDP**, se citeste un mesaj de tip `message`. Pe baza acestui mesaj de construieste un mesaj de tip `udp_message`, care contine ip-ul si portul clientului UDP, precum si mesajul primit de la acesta. Incerc sa adaug acest mesaj in lista mesajelor cu topicul respectiv, in cazul in care sunt clienti abonati la acel topic. Daca nu sunt clienti abonati la acel topic, mesajul este ignorat. De asemenea, incerc sa trimit acel mesaj spre toti clientii abonati care sunt conectati la server in acel moment.

Daca este socketul **initial** de **TCP**, inseamna ca un nou client doreste sa se conecteze si ii adaug fd ul in lista de fd uri. 

Daca este un socket nou adaugat, inseamna ca primesc un mesaj de la un client **TCP**. Tipurile de mesaje pe care pot sa le primesc se vor regasi in campul topic, iar continutul este in payload. Acestea pot fi:
* `connect` - clientul doreste sa se deconecteze de la server, deci o sa trimita in payload id-ul cu care doreste sa se conecteze. Pentru fiecare id am memorat socketul de TCP corespunzator(sau -1 in cazul in care clientul s-a deconectat). Daca pentru id ul primit avem deja un fd, printam faptul ca acel id este folosit deja si inchidem conexiunea. 
* `subscribe` - clientul doreste sa se aboneze la un topic, deci o sa trimita in payload topicul la care doreste sa se aboneze si *sf-ul*. Adaugam acel topic in lista de topicuri la care este abonat clientul si salvam *sf-ul* corespunzator. Pentru a ma asigura ca setez si sf-ul, pentru fiecare topic creez o pereche care pastreaza topicul si o pozitie in vectorul de mesaje ale topicului respectiv. Daca sf-ul este 0, acea pozitie o sa fie inlocuita cu -1.  
* `unsubscribe` - clientul doreste sa se dezaboneze de la un topic, deci o sa trimita in payload topicul de la care doreste sa se dezaboneze. Scoatem acel topic din lista de topicuri la care este abonat clientul. Daca, facand asta, la topicul respectiv nu mai este abonat de niciun client, il stergem din lista de topicuri.

Mai mult, atunci cand un client se conecteaza, verific daca exista mesaje in lista de mesaje cu topicul la care s-a abonat si au sf-ul 1. Daca exista, le trimit pe toate catre clientul respectiv, lipite. Pentru a face aceasta lipire, prima data am trimis un mesaj in care spun cate mesaje urmeaza sa fie in total, iar apoi trimit un singur mesaj cu toate mesajele concatenate.

Daca este **STDIN**, inseamna ca serverul trebuie sa se inchida. Inchid socketii de UDP si TCP, eliberez memoria alocata dinamic si inchid serverul.

### Implementare Client

Clientul este implementat in fisierul `client.cpp`, iar functionalitatile acestora sunt pastrate in `run_client.cpp`.

Clientul poate sa primeasca mesaje doar de la STDIN sau de la server. 

De la **STDIN** poate sa primeasca comenzi de la utilizator. Acestea pot fi:
* `exit` - clientul se deconecteaza de la server si se inchide
* `subscribe` - clientul se aboneaza la un topic, deci o sa trimit un mesaj de tip `message`, punand in topic topicul la care vreau sa ma abonez si in sf 1 sau 0, in functie de ce a ales utilizatorul. Daca utilizatorul a uitat sa puna sf, o sa primeasca un avertisment si instructiunea nu o sa se execute.
* `unsubscribe` - clientul se dezaboneaza de la un topic, deci o sa trimit un mesaj de tip `message`, punand in topic topicul de la care vreau sa ma dezabonez.

De la **server** poate sa primeasca mesaje legat de topicurile la care este abonat. Acestea pot fi:
* `INT` - clientul a primit un mesaj de tip `INT` de la server, deci o sa afisez topicul si valoarea primita.
* `SHORT_REAL` - clientul a primit un mesaj de tip `SHORT_REAL` de la server, deci o sa afisez topicul si valoarea primita.
* `FLOAT` - clientul a primit un mesaj de tip `FLOAT` de la server, deci o sa afisez topicul si valoarea primita.
* `STRING` - clientul a primit un mesaj de tip `STRING` de la server, deci o sa afisez topicul si valoarea primita.
* un mesaj special care anunta ca urmeaza sa primeasca un mesaj care contine mai multe mesaje concatenate. In acest caz, o sa primesc mai intai numarul de mesaje concatenate, iar apoi o sa primesc mesajul concatenat. Acesta o sa fie despartit in mesaje individuale, care o sa fie afisate pe ecran.

Parsarea mesajelor are loc in functia `void print_udp_message(udp_message msg)`.

### Testare
Pentru testare m-am folosit atat de scriptul `test.py`, asigurandu-ma ca imi trec toate testele propuse, cat si de testare manuala, pentru a ma asigura ca nu am uitat vreun caz particular si am implementat corect functionalitati neacoperite de checker (ex. input incorect de la client).

### Bibliografie
* [Laborator 7](https://pcom.pages.upb.ro/labs/lab7/lecture.html)
