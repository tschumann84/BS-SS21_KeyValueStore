#include "sub.h"
#include "keyValStore.h"
#include "sys/types.h"
#include "sys/ipc.h"
#include "sys/msg.h"
#include "server.h"

struct liste {
    char key[LENGTH_KEY];
    int cfd;
};
struct liste* subliste;

int pub(char* key, char* res, int funktion){
    char out[BUFSIZE];
    int i = 0;
    switch(funktion){
        case 0: snprintf(out, BUFSIZE, "PUT:%s:%s\r\n",key,res); break;
        case 1: snprintf(out, BUFSIZE, "DEL:%s\r\n",key); break;
    }
    if(strcmp(subliste[i].key, "\0") != 0) {
        do {
            if (strcmp(subliste[i].key, key) == 0) {
//                for( n = 0; n < ServerSocket->Socket->ActiveConnections; n++ )
//                {
//                    ServerSocket->Socket->Connections[n]->SendText("%out",out);
//                }
                log_info(":sub Nachricht gedeset bei Key: %s",subliste[i].key);
            }
            i++;
        }while ((strcmp(subliste[i].key, "\0") != 0));

    }
    else {
        log_info(":sub Liste war Leer, keine Nachricht gesendet");
        return 0;
    }
}

int sub(char* key, int cfd) {
    char res = "";
    if(get(key, res)==1){

    log_info(":sub start");
    int i = 0;
    //locksem(semid, SEM_Store);
    if(strcmp(subliste[i].key, "\0") != 0) {
        log_info(":sub Liste leer, füge neues Element ein");
        strcpy(subliste[i].key, key);
        subliste[i].cfd = cfd;
        log_info(":sub wurde erstellt.");
        return 0;
    }
    else{
        int j = i+1;
        do{
        if(strcmp(subliste[i].key, "\0") != 0) {
            strcpy(subliste[i].key, key);
            subliste[i].cfd = cfd;
            //0 Byte an ende dran gesetzt
            strcpy(subliste[j].key, "\0");
        }
            i++;
    } while ((strcmp(subliste[i].key, "\0") != 0));
        return 0;
    }
    }
    else
    {
        log_info(":sub Key existiert nicht. Kein Sub möglich!");
        return -1;
    }
}

int desub(char* key, int cfd){
    log_info(":desub start");
    int i = 0;
    log_info(":desub suche nach Key.");
    if(strcmp(subliste[i].key, "\0") != 0) {
        // Wir suchen in der Kette, ob das Element vorhanden ist.
        do{
            if(strcmp(subliste[i].key, key) == 0 && subliste[i].cfd, cfd) {
                log_info(":desub Key gefunden.");
                int j = i+1;

                do{
                    strcpy(subliste[i].key, subliste[j].key);
                    subliste[i].cfd =subliste[j].cfd;
                    j++;
                    i++;
                    log_info(":desub Key gelöscht.");
                }while ((strcmp(subliste[j-1].key, "\0") != 0));
                return 0;
            }
            i++;
        } while ((strcmp(subliste[i].key, "\0") != 0));
        log_info(":desub Key nicht gefunden.");
        return -1;
    }
    else {
        log_info(":desub keine Keys in Subliste.");
        return -1;
    }
}

//    void schlange(int argc, char **argv) {
//        int msid, v;
//        struct text_message mess;
//        if(argc!= 4){
//            printf(2, "usage:msgsnd<key> <type> <text>\n");
//            exit(1)
//        }
//        //Schlange wurde erstellt, wenn notwendig
//        msid = msgget((key_t)atoi(argv[1]), IPC_CREAT|0666);
//        if(msid==-1) {
//            printf("can not get message queue\n");
//            exit(1);
//        }
//        mess.key = atoi(argv[2]);
//        strcpy(mess.mtext,argv[3]);
//        //Nachricht wird in schlange geschrieben
//        v = msgsnd(msid, & mess, strlen(argv[3])+1, 0);
//        if(v < 0) {
//            printf("error writing to queue\n");
//        }
//    }
