#ifndef ANSI_IRC_USER_H
#define ANSI_IRC_USER_H

#define BUFF_SIZE 256

typedef struct userStructure {
    char login[BUFF_SIZE];
    char password[BUFF_SIZE];
    int key;
    int queue;
    bool active;
} user;

user* authenticate();
void deauthenticate(user* usr);
user* importFromFile(char* path);

#endif //ANSI_IRC_USER_H