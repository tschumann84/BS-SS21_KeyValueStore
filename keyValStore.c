//
// Created by benja on 20.04.2021.
//

#include "keyValStore.h"
#include <stdio.h>
#include <string.h>

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

//Länge von Key und Value
const lengthKey = 10;
const lengthValue = 10;

//Leere Variable für get-Funktion
char res[lengthValue] = "";

struct keyValKomb {
    char key[lengthKey];
    char value[lengthValue];
    struct keyValKomb *next;
};

    struct keyValKomb *next   = NULL;
    struct keyValKomb *anfang = NULL;

int put(char* key, char* value){
    if (anfang==NULL){

    }
};

int writeToList(struct keyValKomb *point, char* key, char* value){

}

int get(char* key, char* res){
    struct keyValKomb *zeiger;
    if(anfang != NULL) {
        zeiger=anfang;
        while(zeiger->next != NULL) {
            if(zeiger->key == key) {
                strcpy(zeiger->value, res);
                zeiger->next=zeiger->next;
                break;
            }
            zeiger = zeiger->next;
        }
    }
    else
        printf("Es sind keine Daten zum Löschen vorhanden!!!\n");
};

int del(char* key){
    struct keyValKomb *zeiger, *zeiger1;

    /* Ist überhaupt ein Element vorhanden? */
    if(anfang != NULL) {
        /* Ist unser 1. Element das von uns gesuchte (wen[])? */
        if(anfang->key == key) {
            zeiger=anfang->next;
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
                    break;
                }
                zeiger=zeiger1;
            }  /* Ende while */
        }     /* Ende else */
    }        /* Ende if(anfang != NULL) */
    else
        printf("Es sind keine Daten zum Löschen vorhanden!!!\n");
}
