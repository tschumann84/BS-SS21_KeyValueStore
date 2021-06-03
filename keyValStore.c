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
//#define SEM_Store 0
//#define SEM_Array 1

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
//void writeToEnd(char* key, char* value);

static void delete (void) {
    log_debug(":delete Start");
    int res;
    log_info(":delete Server wird beendet...");
    log_debug(":delete Lösche Semaphore semid: %d", DeleteShmid);
    if(semctl (DeleteSemid, 0, IPC_RMID, 0) == -1) {
        log_error(":delete Fehler beim löschen des Semaphores.");
    } else {
        log_info(":delete Semaphoren gelöscht.");
    }
    log_debug(":delete Lösche Shared Memory shmid: %d", DeleteShmid);
    res = shmctl (DeleteShmid, IPC_RMID, NULL);
    if(res == -1) {
        log_error(":delete Fehler beim löschen des Shared Memorys.");
        log_debug(":delete shmctl shmid: %d, Kommando %d\n", DeleteShmid, IPC_RMID);
    } else {
        log_info(":delete Shared Memory gelöscht");
    }
    return;
}
static void sigdelete (int signum) {
    exit(EXIT_FAILURE);
}
void sharedStore (void) {
    log_debug(":sharedStore Start");
    union semun sunion;
    int res;
    char *buffer;
    char* shm_addr;

// Semaphore erstellen
    semid = safesemget(IPC_PRIVATE, 2, SHM_R | SHM_W);
    log_debug(":sharedStore Semaphore erstellt semid: %d", semid);
    DeleteSemid = semid;
// Semaphor beim Beenden löschen
    //atexit (&delete);
// SIGHANDLER einrichten
    my_signal(SIGINT, &sigdelete);
// Semaphor init
    sunion.val = 1;
    safesemctl(semid, SEM_Store, SETVAL, sunion);
    sunion.val = 0;
    safesemctl(semid, SEM_Array, SETVAL, sunion);
// Shared Memory einrichten
    shmid = shmget(IPC_PRIVATE, SHAREDMEMSIZE, IPC_CREAT | SHM_R | SHM_W);
    if (shmid == -1) {
        log_error(":SharedStore Fehler bei Erstellung Shared Memory. Key: %d | Größe: %ld", IPC_PRIVATE, SHAREDMEMSIZE);
    } else {
        log_info(":sharedStore Shared Memory erstellt shmid: %d", shmid);
    }
    DeleteShmid = shmid;
// Shared Memory anbindung
    log_info(":sharedStore Shared Memory anbinden... ");
    shm_addr = shmat(shmid, NULL, 0);
    log_debug(":sharedStore shm_addr %d", shm_addr);
    if (!shm_addr) { /* operation failed. */
        log_error(":sharedStore Fehler bei Anbindung Shared Memory.");
        perror("shmat: ");
        exit(1);
    } else {
        log_info(":sharedStore Shared Memory angebunden.");
    }

    log_info(":sharedStore keyValStore im Shared Memory erstellen...");
    keyValNum = (int*) shm_addr;
    *keyValNum = 0;
    keyValStore = (struct keyValKomb*) ((void*)shm_addr+sizeof(int));
    log_debug(":sharedStore keyValStore: %d", keyValStore);
    if(keyValStore == (void *) -1) {
        log_error(":sharedStore Fehler, keyValStore konnte nicht erstellt werden.");
//        printf("Fehler bei shmat(): shmid %d\n", shmid);
    } else {
        log_info(":sharedStore keyValStore erstellt.");
    }

    //buffer = keyValStore + sizeof (int);
    //printf("Shared Memory hat ID %d\n", shmid);

    log_debug(":sharedStore *KeyValnum %d", *keyValNum);
    return;
}

int put(char* key, char* value){
    log_debug("locksem(semid, SEM_Store);");
    locksem(semid, SEM_Store);
    //Checken ob das Element bereits in der Liste ist.
    int i;
    for (i = 0; i<(*keyValNum); i++){
        //locksem(semid, SEM_Store);
        if(strcmp(keyValStore[i].key, key)==0){
            //locksem(semid,0);
            strcpy(keyValStore[i].value, value);
            log_debug("unlocksem(semid, SEM_Array);");
            unlocksem(semid, SEM_Store);
            return 0;
        }
        //unlocksem(semid,SEM_Array);
    }
    // Wenn nicht in der Liste, an die letzte Stelle schreiben.
    if(((*keyValNum)+1) < STORESIZE) {
        //locksem(semid,SEM_Store);
        strcpy(keyValStore[(*keyValNum)].key, key);
        strcpy(keyValStore[(*keyValNum)].value, value);
        (*keyValNum)++;
        //unlocksem(semid,SEM_Array);
    }
    log_debug("unlocksem(semid, SEM_Store);");
    unlocksem(semid, SEM_Store);
    return 0;
}

int get(char* key, char* res){
    log_debug("locksem(semid, SEM_Store);");
    locksem(semid, SEM_Store);
    clearArray(res);
    int i = 0;
    //locksem(semid, SEM_Array);
    //Ist überhaupt ein Element vorhanden?
    if(strcmp(keyValStore[i].key, "\0") != 0) {
        log_info(":get Erstes Element hat den Wert: %s", keyValStore[i].key);
        // Wir suchen in der Kette, ob das Element vorhanden ist.
        do{
            if(strcmp(keyValStore[i].key, key) == 0) {
                strcpy(res, keyValStore[i].value);
                log_info(":get Gesuchter Key wurde gefunden: %s", keyValStore[i].key);
                log_debug("unlocksem(semid, SEM_Store);");
                unlocksem(semid, SEM_Store);
                return 0;
            }
            i++;
            log_info(":get Nächstes Element hat den neuen Wert: %s", keyValStore[i].key);
        } while ((strcmp(keyValStore[i].key, "\0") != 0));
        log_debug("unlocksem(semid, SEM_Store);");
        unlocksem(semid, SEM_Store);
        log_info(":get Key wurde nicht gefunden Key: %s",key);
        return -2;
    }
    else {
        log_info(":get LinkedList ist leer");
        log_debug("unlocksem(semid, SEM_Store);");
        unlocksem(semid, SEM_Store);
        return -2;
    }
    //return 0;
    //ich habe was geändert
}

int del(char* key){
    log_debug("locksem(semid, SEM_Store);");
    locksem(semid, SEM_Store);
    int i = 0;
    //Ist überhaupt ein Element vorhanden?
    //locksem(semid, SEM_Store);
    if(strcmp(keyValStore[i].key, "\0") != 0) {
        log_info(":del Erstes Element hat den Wert: %s", keyValStore[i].key);
        // Wir suchen in der Kette, ob das Element vorhanden ist.
        do{
            if(strcmp(keyValStore[i].key, key) == 0) {
                int j = i+1;

                //locksem(semid,0);
                do{
                    strcpy(keyValStore[i].key, keyValStore[j].key);
                    strcpy(keyValStore[i].value, keyValStore[j].value);
                    j++;
                    i++;
                }while ((strcmp(keyValStore[j-1].key, "\0") != 0));
                log_debug(":del Gesuchter Key wurde gefunden und gelöscht!");
                (*keyValNum)--;
                log_debug("unlocksem(semid, SEM_Store);");
                unlocksem(semid, SEM_Store);
                return 0;
            }
            i++;
            log_info(":del Nächstes Element hat den neuen Wert: %s", keyValStore[i].key);
        } while ((strcmp(keyValStore[i].key, "\0") != 0));
        log_debug("unlocksem(semid, SEM_Store);");
        unlocksem(semid, SEM_Store);
        log_info(":del Key wurde nicht gefunden Key: %s",key);
        return -1;
    }
    else {
        log_info(":del LinkedList ist leer");
        log_debug("unlocksem(semid, SEM_Store);");
        unlocksem(semid, SEM_Store);
        return -1;
    }
    //return 0;
}

void beginExklusive(){
    //struct sembuf semaphore_lock[1]   = { 0, -1, SEM_UNDO };
    //safesemop(semid, &semaphore_lock[0], 1);
    //locksem(semid,1);
};

void endExklusive(){
    //struct sembuf semaphore_unlock[1] = { 0, 1,  SEM_UNDO };
    //safesemop(semid,&semaphore_unlock[0],1);
    //unlocksem(semid,1);
};
