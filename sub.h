//
// Created by benja on 20.04.2021.
//
#include "keyValStore.h"

#ifndef BS21_SUB_H
#define BS21_SUB_H
struct liste {
    char key[LENGTH_KEY];
    int cfd;
};

int sub(char* key, int cfd);
int desub(char* key, int cfd);
int pub(char* key, char* res, int funktion);
#endif //BS21_SUB_H
