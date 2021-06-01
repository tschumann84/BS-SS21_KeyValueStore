//
// Created by benja on 20.04.2021.
//

#ifndef BS21_KEYVALSTORE_H
#define BS21_KEYVALSTORE_H

//Anzahl Schluessel für Store
#define STORESIZE 500

//Länge von Key und Value
#define LENGTH_KEY 100
#define LENGTH_VALUE 100

// struct für Schlüssel und Wert zur Speicherung als verkettete List.

struct keyValKomb {
    char key[LENGTH_KEY];
    char value[LENGTH_VALUE];
//    struct keyValKomb *next;
};

// ### Öffentliche Funktionen
int put(char* key, char* value);
int get(char* key, char* res);
int del(char* key);

#endif //BS21_KEYVALSTORE_H
