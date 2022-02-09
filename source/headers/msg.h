#ifndef ANSI_IRC_MSG_H
#define ANSI_IRC_MSG_H

#define MSG_SIZE 256

typedef struct messageStructure {
    long msgType;
    int msgSender;
    char msgText[MSG_SIZE];
} queuedMessage;

#endif //ANSI_IRC_MSG_H
