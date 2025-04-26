#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../src/httplib.h"

static void str_cat_test_helloworld(void);

static void str_decode_test_space(void);

static void str_decode_test_doublespace(void);

static void str_decode_test_einmalmitalles(void);

static void split_str_test(void);

static void split_str_at_index_test(void);

static void str_start_with_test(void);

static void str_trim_test(void);

static void str_to_http_request_test(void);

static void str_replace_with_test(void);

static void read_file_into_string_test(void);

static void validate_file_access_test(void);

static void response_string_test(void);

static void add_null_terminator_test(void);

static void str_to_lower_case_test(void);

static void str_format_test(void);

static void str_equals_test(void);

int main(void) {
    str_cat_test_helloworld();
    str_decode_test_space();
    str_decode_test_doublespace();
    str_decode_test_einmalmitalles();
    split_str_test();
    split_str_at_index_test();
    str_start_with_test();
    str_trim_test();
    str_to_http_request_test();
    str_replace_with_test();
    response_string_test();
    read_file_into_string_test();
    validate_file_access_test();
    add_null_terminator_test();
    str_to_lower_case_test();
    str_format_test();
    str_equals_test();
    printf("INFO in file %s, line %d: All httplib tests passed successfully.\n", __FILE__, __LINE__);
    return 0;
}

static void str_cat_test_helloworld(void) {
    const char *str1 = "Hello ";
    const char *str2 = "World!";
    const char *str3 = "Hello World!";
    string *proof = str_cpy(str3, 12);
    string *dest = str_cpy(str1, 6);
    str_cat(dest, str2, 6);
    assert(str_cmp(dest, proof) == 0);
    assert(dest->len == 12);
    str_free(dest);
    str_free(proof);
}

static void str_decode_test_space(void) {
    const char *str1 = "We%20love%20space!";
    const char *str2 = "We love space!";
    string *proof = str_cpy(str2, 14);
    string *dest = str_cpy(str1, 18);
    string *test = str_decode(dest);
    assert(str_cmp(test, proof) == 0);
    str_free(dest);
    str_free(test);
    str_free(proof);
}

static void str_decode_test_doublespace(void) {
    const char *str1 = "We%20%20love%20space!";
    const char *str2 = "We  love space!";

    string *dest = str_cpy(str1, 21);
    string *test = str_decode(dest);
    string *proof = str_cpy(str2, 15);
    assert(str_cmp(test, proof) == 0);
    str_free(dest);
    str_free(test);
    str_free(proof);
}

static void str_decode_test_einmalmitalles(void) {
    const char *str1 = "%20%22%25%2D%2E%3C%3E%5C%5F%5E%60%7B%7C%7D%7E";
    const char *str2 = " \"%-.<>\\_^`{|}~";
    string *dest = str_cpy(str1, 45);
    string *test = str_decode(dest);
    string *proof = str_cpy(str2, 15);
    assert(str_cmp(test, proof) == 0);
    str_free(dest);
    str_free(test);
    str_free(proof);
}

static void split_str_test(void) {
    char *c = "Hallo Test 123 47";
    string *str = str_cpy(c, 17);
    string **arr = str_split(str, ' ');
    assert(arr[0]->len == 5);
    assert(arr[1]->len == 4);
    assert(arr[2]->len == 3);
    assert(arr[3]->len == 2);

    str_free(str);
    for (int i = 0; arr[i]; ++i) {
        str_free(arr[i]);
    }
    free(arr);
}

static void split_str_at_index_test(void) {
    char *c = "Hallo Test";
    string *str = str_cpy(c, strlen(c));
    string **arr = str_split_at_index(str, 5);

    assert(arr[0]->len == 5);
    assert(arr[1]->len == 4);
    assert(c[5] == ' ');

    str_free(str);
    for (int i = 0; arr[i]; ++i) {
        str_free(arr[i]);
    }
    free(arr);
}

static void str_start_with_test(void) {
    char *c1 = "Starte mit mir";
    string *str1 = str_cpy(c1, strlen(c1));
    char *c2 = "Start";
    string *str2 = str_cpy(c2, strlen(c2));
    char *c3 = "Hallo";
    string *str3 = str_cpy(c3, strlen(c3));

    assert(str_start_with(str1, str2) == 1);
    assert(str_start_with(str1, str3) == 0);

    str_free(str1);
    str_free(str2);
    str_free(str3);
}

static void str_trim_test(void) {
    char *c = " Hallo ";
    string *str = str_cpy(c, strlen(c));

    str_trim(str);
    assert(str->str[0] != ' ');
    assert(str->str[str->len - 1] != ' ');

    str_free(str);
}

static void str_to_http_request_test(void) {
    char *example_request = "GET / HTTP/1.1\r\nHost: localhost:31337\r\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:137.0) Gecko/20100101 Firefox/137.0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nAccept-Language: en-US,en;q=0.7,de;q=0.3\r\nAccept-Encoding: gzip, deflate, br, zstd\r\nConnection: keep-alive\r\n \r\ntest hallo 123";
    string *str = str_cpy(example_request, strlen(example_request));
    http_request *req = str_to_http_request(str);

    assert(req->body != NULL && req->body->len == 14);

    str_free(str);
    free_request(req);
}

static void str_replace_with_test(void) {
    char *c1 = "Hello to Earth!";
    char *c2 = "Hello World!";
    string *str1 = str_cpy(c1, strlen(c1));
    string *str2 = str_cpy(c2, strlen(c2));
    str_replace_with(str1, "to Earth", "World");
    assert(str_cmp(str1, str2) == 0);
    str_free(str2);
    str_free(str1);
}

static void read_file_into_string_test(void) {
    char *uri = "/images/tux.jpg";
    string *file = read_file_into_string(uri, (unsigned int) strlen(uri));
    //Assert filesize of 9,882 kB
    assert(file->len == 9882);
    str_free(file);
}

static void validate_file_access_test(void) {
    char *c1 = "index.html";
    char *c2 = "../test/resources/test.txt";
    char *c3 = "a.txt";
    assert(validate_file_access(c1, (unsigned int) strlen(c1)) == 1);
    assert(validate_file_access(c2, (unsigned int) strlen(c2)) == 0);
    assert(validate_file_access(c3, (unsigned int) strlen(c3)) == 2);
}

static void response_string_test(void) {
    http_response *test_respo = calloc(1, sizeof(http_response));
    test_respo->protocol = str_cpy("HelloWorld!", 11);;
    test_respo->status_code = str_cpy("http", 4);
    test_respo->status_description = str_cpy("Heinz", 5);
    test_respo->entity_header = calloc(1, sizeof(entity_header));
    test_respo->entity_header->content_type = str_cpy("StarWars", 8);
    test_respo->body = str_cpy("Hello World!", 12);

    string *test22 = response_string(test_respo);
    char *c = "HelloWorld! http Heinz\r\nContent-Type: StarWars\r\nContent-Length: 12\r\n\r\nHello World!";
    string *test23 = str_cpy(c, strlen(c));
    assert (str_cmp(test22, test23) == 0);

    str_free(test22);
    str_free(test23);
    free_response(test_respo);
}

static void add_null_terminator_test(void) {
    char *c1 = malloc(6 * sizeof(char));
    c1[0] = 'H';
    c1[1] = 'e';
    c1[2] = 'l';
    c1[3] = 'l';
    c1[4] = '0';
    c1[5] = '9';
    string *str = str_cpy(c1, 5);
    char *c2 = get_nullterminated_char_str(str);
    assert(c2[5] == '\0');
    free(c2);
    str_free(str);
    free(c1);
}

static void str_to_lower_case_test(void) {
    char *c1 = "Hello to Earth!";
    char *c2 = "hello to earth!";
    string *str1 = str_cpy(c1, strlen(c1));
    string *str2 = str_cpy(c2, strlen(c2));
    str_to_lower_case(str1);
    assert(str_cmp(str1, str2) == 0);

    str_free(str1);
    str_free(str2);
}

static void str_format_test(void) {
    string *unformatted = char_to_string(" hallo test \r\n was geht !!!     ");
    string *formatted = char_to_string("hallotestwasgeht!!!");

    str_format(unformatted);
    assert(str_cmp(unformatted, formatted) == 0);

    str_free(unformatted);
    str_free(formatted);
}

static void str_equals_test(void) {
    string *s1 = char_to_string("ABCDE");
    string *s2 = char_to_string("ABCD");
    assert(str_equals(s1,s2)==0);
    assert(str_equals(s1,s1));
}