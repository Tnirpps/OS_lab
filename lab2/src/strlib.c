//
// Created by hplp739 on 16.09.22.
//

#include "../headers/strlib.h"

int read_line(char **ptr) {
    int capacity = 10;
    (*ptr) = malloc(sizeof (char) * capacity);
    // cannot malloc memory
    if ((*ptr) == NULL) {
        return -1;
    }
    char *new_ptr; // tmp pointer to expand string in future
    char c;
    int i = 0;     //  string size
    while ((c = getchar()) != EOF && c != '\0' && c != '\n') {
        (*ptr)[i++] = c;
        // expend string
        if (i >= capacity / 2) {
            new_ptr = malloc(sizeof (char) * capacity * 2);
            // cannot malloc new memory
            if (new_ptr == NULL) {
                return -1;
            }
            // copy string to new pointer
            for (int j = 0; j < i; ++j) {
                new_ptr[j] = (*ptr)[j];
            }
            // clear old data
            free((*ptr));
            (*ptr) = new_ptr;
            capacity *= 2;
        }

    }
    (*ptr)[i++] = '\0';
    return i;
}

int is_vowel(const char c) {
    return (int) (
            c == 'a' ||
            c == 'e' ||
            c == 'u' ||
            c == 'o' ||
            c == 'i');
}
