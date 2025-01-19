cat main.c
#include "common.h"
#include "dispatcher.h"
#include "plane.h"
#include "passenger.h"
#include "captain.h"
#include "security.h"
#include <semaphore.h>

Plane plane1;
sem_t stairsSem;

int main(int argc, char *argv[])
{
   
    while (1) {
        printf("Podaj liczbe pasazerow (1..%d): ", MAX_PASSENGERS);
        if (scanf("%d", &total_passengers) != 1) {
            fprintf(stderr, "Blad wczytywania liczby pasazerow.\n");
            
            fseek(stdin,0,SEEK_END);
            continue;
        }
        if (total_passengers < 1 || total_passengers > MAX_PASSENGERS) {
            fprintf(stderr, "Niewlasciwa liczba pasazerow.\n");
            continue;
        }
        break;
    }

    while (1) {
        printf("Podaj liczbe samolotow (1..%d): ", MAX_PLANES);
        if (scanf("%d", &total_planes) != 1) {
            fprintf(stderr, "Blad wczytywania liczby samolotow.\n");
            fseek(stdin,0,SEEK_END);
            continue;
        }
        if (total_planes < 1 || total_planes > MAX_PLANES) {
            fprintf(stderr, "Niewlasciwa liczba samolotow.\n");
            continue;
        }
        break;
    }

    while (1) {
        printf("Podaj pojemnosc schodow K (1..10): ");
        if (scanf("%d", &stairs_capacity) != 1) {
            fprintf(stderr, "Blad wczytywania schodow.\n");
            fseek(stdin,0,SEEK_END);
            continue;
        }
        if (stairs_capacity < 1 || stairs_capacity > 10) {
            fprintf(stderr, "Niewlasciwa pojemnosc schodow.\n");
            continue;
        }
        break;
    }

    while (1) {
        printf("Podaj pojemnosc samolotu P (1..50): ");
        if (scanf("%d", &plane_capacity) != 1) {
            fprintf(stderr, "Blad wczytywania pojemnosci.\n");
            fseek(stdin,0,SEEK_END);
            continue;
        }
        if (plane_capacity < 1 || plane_capacity > 50) {
            fprintf(stderr, "Niewlasciwa pojemnosc samolotu.\n");
            continue;
        }
        break;
    }

    while (1) {
        printf("Podaj dopuszczalna wage bagazu Md (1..100): ");
        if (scanf("%d", &max_baggage) != 1) {
            fprintf(stderr, "Blad wczytywania wagi.\n");
            fseek(stdin,0,SEEK_END);
            continue;
        }
        if (max_baggage < 1 || max_baggage > 100) {
            fprintf(stderr, "Niewlasciwa waga bagazu.\n");
            continue;
        }
        break;
    }

    srand(time(NULL));

    init_plane(&plane1, 1, plane_capacity, max_baggage);

    init_security_stations();

    sem_init(&stairsSem, 0, stairs_capacity);

    msgqid = create_msg_queue();
    if (msgqid < 0) {
        fprintf(stderr, "Nie udalo sie stworzyc kolejki komunikatow.\n");
        exit(EXIT_FAILURE);
    }

    int logfd = open("simulation.log", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (logfd == -1) {
        perror("open simulation.log");
    } else {
        const char *header = "=== Symulacja start ===\n";
        if (write(logfd, header, strlen(header)) == -1) {
            safe_perror("write to simulation.log");
        }
        close(logfd);
    }

    for (int i = 0; i < total_passengers; i++) {
        passengers[i].id       = i + 1;
        passengers[i].weight   = (rand() % (max_baggage+5)) + 1; 
        passengers[i].gender   = (rand() % 2 == 0) ? 'M' : 'F';
        passengers[i].isVIP    = ((i % 5) == 0); // co 5-ty to VIP
        passengers[i].yields   = 0;
        passengers[i].isChecked = false;
    }

    pthread_t passThreads[MAX_PASSENGERS];
    for (int i = 0; i < total_passengers; i++) {
        if (pthread_create(&passThreads[i], NULL, passenger_thread, &passengers[i]) != 0) {
            safe_perror("pthread_create passenger");
        }
    }

    CaptainParams *cp = malloc(sizeof(CaptainParams));
    cp->plane       = &plane1;
    cp->flightTime  = 3;   
    cp->T1          = 10;  // cz 10 sekund (lub do force_departure)

    pthread_t captThread;
    if (pthread_create(&captThread, NULL, captain_thread, cp) != 0) {
        safe_perror("pthread_create captain");
        free(cp);
        return 1;
    }

    pthread_t dispatcherThreadId;
    DispatcherParams dp;
    dp.planesCount = total_planes;
    dp.T1 = 10;

    if (pthread_create(&dispatcherThreadId, NULL, dispatcher_thread, &dp) != 0) {
        safe_perror("pthread_create dispatcher");
    }


    for (int i = 0; i < total_passengers; i++) {
        pthread_join(passThreads[i], NULL);
    }


    pthread_join(captThread, NULL);
    free(cp);


    pthread_join(dispatcherThreadId, NULL);

    
    remove_msg_queue(msgqid);
    sem_destroy(&stairsSem);

    fprintf(stderr, "Symulacja zakończona.\n");
    return 0;
}