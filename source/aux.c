#include "headers/aux.h"

int countLines(char* path) {
    FILE * file = fopen(path, "r");
    if(file == NULL) {
        perror("Error");
        exit(-1);
    }

    int lines = 0;
    int ch=0;
    while(!feof(file)) {
        ch = fgetc(file);
        if(ch == '\n') lines++;
    }
    fclose(file);
    return lines;
}

char * getTime() {
    char *buff = malloc(21);
    time_t now = time (0);
    strftime (buff, 100, "[%Y-%m-%d %H:%M:%S]", localtime (&now));
    return buff;
}