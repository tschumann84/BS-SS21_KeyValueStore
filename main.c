#include <stdio.h>
#include "main.h"
#include "log/log.h"
#include "keyValStore.h"
#include "server.h"
#include "sub.h"

int main() {
    // Signal Handler
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = (__sighandler_t) server_stop;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);

    struct sigaction sigput;
    memset(&sigput, 0, sizeof(struct sigaction));
    sigput.sa_handler = (__sighandler_t) getMsgPut;
    sigaction(SIGTTIN, &sigput, NULL);

    struct sigaction sigdel;
    memset(&sigdel, 0, sizeof(struct sigaction));
    sigdel.sa_handler = (__sighandler_t) getMsgDel;
    sigaction(SIGTTOU, &sigdel, NULL);



    //Logging Level
    log_set_level(1);

    //Starten des Servers
    sharedStore();
    sub_sharedStore();
    server_start();

    return 0;
}