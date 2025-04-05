#include <kstring.h>




char *strtok(char *str, const char *delim) {
    char *save = NULL, *token_start = NULL, *token_end = NULL;

    if (str != NULL) {
        save = str;
    } else if (save == NULL) {
        return NULL;
    }

    token_start = save;

    while(*token_start != '\0' && strchr(delim, *token_start) != NULL) {
        token_start++;
    }

    if (*token_start == '\0') {
        save = NULL;
        return NULL;
    }

    token_end = token_start;

    while(*token_end != '\0' && strchr(delim, *token_end) == NULL) {
        token_end++;
    }

    if (*token_end == '\0') {
        save = NULL;
    } else {
        *token_end = '\0';
        save = token_end + 1;
    }

    return token_start;
}