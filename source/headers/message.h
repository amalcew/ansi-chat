#ifndef ANSI_IRC_MESSAGE_H
#define ANSI_IRC_MESSAGE_H

#define MSG_SIZE 256

typedef struct messageStructure {
    long msgType;
    char msgText[MSG_SIZE];
} queuedMessage;

#endif //ANSI_IRC_MESSAGE_H
