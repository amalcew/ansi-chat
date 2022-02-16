#ifndef ANSI_IRC_GROUP_H
#define ANSI_IRC_GROUP_H

#define BUFF_SIZE 256

typedef struct groupStructure {
    char name[BUFF_SIZE];
    char description[BUFF_SIZE];
    key_t key;
    int queue;
    int subscriptions[32];
} group;

group* groupsFromFile(char* path);

#endif //ANSI_IRC_GROUP_H
