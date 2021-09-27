/*******************************************************************************

  Ein TCP-Server.
  Quelle: https://wiki.moxd.io/pages/viewpage.action?pageId=94700009

  In seiner Funktionsweise an die Anforderungen angepasst.
*******************************************************************************/

#include "server.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include "sub.h"

#define PORT 5678
#define SERVER_BACKLOG 15

int rfd, cfd;
pid_t childpid;
volatile sig_atomic_t schleife = 1;
struct sockaddr_in client;

int server_stop() {
    //Abfangen des Sonderfalls, dass ein Prozess noch keinen Fork erstellt hat.
    log_info(":stopServer aufgerufen.");
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

        //Routine für den Vaterprozess
        if (childpid !=0){
            while (true){
                if (getProcCount()==0){
                    log_info(":stopServer Vaterprozess macht den Rest sauber.");
                    saveBlockShutdown(getpid());
                    close(cfd);
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
            //Routine für die Kindprozesse
        } else{
            saveBlockShutdown(getpid());
            schleife = 0;
            shutdown(cfd, 2);
            close(cfd);
            saveUnblockShutdown(getpid());
        }
    }
    return 0;
}



int server_start() {
    // Länge der Client-Daten
    socklen_t client_len;

    // Daten vom Client an den Server
    char in[BUFSIZE];

    // Daten vom Server an den Client
    char out[BUFSIZE];

    // Anzahl der Bytes, die der Client geschickt hat
    int bytes_read;
// #3a-2 [
    // Socket erstellen
    rfd = socket(AF_INET, SOCK_STREAM, 0);
    if (rfd < 0 ){
        log_fatal(":server_start Socket konnte nicht erstellt werden Fehler: %s", strerror(errno));
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
        log_fatal(":server_start Socket konnte nicht gebunden werden Fehler: %s", strerror(errno));
        exit(-1);
    }
    log_info(":server_start Socket wurde gebunden");

    // Socket lauschen lassen
    if(listen(rfd, SERVER_BACKLOG)==0){
        log_info(":server_start Socket lauscht");
    } else{
        log_fatal(":server_start Socket lauscht nicht. Fehler: %s", strerror(errno));
    }
// ] #3a-2
    // Client_Len initialisieren um Abstürze zu verhindern.
    client_len = sizeof( (struct sockaddr *) &server);

// #2a-1 [
    while (schleife) {
        // Verbindung eines Clients wird entgegengenommen
// #1a-1 [
        cfd = accept(rfd, (struct sockaddr *) &client, &client_len);
// ] #1a-1
        // Sonderlocke für ein geregeltes Terminieren der Sockets
        if (cfd < 0 && getProcCount()!=0) {
            exit(-1);
        } else if(cfd <0){
            return 0;
        }
        // Prozess Counter inkrementieren inkl. Log-Ausgabe
        incrementProcCount();

        if ((childpid = fork()) == 0) {
// ] #2a-1
            snprintf(out, BUFSIZE, "Welcome Nr %d\r\n", welcome());
            write(cfd, out, strlen(out));
            do{
                // Variablen in & out säubern, um saubere Ein- und Ausgaben zu erhalten.
                clearArray(in);
                clearArray(out);
// #2a-2 [
                //Socket wird eingelesen. Wenn es durch ein Signal unterbrochen wird, dann nochmal probieren.
                do {
// #1a-2 [
                    bytes_read = read(cfd, in, BUFSIZE);
// ] #1a-2
                } while(errno == EINTR && bytes_read <=0);
                log_debug(":server_start %d bytes empfangen", bytes_read);
// #2a-3 [
                //Interpretation der Daten
                interface(in, out);
// ] #2a-2
                //Ausgabe auf dem Socket
                write(cfd, out, strlen(out));
// ] #2a-3
            } while (bytes_read > 0);
            //Routine zum Schließen eines Sockets
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