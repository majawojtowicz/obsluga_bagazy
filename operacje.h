#ifndef operacje_h
#define operacje_h

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <errno.h>
#include <time.h>

#define MAX_PASSENGERS 100
#define MAX_PLANES 10
#define BAGGAGE_LIMIT 100
#define STAIRS_LIMIT 10

#define shm_key 1234
#define sem_key 5678
#define msg_key 9101

typedef struct {
    long mtype;
    char mtext[100];
    int plane_id;
    int passenger_id;
    int baggage_amount;
    int is_VIP;

}message_t;

typedef struct {
    int passengers_capacity;
    int passengers_on_stairs;
    int baggage_capacity;
    int baggage_on_stairs;
    int current_plane;
    int finished_planes;

}shared_data_t;

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};

int wait_semaphore(int sem_id, int sem_num){
    struct sembuf operacje;
    operacje.sem_num = sem_num;
    operacje.sem_op = -1;
    operacje.sem_flg = 0;
    return semop(sem_id, &operacje, 1);
}

int signal_semaphore(int sem_id, int sem_num){
    struct sembuf operacje;
    operacje.sem_num = sem_num;
    operacje.sem_op = 1;
    operacje.sem_flg = 0;
    return semop(sem_id, &operacje, 1);
}

#endif
