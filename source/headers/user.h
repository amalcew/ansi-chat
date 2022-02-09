#ifndef ANSI_IRC_USER_H
#define ANSI_IRC_USER_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aux.h"

#define BUFF_SIZE 256

typedef struct userStructure {
    char login[BUFF_SIZE];
    char password[BUFF_SIZE];
    int* key;
    int* queue;
} user;

user* importFromFile(char* path);
bool authenticate(char* content);
#endif //ANSI_IRC_USER_H