#include <stdio.h>
#include "main.h"
#include "log/log.h"
#include "keyValStore.h"
#include "server.h"

int main() {
    // Signal Handler
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = (__sighandler_t) server_stop;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);

    //Logging Level
    log_set_level(1);

    //Starten des Servers
    sharedStore();
    server_start();

    return 0;
}