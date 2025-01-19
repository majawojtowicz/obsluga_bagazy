#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <errno.h>
#include <time.h>
#include <sys/wait.h>
#include "operacje.h"

int main() {
    int shmid;
    int semid;
    int msgid;
    shared_data_t *shared_data;

    shmid = shmget(shm_key, sizeof(shared_data_t), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    shared_data = (shared_data_t *)shmat(shmid, NULL, 0);
    if (shared_data == (void *)-1) {
        perror("shmat");
        exit(1);
    }

    semid = semget(sem_key, 4, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget");
        exit(1);
    }

    union semun semun;
    unsigned short vals[4] = {1, 1, 1, 1};
    semun.array = vals;
    if (semctl(semid, 0, SETALL, semun) == -1) {
        perror("semctl");
        exit(1);
    }

    msgid = msgget(msg_key, IPC_CREAT | 0666);
    if (msgid == -1) {
        perror("msgget");
        exit(1);
    }

    shared_data->passengers_capacity = 0;
    shared_data->passengers_on_stairs = 0;
    shared_data->baggage_capacity = 0;
    shared_data->current_plane = -1;
    shared_data->finished_planes = 0;

    for (int i = 0; i < MAX_PASSENGERS; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            char passenger_id_str[16];
            snprintf(passenger_id_str, sizeof(passenger_id_str), "%d", i);
            int baggage_amount = rand() % BAGGAGE_LIMIT;
            char baggage_amount_str[16];
            snprintf(baggage_amount_str, sizeof(baggage_amount_str), "%d", baggage_amount);
            char is_VIP_str[16];
            snprintf(is_VIP_str, sizeof(is_VIP_str), "%d", (i % 3 == 0) ? 1 : 0);
            char plane_id_str[16];
            snprintf(plane_id_str, sizeof(plane_id_str), "%d", rand() % MAX_PLANES);
            execl("./pasazer", "pasazer", passenger_id_str, baggage_amount_str, is_VIP_str, plane_id_str, NULL);
            perror("execl");
            exit(1);
        } else if (pid == -1) {
            perror("fork");
            exit(1);
        }
    }

    for (int i = 0; i < MAX_PLANES; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            char plane_id_str[16];
            snprintf(plane_id_str, sizeof(plane_id_str), "%d", i);
            char capacity_str[16];
            snprintf(capacity_str, sizeof(capacity_str), "%d", rand() % MAX_PASSENGERS);
            char baggage_capacity_str[16];
            snprintf(baggage_capacity_str, sizeof(baggage_capacity_str), "%d", rand() % BAGGAGE_LIMIT);
            execl("./plane", "plane", plane_id_str, capacity_str, baggage_capacity_str, NULL);
            perror("execl");
            exit(1);
        } else if (pid == -1) {
            perror("fork");
            exit(1);
        }
    }

    for (int i = 0; i < 1; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            char dyspozytor_id_str[16];
            snprintf(dyspozytor_id_str, sizeof(dyspozytor_id_str), "%d", i);
            execl("./dyspozytor", "dyspozytor", dyspozytor_id_str, NULL);
            perror("execl");
            exit(1);
        } else if (pid == -1) {
            perror("fork");
            exit(1);
        }
    }

    for (int i = 0; i < MAX_PASSENGERS + MAX_PLANES + 1; i++) {
        wait(NULL);
    }

    printf("All processes finished\n");
fflush(stdout);
    if (shmdt(shared_data) == -1) {
        perror("shmdt");
        exit(1);
    }

    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(1);
    }

    if (semctl(semid, 0, IPC_RMID, NULL) == -1) {
        perror("semctl");
        exit(1);
    }

    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }

    printf("All resources removed\n");
fflush(stdout);
    printf("End of program\n");
fflush(stdout);

    return 0;
}