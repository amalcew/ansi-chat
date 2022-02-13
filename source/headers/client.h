#ifndef ANSI_IRC_CLIENT_H
#define ANSI_IRC_CLIENT_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

#include "globals.h"
#include "message.h"

int authenticate(queuedMessage msg);
void chat(int ownKey, int ownQueue, int serverQueue, queuedMessage msg);
void catchSignal(int signum);

#endif //ANSI_IRC_CLIENT_H
