//
// Created by hplp739 on 15.09.22.
//
#include "../headers/strlib.h"

int main(int argc, const char *argv[]) {
    if (argc < 1) {
        fprintf(stderr, "Arguments missing\n");
        return 1;
    }
    char *filename;
    if (read_line(&filename) <= 1) {
        fprintf(stderr, "%s cannot read filename\n", argv[0]);
        return 1;
    }
    FILE *fp;
    fp = fopen(filename, "w");
    if (fp == NULL) {
        fprintf(stderr,"%s cannot open file: %s\n", argv[0], filename);
        return 1;
    }
    free(filename);
    filename = NULL;
    char *line;
    int i = 2;
    while ((i = read_line(&line)) > 1) {
        for (int j = 0; j < i - 1; ++j) {
            if (is_vowel(line[j])) continue;
            putc(line[j], fp);
        }
        putc('\n', fp);
        free(line);
    }
    free(line);
    fclose(fp);
    return 0;
}
