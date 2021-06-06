//
// Created by benja on 20.04.2021.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pwd.h>
#include "log/log.h"
#include "interface.h"

#ifndef BS21_KEYVALSTORE_H
#define BS21_KEYVALSTORE_H

//Anzahl Schluessel für Store
#define STORESIZE 500

//Länge von Key und Value
#define LENGTH_KEY 100
#define LENGTH_VALUE 100

// Tempfile für Transaktion Block
#define TRANSACTION_FILE "/tmp/.ta.file"
//#define TAID 0

// struct für Schlüssel und Wert zur Speicherung als verkettete List.

struct keyValKomb {
    char key[LENGTH_KEY];
    char value[LENGTH_VALUE];
//    struct keyValKomb *next;
};

//shared Memory & Semaphore
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
//#include <linux/sem.h>
#include <sys/shm.h>
#include <errno.h>
#include <signal.h>
//#define keyVakKombSize (sizeof(keyValKomb))
//#define SHMDATASIZE (STORESIZE*keyValKombSize)
//#define SHAREDMEMSIZE ((STORESIZE*sizeof(keyValKomb)).int)
#define SHAREDMEMSIZE (((LENGTH_KEY+LENGTH_VALUE)*STORESIZE)+(2*sizeof(int)))
#define BUFFERSIZE (SHAREDMEMSIZE - sizeof(int))
#define SEM_Store 0 // SEM für KeyValStore Array
#define SEM_Trans 1 // SEM für PID im aktuellen Transaktionprozess
#define SEM_TAID 2 // SEM für "Wait for Zero" während Transaktion
/* ---------- Bei BSD-UNIXen auskommentieren ------------ */
#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
/* union semun is defined by including <sys/sem.h> */
#else
/* according to X/OPEN we have to define it ourselves */
union semun {
    int val;                 /* Werte für  SETVAL        */
    struct semid_ds *buf;    /* Puffer IPC_STAT, IPC_SET */
    unsigned short *array;   /* Array für GETALL, SETALL */
    /* Linux specific part:     */
    struct seminfo *__buf;   /* Puffer für IPC_INFO      */
};
//union semun {
//    int val;
//    struct semun *buf;
//    ushort array[];
//};
#endif
/* -------------------------------------------------------- */
// ### SharedMem und Semaphoren
static void locksem (int semid, int semnum);
static void unlocksem (int semid, int semnum);
static void waitzero (int semid, int semnum);
static int safesemget (key_t key, int nsems, int semflg);
static int safesemctl (int semid, int semnum, int cmd, union semun arg);
static int safesemop (int semid, struct sembuf *sops, unsigned nsops);
void sharedStore (void);
static int DeleteSemid = 0;
static int DeleteShmid = 0;

void beginExklusive(int ID);
void endExklusive(int ID);

// ### Nebenfunktionen für Shared Memory und Semaphore
// Vielleicht auch in Header auslagern
static void locksem (int semid, int semnum) {
    struct sembuf sb;
    sb.sem_num = semnum;
    sb.sem_op = -1;
    sb.sem_flg = SEM_UNDO;
    safesemop (semid, &sb, 1);
}
static void unlocksem (int semid, int semnum) {
    struct sembuf sb;
    sb.sem_num = semnum;
    sb.sem_op = 1;
    sb.sem_flg = SEM_UNDO;
    safesemop (semid, &sb, 1);
}
static void waitzero (int semid, int semnum) {
    struct sembuf sb;
    sb.sem_num = semnum;
    sb.sem_op = 0;
    sb.sem_flg = 0;
    safesemop (semid, &sb, 1);
}
static int safesemctl (int semid, int semnum, int cmd, union semun arg) {
    int retval;
    retval = semctl (semid, semnum, cmd, arg);
    if (retval == -1)
        printf ("Fehler: Semaphor mit ID %d, semnum %d, "
                "Kommando %d\n", semid, semnum, cmd);
    return retval;
}
static int safesemget (key_t key, int nsems, int semflg) {
    int retval;
    retval = semget (key, nsems, semflg);
    if (retval == -1)
        printf ("Semaphore-Schlüssel %d, nsems %d konnte nicht erstellt werden", key, nsems);
    return retval;
}
static int safesemop (int semid, struct sembuf *sops, unsigned nsops) {
    int retval;
    retval = semop (semid, sops, nsops);
    if (retval == -1)
        printf ("Fehler: Semaphore mit ID %d (%d Operation)\n", semid, nsops);
    return retval;
}

// ### Sighandler um Shared Memory und Semaphoren bei SIG_KILL zu löschen
typedef void (*sighandler_t)(int);
static sighandler_t my_signal(int sig_nr, sighandler_t signalhandler) {
    struct sigaction neu_sig, alt_sig;
    neu_sig.sa_handler = signalhandler;
    sigemptyset (&neu_sig.sa_mask);
    neu_sig.sa_flags = SA_RESTART;
    if (sigaction(sig_nr, &neu_sig, &alt_sig) < 0)
        return SIG_ERR;
    return alt_sig.sa_handler;
}

// ### Öffentliche Funktionen
int put(char* key, char* value);
int get(char* key, char* res);
int del(char* key);

#endif //BS21_KEYVALSTORE_H
