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

    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = catchSignal;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);

    if ((authQueue = msgget(authKey, IPC_CREAT | 0666)) == -1) {
        logError("CREATING AUTH QUEUE", errno);
        exit(-1);
    } else {
        logAction("INFO", "initializing auth queue", "");
        printf(" (key=%d, queue_id=%d)\n", authKey, authQueue);
    }

    if ((serverQueue = msgget(serverKey, IPC_CREAT | 0666)) == -1) {
        logError("CREATING MESSAGE QUEUE", errno);
        exit(-1);
    } else {
        logAction("INFO", "initializing message queue", "");
        printf(" (key=%d, queue_id=%d)\n", serverKey, serverQueue);
    }

    for (int i = 0; i < userQuantity; i++) {
        if ((userQueue = msgget(users[i].key, IPC_CREAT | 0666)) == -1) {
            logError("CREATING USER QUEUE", errno);
            exit(-1);
        } else {
            users[i].queue = userQueue;
            logAction("INFO", "initializing user queue", "");
            printf(" (user=%s, key=%d, queue_id=%d)\n", users[i].login, users[i].key, users[i].queue);
        }
    }

    for (int i = 0; i < groupQuantity; i++) {
        if ((groupQueue = msgget(groups[i].key, IPC_CREAT | 0666)) == -1) {
            logError("CREATING GROUP QUEUE", errno);
            exit(-1);
        } else {
            groups[i].queue = groupQueue;
            logAction("INFO", "initializing group queue", "");
            printf(" (group=%s, key=%d, queue_id=%d)\n", groups[i].name, groups[i].key, groups[i].queue);
            for (int j = 0; j < userQuantity; j++) {
                groups[i].subscriptions[j] = 0;
            }
            logAction("INFO", "initializing group subscription list", "");
            printf(" (group=%s)\n", groups[i].name);
        }
    }

    while (running) {

        // user authentication
        if (msgrcv(authQueue, &buf, sizeof buf.msgText, 99, IPC_NOWAIT) != -1) {
            buf.msgType = authTyp - 1;

            bool success = false;
            key_t key;

            char mode[32], login[32], password[32];

            strcpy(mode, strtok(buf.msgText, delimiter));

            if (strcmp(mode, "LOGIN") == 0) {
                strcpy(login, strtok(NULL, delimiter));
                strcpy(password, strtok(NULL, delimiter));

                // iterate through user structure to compare login/password
                for (int i = 0; i < userQuantity; i++) {
                    // if credentials found, extract key and continue
                    if ((strcmp(users[i].login, login) == 0) && (strcmp(users[i].password, password) == 0)) {
                        key = users[i].key;
                        users[i].active = true;
                        success = true;
                        break;
                    }
                }
                if (success) {
                    sprintf(buf.msgText, "TRUE:%s:%s:%d", login, password, key);

                    if (msgsnd(authQueue, &buf, sizeof buf.msgText, 0) == -1) {
                        logError("SENDING CREDENTIALS", errno);
                    }

                    logAction("AUTH", "user login", "");
                    printf("(login=%s)\n", login);
                } else {
                    strcpy(buf.msgText, "FALSE");

                    if (msgsnd(authQueue, &buf, sizeof buf.msgText, 0) == -1) {
                        logError("SENDING CREDENTIALS", errno);
                    }

                    logAction("AUTH", "invalid login attempt", "");
                    printf("(login=%s, password=%s)\n", login, password);
                }
            }

            if (strcmp(mode, "LOGOUT") == 0) {
                strcpy(login, strtok(NULL, delimiter));

                // iterate through user structure to compare login
                for (int i = 0; i < userQuantity; i++) {
                    if (strcmp(users[i].login, login) == 0) {
                        users[i].active = false;
                        break;
                    }
                }

                logAction("AUTH", "user logout", "");
                printf("(login=%s)\n", login);
            }
        }

        // user-server internal communication
        if (msgrcv(authQueue, &buf, sizeof buf.msgText, 199, IPC_NOWAIT) != -1) {
            buf.msgType = comTyp - 1;

            bool success = false;

            char mode[32], login[32], search[32];

            strcpy(mode, strtok(buf.msgText, delimiter));
            strcpy(login, strtok(NULL, delimiter));

            if (strcmp(mode, "LIST_USERS") == 0) {
                // prepare response
                strcpy(buf.msgText, "");
                for (int i = 0; i < userQuantity; i++) {
                    strcat(buf.msgText, users[i].login);
                    if (users[i].active) {
                        strcat(buf.msgText, ANSI_COLOR_GREEN " - online\n" ANSI_COLOR_RESET);
                    } else {
                        strcat(buf.msgText, ANSI_COLOR_RED " - offline\n" ANSI_COLOR_RESET);
                    }
                }

                if (msgsnd(authQueue, &buf, sizeof buf.msgText, 0) == -1) logError("SENDING USERS LIST", errno);
                else logAction("REQUEST", "users lists requested", login);
            }

            if (strcmp(mode, "LIST_GROUPS") == 0) {
                // prepare response
                strcpy(buf.msgText, "");
                for (int i = 0; i < groupQuantity; i++) {
                    strcat(buf.msgText, groups[i].name);
                    strcat(buf.msgText, " - ");
                    strcat(buf.msgText, groups[i].description);
                    strcat(buf.msgText, "\nUsers subscribed to this group: ");
                    for (int j = 0; j < userQuantity; j++) {
                        if (groups[i].subscriptions[j] == 1) {
                            strcat(buf.msgText, users[j].login);
                            strcat(buf.msgText, ", ");
                        }
                    }
                    strcat(buf.msgText, "\n\n");
                }

                if (msgsnd(authQueue, &buf, sizeof buf.msgText, 0) == -1) logError("SENDING GROUPS LIST", errno);
                else logAction("REQUEST", "groups lists requested", login);
            }

            if (strcmp(mode, "CHAT") == 0) {
                strcpy(search, strtok(NULL, delimiter));

                // iterate through user structure to compare login with search
                for (int i = 0; i < userQuantity; i++) {
                    if (strcmp(users[i].login, search) == 0) {
                        success = true;
                        sprintf(buf.msgText, "USER:%d", users[i].key);
                        break;
                    }
                }

                // iterate through group structure to compare name with search
                for (int i = 0; i < groupQuantity; i++) {
                    if (strcmp(groups[i].name, search) == 0) {
                        // iterate through user structure to verify if user is subscribed to the group
                        for (int j = 0; j < userQuantity; j++) {
                            if (strcmp(users[j].login, login) == 0) {
                                if (groups[i].subscriptions[j] == 1) {
                                    sprintf(buf.msgText, "GROUP:%d", groups[i].key);
                                } else {
                                    strcpy(buf.msgText, "NOT_SUBSCRIBED");
                                }
                                break;
                            }
                        }
                        success = true;
                        break;
                    }
                }

                if (success) {
                    if (msgsnd(authQueue, &buf, sizeof buf.msgText, 0) == -1) logError("SENDING VERIFICATION", errno);
                    else logAction("REQUEST", "user/group verification", login);
                } else {
                    strcpy(buf.msgText, "FALSE");

                    if (msgsnd(authQueue, &buf, sizeof buf.msgText, 0) == -1) logError("SENDING VERIFICATION", errno);
                    else logAction("REQUEST", "user/group verification", login);
                }
            }

            if (strcmp(mode, "SUBSCRIBE") == 0) {
                strcpy(search, strtok(NULL, delimiter));

                // iterate through group structure to compare name with search
                for (int i = 0; i < groupQuantity; i++) {
                    if (strcmp(groups[i].name, search) == 0) {
                        success = true;
                        strcpy(buf.msgText, "TRUE");
                        for (int j = 0; j < userQuantity; j++) {
                            if (strcmp(users[j].login, login) == 0) {
                                groups[i].subscriptions[j] = 1;
                                break;
                            }
                        }
                        break;
                    }
                }

                if (success) {
                    if (msgsnd(authQueue, &buf, sizeof buf.msgText, 0) == -1) logError("SENDING VERIFICATION", errno);
                    else logAction("REQUEST", "subscription to group chat", login);
                } else {
                    strcpy(buf.msgText, "FALSE");;

                    if (msgsnd(authQueue, &buf, sizeof buf.msgText, 0) == -1) logError("SENDING VERIFICATION", errno);
                }
            }

            if (strcmp(mode, "UNSUBSCRIBE") == 0) {
                strcpy(search, strtok(NULL, delimiter));

                // iterate through group structure to compare name with search
                for (int i = 0; i < groupQuantity; i++) {
                    if (strcmp(groups[i].name, search) == 0) {
                        success = true;
                        strcpy(buf.msgText, "TRUE");
                        for (int j = 0; j < userQuantity; j++) {
                            if (strcmp(users[j].login, login) == 0) {
                                groups[i].subscriptions[j] = 0;
                                break;
                            }
                        }
                        break;
                    }
                }

                if (success) {
                    if (msgsnd(authQueue, &buf, sizeof buf.msgText, 0) == -1) logError("SENDING VERIFICATION", errno);
                    else logAction("REQUEST", "unsubscription to group chat", login);
                } else {
                    strcpy(buf.msgText, "FALSE");;

                    if (msgsnd(authQueue, &buf, sizeof buf.msgText, 0) == -1) logError("SENDING VERIFICATION", errno);
                }
            }
        }

        // message forwarding
        if (msgrcv(serverQueue, &buf, sizeof buf.msgText, 0, IPC_NOWAIT) != -1) {
            char mode[32], who[32], toWho[32];
            char *content = NULL;

            strcpy(mode, strtok(buf.msgText, delimiter));
            strcpy(who, strtok(NULL, delimiter));
            strcpy(toWho, strtok(NULL, delimiter));
            content = strtok(NULL, delimiter);

            if (strcmp(mode, "USER") == 0) {
                logAction("FORWARD", "message received", "");
                printf(" (from_who=%s, queue_id=%d, key=%ld)\n", who, serverQueue, buf.msgType);

                // prepare message
                sprintf(buf.msgText, ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET ": %s", who, content);

                if (msgsnd(users[buf.msgType - 1].queue, &buf, sizeof buf.msgText, 0) == -1) {
                    logError("SENDING MESSAGE", errno);
                } else {
                    logAction("FORWARD", "message sent", "");
                    printf(" (to_who=%s, queue_id=%d, key=%ld)\n", toWho, users[buf.msgType - 1].queue, buf.msgType);
                }

            } else if (strcmp(mode, "GROUP") == 0) {
                logAction("FORWARD", "message received", "");
                printf(" (from_who=%s, queue_id=%d, key=%ld)\n", who, serverQueue, buf.msgType);

                // prepare message
                sprintf(buf.msgText, ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET ": %s", who, content);
                buf.msgType = buf.msgType + 100;

                for (int i = 0; i < userQuantity; ++i) {
                    if (!(strcmp(users[i].login, who) == 0)) {
                        if (msgsnd(users[i].queue, &buf, sizeof buf.msgText, 0) == -1) logError("SENDING MESSAGE", errno);
                    }
                }
                logAction("FORWARD", "message sent", "");
                printf(" (to_who=%s, queue_id=%d, key=%ld)\n", toWho, users[buf.msgType - 1].queue, buf.msgType);
            }
        }
    }

    while (!running) {
        if (msgctl(authQueue, IPC_RMID, &msgCtlBuf) == -1) {
            perror("Error removing queue");
            exit(-1);
        }
        if (users) free(users);
        if (groups) free(groups);
        return 0;
    }
}

void catchSignal(int signum) {
    printf("\n" ANSI_COLOR_YELLOW "Exiting gracefully..." ANSI_COLOR_RESET "\n");
    running = false;
}

void logError(char* desc, int errorNum) {
    char *now = getTime();
    printf("%s" ANSI_COLOR_RED " ERROR %s" ANSI_COLOR_RESET ": %s\n", now, desc, strerror(errorNum));
    if (now) free(now);
}

void logAction(char* type, char* desc, char* cred) {
    char *now = getTime();
    printf("%s [", now);
    if (strcmp(type, "REQUEST") == 0) {
        printf(ANSI_COLOR_BLUE "%s" ANSI_COLOR_RESET, type);
        printf("] client: %s (%s)\n", cred, desc);
    }
    if (strcmp(type, "INFO") == 0) {
        printf(ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET, type);
        printf("] %s", desc);
    }
    if (strcmp(type, "AUTH") == 0) {
        printf(ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET, type);
        printf("] %s %s", desc, cred);
    }

    if (strcmp(type, "FORWARD") == 0) {
        printf(ANSI_COLOR_CYAN "%s" ANSI_COLOR_RESET, type);
        printf("] %s", desc);
    }

    if (now) free(now);
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

    while ((getline(&line, &len, file)) != -1) {
        if (strcmp(strtok(line, delimiter), "USER") == 0) {
            strcpy(users[index].login, strtok(NULL, delimiter));
            strcpy(users[index].password, strtok(NULL, delimiter));
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
    group *groups = malloc(sizeof(group) * howManyGroups);

    char * line = NULL;
    size_t len = 0;

    while ((getline(&line, &len, file)) != -1) {
        if (strcmp(strtok(line, delimiter), "GROUP") == 0) {
            strcpy(groups[index].name, strtok(NULL, delimiter));
            strcpy(groups[index].description, strtok(NULL, delimiter));
            strtok(groups[index].description, "\n");  // remove "\n" from string

            groups[index].key = index + 101;
            index++;}
    }

    fclose(file);
    if (line) free(line);

    return groups;
}