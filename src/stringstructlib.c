#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "httplib.h"

/**
 * Hängt einen String src mit der Länge len an einen bestehenden String dest an.
 * @param dest An diesen String wird angehängt.
 * @param src Dieser String wird an dest angehängt.
 * @param len Die Länge von src.
 */
void str_cat(string *dest, const char *src, size_t len) {
    char *str = calloc(1, get_length(dest) + len);
    for (unsigned int i = 0; i < get_length(dest); i++) {
        str[i] = get_char_str(dest)[i];
    }
    for (unsigned int i = 0; i < len; i++) {
        str[i + get_length(dest)] = src[i];
    }
    free(dest->str);
    dest->str = str;
    dest->len = len + get_length(dest);
}

/**
 * Trennt den String anhand des Trennzeichens in mehrere Strings auf
 * @param str String der geteilt wird
 * @param seperator Trennzeichen
 * @return String-Array, nullterminiert
 */
string **str_split(string *str, const char seperator) {
    unsigned int counter = 0;
    for (unsigned int i = 0; i < str->len; i++) {
        if (str->str[i] == seperator) {
            counter++;
        }
    }
    string **arr = calloc(counter + 2, sizeof(string *));
    unsigned int links = 0;
    unsigned int rechts = 0;
    int j = 0;
    do {
        while (rechts != str->len && str->str[rechts] != seperator) {
            rechts++;
        }
        arr[j] = str_cpy(str->str + links, rechts - links);
        links = ++rechts;
        j++;
    } while (rechts < str->len - 1);
    arr[counter + 1] = NULL;
    return arr;
}

/**
 * Splits string at specified index in two Strings
 * @param str String to be split
 * @param index split point
 * @return String-Array, length 2
 */
string **str_split_at_index(string *str, const int index) {
    char charAt = str->str[index];
    str->str[index] = '\0';
    string **arr = str_split(str, '\0');
    str->str[index] = charAt;
    return arr;
}

/**
 * Erstellt einen neuen leeren String und gibt das Ergebnis zurück.
 * Im Fehlerfall (z. B. kein Speicher verfügbar), wird das Programm
 * beendet.
 * Dies ist eine "interne" Funktion, die nur innerhalb der HTTPLib
 * aufgerufen werden sollte.
 * @return string* Der neue leere String.
 */
string *str_new(void) {
    string *str = calloc(1, sizeof(string));
    if (str == NULL) {
        exit(2);
    }
    str->str = calloc(1, 1);
    if (str->str == NULL) {
        exit(3);
    }
    str->str[0] = '\0';
    str->len = 0;
    return str;
}

/**
 * Gibt einen String auf stdout aus.
 * @param str Der auszugebende String.
 */
void str_print(string *str) {
    for (unsigned int i = 0; i < str->len; i++) {
        putchar(str->str[i]);
    }
}

/**
 * Wandelt einen char-String src mit einer Länge len um
 * in einen String vom Typ *string. Im Fehlerfall wird das
 * Programm beendet.
 * @param src Der Quell-String.
 * @param len Die Länge des Quell-Strings.
 * @return string* Ein Zeiger auf den neu erzeugten string.
 */
string *str_cpy(const char *src, size_t len) {
    assert(src != NULL);
    string *dest = calloc(1, sizeof(string));
    if (dest == NULL) {
        exit(2);
    }
    dest->str = calloc(len, sizeof(char));
    if (dest->str == NULL) {
        exit(3);
    }
    memcpy(dest->str, src, len);
    dest->len = len;
    return dest;
}

/**
 * Gibt den String str frei.
 * @param str Der freizugebende String.
 */
void str_free(string *str) {
    assert(str != NULL);
    assert(str->str != NULL);
    free(str->str);
    free(str);
}

/**
 * Gibt die Länge des Strings str zurück.
 * @param str Der String.
 * @return size_t die Länge des Strings.
 */
size_t get_length(string *str) {
    assert(str != NULL);
    return str->len;
}

/**
 * Gibt einen Zeiger auf das char-Array des Strings str zurück.
 * Achtung, dieses char-Array ist nicht "null-terminiert"!
 * @param str Der String.
 * @return char* Der Zeiger auf das char-Arrrays des String.
 */
char *get_char_str(string *str) {
    assert(str != NULL);
    return str->str;
}

/**
 * Takes hex coded string and puts the values of the 2 hexdigits into the byte of the char.
 * @param src Codierter String
 * @return Decodierter String
*/
string *str_decode(string *src) {
    assert(src != NULL);
    unsigned int decode_len = 0; // Counter for the length of decoded string
    char *str = calloc(1, get_length(src)); // String for temporally saving the decoded string
    for (unsigned int i = 0; i < get_length(src); i++) {
        // copies source string into decoded string
        if (src->str[i] == '%' && ((i + 2) < get_length(src))) {
            int hi = hex2int(src->str[i + 1]);
            int lo = hex2int(src->str[i + 2]);
            char s = (char) (hi << 4 | lo);
            //the hi int gets shifted 4 bits to the left, so the lo bits can fit into the byte.
            str[decode_len] = s;
            decode_len++;
            i = i + 2; // skips the already decoded part of src string
        } else {
            str[decode_len] = src->str[i]; // Copys char without decoding
            decode_len++;
        }
    }
    str = realloc(str, decode_len);
    assert(str != NULL);
    string *dest = calloc(1, sizeof(string)); // storing the decoded string in string-struct
    dest->str = str;
    dest->len = decode_len;
    return dest;
}

/**
 * Checks if one String includes other String
 * @param str1 String to compare
 * @param str2 String which may be included in the other String
 * @return 1 if true 0 if false
 */
short str_start_with(string *str1, string *str2) {
    if (str1->len < str2->len) {
        return 0;
    }
    for (unsigned int i = 0; i < str1->len && i < str2->len; ++i) {
        if (str1->str[i] != str2->str[i]) {
            return 0;
        }
    }
    return 1;
}

/**
 * Checks if one String includes other char*
 * @param str String to compare
 * @param c char* wich may be included in the other String
 * @return 1 if true, 0 if false
 */
short str_start_with_chars(string *str, char *c, unsigned int length) {
    string *str2 = calloc(1, sizeof(string));
    str2->str = c;
    str2->len = length;
    short result = str_start_with(str, str2);
    free(str2);
    return result;
}

/**
 * Removes Space at first and last index, if any
 * @param str string to be trimmed
 */
void str_trim(string *str) {
    unsigned short start = str->str[0] == ' ' ? 1 : 0;
    unsigned short end = str->str[str->len - 1] == ' ' ? 1 : 0;
    char *new_str = calloc(str->len - start - end, sizeof(char));
    for (unsigned int i = 0; i < (str->len - start - end); ++i) {
        new_str[i] = str->str[i + start];
    }
    free(str->str);
    str->str = new_str;
    str->len -= start + end;
}

/** str_cmp implemented for string-struct without /0 Terminator
 * @param str1 first string to compare
 * @param str2 second string to compare
 * @return 0 if both strings are equal, -1 if not
 */
int str_cmp(string *str1, string *str2) {
    assert(str1 != NULL);
    assert(str2 != NULL);
    if (str1->len != str2->len) {
        return -1;
    }
    for (unsigned int i = 0; i < str1->len; i++) {
        if (str1->str[i] != str2->str[i]) {
            return -1;
        }
    }
    return 0;
}

/**
 * Replaces a given char sequence in a given string struct with another given char sequence
 * @param str the string struct in which you want to replace something in
 * @param to_replace the char sequence to replace
 * @param replace_with the char sequence to replace the to_replace char sequence with
 */
void str_replace_with(string *str, char *to_replace, char *replace_with) {
    unsigned int i;
    for (i = 0; i < str->len; i++) {
        //traversing the given string to see if the left part of it starts with to_replace
        string **pivot = str_split_at_index(str, (int) i);
        if (pivot[1] != NULL && str_start_with_chars(pivot[1], to_replace, (unsigned int) strlen(to_replace))) {
            //save the current pivot element, because it was removed from the string by splitting it
            char *char1 = calloc(1, sizeof(char));
            char1[0] = str->str[i];

            //start building the new string with the part left of the pivot element, the pivot element and replace_with
            string *builder = str_new();
            str_cat(builder, pivot[0]->str, pivot[0]->len);
            str_cat(builder, char1, sizeof(char));
            str_cat(builder, replace_with, strlen(replace_with));

            //now get the remaining chars behind to_replace and concat them, if any
            string **remaining = str_split_at_index(pivot[1], (int) strlen(to_replace));
            char *char2 = calloc(1, sizeof(char));
            char2[0] = str->str[i + strlen(to_replace) + 1];
            str_cat(builder, char2, sizeof(char));
            if (remaining[1] != NULL) {
                str_cat(builder, remaining[1]->str, remaining[1]->len);
                str_free(remaining[1]);
            }
            free(str->str);
            str->str = builder->str;
            str->len = builder->len;
            free(builder);
            str_free(remaining[0]);
            free(remaining);
            free(char2);
            free(char1);
        }
        if (pivot[1] != NULL)
            str_free(pivot[1]);
        str_free(pivot[0]);
        free(pivot);
    }
}

/**
 * Returns a char sequence as string struct
 * USE-Case: warp a hardcoded String with string-struct
 * @param c null-terminated char array
 * @return string containing c, must be freed
 */
string *char_to_string(char *c) {
    string *str = calloc(1, sizeof(string));
    size_t size = strlen(c);
    str->str = calloc(size, sizeof(char));
    memcpy(str->str, c, size);
    str->len = size;
    return str;
}

/**
 * Returns the char sequence of a string struct and adds a null terminator
 * @param str
 * @return terminated_char
 */
char *get_nullterminated_char_str(string *str) {
    char *terminated_char = calloc(get_length(str) + 1, sizeof(char));
    for (unsigned int i = 0; i < get_length(str); i++) {
        terminated_char[i] = get_char_str(str)[i];
    }
    terminated_char[get_length(str)] = '\0';
    return terminated_char;
}

/**
 * Converts number into a string
 * @param number unsigned number (size_t)
 * @return number as string
 */
string *number_to_str(size_t number) {
    size_t temp = number;
    int i;
    for (i = 0; temp > 0; i++) {
        temp /= 10;
    }
    string *ret = calloc(1, sizeof(string));
    ret->len = (unsigned) i;
    char *str = calloc((unsigned) i, sizeof(char));
    for (i--; i >= 0; i--) {
        str[i] = (char) ((int) number % 10 + '0');
        number /= 10;
    }
    ret->str = str;
    return ret;
}

/**
 * replaces capital letters in str with lower case
 * @param str
 */
void str_to_lower_case(string *str) {
    for (unsigned int i = 0; i < get_length(str); i++) {
        if (str->str[i] >= 'A' && str->str[i] <= 'Z') {
            str->str[i] = str->str[i] + ('a' - 'A');
        }
    }
}

/**
 * Removes all Spaces and newlines from a String, original String is modified
 * @param str  input string
 */
void str_format(string *str) {
    unsigned int spaces = 0;
    for (unsigned int i = 0; i < str->len; ++i) {
        if ((str->str[i] == ' ') || (str->str[i] == '\r') || (str->str[i] == '\n')) {
            spaces++;
        }
    }
    char *new_str = calloc(str->len-spaces, sizeof(char));

    unsigned int new_str_index = 0;
    for (unsigned int i = 0; i < str->len; ++i) {
        if ((str->str[i] != ' ') && (str->str[i] != '\r') && (str->str[i] != '\n')) {
            new_str[new_str_index++] = str->str[i];
        }
    }
    free(str->str);
    str->str = new_str;
    str->len = str->len-spaces;
}