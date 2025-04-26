#ifndef ECHO_SERVER_HTTPLIB_H
#define ECHO_SERVER_HTTPLIB_H

#include "stringstructlib.h"

typedef struct request_header {
    string *user_agent;
} request_header;

typedef struct entity_header {
    string *content_type;
    size_t content_length;
} entity_header;

typedef struct http_request {
    string *method;
    string *uri;
    string *protocol;
    string *host;
    request_header *header;
    string *body;
} http_request;

typedef struct http_response {
    string *protocol;
    string *status_code;
    string *status_description;
    entity_header *entity_header;
    string *location;
    string *body;
} http_response;

void free_request_header(request_header *header);

void free_entity_header(entity_header *header);

void free_request(http_request *request);

void free_response(http_response *response);

int hex2int(char c);

http_request *str_to_http_request(string *str);

string *read_file_into_string(char *filepath, unsigned int len);

short validate_file_access(char *filepath, unsigned int len);

string *response_string(http_response *response);

void set_response_status(http_response *response, string *status_code, string *status_description);

void set_response_body(http_response *response, string *body, string *content_type);

void set_response_default_html_body(http_response *response);

string *get_content_type(string *ending);

#endif //ECHO_SERVER_HTTPLIB_H
