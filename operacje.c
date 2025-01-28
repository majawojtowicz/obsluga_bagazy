#include "operacje.h"

typedef struct {
    long mtype;
    char mtext[128];
} Message;

int create_msg_queue(void)
{
    key_t key = ftok(FTOK_PATH, FTOK_CHAR);
    if (key == -1) {
        perror("ftok");
        return -1;
    }

    int msgid = msgget(key, IPC_CREAT | 0600);
    if (msgid == -1) {
        perror("msgget");
        return -1;
    }
    return msgid;
}

void remove_msg_queue(int msgid)
{
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
    } else {
        printf("[OPERACJE] Message queue removed.\n");
    }
}