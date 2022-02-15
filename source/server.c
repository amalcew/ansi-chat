#include "headers/server.h"

int main(){
    // TODO: make module more simple and make comments
    user *users = usersFromFile(userPath);
    group *groups = groupsFromFile(groupPath);
    int userQuantity = countLines(userPath);
    int groupQuantity = countLines(groupPath);

    int serverQueue, authQueue;
    int userQueue;
    int groupQueue;
    queuedMessage buf;
    struct msqid_ds msgCtlBuf;

    time_t t;
    time(&t);

    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = catchSignal;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);

    if ((authQueue = msgget(authKey, IPC_CREAT | 0666)) == -1) {
        char *now = getTime();
        printf("%s" ANSI_COLOR_RED " ERROR CREATING AUTHENTICATION QUEUE" ANSI_COLOR_RESET " (error code no. %d):" ANSI_COLOR_RED " %s" ANSI_COLOR_RESET "\n", now, errno, strerror(errno));
        free(now);
        exit(-1);
    } else {
        char *now = getTime();
        printf("%s" ANSI_COLOR_BLUE " GENERATING AUTHENTICATION QUEUE" ANSI_COLOR_RESET " - key: %d, queue_id: %d\n", now, authKey, authQueue);
        free(now);
    }

    if ((serverQueue = msgget(serverKey, IPC_CREAT | 0666)) == -1) {
        char *now = getTime();
        printf("%s" ANSI_COLOR_RED " ERROR CREATING MESSAGE QUEUE" ANSI_COLOR_RESET " (error code no. %d):" ANSI_COLOR_RED " %s" ANSI_COLOR_RESET "\n", now, errno, strerror(errno));
        free(now);
        exit(-1);
    } else {
        char *now = getTime();
        printf("%s" ANSI_COLOR_BLUE " GENERATING MESSAGE QUEUE" ANSI_COLOR_RESET " - key: %d, queue_id: %d\n", now, serverKey, serverQueue);
        free(now);
    }

    for (int i = 0; i < userQuantity; i++) {
        if ((userQueue = msgget(users[i].key, IPC_CREAT | 0666)) == -1) {
            char *now = getTime();
            printf("%s" ANSI_COLOR_RED " ERROR CREATING QUEUE" ANSI_COLOR_RESET " (error code no. %d):" ANSI_COLOR_RED " %s" ANSI_COLOR_RESET "\n", now, errno, strerror(errno));
            free(now);
            exit(-1);
        } else {
            users[i].queue = userQueue;
            char *now = getTime();
            printf("%s" ANSI_COLOR_BLUE " GENERATING USER %s QUEUE" ANSI_COLOR_RESET " - key: %d, queue_id: %d\n", now, users[i].login, users[i].key, users[i].queue);
            free(now);
        }
    }

    for (int i = 0; i < groupQuantity; i++) {
        if ((groupQueue = msgget(groups[i].key, IPC_CREAT | 0666)) == -1) {
            char *now = getTime();
            printf("%s" ANSI_COLOR_RED " ERROR CREATING QUEUE" ANSI_COLOR_RESET " (error code no. %d):" ANSI_COLOR_RED " %s" ANSI_COLOR_RESET "\n", now, errno, strerror(errno));
            free(now);
            exit(-1);
        } else {
            groups[i].queue = groupQueue;
            char *now = getTime();
            printf("%s" ANSI_COLOR_BLUE " GENERATING GROUP %s QUEUE" ANSI_COLOR_RESET " - key: %d, queue_id: %d\n", now, groups[i].name, groups[i].key, groups[i].queue);
            free(now);
        }
    }

    while (running) {
        // user authentication
        if (msgrcv(authQueue, &buf, sizeof buf.msgText, 99, IPC_NOWAIT) != -1) {
            bool success = false;
            int key;
            char delimiters[] = " ,.-:;";
            char* token;
            char* login[32], password[32];

            token = strtok(buf.msgText, delimiters);

            if (strcmp(token, "LOGIN") == 0) {
                token = strtok(NULL, delimiters);
                strcpy(login, token);
                token = strtok(NULL, delimiters);
                strcpy(password, token);

                for (int i = 0; i < userQuantity; i++) {
                    if ((strcmp(users[i].login, login) == 0) && (strcmp(users[i].password, password) == 0)) {
                        key = users[i].key;
                        users[i].active = true;
                        success = true;
                        break;
                    }
                }
                if (success) {
                    sprintf(&buf.msgText, "TRUE:%s:%s:%d", login, password, key);
                    buf.msgType = 98;
                    if (msgsnd(authQueue, &buf, sizeof buf.msgText, 0) == -1) {
                        char *now = getTime();
                        printf("%s" ANSI_COLOR_RED " ERROR SENDING MESSAGE" ANSI_COLOR_RESET " (error code no. %d):" ANSI_COLOR_RED " %s" ANSI_COLOR_RESET "\n", now, errno, strerror(errno));
                        free(now);
                    }
                    char *now = getTime();
                    printf("%s" ANSI_COLOR_GREEN " LOGIN ATTEMPT SUCCESSFUL" ANSI_COLOR_RESET " - passed credentials: %s, %s\n", now, login, password);
                    free(now);
                } else {
                    strcpy(buf.msgText, "FALSE");
                    buf.msgType = 98;
                    if (msgsnd(authQueue, &buf, sizeof buf.msgText, 0) == -1) {
                        char *now = getTime();
                        printf("%s" ANSI_COLOR_RED " ERROR SENDING MESSAGE" ANSI_COLOR_RESET " (error code no. %d):" ANSI_COLOR_RED " %s" ANSI_COLOR_RESET "\n", now, errno, strerror(errno));
                        free(now);
                    }
                    char *now = getTime();
                    printf("%s" ANSI_COLOR_YELLOW " LOGIN ATTEMPT FAILED" ANSI_COLOR_RESET " - passed credentials: %s, %s\n", now, login, password);
                    free(now);
                }
            }
            if (strcmp(token, "LOGOUT") == 0) {
                token = strtok(NULL, delimiters);
                strcpy(login, token);
                for (int i = 0; i < userQuantity; i++) {
                    if (strcmp(users[i].login, login) == 0) {
                        users[i].active = false;
                        break;
                    }
                }
                strcpy(buf.msgText, "OK");
                buf.msgType = 98;
                if (msgsnd(authQueue, &buf, sizeof buf.msgText, 0) == -1) {
                    char *now = getTime();
                    printf("%s" ANSI_COLOR_RED " ERROR SENDING MESSAGE" ANSI_COLOR_RESET " (error code no. %d):" ANSI_COLOR_RED " %s" ANSI_COLOR_RESET "\n", now, errno, strerror(errno));
                    free(now);
                }
                char *now = getTime();
                printf("%s" ANSI_COLOR_GREEN " USER LOGOUT" ANSI_COLOR_RESET " - passed credentials: %s\n", now, login);
                free(now);

            }
        }

        // user-server internal communication
        if (msgrcv(authQueue, &buf, sizeof buf.msgText, 199, IPC_NOWAIT) != -1) {
            char delimiters[] = " ,.-:;";
            char* token;
            token = strtok(buf.msgText, delimiters);

            if (strcmp(token, "LIST_USERS") == 0) {
                strcpy(buf.msgText, "");
                for (int i = 0; i < userQuantity; i++) {
                    strcat(buf.msgText, users[i].login);
                    if (users[i].active) {
                        strcat(buf.msgText, " - active\n");
                    } else {
                        strcat(buf.msgText, " - inactive\n");
                    }
                }
                buf.msgType = 198;
                if (msgsnd(authQueue, &buf, sizeof buf.msgText, 0) == -1) {
                    char *now = getTime();
                    printf("%s" ANSI_COLOR_RED " ERROR SENDING MESSAGE" ANSI_COLOR_RESET " (error code no. %d):" ANSI_COLOR_RED " %s" ANSI_COLOR_RESET "\n", now, errno, strerror(errno));
                    free(now);
                } else {
                    char *now = getTime();
                    printf("%s" ANSI_COLOR_BLUE " SENDING USERS LIST" ANSI_COLOR_RESET "\n", now);
                    free(now);
                }
            }

            if (strcmp(token, "CHAT") == 0) {
                int success = -1;
                int key;
                token = strtok(NULL, delimiters);

                for (int i = 0; i < userQuantity; i++) {
                    if (strcmp(users[i].login, token) == 0) {
                        success = 1;
                        key = users[i].key;
                        break;
                    }
                }

                for (int i = 0; i < groupQuantity; i++) {
                    if (strcmp(groups[i].name, token) == 0) {
                        success = 2;
                        key = groups[i].key;
                        break;
                    }
                }

                if (success > 0) {
                    if (success == 1) sprintf(&buf.msgText, "USER:%d", key);
                    if (success == 2) sprintf(&buf.msgText, "GROUP:%d", key);
                    buf.msgType = comTyp - 1;
                    if (msgsnd(authQueue, &buf, sizeof buf.msgText, 0) == -1) {
                        char *now = getTime();
                        printf("%s" ANSI_COLOR_RED " ERROR SENDING MESSAGE" ANSI_COLOR_RESET " (error code no. %d):" ANSI_COLOR_RED " %s" ANSI_COLOR_RESET "\n", now, errno, strerror(errno));
                        free(now);
                    }
                } else {
                    strcpy(buf.msgText, "FALSE");
                    buf.msgType = comTyp - 1;
                    if (msgsnd(authQueue, &buf, sizeof buf.msgText, 0) == -1) {
                        char *now = getTime();
                        printf("%s" ANSI_COLOR_RED " ERROR SENDING MESSAGE" ANSI_COLOR_RESET " (error code no. %d):" ANSI_COLOR_RED " %s" ANSI_COLOR_RESET "\n", now, errno, strerror(errno));
                        free(now);
                    }
                }
            }
        }

        // message forwarding
        if (msgrcv(serverQueue, &buf, sizeof buf.msgText, 0, IPC_NOWAIT) != -1) {
            char* delimiter = ";";
            char* token;
            token = strtok(buf.msgText, delimiter);


            if (strcmp(token, "USER") == 0) {
                strcpy(buf.msgText, strtok(NULL, delimiter));

                char *now = getTime();
                printf("%s" ANSI_COLOR_BLUE " RECEIVED MESSAGE" ANSI_COLOR_RESET " on queue_id: %d, channel: %d, content: " ANSI_COLOR_MAGENTA "%s" ANSI_COLOR_RESET "\n", now, serverQueue, buf.msgType, buf.msgText);
                free(now);

                if (msgsnd(users[buf.msgType - 1].queue, &buf, sizeof buf.msgText, 0) == -1) {
                    char *now = getTime();
                    printf("%s" ANSI_COLOR_RED " ERROR SENDING MESSAGE" ANSI_COLOR_RESET " (error code no. %d):" ANSI_COLOR_RED " %s" ANSI_COLOR_RESET "\n", now, errno, strerror(errno));
                    free(now);
                } else {
                    char *now = getTime();
                    printf("%s" ANSI_COLOR_BLUE " SENDING MESSAGE" ANSI_COLOR_RESET " to queue_id: %d, channel: %d\n", now, users[buf.msgType - 1].queue, buf.msgType);
                    free(now);
                }
            } else if (strcmp(token, "GROUP") == 0) {
                strcpy(buf.msgText, strtok(NULL, delimiter));
                // delimiter = ":";
                char *now = getTime();
                printf("%s" ANSI_COLOR_BLUE " RECEIVED MESSAGE" ANSI_COLOR_RESET " on queue_id: %d, channel: %d, content: " ANSI_COLOR_MAGENTA "%s" ANSI_COLOR_RESET "\n", now, serverQueue, buf.msgType, buf.msgText);
                free(now);

                buf.msgType = buf.msgType + 100;

                for (int i = 0; i < userQuantity; ++i) {
                    // if (!(strcmp(users[i].login) == 0))
                    if (msgsnd(users[i].queue, &buf, sizeof buf.msgText, 0) == -1) {
                        char *now = getTime();
                        printf("%s" ANSI_COLOR_RED " ERROR SENDING MESSAGE" ANSI_COLOR_RESET " (error code no. %d):" ANSI_COLOR_RED " %s" ANSI_COLOR_RESET "\n", now, errno, strerror(errno));
                        free(now);
                    } else {
                        char *now = getTime();
                        printf("%s" ANSI_COLOR_BLUE " SENDING MESSAGE" ANSI_COLOR_RESET " to queue_id: %d, channel: %d\n", now, users[i].queue, buf.msgType);
                        free(now);
                    }
                }
                if (msgsnd(users[buf.msgType - 1].queue, &buf, sizeof buf.msgText, 0) == -1) {
                    char *now = getTime();
                    printf("%s" ANSI_COLOR_RED " ERROR SENDING MESSAGE" ANSI_COLOR_RESET " (error code no. %d):" ANSI_COLOR_RED " %s" ANSI_COLOR_RESET "\n", now, errno, strerror(errno));
                    free(now);
                } else {
                    char *now = getTime();
                    printf("%s" ANSI_COLOR_BLUE " SENDING MESSAGE" ANSI_COLOR_RESET " to queue_id: %d, channel: %d\n", now, users[buf.msgType - 1].queue, buf.msgType);
                    free(now);
                }
            }
        }
    }

    while (!running) {
        if (msgctl(authQueue, IPC_RMID, &msgCtlBuf) == -1) {
            perror("Error removing queue");
            exit(-1);
        }
        free(users);
        return 0;
    }
}

void catchSignal(int signum) {
    printf("\n" ANSI_COLOR_YELLOW "Exiting gracefully..." ANSI_COLOR_RESET "\n");
    running = false;
}

user* usersFromFile(char* path) {
    FILE * file = fopen(path, "r");
    if(file == NULL) {
        perror("Error");
        exit(-1);
    }
    int howManyUsers = countLines(path);
    int index = 0;
    user *users = malloc(sizeof(user) * howManyUsers);

    char * line = NULL;
    size_t len = 0;
    char delimiters[] = " ,.-:;";
    char* token;

    while ((getline(&line, &len, file)) != -1) {

        token = strtok(line, delimiters);
        if (strcmp(token, "USER") == 0) {
            token = strtok(NULL, delimiters);
            strcpy(users[index].login, token);

            token = strtok(NULL, delimiters);
            strcpy(users[index].password, token);
            strtok(users[index].password, "\n");  // remove "\n" from string

            users[index].key = index + 1;
            users[index].active = false;
            index++;}
    }

    fclose(file);
    if (line) free(line);

    return users;
}

group* groupsFromFile(char* path) {
    FILE * file = fopen(path, "r");
    if(file == NULL) {
        perror("Error");
        exit(-1);
    }
    int howManyGroups = countLines(path);
    int index = 0;
    group *groups = malloc(sizeof(user) * howManyGroups);

    char * line = NULL;
    size_t len = 0;
    char delimiters[] = " ,.-:;";
    char* token;

    while ((getline(&line, &len, file)) != -1) {

        token = strtok(line, delimiters);
        if (strcmp(token, "GROUP") == 0) {
            token = strtok(NULL, delimiters);
            strcpy(groups[index].name, token);
            strtok(groups[index].name, "\n");  // remove "\n" from string

            groups[index].key = index + 101;
            index++;}
    }

    fclose(file);
    if (line) free(line);

    return groups;
}