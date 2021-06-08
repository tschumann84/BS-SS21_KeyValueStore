#include "sub.h"
#include "keyValStore.h"
#include "sys/types.h"
#include "sys/ipc.h"
#include "sys/msg.h"

struct text_message{
    char key[LENGTH_KEY];
    char mtext[100];
};

int sub(char* key) {
    char res = "";
    get(key, res);
}
    void schlange(int argc, char **argv) {
        int msid, v;
        struct text_message mess;
        if(argc!= 4){
            printf(2, "usage:msgsnd<key> <type> <text>\n");
            exit(1)
        }
        //Schlange wurde erstellt, wenn notwendig
        msid = msgget((key_t)atoi(argv[1]), IPC_CREAT|0666);
        if(msid==-1) {
            printf("can not get message queue\n");
            exit(1);
        }
        mess.key = atoi(argv[2]);
        strcpy(mess.mtext,argv[3]);
        //Nachricht wird in schlange geschrieben
        v = msgsnd(msid, & mess, strlen(argv[3])+1, 0);
        if(v < 0) {
            printf("error writing to queue\n");
        }
    }
