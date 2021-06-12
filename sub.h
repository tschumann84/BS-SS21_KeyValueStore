//
// Created by benja on 20.04.2021.
//
#include "keyValStore.h"

//Anzahl möglicher SUBs
#define ANZAHLSUBS 100

#ifndef BS21_SUB_H
#define BS21_SUB_H
#define SEM_Sub 0
#define SUB_Size ((LENGTH_KEY+sizeof(int)) * ANZAHLSUBS + sizeof(int))

static int sub_DeleteSemid = 0;
static int sub_DeleteShmid = 0;


struct liste {
    char key[LENGTH_KEY];
    int cfd;
};

void sub_delete (void);
void sub_sharedStore (void);
int sub(char* key, int cfd);
int desub(char* key, int cfd);
int pub(char* key, char* res, int funktion);
#endif //BS21_SUB_H
