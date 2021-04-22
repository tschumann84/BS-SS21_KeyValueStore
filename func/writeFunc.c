//
// Created by schumann on 4/21/21.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * ### Schreibfunktionen für die verkettete Liste des Key-Value-Stores
 * Funktionen die in die verkette Liste schreiben. Dabei soll die Speicherung Sortiert ablaufen
 * um spätere Suchalgorithmen wie Binary Search zu unterstützen.
 * Mehrere Szenarien müssen bedacht werden:
 * - Die Liste ist leer, der zu schreibende Key ist der Erste.
 * - Die Liste ist nicht leer:
 *   - Der Key ist der größte
 *   - Der Key ist der kleinste
 *   - Der Key liegt zwischen anderen Keys.
 *   writeToList ist die Hauptfunktion für die Speicherung und verwendet writeToEnd.
 */

void writeToEnd(char* key, char* value);
void writeToList(char* key, char* value);

/*
 * ### put - Elemente sortiert in die Liste eintragen
 * writeToList fügt Key+Value sortiert nach der Wertigkeit von Key in die Liste ein.
 * Die Funktion benutzt writeToEnd() und reserviert den benötigten Speicherplatz.
 * Übergeben wird der Schlüssel (key) und der Wert (value) als char Array.
 */
void writeToList(char* key, char* value) {
    struct keyValKomb *point, *bevorpoint;
    // Prüfen, ob es eine Liste gibt, wenn nicht writeToEnd.
    if (anfang == NULL) {
        writeToEnd(key, value);
    }
    // Liste durchlaufen so lange $key größer als $point.key ist.
    point = anfang;
    while(point != NULL && (strcmp(point->key, key) < 0)) {
        point = point->next;
    }
    // Prüfen, ob key die höchste Wertigkeit hat, dann writeToEnd.
    if(point == NULL) {
        writeToEnd(key, value);
    }
    // Prüfen, ob key die niedrigste Wertigkeit hat, dann key-value auf $anfang speichern und liste schieben.
    else if(point == anfang) {
        anfang=malloc(sizeof(struct keyValKomp));
        strcpy(anfang->key, key);
        strcpy(anfang->value, value);
        anfang->next = point;
    }
    // Position finden an der $point beim Durchlauf stehen geblieben ist und $nextpoint dahinter einfügen.
    // Liste schieben und Kette schließen.
    else {
        bevorpoint=anfang;
        while(bevorpoint->next != point) {
            bevorpoint=bevorpoint->next
        }
        point=malloc(sizeof(struct keyValKomp));
        strcpy(point->key, key);
        strcpy(point->value, value);
        point->next=bevorpoint->next;
        bevorpoint->next=point;
    }
}

/*
 * ### writeToEnd - Element am Ende einfügen
 * writeToEnd hängt Key+Value am Ende der Liste an, ist die Liste leer wird das Element in $anfang gespeichert.
 * Die Funktion reserviert dabei den benötigen Speicherplatz.
 * Übergeben wird der Schlüssel (key) sowie der Wert (value) als char Array.
 */
void writeToEnd(char* key, char* value) {
    struct keyValKomb *point;

    //Prüfen, ob die Liste leer ist.
    if(anfang == NULL) {
        // Wenn die Liste leer ist, wird der Speicher reserviert.
        if((anfang = malloc(sizeof(struct keyValKomb))) == NULL) {
            printf(stderr, "Kein Speicherplatz für keyValKomb vorhanden");
            return;
        }
        // Schlüssel+Wert per Pointer übertragen und $anfang.next auf NULL setzen.
        strcpy(anfang->key, key);
        strcpy(anfang->value, value);
        anfang->next = NULL;
    }
    else {
        // Wenn die Liste nicht leer ist, läuft die Schleife bis zum Letzen Element.
        point=anfang;
        while(point->next != NULL) {
            point = point->next;
        }
        // der Speicher wird reserviert
        if((point->next = malloc(sizeof(struct keyValKomb))) == NULL) {
            printf(stderr, "Kein Speicherplatz fur keyValKomb vorhanden");
            return;
        }
        // Schlüssel und Wert werden übertragen und zuvor $point auf den reservierten Speicher gelegt.
        point=point->next;
        strcpy(point->key, key);
        strcpy(point->value, value);
    }
}
