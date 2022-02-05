#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "msg.h"
#include "colors.h"

int main(){
    key_t ownKey = 1, serverKey = 10;
    int ownQueue, serverQueue;
    queuedMessage msg;

    if ((serverQueue = msgget(serverKey, IPC_CREAT | 0666 )) == -1) {
        perror("Error creating queue");
        exit(-1);
    }

    if ((ownQueue = msgget(ownKey, IPC_CREAT | 0666 )) == -1) {
        perror("Error creating queue");
        exit(-1);
    }

    int pid = fork();
    if (pid == -1) {
        perror("Error forking");
        exit(-1);
    } else if (pid == 0) {
        while (1) {
            if (msgrcv(ownQueue, &msg, sizeof msg.msgText, 1, IPC_NOWAIT) != -1) {
                printf(ANSI_COLOR_GREEN "test%d: %s" ANSI_COLOR_RESET "\n", msg.msgSender, msg.msgText);
            }
        }
    } else {
        while (1) {
            fgets(msg.msgText, sizeof(msg.msgText), stdin);  // read string
            if (strcmp(msg.msgText, "exit\n") == 0) {
                printf("Exiting...\n");
                exit(0);
            }
            msg.msgType = 2;
            msg.msgSender = 1;

            if (msgsnd(serverQueue, &msg, sizeof msg.msgText, 0) == -1) {
                perror("Error sending message");
                exit(-1);
            }
        }
    }

    return 0;
}