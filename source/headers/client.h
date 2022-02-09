#ifndef ANSI_IRC_CLIENT_H
#define ANSI_IRC_CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

#include "globals.h"
#include "message.h"

int authenticate();

#endif //ANSI_IRC_CLIENT_H
