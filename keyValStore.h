//
// Created by benja on 20.04.2021.
//

#ifndef BS21_KEYVALSTORE_H
#define BS21_KEYVALSTORE_H

#endif //BS21_KEYVALSTORE_H

// ### const und structs
//Länge von Key und Value
const lengthKey = 10;
const lengthValue = 10;

// struct für Schlüssel und Wert zur Speicherung als verkettete List.
typedef struct keyValKomb {
    char key[lengthKey];
    char value[lengthValue];
    struct keyValKomb *next;
};
// ### Öffentliche Funktionen
int put(char* key, char* value);
int get(char* key, char* res);
int del(char* key);