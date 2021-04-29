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
#include <netinet/in.h>
#include "log/log.h"
#include <string.h>
#include "interface.h"

#define ENDLOSSCHLEIFE 1
#define PORT 5678

int server_start() {
    int rfd; // Rendevouz-Descriptor
    int cfd; // Verbindungs-Descriptor

    struct sockaddr_in client; // Socketadresse eines Clients
    socklen_t client_len; // Länge der Client-Daten
    char in[BUFSIZE]; // Daten vom Client an den Server
    char out[BUFSIZE]; // Daten vom Server an den Client
    int bytes_read; // Anzahl der Bytes, die der Client geschickt hat

    // Socket erstellen
    rfd = socket(AF_INET, SOCK_STREAM, 0);
    log_info(":server_start Socket wurde erstellt");

    if (rfd < 0 ){
        log_fatal(":server_start Socket konnte nicht erstellt werden");
        exit(-1);
    }

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
    int lrt = listen(rfd, 5);
    if (lrt < 0 ){
        log_fatal(":server_start Socket konnte nicht auf listen gesetzt werden");
        exit(-1);
    }

    while (ENDLOSSCHLEIFE) {
        // Verbindung eines Clients wird entgegengenommen
        log_info(":server_start Socket lauscht");
        cfd = accept(rfd, (struct sockaddr *) &client, &client_len);

  /*      // Lesen von Daten, die der Client schickt
        bytes_read = read(cfd, in, BUFSIZE);

        log_debug(":server_start %d bytes empfangen", bytes_read);
        log_debug(":server_start folgende Daten empfangen: %s", in);*/

        // Hier befindet sich die Programmlogik
        do{
            clearArray(in);
            clearArray(out);

            //Socket wird eingelesen
            bytes_read = read(cfd, in, BUFSIZE);
            log_debug(":server_start %d bytes empfangen", bytes_read);

            int returnCodeInterface = interface(in, out);
            //Überprüfung ob Socket geschlossen werden soll.
            if (returnCodeInterface==-3){
                close(cfd);
            }
            //Ausgabe
            write(cfd, out, strlen(out));
        } while (bytes_read > 0);
        close(cfd);
        log_warn(":server_start %d Socket geschlossen");
    }
    // Rendevouz Descriptor schließen
    close(rfd);

}