#include <iostream>
#include <unordered_map>
#include <cstring>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>

#include "nlohmann/json.hpp"
#include "requests.hpp"
#include "utils.hpp"

#define ACTION_REGISTER 0
#define ACTION_LOGIN 1
#define ACTION_ENTER_LIBRARY 2
#define ACTION_GET_BOOKS 3
#define ACTION_GET_BOOK 4
#define ACTION_ADD_BOOK 5
#define ACTION_DELETE_BOOK 6
#define ACTION_LOGOUT 7
#define ACTION_EXIT 8

#define SERVER_PORT 8080

using namespace std;
using json = nlohmann::json;

const char* SERVER_IP = "34.246.184.49";
unordered_map <string, int> action_id;

unordered_map<string, int> fill_map() {
    unordered_map <string, int> action_id;

    action_id.insert(make_pair("register", ACTION_REGISTER));
    action_id.insert(make_pair("login", ACTION_LOGIN));
    action_id.insert(make_pair("enter_library", ACTION_ENTER_LIBRARY));
    action_id.insert(make_pair("get_books", ACTION_GET_BOOKS));
    action_id.insert(make_pair("get_book", ACTION_GET_BOOK));
    action_id.insert(make_pair("add_book", ACTION_ADD_BOOK));
    action_id.insert(make_pair("delete_book", ACTION_DELETE_BOOK));
    action_id.insert(make_pair("logout", ACTION_LOGOUT));
    action_id.insert(make_pair("exit", ACTION_EXIT));

    return action_id;
}

bool is_it_number(char *my_string) {

    for (unsigned long i = 0; i < strlen(my_string); i++) {
        if (my_string[i] > 57 || my_string[i] < 48) {
            return false;
        } 
    }

    return true;
}

void do_register() {
    char username[BUFLEN], password[BUFLEN];

    json request;

    cout << "username=";
    cin.getline(username, BUFLEN);
    cout << "password=";
    cin.getline(password, BUFLEN);

    /* ma asigur ca credentialele nu au caractere invalide */
    if (strchr(username, ' ')) {
        cout << "Error: The username shouldn't contain spaces!" << endl;
        return;
    }

    if (strchr(password, ' ')) {
        cout << "Error: The password shouldn't contain spaces " << endl;
        return;
    }
    request["username"] = username;
    request["password"] = password;

    string stringed_json = request.dump();

    int sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    char *message = post_request("34.246.184.49:8080", "/api/v1/tema/auth/register", "application/json", stringed_json, {}, 0, NULL);

    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    if (strstr(response, "error")) {
        cout << "Error: The username " << username << " is already taken!" << endl;
    } else {
        cout << "Successfull registration!" << endl;
    }

    close(sockfd);
}

string do_logging() {
    char username[BUFLEN], password[BUFLEN], connection_seed[BUFLEN];
    json request;
    string login_cookie;

    cout << "username=";
    cin.getline(username, BUFLEN);
    cout << "password=";
    cin.getline(password, BUFLEN);

    /* ma asigur ca credentialele nu au caractere invalide */
    if (strchr(username, ' ')) {
        cout << "Error: The username shouldn't contain spaces!" << endl;
        return "";
    }

    if (strchr(password, ' ')) {
        cout << "Error: The password shouldn't contain spaces " << endl;
        return "";
    }

    request["username"] = username;
    request["password"] = password;
    string stringed_json = request.dump();
    
    int sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    
    char *message = post_request("34.246.184.49:8080", "/api/v1/tema/auth/login", "application/json", stringed_json, {} , 0, NULL);
    send_to_server(sockfd, message);
    char *response = receive_from_server (sockfd);

    if (strstr(response, "error")) {
        char *json_error = strstr(response, "error");
        char delim[3];
        
        delim[0] = ';';
        delim[1] = '"';
        delim[2] = ':';
    
        strtok(json_error, delim);
        cout << strtok(NULL, delim) << endl;

        return "";
    } else {
        cout << "You logged in successfully!" << endl;
    }

    /* salvez substring-ul ce incepe cu connect */
    strcpy(connection_seed, strstr(response, "connect"));

    char *rest = strchr(connection_seed, ';');

    string response_string = response;

    /* aici se salveaza cookie-ul final */
    login_cookie = response_string.substr(strstr(response, "connect") - response, rest - connection_seed);

    close(sockfd);
    return login_cookie;
}

char* access_library(string login_cookie) {
    char *new_token = new char[BUFLEN];
    if (login_cookie.size() == 0) {
        cout << "Error: You are not logged in!" << endl;
        strcpy(new_token, "");
        return new_token;
    }

    int sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        strcpy(new_token, "");
        return new_token;
    }

    /* salvez login cookie-ul in vectorul de cookies */
    char *cookies[1];
    cookies[0] = strdup(login_cookie.c_str());

    char *message = compute_get_request("34.246.184.49:8080", "/api/v1/tema/library/access", NULL, cookies, 1, NULL);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    if (strstr(response, "token") != NULL) {
        new_token = new char[BUFLEN];
        /* copiez in token substring-ul din campul "token", inclusiv ghilimelele */
        strcpy(new_token, strstr(response, "token") + strlen("token") + 3);

        /* inchid sirul de caractere */
        new_token[strlen(new_token) - 2] = '\0';
        cout << "Success: You accessed the library successfully!" << endl;
    } else {
        /* aici verific daca cumva serverul a intampinat erori in procesul de decoding*/
        if (strstr(response, "decoding")) {
            cout << "Error: There was an error decoding your login credentials!" << endl;
        } else {
            /* au fost alte erori */
            cout << "Error: There was an error accessing server's api." << endl;
        }

        /* trebuie sa golesc tokenul daca au fost intampinate erori, pentru evitarea ambiguitatilor */
        new_token[0] = '\0';
    }

    close(sockfd);
    return new_token;
}

void do_get_books(string login_cookie, char *token) {
    /* verific ca userul sa fie logat */
    if (login_cookie.size() == 0) {
        cout << "Error: You are not logged in!" << endl;
        return;
    }

    int sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return;
    }

    char *cookies[1];
    cookies[0] = strdup(login_cookie.c_str());
    char *message = compute_get_request("34.246.184.49:8080", "/api/v1/tema/library/books", NULL, cookies, 1, token);

    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    if (strstr(response, "error")) {
        char *json_error = strstr(response, "error");
        char delim[3];
        
        delim[0] = ';';
        delim[1] = '"';
        delim[2] = ':';
    
        strtok(json_error, delim);

        cout << "Error: " << strtok(NULL, delim) << endl;
    } else {
        /* afisez lista cu carti */
        cout << "Success: " << strstr(response, "[") << endl;
    }
    close(sockfd);
}

void do_get_book(string login_cookie, char *token) {
    char id[IDLEN];

    cout << "id=";
    cin.getline(id, BUFLEN);

    /* verific daca userul este logat */
    if (login_cookie.size() == 0) {
        cout << "Error: You are not logged in!" << endl;
        return;
    }

    /* verific daca id-ul este un numar */

    if (is_it_number(id) == false) {
        cout << "Error: Id not a number" << endl;
        return;
    }

    int sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        return;
    }

    /* salvez login cookie-ul in vectorul de cookies */
    char *cookies[1];
    cookies[0] = strdup(login_cookie.c_str());

    char url[BUFLEN];
    sprintf(url, "/api/v1/tema/library/books/%s", id);

    char *message = compute_get_request("34.246.184.49:8080", url, NULL, cookies, 1, token);

    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    if (strstr(response, "error")) {
        char *json_error = strstr(response, "error");
        char delim[3];
        
        delim[0] = ';';
        delim[1] = '"';
        delim[2] = ':';
    
        strtok(json_error, delim);
        if (strstr(response, "404")) {
            cout << "Error: " << "Your book is not here! :(" << endl;
        } else {
            cout << "Error: " << strtok(NULL, delim) << endl;
        }
    } else {
        /* se parseaza substring-ul care incepe cu prima acolada */
        stringstream new_ss;
        new_ss << strstr(response, "{");
        json json_response = json::parse(new_ss);
        cout << "Success: You found the requested book. Here is it's information: " << endl << endl;

        cout << json_response.dump(4) << endl;
    }
    close(sockfd);
}

void add_book(string login_cookie, char *token) {
    if (login_cookie.size() == 0) {
        cout << "You need to log in!" << endl;
        return;
    }

    int sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    
    /* toate informatiile despre o carte, mai putin id-ul care este dat automat */
    char title[BUFLEN], author[BUFLEN], genre[BUFLEN], publisher[BUFLEN], page_count[BUFLEN];
    int page_count_integer;

    cout << "title=";
    cin.getline(title, BUFLEN);
    cout << "author=";
    cin.getline(author, BUFLEN);
    cout << "genre=";
    cin.getline(genre, BUFLEN);
    cout << "publisher=";
    cin.getline(publisher, BUFLEN);
    cout << "page_count=";
    cin.getline(page_count, BUFLEN);

    /* verific daca exista vreun field gol */
    if (!strlen(title) || !strlen(author) || !strlen(genre) || !strlen(publisher) || !strlen(page_count)) {
        cout << "Error: Your entries are invalid! No information is forbidden." << endl;
        return;
    }

    /* verific daca numarul de pagini este intreg */
    if (is_it_number(page_count) == false) {
        cout << "Error: Wrong data type for the number of pages!" << endl;
        return;
    }

    page_count_integer = atoi(page_count);

    /* declar obiectul cartii de tip json */
    json new_request_object;

    new_request_object["title"] = title;
    new_request_object["author"] = author;
    new_request_object["genre"] = genre;
    new_request_object["publisher"] = publisher;
    new_request_object["page_count"] = page_count_integer;

    string stringed_json = new_request_object.dump();


    char *message = post_request("34.246.184.49:8080", "/api/v1/tema/library/books", "application/json", stringed_json, {} , 0, token);
    send_to_server(sockfd, message);
    char *response = receive_from_server (sockfd);

    if (strstr(response, "error")) {
        cout << strstr(response, "error") << endl;
    }
    cout << "Success: You book named "<< new_request_object["title"] <<" was added successfully!" << endl;

    close(sockfd);
    return;
}

void delete_book(string login_cookie, char *token) {

    /* id-ul pe care urmeaza sa-l citesc */
    char id[IDLEN];

    cout << "id=";
    cin.getline(id, BUFLEN);            /* citirea propriu-zisa */

    /* verific ca userul este logat */
    if (login_cookie.size() == 0) {
        cout << "You are not logged in!" << endl;
        return;
    }

    /* deschid conexiunea */
    int sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return;
    }

    /* verific daca id-ul este un numar */
    if (is_it_number(id) == false) {
        cout << "Your id is not a number" << endl;
        return;
    } 

    /* salvez login cookie-ul in vectorul de cookies */
    char *cookies[1];
    cookies[0] = strdup(login_cookie.c_str());

    /* adaug id-ul in url */
    char url[BUFLEN];
    sprintf(url, "/api/v1/tema/library/books/%s", id);

    /* trimit cererea de delete */
    char *message = compute_delete_request("34.246.184.49:8080", url, NULL, cookies, 1, token);

    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    if (strstr(response, "error")) {
        char *json_error = strstr(response, "error");
        char delim[3];
        
        delim[0] = ';';
        delim[1] = '"';
        delim[2] = ':';

        strtok(json_error, delim);
        if (strstr(response, "404")) {
            cout << "Error: " << "Your book is not here! :(" << endl;
        } else {
            cout << "Error: " << strtok(NULL, delim) << endl;
        }
    } else {
        cout << "Success: Your book was deleted successfully :)" << endl;
    }

    close(sockfd);
}

void do_logout(string login_cookie) {
    int sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        return;
    }

    /* salvez login cookie-ul in vectorul de cookies */
    char *cookies[1];
    cookies[0] = strdup(login_cookie.c_str());

    char *message = compute_get_request("34.246.184.49:8080", "/api/v1/tema/auth/logout", NULL, cookies , 1, NULL);
    send_to_server(sockfd, message);
    char *response = receive_from_server (sockfd);
    
    json json_response;
    if (strstr(response, "error")) {
        char *json_error = strstr(response, "error");
        char delim[3];
        
        delim[0] = ';';
        delim[1] = '"';
        delim[2] = ':';
    
        strtok(json_error, delim);
        cout << "Error: " << strtok(NULL, delim) << endl;
    } else {
        cout << "Success: You successfully logged out!" << endl;
    }

    close(sockfd);    
}

int main() {
    char action[BUFLEN];          /* actiunea */

    /* aici salvez login_cookie-ul*/
    string login_cookie = "";

    /* 
        aici imi umplu map-ul cu toate optiunile aferente fiecarei actiuni
    */
    action_id = fill_map();

    char token[BUFLEN];             /* tokenul folosit la autorizare*/
    memset(token, 0, BUFLEN);

    bool in_loop = true;    /* flag resetat la comanda exit */
    bool do_exiting = false;    /* flag resetat la comanda exit */
    bool is_registered = false;     /* flag setat la comanda register */

    while(in_loop) {
        fflush(stdout);
        cin.getline(action, BUFLEN);

        /* verifica ca actiunea sa nu aiba spatii */
    
        if (strchr(action, ' ') != NULL) {
            cout << "Your action has invalid characters. Try again!" << endl;
            continue;
        }

        /* verifica ca actiunea sa se afle printre cele valide */
        if (action_id.find(action) == action_id.end()) {
            cout << "Your command is not valid, try again!" << endl;
            continue;
        }

        /* salvez actiunea curenta */
        int action_encountering = action_id[action];

        /* gestionez actiunea curenta */
        switch(action_encountering) {
            case ACTION_REGISTER:
                /* 
                    daca userul e deja inregistrat, n-are sens sa mai inregistrez un alt cont in timpul
                    sesiunii
                */
                if (is_registered == true) {
                    cout << "Error: " << "The username is already registered right now!" << endl;
                } else {
                    do_register();
                    is_registered = true;
                }
                break;

            case ACTION_LOGIN:
                /* daca cookie-ul de login nu a fost sters sau utilizatorul inca este logat */
                if (login_cookie.size() != 0) {
                    cout << "The username is already logged in!" << endl;
                    break;
                }
                login_cookie = do_logging();
                break;
            
            case ACTION_ENTER_LIBRARY:
                /* salvez tokenul returnat de functie in token */
                strcpy(token, access_library(login_cookie));
                break;
            
            case ACTION_GET_BOOKS:
                do_get_books(login_cookie, token);
                break;

            case ACTION_GET_BOOK:
                do_get_book(login_cookie, token);
                break;

            case ACTION_ADD_BOOK:
                add_book(login_cookie, token);
                break;
            
            case ACTION_DELETE_BOOK:
                delete_book(login_cookie, token);
                break;
            
            case ACTION_LOGOUT:
                /* reseteaza tokenul */
                token[0] = '\0';

                /* delogam userul */
                do_logout(login_cookie);

                /* reseteaza cookie-ul */
                login_cookie = "";

                /* reseteaza flagul de register */
                is_registered = false;
                break;

            case ACTION_EXIT:

                /* activeaza flagul */
                do_exiting = true;
                cout << "Thank you for using our services!" << endl;
                break;

            default:
                cout << "Your command is invalid!" << endl;
                break;
        }

        /* flagul de exit activat */
        if (do_exiting) {
            break;
        }
    }
}