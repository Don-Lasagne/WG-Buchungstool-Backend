#include <errno.h>
#include <netinet/ip.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "httplib.h"

#define PORT 31337
#define BUFFER_SIZE (1024*1024)
#define FRONTEND_LOCATION "http://localhost:4200"

string *process(string *request);

static bool run = true;

/**
 * Gibt eine Fehlermeldung *msg* aus und beendet das Programm.
 * @param msg Die Fehlermeldung.
 */
static void error(char *msg) {
    fprintf(stderr, "%s", msg);
    if (errno) {
        fprintf(stderr, ", errno: %s", strerror(errno));
    }
    fprintf(stderr, "\n");
    exit(1);
}

/**
 * Diese Funktion wird aufgerufen, wenn das Programm das *SIGINT*-Signal empfängt. Es beendet den Server.
 * @param signum Die Signalnummer.
 */
static void handle_signal(int signum) {
    if (signum != SIGINT) {
        error("ERROR unexpected signal");
    }
    //Beende den Server nach dem Abarbeiten des letzten Clients.
    run = false;
}

/**
 * Registriert das SIGINT-Signal (Strg+C) um den Server beenden zu können.
 */
static void register_signal(void) {
    struct sigaction action;

    //Konfigurieren des Signal-Handlers.
    memset(&action, 0, sizeof(action));
    action.sa_handler = handle_signal;
    //Registrierung des Signal-Handlers.
    if (sigaction(SIGINT, &action, NULL) < 0) {
        error("ERROR registering signal handler");
    }
}

/**
 * Erstellt und konfiguriert den Netzwerk-Socket, über den die Verbindungen
 * angenommen werden.
 */
static int setup_socket(void) {
    int opt = 1;
    int sockfd = 0;
    struct sockaddr_in serv_addr;

    //Setzt Konfigurationsvariablen für den Socket, z.B. die Portnummer.
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    //Erstelle den Socket.
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }

    //Verwende den Socket, selbst wenn er aus einer vorigen Ausführung im TIME_WAIT Status ist.
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char *) &opt, sizeof(int)) < 0)
        error("ERROR on setsockopt");

    //Melde, dass der Socket eingehende Verbindungen akzeptieren soll.
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR on binding");
    }

    //Horche auf dem Socket nach eingehenden Verbindungen. Es werden maximal fünf gleichzeitige Verbindungen erlaubt.
    if (listen(sockfd, 5) < 0) {
        error("listen");
    }
    return sockfd;
}

static void main_loop_stdin(void) {
    void *const buffer = malloc(BUFFER_SIZE);
    if (buffer == NULL) {
        error("ERROR at malloc.");
    }

    //Lies die ankommenden Daten von dem Socket in das Array buffer.
    memset(buffer, 0, BUFFER_SIZE);
    ssize_t length = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1);
    if (length < 0) {
        if (errno != EINTR) {
            error("ERROR reading from socket");
        }
    }
    string *request = str_cpy(buffer, (size_t) length);
    string *response = process(request);

    size_t response_len = get_length(response);
    char *response_char = get_char_str(response);
    //Schreibe die ausgehenden Daten auf stdout.
    if (write(STDOUT_FILENO, response_char, response_len) < 0) {
        error("ERROR writing to STDOUT");
    }
    str_free(response);
    free(buffer);
}

/**
 * Die Hauptschleife, in der eingehende Verbindungen angenommen werden.
 */
static void main_loop(void) {
    const int sockfd = setup_socket();
    int newsockfd;
    ssize_t length;

    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);

    void *const buffer = malloc(BUFFER_SIZE);
    if (buffer == NULL) {
        error("ERROR at malloc.");
    }
    //Die Hauptschleife des Programms.
    while (run) {
        //Der accept()-Aufruf blockiert, bis eine neue Verbindung hereinkommt.
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
            //Wenn der Server mit dem SIGINT-Signal beendet wird, schlägt accept mit EINTR (interrupted) fehl.
            if (errno == EINTR) {
                break;
            }
            error("ERROR on accept");
        }

        //Lies die ankommenden Daten von dem Socket in das Array buffer.
        memset(buffer, 0, BUFFER_SIZE);
        length = read(newsockfd, buffer, BUFFER_SIZE - 1);
        if (length < 0) {
            if (errno == EINTR) {
                break;
            }
            error("ERROR reading from socket");
        }
        string *request = str_cpy(buffer, (size_t) length);
        string *response = process(request);

        //Schreibe die ausgehenden Daten auf den Socket.
        size_t response_len = get_length(response);
        char *response_char = get_char_str(response);
        length = write(newsockfd, response_char, response_len);
        if (length < 0) {
            error("ERROR writing to socket");
        }
        str_free(request);
        str_free(response);

        //Schließe die Verbindung.
        if (close(newsockfd) < 0) {
            error("ERROR on close");
        }
    }
    free(buffer);
    if (close(sockfd) < 0) {
        error("ERROR on close");
    }
}

/**
 * Die Funktion akzeptiert den eingehenden Request und gibt eine entsprechende Response zurück.
 * @param request Der eingehende Request.
 * @return Die ausgehende Response.
 */
string *process(string *request) {
    //Validate Request-Line
    short space_counter = 0;
    for (unsigned int i = 0; i < request->len; ++i) {
        if (request->str[i] == '\n') {
            break;
        } else if (request->str[i] == ' ') {
            if (i > 0 && request->str[i - 1] == ' ') {
                space_counter = 0;
                break;
            }
            space_counter++;
        }
    }
    short line_break_counter = 0;
    for (unsigned int i = 0; i < request->len; ++i) {
        if (i > 0 && request->str[i] == '\n' && request->str[i - 1] == '\r') {
            line_break_counter++;
        }
    }

    http_request *req;
    string *response_str;

    if (space_counter == 2 && line_break_counter >= 2) {
        req = str_to_http_request(request);

        http_response *resp = calloc(1, sizeof(http_response));
        resp->entity_header = calloc(1, sizeof(entity_header));
        for (unsigned int i = 0; i < req->uri->len; i++) {
            if (req->uri->str[i] == '\0') {
                set_response_status(resp, char_to_string("400"), char_to_string("Bad Request"));
                set_response_default_html_body(resp);
                response_str = response_string(resp);
                free_request(req);
                free_response(resp);
                return response_str;
            }
        }
        string *http_version = char_to_string("HTTP/1.1");
        if (req->uri->len > 0 && req->uri->len < 256 &&
            str_start_with_chars(req->uri, "/", strlen("/"))) {

            string *get = char_to_string("GET");
            string *post = char_to_string("POST");
            if (str_equals(req->method, get)) {
                //Get File
                if (req->uri->len == 1) {
                    set_response_status(resp, char_to_string("308"), char_to_string("Permanent Redirect"));
                    resp->location = char_to_string(FRONTEND_LOCATION);
                } else {
                    string *file_path = str_cpy(req->uri->str, req->uri->len);

                    string *file;
                    switch (validate_file_access(file_path->str, (unsigned int) file_path->len)) {
                        case 1: //File exists
                            file = read_file_into_string(file_path->str, (unsigned int) file_path->len);
                            if (file == NULL) {
                                //Filepath is directory, not a file
                                set_response_status(resp, char_to_string("404"), char_to_string("Not Found"));
                                set_response_default_html_body(resp);
                                break;
                            }

                        //Get File Type
                            string **split_pathsplit_str = str_split(file_path, '.');
                            int i = 0;
                            for (i = 0; split_pathsplit_str[i]; ++i) {
                                ;
                            }
                            string *ending = split_pathsplit_str[i - 1];
                            for (int j = 0; j < i - 1; ++j) {
                                str_print(split_pathsplit_str[j]);
                                str_free(split_pathsplit_str[j]);
                            }
                            free(split_pathsplit_str);

                            set_response_status(resp, char_to_string("200"), char_to_string("OK"));
                            set_response_body(resp, file, get_content_type(ending));

                            break;
                        case 2: //File not found
                            set_response_status(resp, char_to_string("404"), char_to_string("Not Found"));
                            set_response_default_html_body(resp);
                            break;
                        default: //File not in doc-root
                            set_response_status(resp, char_to_string("403"), char_to_string("Forbidden"));
                            set_response_default_html_body(resp);
                            break;
                    }
                    str_free(file_path);
                }
                str_free(get);
                str_free(post);
            } else if (str_equals(req->method, post)) {
                set_response_status(resp, char_to_string("501"), char_to_string("Not Implemented"));
                set_response_default_html_body(resp);
            }
        } else if (req->uri->len > 255) {
            set_response_status(resp, char_to_string("414"), char_to_string("URI too long"));
            set_response_default_html_body(resp);
        } else {
            set_response_status(resp, char_to_string("501"), char_to_string("Not Implemented"));
            set_response_default_html_body(resp);
        }
        response_str = response_string(resp);
        str_free(http_version);
        free_request(req);
        free_response(resp);
        return response_str;
    }
    //Bad Request
    http_response *resp = calloc(1, sizeof(http_response));
    resp->entity_header = calloc(1, sizeof(entity_header));
    set_response_status(resp, char_to_string("400"), char_to_string("Bad Request"));
    set_response_default_html_body(resp);
    response_str = response_string(resp);

    free_response(resp);
    return response_str;
}

int main(int argc, char *argv[]) {
    register_signal();
    if (argc == 2 && strcmp("stdin", argv[1]) == 0) {
        main_loop_stdin();
    } else {
        main_loop();
    }
    return 0;
}
