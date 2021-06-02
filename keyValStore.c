//
// Created by benja on 20.04.2021.
//

#include "keyValStore.h"
//#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>
//#include "log/log.h"
//#include "interface.h"

////shared Memory & Semaphore
//#include <sys/types.h>
//#include <sys/ipc.h>
//#include <sys/shm.h>
//#include <sys/sem.h>
//#include <signal.h>
//#define SHAREDMEMSIZE (STORESIZE*sizeof(keyValKomb))
//#define BUFFERSIZE (SHAREDMEMSIZE - sizeof(int))
//#define SN_EMPTY 0
//#define SN_FULL 1

int semid, shmid;
int* keyValNum;
struct keyValKomb* keyValStore;


void beginExklusive();
void endExklusive();

/*
### Datenhaltungskonzept

Key Value Stores bestehen im Grunde aus einer zweispaltigen Tabelle.
Eine Spalte enthält den Schlüssel die andere den zugehörigen Wert.
Sowohl die Schlüssel als auch die Werte können von unterschiedlichen Typen sein.
Laut Aufgabenstellung sollen sie Alphanummerisch sein.

 Die Speicherung kann im Arbeitsspeicher oder auf dem Festspeicher erfolgen.
 Beide Speicherungen unterscheiden sich voneinander.
Im Arbeitsspeicher:
 - Speicherung als Array, wobei die Position im Array dem Schlüssel entspricht.
 - Speicherung als verkettete Liste, dabei kann Schlüssel und Wert in einer Struktur gespeichert werden.
 - Speicherung als Struktur, dabei ist der Name des structs der des Schlüssels und im struct befindet sich der Wert.
 ... Andere Lösungen oder Kombinationen möglich.
Im Festspeicher:
 - Speicherung in einer einzelnen Datei die durch Zeilen und Kommas getrennt wird.
 - Jeder Schlüssel mit seinem Wert in einer separaten Datei.
 - Jeder Schlüssel mit seinem Wert in einer separaten Datei in einer Verzeichnisstruktur dessen Aufbau hierarchisch dem des Schlüssels folgt.
 ... Andere Lösungen und Kombinationen möglich.

### Festlegungen/Vereinbarungen

Festlegungen für den Aufbau des Schlüssels:
 - Der Schlüssel enthält keine Leerzeichen.
 - Der Schlüssel enthält keine Sonderzeichen.
 - Der Schlüssel enthält nur ???Kleinbuchstaben??? und Zahlen.
 - ???Länge des Schlüssels???

Festlegungen für den Inhalt der Werte:
 - ???Werte enthalten keine Leerzeichen.???
 - Werte enthalten keine Sonderzeichen.
 - Werte bestehen aus Buchstaben und Zahlen.
 - ???Länge der Werte???
*/

//Leere Variable für get-Funktion
char res[LENGTH_VALUE] = "";


// ### Öffentliche Funktionen für keyValStore
int put(char* key, char* value);
int get(char* key, char* res);
int del(char* key);
// ### Private Funktionen für Hauptfunktionen
void writeToEnd(char* key, char* value);

static void delete (void) {
    int res;
    printf ("\nServer wird beendet - Lösche Semaphor %d.\n", DeleteSemid);
    if(semctl (DeleteSemid, 0, IPC_RMID, 0) == -1) {
        printf ("Fehler beim Löschen des Semaphors.\n");
    }
    res = shmctl (DeleteShmid, IPC_RMID, NULL);
    if(res == -1)
        printf ("Fehler bei shmctl() shmid %d, Kommando %d\n", DeleteShmid, IPC_RMID);
    return;
}
static void sigdelete (int signum) {
    exit(EXIT_FAILURE);
}
void sharedStore (void) {
    union semun sunion;
    int res;
    char *buffer;
    char* shm_addr;

// Semaphore erstellen
    semid = safesemget(IPC_PRIVATE, 2, SHM_R | SHM_W);
    DeleteSemid = semid;
// Semaphor beim Beenden löschen
    atexit (&delete);
// SIGHANDLER einrichten
    my_signal(SIGINT, &sigdelete);
// Semaphor init
    sunion.val = 1;
    safesemctl(semid, SN_EMPTY, SETVAL, sunion);
    sunion.val = 0;
    safesemctl(semid, SN_FULL, SETVAL, sunion);
// Shared Memory einrichten
    shmid = shmget(IPC_PRIVATE, SHAREDMEMSIZE, IPC_CREAT | SHM_R | SHM_W);
    if (shmid == -1)
        printf ("Fehler bei key %d, mit der Größe %ld \n", IPC_PRIVATE, SHAREDMEMSIZE);
    DeleteShmid = shmid;
// Shared Memory anbindung
    shm_addr = shmat(shmid, NULL, 0);
    if (!shm_addr) { /* operation failed. */
        perror("shmat: ");
        exit(1);
    }
    keyValNum = (int*) shm_addr;
    *keyValNum = 0;
    keyValStore = (struct keyValKomb*) ((void*)shm_addr+sizeof(int));
    if(keyValStore == (void *) -1)
        printf("Fehler bei shmat(): shmid %d\n", shmid);

    //buffer = keyValStore + sizeof (int);
    printf("Shared Memory hat ID %d\n", shmid);


    log_info("*KeyValnum %d", *keyValNum);
    return;
}

int put(char* key, char* value){
    //Checken ob das Element bereits in der Liste ist.
    int i;
    for (i = 0; i<(*keyValNum); i++){
        if(strcmp(keyValStore[i].key, key)==0){
            locksem(semid,0);
            strcpy(keyValStore[i].value, value);
            unlocksem(semid,0);
            return 0;
        }
    }
    // Wenn nicht in der Liste, an die letzte Stelle schreiben.
    if(((*keyValNum)+1) < STORESIZE) {
        //locksem(semid,SN_EMPTY);
        strcpy(keyValStore[(*keyValNum)].key, key);
        strcpy(keyValStore[(*keyValNum)].value, value);
        (*keyValNum)++;
        //unlocksem(semid,SN_FULL);
    }
    return 0;
}

int get(char* key, char* res){
    clearArray(res);
    int i = 0;
    locksem(semid,0);
    //Ist überhaupt ein Element vorhanden?
    if(strcmp(keyValStore[i].key, "\0") != 0) {
        log_info(":get Erstes Element hat den Wert: %s", keyValStore[i].key);
        // Wir suchen in der Kette, ob das Element vorhanden ist.
        do{
            if(strcmp(keyValStore[i].key, key) == 0) {
                strcpy(res, keyValStore[i].value);
                log_info(":get Gesuchter Key wurde gefunden: %s", keyValStore[i].key);
                unlocksem(semid,0);
                return 0;
            }
            i++;
            log_info(":get Nächstes Element hat den neuen Wert: %s", keyValStore[i].key);
        } while ((strcmp(keyValStore[i].key, "\0") != 0));
        unlocksem(semid,0);
        log_info(":get Key wurde nicht gefunden Key: %s",key);
        return -2;
    }
    else {
        log_info(":get LinkedList ist leer");
        unlocksem(semid,0);
        return -2;
    }
    //return 0;
    //ich habe was geändert
}

int del(char* key){
    int i = 0;
    //Ist überhaupt ein Element vorhanden?
    if(strcmp(keyValStore[i].key, "\0") != 0) {
        log_info(":del Erstes Element hat den Wert: %s", keyValStore[i].key);
        // Wir suchen in der Kette, ob das Element vorhanden ist.
        do{
            if(strcmp(keyValStore[i].key, key) == 0) {
                int j = i+1;

                locksem(semid,0);
                do{
                    strcpy(keyValStore[i].key, keyValStore[j].key);
                    strcpy(keyValStore[i].value, keyValStore[j].value);
                    j++;
                    i++;
                }while ((strcmp(keyValStore[j-1].key, "\0") != 0));
                log_debug(":del Gesuchter Key wurde gefunden und gelöscht!");
                (*keyValNum)--;
                unlocksem(semid,0);
                return 0;
            }
            i++;
            log_info(":del Nächstes Element hat den neuen Wert: %s", keyValStore[i].key);
        } while ((strcmp(keyValStore[i].key, "\0") != 0));
        log_info(":del Key wurde nicht gefunden Key: %s",key);
        return -1;
    }
    else {
        log_info(":del LinkedList ist leer");
        return -1;
    }
    //return 0;
}

void beginExklusive(){
    struct sembuf semaphore_lock[1]   = { 0, -1, SEM_UNDO };
    //locksem(semid,1);
};

void endExklusive(){
    struct sembuf semaphore_unlock[1] = { 0, 1,  SEM_UNDO };
    safesemop(semid,&semaphore_unlock[0],1);
    //unlocksem(semid,1);
};
