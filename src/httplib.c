#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>
#include <stdio.h>

#include "httplib.h"

#define DOC_ROOT "../resources/"

void free_request_header(request_header *header) {
    if (header->user_agent != NULL)
        str_free(header->user_agent);
    free(header);
}

void free_entity_header(entity_header *header) {
    assert(header != NULL);
    if (header->content_type != NULL)
        str_free(header->content_type);
    free(header);
}

void free_request(http_request *request) {
    assert(request != NULL);
    if (request->host != NULL)
        str_free(request->host);
    if (request->body != NULL)
        str_free(request->body);
    if (request->method != NULL)
        str_free(request->method);
    if (request->uri != NULL)
        str_free(request->uri);
    if (request->protocol != NULL)
        str_free(request->protocol);
    if (request->header != NULL)
        free_request_header(request->header);
    free(request);
}

void free_response(http_response *response) {
    assert(response != NULL);
    if (response->protocol != NULL)
        str_free(response->protocol);
    if (response->status_code != NULL)
        str_free(response->status_code);
    if (response->status_description != NULL)
        str_free(response->status_description);
    if (response->entity_header != NULL)
        free_entity_header(response->entity_header);
    if (response->body != NULL && response->body->str != NULL)
        str_free(response->body);
    free(response);
}

/**
 * takes a char and calculates its integer value
 * @param c input hex - char
 * @return returns integer value
 */
int hex2int(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    return -1;
}

/**
 * Verarbeitet einen http_request als String und verpackt diesen in einen http_request struct
 * @param str http_request vom Client
 * @return http_request dargestellt im struct
 */
http_request *str_to_http_request(string *str) {
    http_request *req = calloc(1, sizeof(http_request));
    req->header = calloc(1, sizeof(request_header));
    string **split_req = str_split(str, '\n');

    //Set Request Line
    string **request_line = str_split(split_req[0], ' ');
    req->method = request_line[0];
    req->uri = str_decode(request_line[1]);
    if (request_line[2] != NULL) {
        req->protocol = str_cpy(request_line[2]->str, request_line[2]->len);
        str_free(request_line[2]);
    }
    str_free(request_line[1]);
    free(request_line);

    for (int i = 0; split_req[i]; ++i) {
        str_to_lower_case(split_req[i]);
        string *host_str = char_to_string("host:");
        string *user_agent_str = char_to_string("user_agent");

        if (str_start_with(split_req[i], host_str)) {
            string **host = str_split_at_index(split_req[i], (int) host_str->len);
            req->host = host[1];
            str_format(host[1]);
            str_free(host[0]);
            free(host);
        } else if (str_start_with(split_req[i], user_agent_str)) {
            string **ua = str_split_at_index(split_req[i], (int) user_agent_str->len);
            req->header->user_agent = ua[1];
            str_trim(ua[1]);
            str_free(ua[0]);
            free(ua);
        }
        str_free(host_str);
        str_free(user_agent_str);
    }
    //Set body
    for (unsigned int i = 0; split_req[i]; ++i) {
        if (split_req[i]->len > 0 && split_req[i]->str[0] == ' ') {
            size_t start = i + 1;
            for (unsigned int j = 0; j <= i; ++j) {
                start += split_req[j]->len;
            }
            if (start < str->len) {
                req->body = str_cpy(str->str + start, str->len - start);
                break;
            } else {
                req->body = NULL;
                break;
            }
        }
    }
    for (int i = 0; split_req[i]; ++i) {
        str_free(split_req[i]);
    }
    free(split_req);
    return req;
}

/**
 * Returns the given file's content as string struct
 * @param filepath path to file from document root (resources directory)
 * @param len length of the filepath in chars
 */
string *read_file_into_string(char *filepath, unsigned int len) {
    string *doc_root = str_cpy(DOC_ROOT, 13);
    str_cat(doc_root, filepath, len);
    char *c = get_nullterminated_char_str(doc_root);
    FILE *file = fopen(c, "rb");
    //get the file's size
    fseek(file, 0, SEEK_END);
    unsigned long file_size = (unsigned long) ftell(file) - 1;
    fseek(file, 0, SEEK_SET);
    string *str = calloc(1, sizeof(string));
    str->str = calloc(file_size, sizeof(char));
    fread(str->str, sizeof(char), file_size, file);
    str->len = file_size;
    fclose(file);
    free(c);
    str_free(doc_root);
    return str;
}

/**
 * Checks whether or not a file exists in document root (src/resources)
 * @param filepath path to file from document root (resources directory)
 * @return 1 if file is in document root, 2 if there is no file but you are in document root, else 0
 */
short validate_file_access(char *filepath, unsigned int len) {
    short i;
    //build absolute path to file
    string *abs_path = str_cpy(DOC_ROOT, strlen(DOC_ROOT));
    str_cat(abs_path, filepath, len);
    char *c = get_nullterminated_char_str(abs_path);

    //resolve path
    char *resolved = calloc(PATH_MAX, sizeof(char));
    realpath(c, resolved);
    free(abs_path->str);
    abs_path->str = resolved;
    abs_path->len = strlen(resolved);

    //build document root absolute path
    char *doc_root = realpath(DOC_ROOT, NULL);
    if (str_start_with_chars(abs_path, doc_root, (unsigned int) strlen(doc_root)) == 1) {
        char *file = realpath(abs_path->str, NULL);
        if (file != NULL) {
            //file exists in document root
            free(file);
            i = 1;
        } else {
            //file does not exist, but we are in document root (404)
            i = 2;
        }
    } else {
        //left document root, access denied (403)
        i = 0;
    }
    free(c);
    free(doc_root);
    str_free(abs_path);
    return i;
}

/**
 * takes the http_response struct and puts the contents into a string.
 * @param src the http_response struct
 * @return string with the contents of the src struct
 */
string *response_string(http_response *src) {
    string *temp = str_cpy(src->protocol->str, src->protocol->len);
    str_cat(temp, " ", 1);
    str_cat(temp, src->status_code->str, src->status_code->len);
    str_cat(temp, " ", 1);
    str_cat(temp, src->status_description->str, src->status_description->len);
    str_append_new_line(temp);

    if (src->location != NULL && src->location->str != NULL) {
        str_cat(temp, "Location: ", 10);
        str_cat(temp, src->location->str, src->location->len);
        str_append_new_line(temp);
    }
    if (src->entity_header->content_type != NULL && src->entity_header->content_type->str != NULL) {
        str_cat(temp, "Content-Type: ", 14);
        str_cat(temp, src->entity_header->content_type->str, src->entity_header->content_type->len);
        str_append_new_line(temp);
    }
    if (src->body != NULL && src->body->str != NULL) {
        str_cat(temp, "Content-Length: ", 16);
        string *body_len_str = number_to_str(src->body->len);
        str_cat(temp, body_len_str->str, body_len_str->len);
        str_free(body_len_str);
        str_append_new_line(temp);

        //Set body
        str_append_new_line(temp);
        str_cat(temp, src->body->str, src->body->len);
    } else {
        str_cat(temp, "Content-Length: ", 16);
        str_cat(temp, "0", 1);
        str_append_new_line(temp);
    }
    return temp;
}

/**
 * Sets the given statuscode and description in the response-struct
 * Sets the http-version to HTTP/1.1 in the respones-struct
 * @param response Response-struct to be set
 * @param status_code HTTP Statuscode
 * @param status_description HTTP Statuscode description
 */
void set_response_status(http_response *response, string *status_code, string *status_description){
    response->status_code = status_code;
    response->protocol = char_to_string("HTTP/1.1");
    response->status_description = status_description;
}

/**
 * Sets the body and Content-Type in the given struct
 * Content-Lengt is set as the body lengt
 * @param response Response-struct to be set
 * @param body HTTP body
 * @param content_type Body content-type
 */
void set_response_body(http_response *response, string *body, string *content_type){
    response->body = body;
    response->entity_header->content_length = body->len;
    response->entity_header->content_type = content_type;
}

/**
 * Sets basic HTML body with status-code and status-code-description of the struct
 * status-code and description must be set, in the given struct
 * @param response
 */
void set_response_default_html_body(http_response *response) {
    string *body = char_to_string("<!DOCTYPE html><html lang=\"de\"><body><h1>");
    str_cat(body, response->status_code->str, response->status_code->len);
    str_cat(body, " ", 1);
    str_cat(body, response->status_description->str, response->status_description->len);
    str_cat(body, "</h1></body></html>", strlen("</h1></body></html>"));
    set_response_body(response, body, char_to_string("text/html"));
}

/**
 * Returns the content_type of a file by the given file-ending
 * @param ending file-ending z.B. png
 * @return HTTP content-type as string, must be freed
 */
string *get_content_type(string *ending) {
    string *content_type = str_new();
    if (ending != NULL){
        str_to_lower_case(ending);
        if (str_start_with_chars(ending, "png", 3) ||
            str_start_with_chars(ending, "jpg", 3) ||
            str_start_with_chars(ending, "gif", 3) ||
            str_start_with_chars(ending, "jpeg", 4)) {
            str_cat(content_type, "image/", strlen("image/"));
            str_cat(content_type, ending->str, ending->len);
            }
        else if (str_start_with_chars(ending, "pdf", 3) ||
            str_start_with_chars(ending, "js", 2)) {
            str_cat(content_type, "application/", strlen("application/"));
            str_cat(content_type, ending->str, ending->len);
            }
        else if (str_start_with_chars(ending, "html", 4)){
            str_cat(content_type, "text/", strlen("text/"));
            str_cat(content_type, ending->str, ending->len);

        }
        str_free(ending);
    }
    else{
        str_cat(content_type, "application/octet-stream/", strlen("application/octet-stream/"));
    }
    return content_type;
}