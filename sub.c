#include "sub.h"
#include "keyValStore.h"
#include "sys/msg.h"
#include "server.h"

struct liste* subliste;
int* tatsaechliche_anzahl_subs;
int sub_semid, sub_shmid;


void sub_delete (void) {
    int res;
    //log_debug(":delete Lösche Semaphore semid: %d", DeleteShmid);
    if(semctl (sub_DeleteSemid, 0, IPC_RMID, 0) == -1) {
        log_error(":sub_delete Fehler beim löschen des Semaphores.");
    } else {
        log_info(":sub_delete Semaphoren gelöscht.");
    }
    //log_debug(":delete Lösche Shared Memory shmid: %d", DeleteShmid);
    res = shmctl (sub_DeleteShmid, IPC_RMID, NULL);
    if(res == -1) {
        log_error(":sub_delete Fehler beim löschen des Shared Memorys.");
        log_debug(":sub_delete shmctl shmid: %d, Kommando %d\n", sub_DeleteShmid, IPC_RMID);
    } else {
        log_info(":sub_delete Shared Memory gelöscht");
    }
}

void sub_sharedStore (void) {
    log_debug(":sub_sharedStore Start");
    union semun sub_union;
    int res;
    char *buffer;
    char* shm_addr;

//### Semaphore erstellen
    sub_semid = safesemget(IPC_PRIVATE, 1, IPC_CREAT | 0770);
    log_debug(":sub_sharedStore Semaphore erstellt semid: %d", sub_semid);
    sub_DeleteSemid = sub_semid;

// Semaphor init
    sub_union.val = 1;
    safesemctl(sub_semid, SEM_Sub, SETVAL, sub_union);

//### Shared Memory erstellen
    sub_shmid = shmget(IPC_PRIVATE, SUB_Size, IPC_CREAT | SHM_R | SHM_W );
    if (sub_shmid == -1) {
        log_error(":sub_sharedStore Fehler bei Erstellung Shared Memory. Key: %d | Größe: %ld", IPC_PRIVATE, SUB_Size);
    } else {
        log_info(":sub_sharedStore Shared Memory erstellt shmid: %d", sub_shmid);
    }
    sub_DeleteShmid = sub_shmid;

// Shared Memory anbindung
    log_info(":sub_sharedStore Shared Memory anbinden... ");
    shm_addr = shmat(sub_shmid, NULL, 0);
    log_debug(":sub_sharedStore shm_addr %d", shm_addr);
    if (!shm_addr) { /* operation failed. */
        log_error(":sub_sharedStore Fehler bei Anbindung Shared Memory.");
        perror("shmat: ");
        exit(1);
    } else {
        log_info(":sub_sharedStore Shared Memory angebunden.");
    }
    // Shared Memory tatsaechliche_anzahl_subs = Anzahl der Subs im Array
    log_info(":sub_sharedStore subliste im Shared Memory erstellen...");
    tatsaechliche_anzahl_subs = (int*) shm_addr;
    *tatsaechliche_anzahl_subs = 0;

    // Shared Memory Subliste = Der Bereich für den Subliste
    subliste = (struct liste*) ((void*)shm_addr+(sizeof(int)));
    log_debug(":sub_sharedStore subliste: %s", subliste);
    if(subliste == (void *) -1) {
        log_error(":sub_sharedStore Fehler, subliste konnte nicht erstellt werden.");
    } else {
        log_info(":sub_sharedStore subliste erstellt.");
    }
    log_debug(":sub_sharedStore *tatsaechliche_anzahl_subs %d", *tatsaechliche_anzahl_subs);
}
int getSuber(char* key, int pid) {
    locksem(sub_semid, SEM_Sub);
    int i = 0;
    do {
        log_info(":getSuber gehe in DO rein");
        if (subliste[i].key == key && subliste[i].pid == pid) {
            log_info(":getSuber Subber exestiert schon für den Prozess %i!",pid);
            return -2;
        }
        i++;
    } while ((strcmp(subliste[i].key, "\0") != 0));
    log_info(":getSuber Sub existiert noch nicht für den Prozess %i",pid);
    unlocksem(sub_semid, SEM_Sub);
    return 0;
}

int sub(char* key, int pid) {
    log_info(":sub start");
    if (getSuber(key, pid) == 0) {
        locksem(sub_semid, SEM_Sub);
        if (((*tatsaechliche_anzahl_subs) + 1) < ANZAHLSUBS) {
            log_info(":sub Subplätze noch frei");
            char res[LENGTH_VALUE];
            if (get(key, res) == 0) {
                log_info(":sub Key existiert zum Subben");
                strcpy(subliste[(*tatsaechliche_anzahl_subs)].key, key);
                subliste[(*tatsaechliche_anzahl_subs)].pid = pid;

                (*tatsaechliche_anzahl_subs)++;
                log_info(":sub Subbing ist gelungen!");
                unlocksem(sub_semid, SEM_Sub);
                return 0;
            } else {
                log_info(":sub Key existiert nicht. Kein Sub möglich!");
                unlocksem(sub_semid, SEM_Sub);
                return -1;
            }
        } else {
            log_info(":sub Maximale Anzahl an Subs erreicht!");
            unlocksem(sub_semid, SEM_Sub);
            return -1;
        }
    }  else {
        log_info("sub: Error, subber gibt es schon!");
    }
}

int pub(char* key, char* res, int funktion){
    log_info(":pub start");
    locksem(sub_semid,SEM_Sub);
    int i = 0;
    if(strcmp(subliste[i].key, "\0") != 0) {
        do {
            if (strcmp(subliste[i].key, key) == 0) {
              log_info(":pub Nachricht gesendet an Subber des Key: %s",subliste[i].key);
                if(funktion == 0){
                    kill(subliste[i].pid, SIGTTIN);
                } else {
                    kill(subliste[i].pid, SIGTTOU);
                }
                log_info(":pub Nachricht gesendet an Subber des Key: %s",subliste[i].key);
            }
            i++;
        }while ((strcmp(subliste[i].key, "\0") != 0));
        unlocksem(sub_semid,SEM_Sub);
        if(funktion == 1){
            desub(key);
        }
        return 0;
    }
    else {
        log_info(":pub Subliste war Leer, keine Nachricht gesendet");
        unlocksem(sub_semid,SEM_Sub);
        return 0;
    }
}

int desub(char* key){
    log_info(":desub start");
    locksem(sub_semid,SEM_Sub);
    int i = 0;
    log_info(":desub suche nach Key.");
    if(strcmp(subliste[i].key, "\0") != 0) {
        // Wir suchen in der Kette, ob das Element vorhanden ist.
        do{
            if(strcmp(subliste[i].key, key) == 0) {
                log_info(":desub Key gefunden.");
                int j = i+1;
                do{
                    strcpy(subliste[i].key, subliste[j].key);
                    subliste[i].pid =subliste[j].pid;
                    j++;
                    i++;
                }while ((strcmp(subliste[j-1].key, "\0") != 0));
                log_info(":desub Key gelöscht aus Subliste");
                (*tatsaechliche_anzahl_subs)--;
                log_info(tatsaechliche_anzahl_subs);
            }
            i++;
        } while ((strcmp(subliste[i].key, "\0") != 0));
       unlocksem(sub_semid,SEM_Sub);
        return 0;
    }
    else {
        log_info(":desub keine Keys in Subliste.");
        unlocksem(sub_semid,SEM_Sub);
        return -1;
    }
}

int getMsgPut() {
    int i = 0;
    char out[LENGTH_KEY + LENGTH_VALUE + 7];
    clearArray(out);

    locksem(sub_semid, SEM_Sub);
    while ((strcmp(subliste[i].key, "\0") != 0)) {
        if (subliste[i].pid == getpid()) {
            char value[LENGTH_VALUE];
            get(subliste[i].key, value);
            sprintf(out, "PUT:%s:%s\r\n", subliste[i].key, value);
            if (write(getCFD(), out, strlen(out)) < 0) {
                log_error(":getMsgPut Pub war nicht erfolgreich Fehler: %s", strerror(errno));
            }
            unlocksem(sub_semid, SEM_Sub);
            return 0;
        }
        i++;
    }
    log_error(":getMsgPut Pub war nicht erfolgreich, Sub nicht gefunden.");
    unlocksem(sub_semid, SEM_Sub);
    return -1;
}

int getMsgDel(){
    int i = 0;
    char out[LENGTH_VALUE + 18];
    clearArray(out);

    locksem(sub_semid, SEM_Sub);
    while ((strcmp(subliste[i].key, "\0") != 0)){
        if(subliste[i].pid == getpid()){
            sprintf(out, "DEL:%s:key_deleted\r\n", subliste[i].key);
            if (write(getCFD(), out, strlen(out))<0){
                log_error(":getMsgDel Pub war nicht erfolgreich Fehler: %s", strerror(errno));
            }
            unlocksem(sub_semid, SEM_Sub);
            return 0;
        }
        i++;
    }
    log_error(":getMsgDel Pub war nicht erfolgreich, Sub nicht gefunden.");
    unlocksem(sub_semid, SEM_Sub);
    return -1;
}
