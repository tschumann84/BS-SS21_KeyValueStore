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
    /*put("1","erster");
    put("2","zweiter");
    put("3","dritter");*/
    sharedStore();
    server_start();
    return 0;
}