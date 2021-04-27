#include <stdio.h>
#include "main.h"
#include "log/log.h"
#include "keyValStore.h"
#include "server.h"


int main() {
    /* Logs:
     * Level 0: trace
     * Level 1: debug
     * Level 2: info
     * Level 3: warn
     * Level 4: error
     * Level 5: fatal */
    log_set_level(1);

    char hallo[10] = "c string";
    char diesdas[10] = "c string";
    log_info(hallo);
char res[LENGTH_VALUE];
    put("12b3","Irgenein1");
    put("4","Happy");
    put("1ab4", "5000");
    put("8g4h", "80000");
    get("12b3", hallo);
//    log_fatal(hallo);
    get("4", diesdas);
    del("12b3");
    get("12b3",res);
    printf("%s",res);
//    log_warn(diesdas);
    server_start();
    return 0;
}