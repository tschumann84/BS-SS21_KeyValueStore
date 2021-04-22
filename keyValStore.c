//
// Created by benja on 20.04.2021.
//

#include "keyValStore.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "log/log.h"

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

//Variablen für den Anfang der verketteten Liste
struct keyValKomb *next   = NULL;
struct keyValKomb *anfang = NULL;

// ### Öffentliche Funktionen für keyValStore
int put(char* key, char* value);
int get(char* key, char* res);
int del(char* key);
// ### Private Funktionen für Hauptfunktionen
void writeToEnd(char* key, char* value);
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

int put(char* key, char* value){
    log_debug(":put(Start) KEY: %s VALUE: %s", key, value);
    struct keyValKomb *point, *bevorpoint;
    // Prüfen, ob es eine Liste gibt, wenn nicht run writeToEnd.
    if (anfang == NULL) {
        log_info(":put LinkedList ist leer");
        log_debug(":put call writeToEnd(%s, %s)", key, value);
        writeToEnd(key, value);
        return 1;
    } else {
        // Liste durchlaufen so lange $key größer als $point.key ist.
        log_info(":put LinkedList ist nicht leer");
        point = anfang;
        while (point != NULL && (strcmp(point->key, key) < 0)) {
            point = point->next;
        }
        // Prüfen, ob key die höchste Wertigkeit hat, dann writeToEnd.
        if (point == NULL) {
            log_info(":put KEY hat den höchsten Wert: %s", key);
            log_debug(":put call writeToEnd(%s, %s)", key, value);
            writeToEnd(key, value);
            return 0;
        }
            // Prüfen, ob key die niedrigste Wertigkeit hat, dann key-value auf $anfang speichern und liste schieben.
        else if (point == anfang) {
            log_info(":put Key hat den niedrigsten Wert: %s", key);
            log_debug(":put nächster Key hat den Wert: %s", anfang->key);
            anfang = malloc(sizeof(struct keyValKomb));
            strcpy(anfang->key, key);
            strcpy(anfang->value, value);
            anfang->next = point;
            return 0;
        }
            // Position finden an der $point beim Durchlauf stehen geblieben ist und $nextpoint dahinter einfügen.
            // Liste schieben und Kette schließen.
        else {
            log_info("Key hat einen Wert dazwischen: %s", key);
            bevorpoint = anfang;
            while (bevorpoint->next != point) {
                bevorpoint = bevorpoint->next;
            };
            point = malloc(sizeof(struct keyValKomb));
            strcpy(point->key, key);
            strcpy(point->value, value);
            log_info("Key davor hat Wert: %s", bevorpoint->key);
            log_info("Key danach hat Wert: %s", bevorpoint->next->key);
            point->next = bevorpoint->next;
            bevorpoint->next = point;
            return 0;
        }
    }
}

int get(char* key, char* res){
    struct keyValKomb *zeiger;

    /* Ist überhaupt ein Element vorhanden? */
    if(anfang != NULL) {
//        log_trace(anfang->value);
        zeiger=anfang;
//        log_trace(zeiger->value);
        /* Wir suchen in der Kette, ob das Element vorhanden ist. */
        do{
//            log_trace(zeiger->key);
//            log_trace(key);
            if((strcmp(key, zeiger->key)==0)) {
                strcpy(res, zeiger->value);
//                log_trace(res);
                break;
            }
            zeiger = zeiger->next;
//            log_trace(res);
        } while (zeiger != NULL);
    }
    else
        printf("Es sind keine Daten zum Löschen vorhanden!!!\n");
}

int del(char* key){
    struct keyValKomb *zeiger, *zeiger1;

    /* Ist überhaupt ein Element vorhanden? */
    if(anfang != NULL) {
        /* Ist unser 1. Element das von uns gesuchte (wen[])? */
        if(anfang->key == key) {
            zeiger=anfang->next;
            free(anfang);
            anfang=zeiger;
        }
        else {
            /* Es ist nicht das 1. Element zu löschen. Wir suchen in
             * der weiteren Kette, ob das zu löschende Element vor-
             * handen ist. */
            zeiger=anfang;
            while(zeiger->next != NULL) {
                zeiger1=zeiger->next;

                /* Ist die Adresse von zeiger1 der gesuchte Name? */
                if(zeiger1->key == key) {
                    /* Falls ja, dann ... */
                    zeiger->next=zeiger1->next;
                    free(zeiger1);
                    break;
                }
                zeiger=zeiger1;
            }  /* Ende while */
        }     /* Ende else */
    }        /* Ende if(anfang != NULL) */
    else
        printf("Es sind keine Daten zum Löschen vorhanden!!!\n");
}
// ### Nebenfunktionen
/*
 * ### writeToEnd - Element am Ende einfügen
 * writeToEnd hängt Key+Value am Ende der Liste an, ist die Liste leer wird das Element in $anfang gespeichert.
 * Die Funktion reserviert dabei den benötigen Speicherplatz.
 * Übergeben wird der Schlüssel (key) sowie der Wert (value) als char Array.
 */
void writeToEnd(char* key, char* value) {
    log_debug(":writeToEnd(Start) KEY: %s  VALUE: %s", key, value);
    struct keyValKomb *point;

    //Prüfen, ob die Liste leer ist.
    if(anfang == NULL) {
        log_info(":writeToEnd LinkedList ist leer");
        // Wenn die Liste leer ist, wird der Speicher reserviert.
        if((anfang = malloc(sizeof(struct keyValKomb))) == NULL) {
            log_fatal(":writeToEnd, Memory Allocation für keyValKomb schlug fehl");
            return;
        }
        log_info(":writeToEnd LinkedList wird erstellt");
        // Schlüssel+Wert per Pointer übertragen und $anfang.next auf NULL setzen.
        strcpy(anfang->key, key);
        strcpy(anfang->value, value);
        anfang->next = NULL;
        log_debug(":writeToEnd LinkedList erstellt: KEY %s, VALUE %s, NEXT %s",anfang->key, anfang->value, anfang->next);
    }
    else {
        // Wenn die Liste nicht leer ist, läuft die Schleife bis zum Letzen Element.
        log_info(":writeToEnd LinkedList ist nicht leer");
        point=anfang;
        while(point->next != NULL) {
            point = point->next;
        }
        // der Speicher wird reserviert
        if((point->next = malloc(sizeof(struct keyValKomb))) == NULL) {
            log_fatal(":writeToEnd, Memory Allocation für keyValKomb schlug fehl");
            return;
        }
        log_info(":writeToList KeyValue wird an LinkedList angehängt");
        // Schlüssel und Wert werden übertragen und zuvor $point auf den reservierten Speicher gelegt.
        point=point->next;
        strcpy(point->key, key);
        strcpy(point->value, value);
        log_debug(":writeToEnd KeyValue angehängt: KEY %s, VALUE %S, NEXT %s", point->key, point->value, point->next);
    }
}