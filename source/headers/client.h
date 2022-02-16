#ifndef ANSI_IRC_CLIENT_H
#define ANSI_IRC_CLIENT_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

#include "globals.h"
#include "message.h"
#include "user.h"

void printHelp();
void printVars(user* usr);
void chat(user* usr);
void subscribe(user* usr);
void unsubscribe(user* usr);
void printUsers(user* usr);
void printGroups(user* usr);

#endif //ANSI_IRC_CLIENT_H
