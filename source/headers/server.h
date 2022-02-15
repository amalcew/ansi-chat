#ifndef ANSI_IRC_SERVER_H
#define ANSI_IRC_SERVER_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "message.h"
#include "aux.h"
#include "user.h"
#include "group.h"
#include "globals.h"

volatile sig_atomic_t running = true;

void catchSignal(int signum);
void logError(char* desc, int errorNum);
void logAction(char* type, char* desc, char* cred);
#endif //ANSI_IRC_SERVER_H
