* &copy; Pogan Alexandru-Mihail, 325CA

# Protocoale de comunicatii - Implementare broker de mesaje

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Aceasta tema consta in implementarea unui sistem de tip client TCP - server - client UDP, in cadrul caruia serverul joaca rolul unui broker de mesaje, care gestioneaza fluxul mesajelor transmise de catre clientii UDP, cu scopul de a fi receptionate de catre clientii TCP, acestia din urma avand posibilitatea de a se abona la topicuri, pentru prevenirea primirii de mesaje neinteresante.
P.S. Am folosit scheletul de laborator de la laboratorul 7 - TCP si am folosit un sleep day.

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; In cadrul acestui sistem poate participa un numar variabil de clienti TCP si UDP, serverul avand responsabilitatea de a realiza acest proces de conectare a clientilor cat mai rapid si eficient.

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Acest readme este structurat in doua capitole:
1. Structuri folosite in tema

```cpp
    struct udp_request_connection {
        char topic[TOPIC_MAXSIZE];
        int data_type;
        char payload[TOPIC_PAYLOAD];
    };

    struct tcp_message{
        uint16_t len; /* the payload length */
        uint16_t operation_type;  /* for checking if the user wants to subscribe, unsubscribe or just to exit */
        char data[TCP_MAXSIZE];  /* the client's payload */
    };
```
2. descrierea functionalitatii serverului
    
    * Ce structuri de date foloseste?
    * Cum le foloseste?
    * Ce poate sa faca?
    * Cum receptioneaza informatiile venite?

3. descriere functionalitatii clientului

    * Ce structuri de date foloseste?
    * Cum le foloseste?
    * Ce informatii poate primi de la useri si cum le foloseste?
    * Cum receptioneaza informatiile de la server?



    
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Serverul a fost implementat in fisierul server.cpp. Acesta foloseste o serie de structuri de date de baza.

* `poll(pollfds, current_number_of_clients, -1);` --> reprezinta o structura ce permite multiplexarea I/O, fapt care mareste eficienta programului. De asemenea, aceasta structura permite abonarea oricator clienti, datorita mecanismului prin care acesta se extinde, cand numarul de conexiuni depaseste jumatate din numarul maxim de clienti conectati.

* `unordered_map <int, string> socket_id_map;` --> structura de tip map, care retine perechi socket --- id, unde id-ul reprezinta un string, iar socketul, o variabila de tip int. Aceasta este folosita pentru gestionarea abonarii/dezabonarii, prin obtinerea id-ului unui client, folosind socketul de pe care au venit datele

* `unordered_map <string, int> id_socket_map;` --> structura de tip map, care retine perechi id --- socket. Aceasta este folosita pentru gestionarea mesajelor de abonare/dezabonare, respectiv pentru robustete in situatiile in care vrem sa fim siguri ca respectivul client cu un id dat sa nu se poata abona de mai multe ori.

* `unordered_map <string, vector<string>> id_topics_map;` --> structura ce retine topicurile la care s-a abonat fiecare abonat, o structura folosita in cadrul procesului de abonare/dezabonare.

* `unordered_map <string, vector <string>> topics_id_map;` --> structura ce retine clientii abonati pentru fiecare topic.

1. Pregatirea serverului
    * acesta este pornit de catre user din linia de comanda. Programul verifica la inceput daca programul este apelat corect, fiind aplicat principiul programarii defensive. 
    * Dupa acea, fiecare parametru este prelucrat corespunzator, verificand totodata daca id-ul clientului are lungimea mai mare de 10 caractere.
    * Dezactiveaza buffering-ul. 
    * Acesta initializeaza un socket TCP, cu care poate asculta cererile venite pe acesta, nu inainte de a da bind la adresa sa ip si port. De asemenea, dezactiveaza algoritmul lui Nagle.
    * De asemenea, mai initializeaza un socket UDP, pentru care da binding pe aceeasi adresa a serverului
    * Initializeaza un vector poll, in care introduce socketii TCP si UDP, precum si file descriptorul stdin, pentru a putea primi informatii si din terminal direct.
    * Porneste bucla infinita, in care trateaza patru mari cazuri:
        
    1. Citeste din stdin 
        * atunci serverul verifica daca utilizatorul a scris "exit" in terminal.
        Atunci, serverul inchide bucla infinita si inchide toti socketii.

    2. Primeste cereri de conectare de la clienti tcp.
        * accepta cererea de conexiune, primeste id-ul clientului folosind functia "recv_all" si dupa verifica daca in bazele sale de date descrise mai sus contin id-ul respectiv. In acest caz, printeaza un mesaj sugestiv si inchide conexiunea si socketul. Altfel, printeaza un mesaj corespunzator, urmand sa-l introduca in map-urile cu clienti, precum si in vectorul poll_fds.

    3. Primeste cereri de pe socketii clientilor conectati
        * pregateste niste stringuri pentru a retine topicul dorit, dupa foloseste in switch pe tipul de actiune setat din client, urmand ca in functie de acesta, fie aboneaza clientul si il introduce in baza de date a topicului, introducand si topicul intr-o baza de date destinata fiecarui id, fie il dezaboneaza.
        * daca actiunea nu este de iesire a clientului, serverul trimite informatia catre client. In caz contrar, inchide socketul clientului si il scoate din poll.

    4. Primeste informatii de la clientii UDP.
        * citeste informatia intr-un buffer, salveaza fiecare componenta in niste variabile corespunzatoare si in functie de tipul de date, serverul construieste mesajul.
        * pentru stringuri afiseaza pur si simplu payload-ul, fara vreo prelucrare
        * pentru int-uri, salveaza primul octet de semn si restul octetilor definesc valoarea.
        * pentru short_real-uri, ia valoarea si o imparte la 100.00, cu precizie de doua zecimale.
        * pentru numere reale, salveaza mantisa, bitul de semn si exponentul si imparte la exponentul dat.
        * mai adaugam si portul si adresa ip de la clientii udp si se trimit catre client. 

2. Pregatirea clientului
    * Se verifica faptul ca a fost apelat corect
    * Se dezactiveaza buffering-ul 
    * Se ia un socket TCP, se conecteaza la serverul dat din ip si se dezactiveaza algoritmul lui Nagle
    * Initializeaza un vector poll, in care adaug stdin-ul si socketul prin intermediul caruia clientul s-a conectat la server
    * Porneste bucla infinita si trateaza doua cazuri.
    * 1. Primeste informatii de la stdin
        * Subscribe - salveaza mesajul intr-o structura peste TCP si o trimite la server. Salveaza mesajul in campul .data.
        * Unsubscribe - acelasi lucru ca la subscribe.
        * Exit - Notifica serverul, iese din bucla si inchide socketul serverului. 
    * 2. Primeste notificare de la server
        * in functie de tipul de actiune returnat de catre server prin intermediul campului OPERATION_TYPE, acesta afiseaza mesajul corespunzator generat de catre server. 
        * poate primi notificare de la client legat de faptul ca a incercat sa se conecteze cu un id deja existent. In acest caz, inchide descriptorii si iese fortat.

