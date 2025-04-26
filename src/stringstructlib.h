#ifndef STRING_H
#define STRING_H

typedef struct stringstructlib {
    size_t len;
    char *str;
} string;

void str_cat(string *dest, const char *src, size_t len);

string **str_split(string *str, char separator);

string **str_split_at_index(string *str, int index);

string *str_new(void);

void str_print(string *str);

string *str_cpy(const char *src, size_t len);

void str_free(string *str);

size_t get_length(string *str);

char *get_char_str(string *str);

string *str_decode(string *src);

short str_start_with(string *str1, string *str2);

short str_start_with_chars(string *str, char *c, unsigned int length);

void str_trim(string *str);

int str_cmp(string *str1, string *str2);

void str_replace_with(string *str, char *to_replace, char *replace_with);

string *char_to_string(char *c);

char *get_nullterminated_char_str(string *str);

string *number_to_str(size_t number);

void str_to_lower_case(string *str);

void str_format(string *str);

#endif //STRING_H

