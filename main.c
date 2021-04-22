#include <stdio.h>
#include "main.h"
#include "log/log.h"
#include "keyValStore.h"



int main() {
    char hallo[10] = "c string";
    char diesdas[10] = "c string";
    log_info(hallo);

    put("12b3","Irgenein1");
    put("4","Happy");
    get("12b3", hallo);
    log_info(hallo);
    get("4", diesdas);
    log_info(diesdas);
    return 0;
}
