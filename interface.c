/*******************************************************************************

  Diese Datei stellt die Schnittstelle zwischen den Clienteingaben und
  dem Server her.
  Die empfangenen Datenströhme werden hier interpretiert und genutzt um die
  gewünschten Funktionen zu starten.


*******************************************************************************/

#include "interface.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "log/log.h"
#include "keyValStore.h"
#include "server.h"
#include <unistd.h>

bool startsWith(const char *pre, const char *str);
int interface(char* in, char* out);
int clearArray(char* array);
int getKey(char* in, char* out);
int getValue(char* in, char* out);
int cpyPartOfArray(char* in, char* out, int start, int end);


int interface(char* in, char* out){
    char key[LENGTH_KEY];
    char value[LENGTH_VALUE];

    char resValue[LENGTH_VALUE];

    clearArray(out);
    /********
       GET
     *******/
    if (startsWith("GET",in)){
        //Key aus dem Aufrufsstring ermitteln
        int returnCodeKey;
        returnCodeKey = getKey(in, key);

        //Mögliche Fehler abfangen
        switch(returnCodeKey){
            case -1: snprintf(out, BUFSIZE, "%s", "command_nonexistent\r\n"); return 0;
            case -2: snprintf(out, BUFSIZE, "%s", "key_too_long\r\n"); return 0;
        }

        //get Funktion aufrufen
        switch(get(key, resValue)){
            //Ausgabe
            case -2: snprintf(out, BUFSIZE, "GET:%s:key_nonexistent\r\n", key); return 0;
            case 0: snprintf(out, BUFSIZE, "GET:%s:%s\r\n",key, resValue); return 0;
        }
    }
    /********
       PUT
    *******/
    else if (startsWith("PUT",in)){
        //Key und Value aus dem Aufrufsstring ermitteln
        int returnCodeKey = getKey(in, key);
        int returnCodeValue = getValue(in, value);

        //Mögliche Fehler abfangen
        switch(returnCodeKey){
            case -1: snprintf(out, BUFSIZE, "%s", "command_nonexistent\r\n"); return 0;
            case -2: snprintf(out, BUFSIZE, "%s", "key_too_long\r\n"); return 0;
        }
        switch(returnCodeValue){
            case -1: snprintf(out, BUFSIZE, "%s", "command_nonexistent\r\n"); return 0;
            case -2: snprintf(out, BUFSIZE, "%s", "value_too_long\r\n"); return 0;
        }

        //put Funktion aufrufen
        put(key, value);

        //Ausgabe
        snprintf(out, BUFSIZE, "PUT:%s:%s\r\n", key, value);
        return 0;
    }
    /********
       DEL
    *******/
    else if (startsWith("DEL",in)){
        //Key aus dem Aufrufsstring ermitteln
        int returnCodeKey;
        returnCodeKey = getKey(in, key);

        //Mögliche Fehler abfangen
        switch(returnCodeKey){
            case -1: snprintf(out, BUFSIZE, "%s", "command_nonexistent\r\n"); return 0;
            case -2: snprintf(out, BUFSIZE, "%s", "key_too_long\r\n"); return 0;
        }

        //del Funktion aufrufen
        switch(del(key)){
            //Ausgabe
            case -1: snprintf(out, BUFSIZE, "DEL:%s:key_nonexistent\r\n", key); return 0;
            case 0: snprintf(out, BUFSIZE, "DEL:%s:key_deleted\r\n", key); return 0;
        }
    }
    /********
       BEG
    *******/
    else if (startsWith("BEG",in)){
        log_info(":interface Exklusive Transaktion gestartet (BEG)");
        log_debug(":interface Prozess ID %d", getpid());
        beginExklusive(getpid());
        snprintf(out, BUFSIZE, "BEG\r\n", key, value);
        return 0;
    }
    /********
       END
    *******/
    else if (startsWith("END",in)){
        log_info(":interface Exklusive Transaktion beendet (END)");
        log_debug(":interface Prozess ID %d", getpid());
        endExklusive(getpid());
        snprintf(out, BUFSIZE, "END\r\n", key, value);
        return 0;
    }
    /********
      QUIT
    *******/
    else if (startsWith("QUIT",in)){
        return -3;
    }
    /*********************
     Unbekannte Eingaben
    **********************/
    else if (in[0]=='\r' || in[0]=='\0'){
        log_info(":interface Leere Eingabe wurde ignoriert");
    }
    else {
        snprintf(out, BUFSIZE, "%s", "command_nonexistent\r\n");
        log_error(":interface Daten empfangen jedoch wurde kein Keyword erkannt");
        return -2;
    }
}

int getKey(char* in, char* out){
    // Schleife fängt ab 4 an, da die ersten Zeichen immer der Methodenaufruf sind.
    const int keyAnfang = 4;

    //Herausfinden wie lang der Key ist
    int keyEnde = keyAnfang;
    while(in[keyEnde] != '\r' && in[keyEnde] != ' ' && in[keyEnde] != '\0' && in[keyEnde] != '\n'){
        keyEnde++;
    }

    //Überprüfen ob Key vorhanden ist, sonst fehler.
    if(keyEnde == keyAnfang){
        log_error(":getKey kein Key vorhanden.");
        return -1;
    }

    //Überprüfung ob Key zu lang ist.
    if(keyEnde - keyAnfang > LENGTH_KEY-1){
        log_error(":getKey Key ist zu lang");
        return -2;
    }

    //Array leeren um den Key nicht zu verfälschen
    clearArray(out);

    // Key wird in das Char Array out geschrieben.
    cpyPartOfArray(in, out, keyAnfang, keyEnde);
    log_debug(":getValue folgender Key wurde ermittelt: %s", out);
    return 0;
}

int getValue(char* in, char* out){
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
    while(in[valueEnde] != '\r' && in[valueEnde] != ' ' && in[valueEnde] != '\0' && in[valueEnde] != '\n'){
        valueEnde++;
    }

    //Überprüfen ob Key vorhanden ist, sonst fehler.
    if(valueEnde == valueAnfang){
        log_error(":getValue kein Value vorhanden.");
        return -1;
    }

    //Überprüfung ob Key zu lang ist.
    if(valueEnde - valueAnfang > LENGTH_VALUE-1){
        log_error(":getValue Value ist zu lang");
        return -2;
    }

    //Array leeren um den Key nicht zu verfälschen
    clearArray(out);

    // Key wird in das Char Array out geschrieben.
    cpyPartOfArray(in, out, valueAnfang, valueEnde);
    log_debug(":getValue folgender Value wurde ermittelt: %s", out);
    return 0;
}

int cpyPartOfArray(char* in, char* out, int start, int end){
    clearArray(out);
    int j;
    int k = 0;
    for (j=start; j<end; j++){
        out[k] = in[j];
        k++;
    }
    out[k] = '\0';
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
    memset(&array[0], '\0', sizeof(array));
    return 0;
}