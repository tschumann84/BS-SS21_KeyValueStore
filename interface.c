//
// Created by benja on 27.04.2021.
//

#include "interface.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "log/log.h"
#include "keyValStore.h"

bool startsWith(const char *pre, const char *str);
int interface(char* in);
int clearArray(char* array);
int getKey(char* in, char* out);
int getValue(char* in, char* out);
int cpyPartOfArray(char* in, char* out, int start, int end);


int interface(char* in){
    char key[LENGTH_KEY];
    char value[LENGTH_VALUE];

    char resValue[LENGTH_VALUE];

    if (startsWith("GET",in)){
        log_info(":interface GET() Aufruf wurde erkannt");
        //Key aus dem Aufrufsstring ermitteln
        getKey(in, key);

        //get Funktion aufrufen
        get(key, resValue);
    }else if (startsWith("PUT",in)){
        log_info(":interface PUT() Aufruf wurde erkannt");

        //Key und Value aus dem Aufrufsstring ermitteln
        getKey(in, key);
        getValue(in, value);

        //put Funktion aufrufen
        put(key, value);
    }else if (startsWith("DEL",in)){
        //Key aus dem Aufrufsstring ermitteln
        getKey(in, key);

        //del Funktion aufrufen
        del(key);
    }else if (startsWith("QUIT",in)){
        log_info(":interface QUIT() Aufruf wurde erkannt.");
    }else {
        log_error(":interface Daten empfangen jedoch wurde kein Keyword erkannt");
    }
}

int getKey(char* in, char* out){
    log_info(":getKey Key wird ermittelt");

    // Schleife fängt ab 4 an, da die ersten Zeichen immer der Methodenaufruf sind.
    const int keyAnfang = 4;

    int keyEnde = keyAnfang;
    //Herausfinden wie lang der Key ist
    while(in[keyEnde] != '\r' && in[keyEnde] != ' ' && in[keyEnde] != '\0'){
        keyEnde++;
    }

    //Überprüfung ob Key zu lang ist.
    if(keyEnde - keyAnfang > LENGTH_KEY){
        log_error(":getKey Key ist zu lang");
        return -1;
    }

    //Array leeren um den Key nicht zu verfälschen
    clearArray(out);

    // Key wird in das Char Array out geschrieben.
    cpyPartOfArray(in, out, keyAnfang, keyEnde);
    log_debug(":getValue folgender Key wurde ermittelt: %s", out);
    return 0;
}

int getValue(char* in, char* out){
    log_info(":getValue Value wird ermittelt");

    // Schleife fängt ab 4 an, da die ersten Zeichen immer der Methodenaufruf sind.
    const int schleifenAnfang = 4;

    //Herausfinden wann der Value beginnt
    int valueAnfang = schleifenAnfang;
    while(in[valueAnfang] != ' '){
        valueAnfang++;
    }
    valueAnfang++;

    //Herausfinden wann der Value endet
    int valueEnde = valueAnfang;
    while(in[valueEnde] != '\r' && in[valueEnde] != ' ' && in[valueEnde] != '\0'){
        valueEnde++;
    }

    //Überprüfung ob Key zu lang ist.
    if(valueEnde - valueAnfang > LENGTH_VALUE){
        log_error(":getValue Value ist zu lang");
        return -1;
    }

    //Array leeren um den Key nicht zu verfälschen
    clearArray(out);

    // Key wird in das Char Array out geschrieben.
    cpyPartOfArray(in, out, valueAnfang, valueEnde);
    log_debug(":getValue folgender Value wurde ermittelt: %s", out);
    return 0;
}

int cpyPartOfArray(char* in, char* out, int start, int end){
    log_info(":cpyPartOfArray wurde aufgerufen");
    int j;
    int k = 0;
    for (j=start; j<end; j++){
        out[k] = in[j];
        k++;
    }
    return 0;
}

//Prüft ob der Anfang des Strings pre im String str enthalten ist.
bool startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
            lenstr = strlen(str);
    return lenstr < lenpre ? false : memcmp(pre, str, lenpre) == 0;
}

int clearArray(char* array){
    log_info(":clearArray wurde aufgerufen");
    memset(&array[0], 0, sizeof(array));
    return 0;
}