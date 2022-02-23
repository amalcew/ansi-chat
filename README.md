# Ansi-IRC

Simple chat project written in Ansi C, presenting the idea behind message queues and process-to-process communication.

## Compiling

To properly use the project, follow below steps:

1. Clone the repository
```
git clone https://github.com/amalcew/ansi-chat.git
```
2. Compile the project
```
cd ansi-chat && ./compile
```
3. Compiling process will return two binary files in *build/* directory, which now can be executed:
  - change files permissions:
```
chmod +x build/*
```
  - run server (in separate terminal):
```
build/server
```
  - run client (in separate terminal):
```
build/client
```

## How does it work?

The project consists of two separate programs, **server.c** and **client.c**. Server loads the configuration of users and groups from config files (present in *config/* directory) and handles the clients requests. Client contains user interface and allows the user to perform commands, write messages, check users list, etc.

Entire communication between processes of clients and server is based on *Interprocess Communication* and uses message queues (ipc.h, msg.h headers). Each action performed by client (such as authentication, addresser verification, chatting, etc.) is handled by the server, which centralises the entire communication between each client. This solution mimics the existing modern protocols like TCP and allows easy administration of performed actions.

Server initializes two message queues, one for internal communication beween server and the clients and one for message forwarding. First queue is used in every action that need to be verified by the server (authentication, adresser verification, chat group name verification, chat group subscribtion, etc.). Second queue is used to forward messages written by the user to specific user/group.


