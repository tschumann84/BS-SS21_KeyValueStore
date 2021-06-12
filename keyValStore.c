/*
### Datenhaltungskonzept

Key Value Stores bestehen im Grunde aus einer zweispaltigen Tabelle.
Eine Spalte enthält den Schlüssel die andere den zugehörigen Wert.
Sowohl die Schlüssel als auch die Werte können von unterschiedlichen Typen sein.
Laut Aufgabenstellung sollen sie Alphanummerisch sein.

### Festlegungen/Vereinbarungen
Festlegungen für den Aufbau des Schlüssels:
 - Der Schlüssel enthält keine Leerzeichen.
 - Der Schlüssel enthält keine Sonderzeichen.
 - Der Schlüssel enthält nur Buchstaben und Zahlen.

Festlegungen für den Inhalt der Werte:
 - Werte enthalten keine Leerzeichen.
 - Werte enthalten keine Sonderzeichen.
 - Werte bestehen aus Buchstaben und Zahlen.
*/

#include "keyValStore.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include "server.h"
#include "sub.h"

int semid, shmid;
int* keyValNum;
struct keyValKomb* keyValStore;
int* TAID;
int* procCount;
struct liste* subshared_memory;

//Leere Variable für get-Funktion
char res[LENGTH_VALUE] = "";

// ### Private Funktionen für Hauptfunktionen
int put_in(char* key, char* value);
int get_in(char* key, char* res);
int del_in(char* key);


void delete (void) {
    int res;
    //log_debug(":delete Lösche Semaphore semid: %d", DeleteShmid);
    if(semctl (DeleteSemid, 0, IPC_RMID, 0) == -1) {
        log_error(":delete Fehler beim löschen des Semaphores.");
    } else {
        log_info(":delete Semaphoren gelöscht.");
    }
    //log_debug(":delete Lösche Shared Memory shmid: %d", DeleteShmid);
    res = shmctl (DeleteShmid, IPC_RMID, NULL);
    if(res == -1) {
        log_error(":delete Fehler beim löschen des Shared Memorys.");
        log_debug(":delete shmctl shmid: %d, Kommando %d\n", DeleteShmid, IPC_RMID);
    } else {
        log_info(":delete Shared Memory gelöscht");
    }
}

void sharedStore (void) {
    log_debug(":sharedStore Start");
    union semun sunion;
    int res;
    char *buffer;
    char* shm_addr;

//### Semaphore erstellen
    semid = safesemget(IPC_PRIVATE, 4, IPC_CREAT | 0770);
    log_debug(":sharedStore Semaphore erstellt semid: %d", semid);
    DeleteSemid = semid;

// Semaphor init
    sunion.val = 1;
    safesemctl(semid, SEM_Store, SETVAL, sunion);
    safesemctl(semid, SEM_TAID, SETVAL, sunion);
    safesemctl(semid, SEM_DEL, SETVAL, sunion);
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

    // Shared Memory keyValNum = Anzahl der Keys im Array
    log_info(":sharedStore keyValStore im Shared Memory erstellen...");
    keyValNum = (int*) shm_addr;
    *keyValNum = 0;

    // Shared Memory TAID = ?
    log_info(":sharedStore TAID im shared Memory erstellen...");
    TAID = (int*) ((void*)shm_addr+sizeof(int));
    *TAID = 0;

    // Shared Memory procCount = Anzahl der aktiven Kindprozesse
    log_info(":sharedStore procCount im shared Memory erstellen...");
    procCount = (int*) ((void*)shm_addr+(2*sizeof(int)));
    *procCount = 0;

    // Shared Memory keyValStore = Der Bereich für den KeyValStore
    keyValStore = (struct keyValKomb*) ((void*)shm_addr+(3*sizeof(int)));
    log_debug(":sharedStore keyValStore: %s", keyValStore);
    if(keyValStore == (void *) -1) {
        log_error(":sharedStore Fehler, keyValStore konnte nicht erstellt werden.");
    } else {
        log_info(":sharedStore keyValStore erstellt.");
    }
    log_debug(":sharedStore *KeyValnum %d", *keyValNum);

    // Shared Memory keyValStore = Der Bereich für den KeyValStore
//    subshared_memory = (struct liste*) ((void*)shm_addr+(3*sizeof(int))+ 100000);
//    log_debug(":sharedStore keyValStore: %s", subshared_memory);
//    if(subshared_memory == (void *) -1) {
//        log_error(":sharedStore Fehler, keyValStore konnte nicht erstellt werden.");
//    } else {
//        log_info(":sharedStore keyValStore erstellt.");
//    }
    //log_debug("llllll Größe %i",sizeof(keyValStore));
    //200*500
    //log_debug("lllll Größe %i", sizeof(int));
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
            pub(key, value, 0);
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

int put(char* key, char* value) {
    log_info(":put start");
    log_debug(":put key %s | value %s", key, value);
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
            log_debug(":put Eigene PID (%d) != TAID (%d)", getpid(), *TAID);
            waitzero(semid, SEM_Trans);
            return put_in(key, value);
        }
    }
}

int get_in(char* key, char* res) {
    log_info(":get_in start");
    clearArray(res);
    //log_info(":get_in Array leer");
    int i = 0;
    //log_debug("locksem(semid, SEM_Store);");
    locksem(semid, SEM_Store);
    //log_info(":get_in SEM_Store");
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
    log_debug(":get key %s | res %s", key, res);
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
                pub(key, res, 1);
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

int beginExklusive(int ID) {
    log_debug(":beginExklusive ID = %d", ID);
    locksem(semid, SEM_TAID);
    if (*TAID == 0) {
        *TAID = ID;
        unlocksem(semid, SEM_Trans);
        unlocksem(semid, SEM_TAID);
        return 0;
    } else{
        unlocksem(semid, SEM_TAID);
        return -1;
    }
};

int endExklusive(int ID) {
    log_debug(":endExklusive ID = %d", ID);
    locksem(semid, SEM_TAID);
    if (ID == *TAID) {
        *TAID = 0;
        locksem(semid, SEM_Trans);
        unlocksem(semid, SEM_TAID);
        log_debug(":endExklusive Return 0");
        return 0;
    } else{
        unlocksem(semid, SEM_TAID);
        log_debug(":endExklusive Return -1");
        return -1;
    }
};

int saveBlockShutdown(int ID) {
    waitzero(semid, SEM_Trans);
    locksem(semid, SEM_TAID);

    *TAID = ID;
    unlocksem(semid, SEM_TAID);
    unlocksem(semid, SEM_Trans);
    locksem(semid, SEM_Store);
    return 0;
};

int saveUnblockShutdown(int ID) {
    locksem(semid, SEM_TAID);

    if (ID == *TAID) {
        *TAID = 0;
        locksem(semid, SEM_Trans);
        unlocksem(semid, SEM_TAID);
        unlocksem(semid, SEM_Store);
        return 0;
    } else {
        unlocksem(semid, SEM_TAID);
        return -1;
    }
};

void incrementProcCount(){
    locksem(semid, SEM_DEL);
    *procCount = *procCount+1;
    log_info(":server_start Verbindung akzeptiert von: %s:%d. Aktuell laufende Sockets: %i", inet_ntoa(getSocketaddrClient().sin_addr), ntohs(getSocketaddrClient().sin_port),*procCount);
    unlocksem(semid, SEM_DEL);
}

void decrementProcCount(){
    locksem(semid, SEM_DEL);
    *procCount = *procCount-1;
    log_warn(":server_start %i Socket geschlossen. Aktuell noch laufende Sockets: %i", ntohs(getSocketaddrClient().sin_port), *procCount);
    unlocksem(semid, SEM_DEL);
}

int getProcCount(){
    locksem(semid, SEM_DEL);
    int ergebnis = *procCount;
    unlocksem(semid, SEM_DEL);
    return ergebnis;
}
