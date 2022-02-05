#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "msg.h"

int main(){
    key_t ownKey = 2, serverKey = 10;
    int ownQueue, serverQueue;
    queuedMessage msg;

    if ((ownQueue = msgget(ownKey, IPC_CREAT | 0666 )) == -1) {
        perror("Error creating queue");
        exit(-1);
    }

    while (1) {
        if (msgrcv(ownQueue, &msg, sizeof msg.msgText, 0, IPC_NOWAIT) != -1) {
            printf("test%d: %s", msg.msgSender, msg.msgText);
        }
    }
    return 0;
}