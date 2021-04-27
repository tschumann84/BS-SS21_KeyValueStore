//
// Created by benja on 27.04.2021.
//

#include "interface.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "log/log.h"

bool startsWith(const char *pre, const char *str);
int interface(char* in);

int interface(char* in){
    if (startsWith("GET",in)){
        log_info(":interface GET erkannt");
    }else if (startsWith("PUT",in)){
        log_info(":interface PUT erkannt");
    }else if (startsWith("DEL",in)){
        log_info(":interface DEL erkannt");
    }else if (startsWith("QUIT",in)){
        log_info(":interface QUIT erkannt");
    }else {
        log_info(":interface Daten empfangen jedoch kein Keyword erkannt.");
    }
}

//Prüft ob der Anfang des Strings pre im String str enthalten ist.
bool startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
            lenstr = strlen(str);
    return lenstr < lenpre ? false : memcmp(pre, str, lenpre) == 0;
}