//
// Created by benja on 20.04.2021.
//

#include "keyValStore.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "log/log.h"
#include "interface.h"

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
// ### SharedMem und Semaphoren
static void locksem (int semid, int semnum);
static void unlocksem (int semid, int semnum);
static void waitzero (int semid, int semnum);
static int safesemget (key_t key, int nsems, int semflg);
static int safesemctl (int semid, int semnum, int cmd, union senum arg);
static int safesemop (int semid, struct sembuf *sops, unsigned nsops);
static int DeleteSemid = 0;
static int DeleteShmid = 0;

// ### Nebenfunktionen für Shared Memory und Semaphore
// Vielleicht auch in Header auslagern
static void locksem (int semid, int semnum) {
    struct sembuf sb;
    sb.sem_num = semnum;
    sb.sem_op = -1;
    sb.sem_flg = SEM_UNDO;
    safesemop (semid, &sb, 1);
    return;
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
    return;
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
static sighandler_t
my_signal(int sig_nr, sighandler_t signalhandler) {
    struct sigaction neu_sig, alt_sig;
    neu_sig.sa_handler = signalhandler;
    sigemptyset (&neu_sig.sa_mask);
    neu_sig.sa_flags = SA_RESTART;
    if (sigaction(sig_nr, &neu_sig, &alt_sig) < 0)
        return SIG_ERR;
    return alt_sig.sa_handler;
}
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
static void sharedStore (void) {
    union semun sunion;
    int semid, shmid;
    int res;
    void *shmdata;
    char *buffer;
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
        printf ("Fehler bei key %d, mit der Größe %d\n", IPC_PRIVATE, SHAREDMEMSIZE);
    DeleteShmid = shmid;
// Shared Memory anbindung
    shmdata = shmat(shmid, NULL, 0);
    if(shmdata == (void *) -1)
        printf("Fehler bei shmat(): shmid %d\n", shmid);
// Kennung am Anfang des Shared Memorys schreiben
    *(int *) shmdata = semid;
    buffer = shmdata + sizeof (int);
    printf("Shared Memory hat ID %d\n", shmid);
    return;
}


int store (void) {
    struct keyValKomb keyValStore[STORESIZE];
    strcpy(keyValStore[0].key, NULL);
    strcpy(keyValStore[0].value, NULL);

};


// ### Hauptfunktionen
/*
 * ### Schreibfunktion für die verkettete Liste des Key-Value-Stores
 * Funktionen die in die verkette Liste schreiben. Dabei soll die Speicherung Sortiert ablaufen
 * um spätere Suchalgorithmen wie Binary Search zu unterstützen.
 * Mehrere Szenarien müssen bedacht werden:
 * - Die Liste ist leer, der zu schreibende Key ist der Erste.
 * - Die Liste ist nicht leer:
 *   - Der Key ist der größte
 *   - Der Key ist der kleinste
 *   - Der Key liegt zwischen anderen Keys.
 *   put ist die Hauptfunktion für die Speicherung und verwendet writeToEnd.
 */
/*
 * ### put - Elemente sortiert in die Liste eintragen.
 * put fügt Key+Value sortiert nach der Wertigkeit von Key in die Liste ein.
 * Die Funktion benutzt writeToEnd() und reserviert den benötigten Speicherplatz.
 * Übergeben wird der Schlüssel (key) und der Wert (value) als char Array.
 */

//int put_linkedList(char* key, char* value){
//    log_debug(":put(Start) KEY: %s VALUE: %s", key, value);
//    struct keyValKomb *point, *bevorpoint;
//    // Prüfen, ob es eine Liste gibt, wenn nicht run writeToEnd.
//    if (anfang == NULL) {
//        log_info(":put LinkedList ist leer");
//        log_debug(":put call writeToEnd(%s, %s)", key, value);
//        writeToEnd(key, value);
//        return 1;
//    } else {
//        // Liste durchlaufen so lange $key größer als $point.key ist.
//        log_info(":put LinkedList ist nicht leer");
//        point = anfang;
//        while (point != NULL && (strcmp(point->key, key) < 0)) {
//            point = point->next;
//        }
//        // Prüfen, ob key die höchste Wertigkeit hat, dann writeToEnd.
//        if (point == NULL) {
//            log_info(":put KEY hat den höchsten Wert: %s", key);
//            log_debug(":put call writeToEnd(%s, %s)", key, value);
//            writeToEnd(key, value);
//            return 0;
//        }
//            // Prüfen, ob key die niedrigste Wertigkeit hat, dann key-value auf $anfang speichern und liste schieben.
//        else if (point == anfang) {
//            log_info(":put Key hat den niedrigsten Wert: %s", key);
//            log_debug(":put nächster Key hat den Wert: %s", anfang->key);
//            anfang = malloc(sizeof(struct keyValKomb));
//            strcpy(anfang->key, key);
//            strcpy(anfang->value, value);
//            anfang->next = point;
//            return 0;
//        }
//            // Position finden an der $point beim Durchlauf stehen geblieben ist und $nextpoint dahinter einfügen.
//            // Liste schieben und Kette schließen.
//        else {
//            log_info("Key hat einen Wert dazwischen: %s", key);
//            bevorpoint = anfang;
//            while (bevorpoint->next != point) {
//                bevorpoint = bevorpoint->next;
//            };
//            point = malloc(sizeof(struct keyValKomb));
//            strcpy(point->key, key);
//            strcpy(point->value, value);
//            log_info("Key davor hat Wert: %s", bevorpoint->key);
//            log_info("Key danach hat Wert: %s", bevorpoint->next->key);
//            point->next = bevorpoint->next;
//            bevorpoint->next = point;
//            return 0;
//        }
//    }
//}

//int get_linkedList(char* key, char* res){
//    clearArray(res);
//    struct keyValKomb *zeiger;
//    log_debug(":get (Start) Key: %s, Res: %s",key, res );
//    /* Ist überhaupt ein Element vorhanden? */
//    if(anfang != NULL) {
//        log_info(":get Anfang hat den Wert: %s", anfang->key);
//        zeiger=anfang;
//        log_info(":get Zeiger hat den Wert: %s", zeiger->key);
//        /* Wir suchen in der Kette, ob das Element vorhanden ist. */
//        do{
//            log_info(":get Zeiger hat den Wert: %s", zeiger->key);
//            if((strcmp(key, zeiger->key)==0)) {
//                strcpy(res, zeiger->value);
//                log_info(":get Gesuchter Key wurde gefunden: %s",key);
//                return 0;
//            }
//            zeiger = zeiger->next;
//            log_info(":get Zeiger hat den neuen Wert: %s", zeiger->key);
//        } while (zeiger != NULL);
//        log_info(":get Key wurde nicht gefunden Key: %s",key);
//        return -2;
//    }
//    else {
//        log_info(":get LinkedList ist leer");
//        return -2;
//    }
//}

//int del_linkedList(char* key){
//    struct keyValKomb *zeiger, *zeiger1;
//    log_debug(":del (Start) Key: %s",key);
//    /* Ist überhaupt ein Element vorhanden? */
//    if(anfang != NULL) {
//        /* Ist unser 1. Element das von uns gesuchte (wen[])? */
//        log_info(":del Anfang hat den Wert: %s", anfang->key);
//        if(strcmp(key, anfang->key) == 0) {
//            log_info(":del Gesuchter Key wurde am Anfang gefunden: %s",key);
//            zeiger=anfang->next;
//            if(zeiger == NULL){
//                anfang = NULL;
//            }else {
//                log_info(":del Zeiger hat den naechsten Key: %s", zeiger->key);
//                log_debug(":del (Gelöscht) Key: %s, Value: %s", anfang->key, anfang->value);
//                //free(anfang);
//                strcpy(anfang->key, anfang->next->key);
//                strcpy(anfang->value, anfang->next->value);
//                anfang->next = zeiger->next;
//                //anfang=zeiger;
//                free(zeiger);
//                log_info(":del Neuer Key wurde am Anfang gesetzt: %s", anfang->key);
//            }
//            return 0;
//
//        }
//        else {
//            /* Es ist nicht das 1. Element zu löschen. Wir suchen in
//             * der weiteren Kette, ob das zu löschende Element vor-
//             * handen ist. */
//            zeiger=anfang;
//            log_info(":del Zeiger hat den Wert: %s", zeiger->key);
//            do {
//                zeiger1=zeiger->next;
//                log_info(":del Zeiger1 hat den Wert: %s", zeiger1->key);
//
//                /* Ist die Adresse von zeiger1 der gesuchten Adresse? */
//                if(strcmp(key, zeiger1->key) == 0) {
//                    log_info(":del Gesuchter Key wurde gefunden: %s",key);
//                    /* Falls ja, dann ... */
//                    zeiger->next=zeiger1->next;
//                    log_info(":del Liste wurd neu gelinkt: %s", zeiger->key);
//                    log_debug(":del (Gelöscht) Key: %s, Value: %s",zeiger1 ->key, zeiger1->value);
//                    free(zeiger1);
//                    return 0;
//                }
//                zeiger=zeiger1;
//                log_info(":del Zeiger hat den neuen Wert: %s", zeiger->key);
//            }while(zeiger->next != NULL);
//            /* Ende while */
//        }   /* Ende else */
//        return -1;
//        log_info(":del Key wurde nicht gefunden Key: %s, Res: %s",key, res);
//    }        /* Ende if(anfang != NULL) */
//    else
//        log_info(":del LinkedList ist leer");
//        return -1;
//}
// ### Nebenfunktionen
/*
 * ### writeToEnd - Element am Ende einfügen
 * writeToEnd hängt Key+Value am Ende der Liste an, ist die Liste leer wird das Element in $anfang gespeichert.
 * Die Funktion reserviert dabei den benötigen Speicherplatz.
 * Übergeben wird der Schlüssel (key) sowie der Wert (value) als char Array.
 */
//void writeToEnd_linkedList(char* key, char* value) {
//    log_debug(":writeToEnd(Start) KEY: %s  VALUE: %s", key, value);
//    struct keyValKomb *point;
//
//    //Prüfen, ob die Liste leer ist.
//    if(anfang == NULL) {
//        log_info(":writeToEnd LinkedList ist leer");
//        // Wenn die Liste leer ist, wird der Speicher reserviert.
//        if((anfang = malloc(sizeof(struct keyValKomb))) == NULL) {
//            log_fatal(":writeToEnd, Memory Allocation für keyValKomb schlug fehl");
//            return;
//        }
//        log_info(":writeToEnd LinkedList wird erstellt");
//        // Schlüssel+Wert per Pointer übertragen und $anfang.next auf NULL setzen.
//        strcpy(anfang->key, key);
//        strcpy(anfang->value, value);
//        anfang->next = NULL;
//        log_debug(":writeToEnd LinkedList erstellt: KEY %s, VALUE %s, NEXT %s",anfang->key, anfang->value, anfang->next);
//    }
//    else {
//        // Wenn die Liste nicht leer ist, läuft die Schleife bis zum Letzen Element.
//        log_info(":writeToEnd LinkedList ist nicht leer");
//        point=anfang;
//        while(point->next != NULL) {
//            point = point->next;
//        }
//        // der Speicher wird reserviert
//        if((point->next = malloc(sizeof(struct keyValKomb))) == NULL) {
//            log_fatal(":writeToEnd, Memory Allocation für keyValKomb schlug fehl");
//            return;
//        }
//        log_info(":writeToList KeyValue wird an LinkedList angehängt");
//        // Schlüssel und Wert werden übertragen und zuvor $point auf den reservierten Speicher gelegt.
//        point=point->next;
//        strcpy(point->key, key);
//        strcpy(point->value, value);
//        point->next = NULL;
//        log_debug(":writeToEnd KeyValue angehängt: KEY %s, VALUE %s, NEXT %s", point->key, point->value, point->next);
//    }
//}