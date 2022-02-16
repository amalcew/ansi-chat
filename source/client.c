#include "headers/client.h"

int main(){
    while (true) {
        char command[32];
        user *usr = authenticate();  // authenticate  the user

        printf(ANSI_COLOR_GREEN "Welcome %s!\n" ANSI_COLOR_RESET "type '?help' to get the list of commands\n", usr->login);
        while (true) {  // main loop
            printf(ANSI_COLOR_RESET ": " ANSI_COLOR_RESET);
            fgets(command, sizeof(command), stdin);  // read command from stdin
            strtok(command, "\n");  // remove '\n' from command

            if (strcmp(command, "?help") == 0) {
                printHelp();

            } else if (strcmp(command, "?vars") == 0) {
                printVars(usr);

            } else if (strcmp(command, "?chat") == 0) {
                chat(usr);

            } else if (strcmp(command, "?subscribe") == 0) {
                subscribe(usr);

            } else if (strcmp(command, "?unsubscribe") == 0) {
                unsubscribe(usr);

            } else if (strcmp(command, "?list-users") == 0) {
                printUsers(usr);

            } else if (strcmp(command, "?list-groups") == 0) {
                printGroups(usr);

            } else if (strcmp(command, "?leave") == 0) {
                deauthenticate(usr);
                break;

            } else if (strcmp(command, "?exit") == 0) {
                printf("Exiting...\n");
                deauthenticate(usr);
                return 0;

            } else {
                printf("Unknown command, type '?help' to get a list of commands\n");
            }
        }
    }
}

void printHelp() {
    printf("?help - prints this commands list\n");
    printf("?vars - prints current session variables\n");
    printf("?chat - select the users to chat\n");
    printf("?subscribe - select group to receive access\n");
    printf("?unsubscribe - drop access to selected group\n");
    printf("?list-users - prints users list\n");
    printf("?list-groups - prints groups list\n");
    printf("?leave - close current session\n");
    printf("?exit - closes the client program\n");
}

void printVars(user* usr) {
    printf("Current session variables:\n");
    printf("\tlogin: %s\n", usr->login);
    printf("\tpassword: %s\n", usr->password);
    printf("\tchannel: %d\n", usr->key);
    printf("\tqueue: %d\n", usr->queue);
}

void chat(user* usr) {
    key_t cQueue, mQueue, key;
    queuedMessage message;

    char select[32], prefix[32], input[250], response[32];

    // initialize server communication queue
    if ((cQueue = msgget(authKey, IPC_CREAT | 0666 )) == -1) {
        perror("Error initializing queue");
        exit(-1);
    }

    // initialize message queue
    if ((mQueue = msgget(serverKey, IPC_CREAT | 0666 )) == -1) {
        perror("Error creating queue");
        exit(-1);
    }

    printf("Type user/group to chat: ");
    fgets(select, sizeof(select), stdin);  // read user from stdin
    strtok(select, "\n");  // remove '\n' from user

    // prepare message
    sprintf(message.msgText, "CHAT:%s:%s", usr->login, select);
    message.msgType = comTyp;

    // send selected username to server
    if (msgsnd(cQueue, &message, sizeof message.msgText, IPC_NOWAIT) == -1) {
        perror("Error sending credentials");
        exit(-1);
    }

    // wait for server response
    while (true) {
        if (msgrcv(cQueue, &message, sizeof message.msgText, 198, IPC_NOWAIT) != -1) {
            // tokenize the response
            strcpy(response, strtok(message.msgText, delimiter));

            // validate first token
            if (strcmp(response, "NOT_SUBSCRIBED") == 0) {
                printf(ANSI_COLOR_YELLOW "You are not subscribed to %s\n" ANSI_COLOR_RESET, select);
                return;
            } else if (!(strcmp(response, "FALSE") == 0)) {
                strcpy(prefix, response);
                sscanf(strtok(NULL, delimiter), "%d", &key);  // convert token to integer
                break;
            } else {
                printf(ANSI_COLOR_YELLOW "There is no such user/group as %s\n" ANSI_COLOR_RESET, select);
                return;
            }
        }
    }
    while (true) {
        printf("Type something!\n");

        int pid = fork();
        if (pid == -1) {
            perror("Error forking");
            exit(-1);
        } else if (pid == 0) {
            while (true) {
                if (strcmp(prefix, "USER") == 0) {
                    if (msgrcv(usr->queue, &message, sizeof message.msgText, usr->key, IPC_NOWAIT) != -1) {
                        printf(ANSI_COLOR_RESET "%s\n" ANSI_COLOR_RESET, message.msgText);
                    }
                } else if (strcmp(prefix, "GROUP") == 0) {
                    if (msgrcv(usr->queue, &message, sizeof message.msgText, key + 100, IPC_NOWAIT) != -1) {
                        printf(ANSI_COLOR_RESET "%s\n" ANSI_COLOR_RESET, message.msgText);
                    }
                }
            }
        } else {
            while (true) {
                // prepare message
                fgets(input, sizeof(input), stdin);  // read message from stdin
                strtok(input, "\n");  // remove '\n' from message
                sprintf(message.msgText, "%s:%s:%s:%s", prefix, usr->login, select, input);
                message.msgType = key;

                // if message is special string, execute command
                if (strcmp(input, "?exit") == 0) {
                    printf("Exiting...\n");
                    if (kill(pid, SIGTERM) == -1) {
                        perror("Error killing child process");
                    }
                    wait(&pid);  // wait for child process to end not to generate zombies
                    deauthenticate(usr);
                    exit(0);

                } else if (strcmp(input, "?back") == 0) {
                    if (kill(pid, SIGTERM) == -1) {
                        perror("Error killing child process");
                    }
                    wait(&pid);  // wait for child process to end not to generate zombies
                    return;

                } else {;
                    if (msgsnd(mQueue, &message, sizeof message.msgText, IPC_NOWAIT) == -1) {
                        perror("Error sending message");
                        exit(-1);
                    }
                }
            }
        }
    }
}

void subscribe(user* usr) {
    key_t cQueue;
    queuedMessage message;

    char select[32];

    // initialize server communication queue
    if ((cQueue = msgget(authKey, IPC_CREAT | 0666 )) == -1) {
        perror("Error initializing queue");
        exit(-1);
    }

    printf("Type group to subscribe: ");
    fgets(select, sizeof(select), stdin);  // read user from stdin
    strtok(select, "\n");  // remove '\n' from user

    // prepare message
    sprintf(message.msgText, "SUBSCRIBE:%s:%s", usr->login, select);
    message.msgType = comTyp;

    // send selected username to server
    if (msgsnd(cQueue, &message, sizeof message.msgText, IPC_NOWAIT) == -1) {
        perror("Error sending credentials");
        exit(-1);
    }

    // wait for server response
    while (true) {
        if (msgrcv(cQueue, &message, sizeof message.msgText, 198, IPC_NOWAIT) != -1) {

            // if response is not 'FALSE', continue
            if (!(strcmp(message.msgText, "FALSE") == 0)) {
                printf("ok\n");
                break;
            } else {
                printf(ANSI_COLOR_YELLOW "There is no such group as %s\n" ANSI_COLOR_RESET, select);
                return;
            }
        }
    }
}

void unsubscribe(user* usr) {
    key_t cQueue;
    queuedMessage message;

    char select[32];

    // initialize server communication queue
    if ((cQueue = msgget(authKey, IPC_CREAT | 0666 )) == -1) {
        perror("Error initializing queue");
        exit(-1);
    }

    printf("Type group to unsubscribe: ");
    fgets(select, sizeof(select), stdin);  // read user from stdin
    strtok(select, "\n");  // remove '\n' from user

    // prepare message
    sprintf(message.msgText, "UNSUBSCRIBE:%s:%s", usr->login, select);
    message.msgType = comTyp;

    // send selected username to server
    if (msgsnd(cQueue, &message, sizeof message.msgText, IPC_NOWAIT) == -1) {
        perror("Error sending credentials");
        exit(-1);
    }

    // wait for server response
    while (true) {
        if (msgrcv(cQueue, &message, sizeof message.msgText, 198, IPC_NOWAIT) != -1) {

            // if response is not 'FALSE', continue
            if (!(strcmp(message.msgText, "FALSE") == 0)) {
                printf("ok\n");
                break;
            } else {
                printf(ANSI_COLOR_YELLOW "There is no such group as %s\n" ANSI_COLOR_RESET, select);
                return;
            }
        }
    }
}

void printUsers(user* usr) {
    key_t queue;
    queuedMessage message;

    // initialize server communication queue
    if ((queue = msgget(authKey, IPC_CREAT | 0666 )) == -1) {
        perror("Error creating queue");
        exit(-1);
    }

    // prepare message
    strcpy(message.msgText, "LIST_USERS:");
    strcat(message.msgText, usr->login);
    message.msgType = comTyp;

    // send 'LIST_USERS' signal to server
    if (msgsnd(queue, &message, sizeof message.msgText, IPC_NOWAIT) == -1) {
        perror("Error sending query");
        exit(-1);
    }

    // wait for server response
    while (true) {
        if (msgrcv(queue, &message, sizeof message.msgText, comTyp - 1, IPC_NOWAIT) != -1) {
            printf("%s\n", message.msgText);
            return;
        }
    }
}

void printGroups(user* usr) {
    key_t queue;
    queuedMessage message;

    // initialize server communication queue
    if ((queue = msgget(authKey, IPC_CREAT | 0666 )) == -1) {
        perror("Error creating queue");
        exit(-1);
    }

    // prepare message
    strcpy(message.msgText, "LIST_GROUPS:");
    strcat(message.msgText, usr->login);
    message.msgType = comTyp;

    // send 'LIST_GROUPS' signal to server
    if (msgsnd(queue, &message, sizeof message.msgText, IPC_NOWAIT) == -1) {
        perror("Error sending query");
        exit(-1);
    }

    // wait for server response
    while (true) {
        if (msgrcv(queue, &message, sizeof message.msgText, comTyp - 1, IPC_NOWAIT) != -1) {
            printf("%s\n", message.msgText);
            return;
        }
    }
}

user* authenticate() {
    int attempts = 0;
    char login[32];
    char password[32];

    key_t queue;
    queuedMessage message;

    // initialize server communication queue
    if ((queue = msgget(authKey, IPC_CREAT | 0666 )) == -1) {
        perror("Error creating queue");
        exit(-1);
    }

    while (true) {
        printf("Login: ");
        fgets(login, sizeof(login), stdin);  // read login from stdin
        strtok(login, "\n");  // remove '\n' from login

        printf("Password: ");
        fgets(password, sizeof(password), stdin);  // read password from stdin
        strtok(password, "\n");  // remove '\n' from password

        // prepare message
        sprintf(message.msgText, "LOGIN:%s:%s", login, password);
        message.msgType = authTyp;

        // send credentials to server
        if (msgsnd(queue, &message, sizeof message.msgText, IPC_NOWAIT) == -1) {
            perror("Error sending credentials");
            exit(-1);
        }

        // wait for server response
        while (true) {
            if (msgrcv(queue, &message, sizeof message.msgText, authTyp - 1, IPC_NOWAIT) != -1) {

                // if first token is 'TRUE', authorize the user
                if (strcmp(strtok(message.msgText, delimiter), "TRUE") == 0) {
                    // initialize user structure
                    user *usr = malloc(sizeof(user));

                    // tokenize user login
                    strcpy(usr->login, strtok(NULL, delimiter));

                    // tokenize user password
                    strcpy(usr->password, strtok(NULL, delimiter));

                    // tokenize user key
                    sscanf(strtok(NULL, delimiter), "%d", &usr->key);  // convert token to integer

                    // initialize user queue
                    if ((usr->queue = msgget(usr->key, IPC_CREAT | 0666 )) == -1) {
                        perror("Error creating queue");
                        exit(-1);
                    }
                    return usr;
                } else {
                    // exit the program, if there was too many incorrect attempts
                    if (++attempts == allowedAttempts) {
                        printf(ANSI_COLOR_RED "Too many incorrect attempts, exiting\n" ANSI_COLOR_RESET);
                        exit(0);
                    }
                    printf(ANSI_COLOR_YELLOW  "Incorrect login or password, please try again\n" ANSI_COLOR_RESET);
                    break;
                }
            }
        }
    }
}

void deauthenticate(user* usr) {
    // TODO [deauthenticate()]: implement deauthentication on SIGINT/SIGTERM
    key_t queue;
    queuedMessage message;

    // initialize server communication queue
    if ((queue = msgget(authKey, IPC_CREAT | 0666 )) == -1) {
        perror("Error initializing queue");
        exit(-1);
    }

    // prepare message
    sprintf(message.msgText, "LOGOUT:%s", usr->login);
    message.msgType = authTyp;

    // send 'LOGOUT' signal to server
    if (msgsnd(queue, &message, sizeof message.msgText, IPC_NOWAIT) == -1) {
        perror("Error sending");
        exit(-1);
    }
}