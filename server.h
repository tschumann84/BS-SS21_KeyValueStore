//
// Created by nasenhaarwombat on 27.04.2021.
//

#ifndef BS21_SERVER_H
#define BS21_SERVER_H

#include "keyValStore.h"

#define BUFSIZE (LENGTH_KEY+LENGTH_VALUE+15) // Größe des Buffers
int server_start();
int server_stop();
int getCFD();
struct sockaddr_in getSocketaddrClient();
#endif //BS21_SERVER_H
