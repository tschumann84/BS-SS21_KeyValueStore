//
// Created by benja on 20.04.2021.
//

#include "keyValStore.h"

int semid, shmid;
int* keyValNum;
int* TAID;
struct keyValKomb* keyValStore;

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
void sharedStore (void);
//int beginExklusive(char *f);
//void endExklusive(char *f);
static void delete (void);
// ### Private Funktionen für Hauptfunktionen
int put_in(char* key, char* value);
int get_in(char* key, char* res);
int del_in(char* key);


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

//### Semaphore erstellen
// semid = safesemget(IPC_PRIVATE, 2, SHM_R | SHM_W );
    semid = safesemget(IPC_PRIVATE, 3, IPC_CREAT | 0770);
    log_debug(":sharedStore Semaphore erstellt semid: %d", semid);
    DeleteSemid = semid;
// Semaphor beim Beenden löschen


//#############################################################
// auskommentiert da beenden eines Sockets SHM und SEM löscht.
    //atexit (&delete);
//#############################################################


// SIGHANDLER einrichten
    my_signal(SIGINT, &sigdelete);
// Semaphor init
    sunion.val = 1;
    safesemctl(semid, SEM_Store, SETVAL, sunion);
    safesemctl(semid, SEM_TAID, SETVAL, sunion);
    sunion.val = 0;
    safesemctl(semid, SEM_Trans, SETVAL, sunion);
//### Shared Memory erstellen
    shmid = shmget(IPC_PRIVATE, SHAREDMEMSIZE, IPC_CREAT | SHM_R | SHM_W );
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
    log_info(":sharedStore TAID im shared Memory erstellen...");
    TAID = (int*) ((void*)shm_addr+sizeof(int));
    *TAID = 0;
//    keyValStore = (struct keyValKomb*) ((void*)shm_addr+sizeof(int))
    keyValStore = (struct keyValKomb*) ((void*)shm_addr+sizeof(int)+sizeof(int));
    log_debug(":sharedStore keyValStore: %d", keyValStore);
    if(keyValStore == (void *) -1) {
        log_error(":sharedStore Fehler, keyValStore konnte nicht erstellt werden.");
    } else {
        log_info(":sharedStore keyValStore erstellt.");
    }
    //buffer = keyValStore + sizeof (int);
    //printf("Shared Memory hat ID %d\n", shmid);
    log_debug(":sharedStore *KeyValnum %d", *keyValNum);
    return;
}

int put_in(char* key, char* value) {
    log_info(":put_in Start");
    //Checken ob das Element bereits in der Liste ist.
    int i;
    //log_debug(":put_in locksem(semid, SEM_Store);");
    locksem(semid, SEM_Store);
    log_info(":put_in Überprüfe ob Key bereits Vorhanden.");
    for (i = 0; i < (*keyValNum); i++) {
        if (strcmp(keyValStore[i].key, key) == 0) {
            strcpy(keyValStore[i].value, value);
            //log_debug(":put_in unlocksem(semid, SEM_Store);");
            unlocksem(semid, SEM_Store);
            log_info(":put_in Key gefunden und überschrieben.");
            return 0;
        }
    }
// Wenn nicht in der Liste, an die letzte Stelle schreiben.
    log_info(":put_in Key nicht vorhanden, wird angehängt.");
    if (((*keyValNum) + 1) < STORESIZE) {
        strcpy(keyValStore[(*keyValNum)].key, key);
        strcpy(keyValStore[(*keyValNum)].value, value);
        //log_debug(":put_in unlocksem(semid, SEM_Store);");
        (*keyValNum)++;
        unlocksem(semid, SEM_Store);
    }
    return 0;
}

int put(char* key, char* value){
    log_info(":put start");
    log_debug(":put key %c | value %c", key, value);
    log_info(":put Überprüfung ob Transaktion aktiv.");
    locksem(semid, SEM_TAID);
    if (*TAID == 0) {
        log_info(":put Keine Transaktion aktiv, call put.");
        unlocksem(semid, SEM_TAID);
        return put_in(key, value);
    } else {
        if (*TAID == getpid()) {
            log_info(":put Transaktion aktiv, eigene PID gleich TransaktionID, call put.");
            unlocksem(semid, SEM_TAID);
            log_debug(":put Eigene PID (%d) == TAID (%d)", getpid(), *TAID);
            return put_in(key, value);
        } else {
            log_info(":put Transaktion aktiv, eigene PID ungleich TransaktionID, wartet auf Transaktionsende.");
            unlocksem(semid, SEM_TAID);
            log_debug(":put Eigene PID (%d) != TAID (%d)\", getpid(), *TAID");
            waitzero(semid, SEM_Trans);
            return put_in(key, value);
        }
    }

/*
        //Checken ob das Element bereits in der Liste ist.
        int i;
        log_debug(":put locksem(semid, SEM_Store);");
        locksem(semid, SEM_Store);
        for (i = 0; i < (*keyValNum); i++) {
            if (strcmp(keyValStore[i].key, key) == 0) {
                strcpy(keyValStore[i].value, value);
                log_debug(":put unlocksem(semid, SEM_Store);");
                unlocksem(semid, SEM_Store);
                return 0;
            }
        }
        // Wenn nicht in der Liste, an die letzte Stelle schreiben.
        if (((*keyValNum) + 1) < STORESIZE) {
            strcpy(keyValStore[(*keyValNum)].key, key);
            strcpy(keyValStore[(*keyValNum)].value, value);
            log_debug(":put unlocksem(semid, SEM_Store);");
            unlocksem(semid, SEM_Store);
            (*keyValNum)++;
        }
        return 0;
*/

}
int get_in(char* key, char* res) {
    log_info(":get_in start");
    clearArray(res);
    int i = 0;
    //log_debug("locksem(semid, SEM_Store);");
    locksem(semid, SEM_Store);
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
}


int get(char* key, char* res) {
    log_info(":get start");
    log_debug(":get key %c | res %c", key, res);
    log_info(":get Überprüfung ob Transaktion aktiv.");
    locksem(semid, SEM_TAID);
    if (*TAID == 0) {
        log_info(":get Keine Transaktion aktiv, call put.");
        unlocksem(semid, SEM_TAID);
        return get_in(key, res);
    } else {
        if (*TAID == getpid()) {
            log_info(":get Transaktion aktiv, eigene PID gleich TransaktionID, call put.");
            unlocksem(semid, SEM_TAID);
            log_debug(":get Eigene PID (%d) == TAID (%d)", getpid(), *TAID);
            return get_in(key, res);
        } else {
            log_info(":get Transaktion aktiv, eigene PID ungleich TransaktionID, wartet auf Transaktionsende.");
            unlocksem(semid, SEM_TAID);
            log_debug(":get Eigene PID (%d) != TAID (%d)\", getpid(), *TAID");
            waitzero(semid, SEM_Trans);
            return get_in(key, res);
        }
    }
}

/*
    clearArray(res);
    int i = 0;
    log_debug("locksem(semid, SEM_Store);");
    locksem(semid, SEM_Store);
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
*/

int del_in(char* key) {
    log_info(":del_in start");
    int i = 0;
    //log_debug("locksem(semid, SEM_Store);");
    log_info(":del_in suche nach Key.");
    locksem(semid, SEM_Store);
    if(strcmp(keyValStore[i].key, "\0") != 0) {
        // Wir suchen in der Kette, ob das Element vorhanden ist.
        do{
            if(strcmp(keyValStore[i].key, key) == 0) {
                log_info(":del_in Key gefunden.");
                int j = i+1;

                do{
                    strcpy(keyValStore[i].key, keyValStore[j].key);
                    strcpy(keyValStore[i].value, keyValStore[j].value);
                    j++;
                    i++;
                    log_info(":del_in Key gelöscht.");
                }while ((strcmp(keyValStore[j-1].key, "\0") != 0));
                (*keyValNum)--;
                unlocksem(semid, SEM_Store);
                return 0;
            }
            i++;
        } while ((strcmp(keyValStore[i].key, "\0") != 0));
        //log_debug("unlocksem(semid, SEM_Store);");
        unlocksem(semid, SEM_Store);
        log_info(":del_in Key nicht gefunden.");
        return -1;
    }
    else {
        //log_debug("unlocksem(semid, SEM_Store);");
        unlocksem(semid, SEM_Store);
        log_info(":del_in keine Keys im Store.");
        return -1;
    }
}

int del(char* key) {
    log_info(":del start");
    log_debug(":del key %c | res %c", key, res);
    log_info(":del Überprüfung ob Transaktion aktiv.");
    locksem(semid, SEM_TAID);
    if (*TAID == 0) {
        log_info(":del Keine Transaktion aktiv, call put.");
        unlocksem(semid, SEM_TAID);
        return del_in(key);
    } else {
        if (*TAID == getpid()) {
            log_info(":del Transaktion aktiv, eigene PID gleich TransaktionID, call put.");
            unlocksem(semid, SEM_TAID);
            log_debug(":del Eigene PID (%d) == TAID (%d)", getpid(), *TAID);
            return del_in(key);
        } else {
            log_info(":del Transaktion aktiv, eigene PID ungleich TransaktionID, wartet auf Transaktionsende.");
            unlocksem(semid, SEM_TAID);
            log_debug(":del Eigene PID (%d) != TAID (%d)\", getpid(), *TAID");
            waitzero(semid, SEM_Trans);
            return del_in(key);
        }
    }
}

/*
    int i = 0;
    log_debug("locksem(semid, SEM_Store);");
    locksem(semid, SEM_Store);
    if(strcmp(keyValStore[i].key, "\0") != 0) {
        log_info(":del Erstes Element hat den Wert: %s", keyValStore[i].key);
        // Wir suchen in der Kette, ob das Element vorhanden ist.
        do{
            if(strcmp(keyValStore[i].key, key) == 0) {
                int j = i+1;

                do{
                    strcpy(keyValStore[i].key, keyValStore[j].key);
                    strcpy(keyValStore[i].value, keyValStore[j].value);
                    j++;
                    i++;
                }while ((strcmp(keyValStore[j-1].key, "\0") != 0));
                log_debug(":del Gesuchter Key wurde gefunden und gelöscht!");
                (*keyValNum)--;
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
        log_debug("unlocksem(semid, SEM_Store);");
        unlocksem(semid, SEM_Store);
        log_info(":del LinkedList ist leer");
        return -1;
    }
}
void beginExklusive() {

}
*/

void beginExklusive(int ID) {
    log_debug(":beginExklusive ID = %d", ID);
    locksem(semid, SEM_TAID);
    if (*TAID == 0) {
        *TAID = ID;
        unlocksem(semid, SEM_Trans);
        unlocksem(semid, SEM_TAID);
    }
    unlocksem(semid, SEM_TAID);
};

/*
int beginExklusive(char *f){
    log_info(":beginExklusive");
    int filedes;
    filedes = open(f, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (filedes == -1) {
        return 0;
    } else {
        close(filedes);
        return 1;
    }
    struct sembuf semaphore_lock[1]   = { 0, -1, SEM_UNDO };
    safesemop(semid, &semaphore_lock[0], 1);
    locksem(semid,1);
};
*/

void endExklusive(int ID) {
    locksem(semid, SEM_TAID);
    if (ID == *TAID) {
        *TAID = 0;
        locksem(semid, SEM_Trans);
    }
    unlocksem(semid, SEM_TAID);
};

/*
void endExklusive(char *f){
    log_info(":endExklusive");
    unlink(f);
    struct sembuf semaphore_unlock[1] = { 0, 1,  SEM_UNDO };
    safesemop(semid,&semaphore_unlock[0],1);
    //unlocksem(semid,1);
};
*/