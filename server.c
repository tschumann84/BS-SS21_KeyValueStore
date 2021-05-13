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
#include "log/log.h"
#include <string.h>
#include "interface.h"

#define PORT 5678
#define SERVER_BACKLOG 5

int server_start() {
    int rfd; // Rendevouz-Descriptor
    int cfd; // Verbindungs-Descriptor
    struct sockaddr_in client; // Socketadresse eines Clients
    socklen_t client_len; // Länge der Client-Daten

    char in[BUFSIZE]; // Daten vom Client an den Server
    char out[BUFSIZE]; // Daten vom Server an den Client
    int bytes_read; // Anzahl der Bytes, die der Client geschickt hat
    pid_t childpid;

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

    while (true) {
        // Verbindung eines Clients wird entgegengenommen
        cfd = accept(rfd, (struct sockaddr *) &client, &client_len);
        if (cfd < 0) {
            exit(-1);
        }
        log_info(":server_start Verbindung akzeptiert von: %s:%d", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

        if ((childpid = fork()) == 0) {
            close(rfd);

            do{
                clearArray(in);
                clearArray(out);

                //Socket wird eingelesen
                bytes_read = read(cfd, in, BUFSIZE);
                log_debug(":server_start %d bytes empfangen", bytes_read);

                int returnCodeInterface = interface(in, out);
                //Überprüfung ob Socket geschlossen werden soll.
                if (returnCodeInterface==-3){
                    log_info(":server_start Verbindung geschlossen von: %s:%d", inet_ntoa(client.sin_addr),
                             ntohs(client.sin_port));
                    close(cfd);
                    break;
                }else {
                    write(cfd, out, strlen(out));
                }
            } while (bytes_read > 0);
            close(cfd);
            log_warn(":server_start %d Socket geschlossen");
        }
    }
    close(rfd);
    return 0;
}