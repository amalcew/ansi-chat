#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "msg.h"

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

    while (1) {
        // msg.msgText;
        printf("> ");
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

    return 0;
}