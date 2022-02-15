#ifndef ANSI_IRC_GLOBALS_H
#define ANSI_IRC_GLOBALS_H

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

char* groupPath = "/home/caribou/dev/working/ansi-irc/config/groups";
char* userPath = "/home/caribou/dev/working/ansi-irc/config/users";

key_t authKey = 9999;
key_t serverKey = 2151;

long authTyp = 99;
long comTyp = 199;

int allowedAttempts = 3;

#endif //ANSI_IRC_GLOBALS_H
