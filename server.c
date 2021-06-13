/*******************************************************************************

  Ein TCP-Server.
  Quelle: https://wiki.moxd.io/pages/viewpage.action?pageId=94700009

  In seiner Funktionsweise an die Anforderungen angepasst.
*******************************************************************************/

#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include "log/log.h"
#include <string.h>
#include "interface.h"
#include "keyValStore.h"
#include "sub.h"

#define PORT 5678
#define SERVER_BACKLOG 15

int rfd, cfd;
pid_t childpid;
volatile sig_atomic_t schleife = 1;
struct sockaddr_in client;

int server_stop() {
    //Abfangen des Sonderfalls, dass ein Prozess noch keinen Fork erstellt hat.
    if ((childpid == 0) && (getProcCount() == 0)) {
        saveBlockShutdown(getpid());
        close(rfd);
        saveUnblockShutdown(getpid());
        delete();
        sub_delete();
        log_info(":stopServer Server erfolgreich heruntergefahren. Prozess hatte noch keine Kinder.");
        exit(EXIT_SUCCESS);
    } else {
        //Normalfall
        if (childpid !=0){
            while (true){
                if (getProcCount()==0){
                    log_info(":stopServer Vaterprozess macht den Rest sauber.");
                    saveBlockShutdown(getpid());
                    close(rfd);
                    saveUnblockShutdown(getpid());
                    delete();
                    sub_delete();
                    log_info(":stopServer Server erfolgreich heruntergefahren.");
                    exit(EXIT_SUCCESS);
                } else{
                    usleep(2);
                }
            }
        } else{
            saveBlockShutdown(getpid());
            schleife = 0;
            shutdown(cfd, 2);
            close(cfd);
            saveUnblockShutdown(getpid());
        }
    }
}

int server_start() {
    socklen_t client_len; // Länge der Client-Daten

    char in[BUFSIZE]; // Daten vom Client an den Server
    char out[BUFSIZE]; // Daten vom Server an den Client
    int bytes_read; // Anzahl der Bytes, die der Client geschickt hat

    // Socket erstellen
    rfd = socket(AF_INET, SOCK_STREAM, 0);
    if (rfd < 0 ){
        log_fatal(":server_start Socket konnte nicht erstellt werden");
        exit(-1);
    }
    log_info(":server_start Socket wurde erstellt");


    // Socket Optionen setzen für schnelles wiederholtes Binden der Adresse
    int option = 1;
    setsockopt(rfd, SOL_SOCKET, SO_REUSEADDR, (const void *) &option, sizeof(int));

    // Socket binden
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);
    int brt = bind(rfd, (struct sockaddr *) &server, sizeof(server));
    if (brt < 0 ){
        log_fatal(":server_start Socket konnte nicht gebunden werden");
        exit(-1);
    }
    log_info(":server_start Socket wurde gebunden");

    // Socket lauschen lassen
    if(listen(rfd, SERVER_BACKLOG)==0){
        log_info(":server_start Socket lauscht");
    } else{
        log_fatal(":server_start Socket lauscht nicht, möglicherweise zu viele Clients");
    }

    // Client_Len initialisieren um abstürze zu verhindern.
    client_len = sizeof( (struct sockaddr *) &server);
    while (schleife) {
        // Verbindung eines Clients wird entgegengenommen
        cfd = accept(rfd, (struct sockaddr *) &client, &client_len);
        if (cfd < 0 && getProcCount()!=0) {
            exit(-1);
        } else if(cfd <0){
            return 0;
        }
        incrementProcCount();
        if ((childpid = fork()) == 0) {
            close(rfd);
            do{
                clearArray(in);
                clearArray(out);

                //Socket wird eingelesen
                bytes_read = read(cfd, in, BUFSIZE);
                if (bytes_read <= 0){
                    close(cfd);
                    break;
                }
                log_debug(":server_start %d bytes empfangen", bytes_read);
                interface(in, out);
                write(cfd, out, strlen(out));
            } while (bytes_read > 0);

            shutdown(cfd, 2);
            close(cfd);
            decrementProcCount();
        }
    }
    close(rfd);
    return 0;
}

int getCFD(){
    return cfd;
}

struct sockaddr_in getSocketaddrClient(){
    return client;
}