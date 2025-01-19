#include "operacje.h"

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Usage: %s <plane_id> <passenger_id> <baggage_amount> <is_VIP>\n", argv[0]);
        exit(1);
    }

    int plane_id = atoi(argv[1]);
    int passenger_id = atoi(argv[2]);
    int baggage_amount = atoi(argv[3]);
    bool is_VIP = atoi(argv[4]);

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

    printf("Passenger %d started\n", passenger_id);
fflush(stdout);
    // Sprawdzenie wagi bagażu
    if (baggage_amount > BAGGAGE_LIMIT) {
        printf("Passenger %d has too much baggage (%d > %d)\n", passenger_id, baggage_amount, BAGGAGE_LIMIT);
        shmdt(shared_data);
        exit(1);
    }

    // Symulacja kontroli bezpieczeństwa
    printf("Passenger %d undergoing security check...\n", passenger_id);
fflush(stdout);
    sleep(1);

    // Wejście na schody
    printf("Passenger %d waiting for stairs...\n", passenger_id);
fflush(stdout);
    wait_semaphore(semid, 1); // Zmniejszenie limitu schodów
    wait_semaphore(semid, 0); // Dostęp do sekcji krytycznej
    shared_data->passengers_on_stairs += 1;
    signal_semaphore(semid, 0); // Zwolnienie sekcji krytycznej

    sleep(1); // Symulacja wchodzenia po schodach

    // Wysyłanie komunikatu do samolotu
    message_t message;
    message.mtype = 1; // Typ komunikatu (np. 1 dla pasażerów)
    message.plane_id = plane_id;
    message.passenger_id = passenger_id;
    message.baggage_amount = baggage_amount;
    message.is_VIP = is_VIP;
    snprintf(message.mtext, sizeof(message.mtext), "Passenger %d ready to board", passenger_id);

    if (msgsnd(msgid, &message, sizeof(message_t) - sizeof(long), 0) == -1) {
        perror("msgsnd");
        exit(1);
    }

    // Schodzimy ze schodów i wchodzimy do samolotu
    wait_semaphore(semid, 0); // Dostęp do sekcji krytycznej
    shared_data->passengers_on_stairs -= 1;
    shared_data->passengers_capacity += 1;
    signal_semaphore(semid, 0); // Zwolnienie sekcji krytycznej
    signal_semaphore(semid, 1); // Zwolnienie miejsca na schodach

    printf("Passenger %d finished\n", passenger_id);
fflush(stdout);

    shmdt(shared_data);
    return 0;
}