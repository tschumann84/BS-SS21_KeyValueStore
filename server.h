//
// Created by nasenhaarwombat on 27.04.2021.
//

#ifndef BS21_SERVER_H
#define BS21_SERVER_H

#include <signal.h>

#define BUFSIZE 1024 // Größe des Buffers
int server_start();
int server_stop(int sigid);
int getCFD();
struct sockaddr_in getSocketaddrClient();
#endif //BS21_SERVER_H
