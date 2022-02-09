#include "headers/client.h"

int main(){
    int ownKey = authenticate();
    int ownQueue, serverQueue;
    queuedMessage msg;

    if ((ownQueue = msgget(ownKey, IPC_CREAT | 0666 )) == -1) {
        perror("Error creating queue");
        exit(-1);
    }

    if ((serverQueue = msgget(serverKey, IPC_CREAT | 0666 )) == -1) {
        perror("Error creating queue");
        exit(-1);
    }
    printf("Welcome\nselect user to chat: ");
    char* input[2];
    int choose;
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%d", &choose);

    while (1) {
        int pid = fork();
        if (pid == -1) {
            perror("Error forking");
            exit(-1);
        } else if (pid == 0) {
            while (1) {
                if (msgrcv(ownQueue, &msg, sizeof msg.msgText, ownKey, IPC_NOWAIT) != -1) {
                    printf(ANSI_COLOR_GREEN "test: %s\n" ANSI_COLOR_RESET, msg.msgText);
                }
            }
        } else {
            while (1) {
                fgets(msg.msgText, sizeof(msg.msgText), stdin);  // read string
                strtok(msg.msgText, "\n");
                if (strcmp(msg.msgText, "exit\n") == 0) {
                    printf("Exiting...\n");
                    exit(0);
                }
                msg.msgType = choose;

                if (msgsnd(serverQueue, &msg, sizeof msg.msgText, 0) == -1) {
                    perror("Error sending message");
                    exit(-1);
                }
            }
        }
    }
}


int authenticate() {
    int ownKey;
    int authQueue;
    queuedMessage msg;
    struct msqid_ds msgCtlBuf;
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
        while (1) {
            if (msgrcv(authQueue, &msg, sizeof msg.msgText, 98, IPC_NOWAIT) != -1) {
                char delimiters[] = " ,.-:;";
                char* token;
                char* response[5];
                char* login[128];
                token = strtok(msg.msgText, delimiters);
                printf("%s\n", token);
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