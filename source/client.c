#include "headers/client.h"

int main(){
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = catchSignal;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);

    queuedMessage msg;

    int ownKey = authenticate(msg);
    int ownQueue, serverQueue;

    /* initialize own queue and server queue */
    if ((ownQueue = msgget(ownKey, IPC_CREAT | 0666 )) == -1) {
        perror("Error creating queue");
        exit(-1);
    }
    if ((serverQueue = msgget(serverKey, IPC_CREAT | 0666 )) == -1) {
        perror("Error creating queue");
        exit(-1);
    }

    printf(ANSI_COLOR_GREEN "Welcome\n" ANSI_COLOR_RESET "type '?help' to get the list of commands\n");
    while (true) {
        char* command[32];
        printf(": ");
        fgets(command, sizeof(command), stdin);  // read string
        strtok(command, "\n");

        if (strcmp(command, "?help") == 0) {
            printf("?help - prints this commands list\n");
            printf("?chat - select the users to chat\n");
            printf("?leave - close current session\n");
            printf("?exit - closes the client program\n");

        } else if (strcmp(command, "?chat") == 0) {
            chat(ownKey, ownQueue, serverQueue, msg);

        } else if (strcmp(command, "?leave") == 0) {

        } else if (strcmp(command, "?exit") == 0) {
            printf("Exiting...\n");
            exit(0);

        } else {
            printf("Unknown command, type '?help' to get a list of commands\n");
        }
    }
}

int authenticate(queuedMessage msg) {
    // TODO [client:authenticate]: make function more simple and make comments
    int ownKey;
    int authQueue;

    if ((authQueue = msgget(authKey, IPC_CREAT | 0666 )) == -1) {
        perror("Error creating queue");
        exit(-1);
    }

    int pid = fork();
    if (pid == -1) {
        perror("Error forking");
        exit(-1);
    } else if (pid == 0) {
        char* login[128];
        char* password[128];

        printf("login: ");
        fgets(login, sizeof(login), stdin);
        printf("password: ");
        fgets(password, sizeof(password), stdin);
        strtok(login, "\n");
        strtok(password, "\n");
        sprintf(&msg.msgText, "%s:%s", login, password);
        msg.msgType = 99;

        if (msgsnd(authQueue, &msg, sizeof msg.msgText, IPC_NOWAIT) == -1) {
            perror("Error sending credentials");
            exit(-1);
        }
        exit(0);
    } else {
        wait(&pid);
        while (true) {
            if (msgrcv(authQueue, &msg, sizeof msg.msgText, 98, IPC_NOWAIT) != -1) {
                char delimiters[] = " ,.-:;";
                char* token;
                char* response[5];
                char* login[128];
                token = strtok(msg.msgText, delimiters);
                if (strcmp(token, "FALSE") == 0) {
                    printf("login failed, exiting...\n");
                    exit(0);
                } else {
                    token = strtok(NULL, delimiters);
                    sscanf(token, "%d", &ownKey);  // convert token to integer
                    break;
                }
            }
        }
    }
    return ownKey;
}

void chat(int ownKey, int ownQueue, int serverQueue, queuedMessage msg) {
    // TODO [client:chat]: implement proper chat chooser (current is insufficient)
    // TODO[client:chat]: make function more simple and make comments
    while (true) {
        printf("select user to chat: ");
        char* input[1];
        int choose;
        fgets(input, sizeof(input), stdin);
        sscanf(input, "%d", &choose);

        while (true) {
            int pid = fork();
            if (pid == -1) {
                perror("Error forking");
                exit(-1);
            } else if (pid == 0) {
                while (true) {
                    if (msgrcv(ownQueue, &msg, sizeof msg.msgText, ownKey, IPC_NOWAIT) != -1) {
                        printf(ANSI_COLOR_BLUE "test: %s\n" ANSI_COLOR_RESET, msg.msgText);
                    }
                }
            } else {
                while (true) {
                    fgets(msg.msgText, sizeof(msg.msgText), stdin);  // read string
                    strtok(msg.msgText, "\n");
                    if (strcmp(msg.msgText, "?exit") == 0) {
                        printf("Exiting...\n");
                        exit(0);
                    }

                    if (strcmp(msg.msgText, "?back") == 0) {
                        if (kill(pid, SIGTERM) == -1) {
                            perror("Error killing child process");
                        }
                        wait(&pid);  // wait for child process to end not to generate zombies
                        return;
                        // break;
                    }

                    msg.msgType = choose;

                    if (msgsnd(serverQueue, &msg, sizeof msg.msgText, 0) == -1) {
                        perror("Error sending message");
                        exit(-1);
                    }
                }
            }
            // break;
        }
        // break;
    }
}

void catchSignal(int signum) {
    // printf("\n" ANSI_COLOR_YELLOW "Exiting gracefully..." ANSI_COLOR_RESET "\n");
    exit(0);
}