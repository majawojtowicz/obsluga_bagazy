<<<<<<< Updated upstream
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "operacje.h"

int main(int argc, char *argv[]) {
    if (argc != 6) {
        printf("Usage: %s <plane_id> <id> <capacity> <baggage_capacity> <capitan>\n", argv[0]);
        exit(1);
    }

    // Pobranie argumentów z argv
    int plane_id = atoi(argv[1]);             // ID samolotu
    int id = atoi(argv[2]);                  // ID aktualnego procesu
    int capacity = atoi(argv[3]);            // Pojemność samolotu
    int baggage_capacity = atoi(argv[4]);    // Maksymalna pojemność bagażu
    char *capitan = argv[5];                 // Nazwa kapitana (ciąg znaków)

    int shmid, semid, msgid;
    shared_data_t *shared_data;

    // Połączenie z pamięcią dzieloną
    shmid = shmget(shm_key, sizeof(shared_data_t), 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }
    shared_data = (shared_data_t *)shmat(shmid, NULL, 0);
    if (shared_data == (void *)-1) {
        perror("shmat");
        exit(1);
    }

    // Połączenie z semaforami
    semid = semget(sem_key, 4, 0666);
    if (semid == -1) {
        perror("semget");
        exit(1);
    }

    // Połączenie z kolejką komunikatów
    msgid = msgget(msg_key, 0666);
    if (msgid == -1) {
        perror("msgget");
        exit(1);
    }

    printf("Plane %d (Capitan: %s) started\n", plane_id, capitan);
fflush(stdout);
    // Symulacja pracy samolotu
    sleep(2);

    // Aktualizacja danych w pamięci dzielonej
    wait_semaphore(semid, 0); // Sekcja krytyczna
    shared_data->current_plane = id;
    shared_data->passengers_capacity = capacity;
    shared_data->baggage_capacity = baggage_capacity;
    shared_data->finished_planes += 1;
    signal_semaphore(semid, 0); // Koniec sekcji krytycznej

    printf("Plane %d (Capitan: %s) finished\n", plane_id, capitan);
fflush(stdout);
    // Odłączenie pamięci dzielonej
    shmdt(shared_data);
    return 0;
=======
#include "plane.h"

void init_plane(Plane *plane, int id, int capacity, int maxBaggage)
{
    plane->planeId      = id;
    plane->capacity     = capacity;
    plane->maxBaggage   = maxBaggage;
    plane->currentOnBoard = 0;
}

void plane_board_passenger(Plane *plane)
{
   
    if (plane->currentOnBoard < plane->capacity) {
        plane->currentOnBoard++;
    } else {
        pthread_mutex_lock(&mutex_log);
        fprintf(stderr, "[Plane %d] Błąd: samolot jest pełny!\n", plane->planeId);
        pthread_mutex_unlock(&mutex_log);
    }
}

void plane_clear(Plane *plane)
{
    plane->currentOnBoard = 0;
plane->maxBaggage = (rand() % 10) + 5;
pthread_mutex_lock(&mutex_log);
    fprintf(stderr, "[Plane %d] Nowy limit bagażu: %d\n", plane->planeId, plane->maxBaggage);
    pthread_mutex_unlock(&mutex_log);
>>>>>>> Stashed changes
}