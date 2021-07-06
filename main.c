#include <stdio.h>
#include "main.h"
#include "log/log.h"
#include "keyValStore.h"
#include "server.h"
#include "sub.h"

int main() {
    // Signal Handler für das kontrollierte Herunterfahren
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = (__sighandler_t) server_stop;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);

    // Signal Handler für das Publishen eines PUT Befehls
    struct sigaction sigput;
    memset(&sigput, 0, sizeof(struct sigaction));
    sigput.sa_handler = (__sighandler_t) getMsgPut;
    sigaction(SIGTTIN, &sigput, NULL);

    // Signal Handler für das Publishen eines DEL Befehls
    struct sigaction sigdel;
    memset(&sigdel, 0, sizeof(struct sigaction));
    sigdel.sa_handler = (__sighandler_t) getMsgDel;
    sigaction(SIGTTOU, &sigdel, NULL);

    //Logging Level
    log_set_level(1);

    //Logging in .txt Datei ermöglichen
    remove("logging.txt");
    FILE *fp_test = fopen("logging.txt","w");
    log_add_fp(fp_test, 1);

    //Starten des Servers
    sharedStore();
    sub_sharedStore();
    server_start();

    //Bei Stop des Servers logging beenden.
    fclose(fp_test);
    return 0;
}