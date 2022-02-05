#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <errno.h>

#include "colors.h"
#include "msg.h"

volatile sig_atomic_t done = 0;

char * getTime() {
    char *buff = malloc(21);
    time_t now = time (0);
    strftime (buff, 100, "[%Y-%m-%d %H:%M:%S]", localtime (&now));
    return buff;
}

void catchSignal(int signum) {
    printf("\n" ANSI_COLOR_YELLOW "Exiting gracefully..." ANSI_COLOR_RESET "\n");
    done = 1;
}

int main(){
    int queue[10];
    queuedMessage msg, buf;
    struct msqid_ds msgCtlBuf;

    time_t t;
    time(&t);

    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = catchSignal;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);


    for (int i = 1; i <= 10; i++) {
        if ((queue[i] = msgget(i, IPC_CREAT | 0666)) == -1) {
            perror("Error creating queue");
            exit(-1);
        }
    }

    while (!done) {
        if (msgrcv(queue[10], &buf, sizeof msg.msgText, 0, IPC_NOWAIT) != -1) {
            char *now = getTime();
            printf("%s %d -> %ld\n", now, buf.msgSender, buf.msgType);
            free(now);
            if (msgsnd(queue[buf.msgType], &buf, sizeof buf.msgText, 0) == -1) {
                perror("Error sending message");
            }
        }
    }

    while (done) {
        
    }

    return 0;
}