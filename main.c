#include <stdio.h>
#include "main.h"
#include "log/log.h"
#include "keyValStore.h"



int main() {
    /* Logs:
     * Level 0: trace
     * Level 1: debug
     * Level 2: info
     * Level 3: warn
     * Level 4: error
     * Level 5: fatal */
    log_set_level(0);

    char hallo[10] = "c string";
    char diesdas[10] = "c string";
    log_info(hallo);

    put("12b3","Irgenein1");
    put("4","Happy");
    get("12b3", hallo);
    log_fatal(hallo);
    get("4", diesdas);
    log_warn(diesdas);
    return 0;
}
