#include "common.h"
#include "plane.h"
#include "passenger.h"
#include "security.h"
#include "captain.h"
#include "dispatcher.h"
#include "operacje.h"

sem_t stairsSem;

Plane globalPlane;

volatile sig_atomic_t noMoreCheckIn = 0;


static void noMoreCheckIn_handler(int signo)
{
    if (signo == SIGUSR2) {
        noMoreCheckIn = 1;
    }
}

int main(int argc, char *argv[])
{
    int total_passengers = 0;
    int total_planes = 0;
    int stairs_capacity  = 0;
    int plane_capacity= 0;
    int max_baggage= 0;

   
    struct sigaction sa;
    sa.sa_handler = noMoreCheckIn_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR2, &sa, NULL);

    
    while (1) {
        printf("Podaj liczb pasazerow (%d..%d): ", MIN_PASSENGERS, MAX_PASSENGERS);
        fflush(stdout);
        if (scanf("%d", &total_passengers) != 1) {
            fprintf(stderr, "Blad wczytywania.\n");
            fseek(stdin,0,SEEK_END);
            continue;
        }
        if ( total_passengers < MIN_PASSENGERS || total_passengers > MAX_PASSENGERS) {
            fprintf(stderr, "Niewlasciwa liczba pasazerow.\n");
            continue;
    }
        break;
    }

    while (1) {
        printf("Podaj liczbe samolotow (%d..%d): ", MIN_PLANES, MAX_PLANES_ALLOWED);
        fflush(stdout);
        if (scanf("%d", &total_planes) != 1) {
            fprintf(stderr, "Blad wczytywania.\n");
            fseek( stdin,0, SEEK_END );
            continue;
        }
        if (total_planes < MIN_PLANES || total_planes > MAX_PLANES_ALLOWED) {
            fprintf(stderr, "Niewlasciwa liczba samolotow.\n");
        continue;
    }
    break;
    }

    while (1) {
        printf("Podaj pojemnosc schodow K (%d..%d): ", MIN_STAIRS, MAX_STAIRS);
        fflush(stdout);
        if (scanf("%d", &stairs_capacity) != 1) {
            fprintf(stderr, "Blad wczytywania.\n");
            fseek(stdin,0,SEEK_END);
            continue;
        }
        if (stairs_capacity < MIN_STAIRS || stairs_capacity > MAX_STAIRS) {
            fprintf(stderr, "Niewlasciwa pojemnosc schodow.\n");
            continue;
        }
        break;
    }

    while (1) {
        printf("Podaj pojemnosc samolotu P (%d..%d): ", MIN_CAPACITY, MAX_CAPACITY);
        fflush(stdout);
        if (scanf("%d", &plane_capacity) != 1) {
            fprintf(stderr, "Blad wczytywania.\n");
            fseek(stdin,0,SEEK_END);
            continue;
        }
        if (plane_capacity < MIN_CAPACITY || plane_capacity > MAX_CAPACITY) {
            fprintf(stderr, "Niewlasciwa pojemnosc.\n");
            continue;
        }
        break;
    }

    while (1) {
        printf("Podaj dopuszczalna wage bagazu Md (%d..%d): ", MIN_BAGGAGE, MAX_BAGGAGE);
        fflush(stdout);
        if (scanf("%d", &max_baggage) != 1) {
            fprintf(stderr, "Blad wczytywania.\n");
            fseek(stdin,0,SEEK_END);
            continue;
        }
        if (max_baggage < MIN_BAGGAGE || max_baggage > MAX_BAGGAGE) {
            fprintf(stderr, "Niewlasciwa waga.\n");
            continue;
        }
        break;
    }

    srand(time(NULL));

 
    init_plane(&globalPlane, 1, plane_capacity, max_baggage);
    
    init_security_stations();

 
    if (sem_init(&stairsSem, 0, stairs_capacity) != 0) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    
    int msgqid = create_msg_queue();
    if (msgqid < 0) {
        fprintf(stderr, "Nie udalo sie stworzyc kolejki.\n");
        exit(EXIT_FAILURE);
    }

    
    int logfd = open("simulation.log", O_CREAT | O_WRONLY | O_TRUNC, 0644 );
    if ( logfd == -1) {
    perror("open log");
    } else {
        const char *txt = "=== Start Symulacji ===\n";
        write(logfd, txt, strlen(txt));
        close(logfd);
    }

    
    pthread_t *passThreads = malloc(sizeof(pthread_t)* total_passengers);
    Passenger *passengers  = malloc(sizeof(Passenger)* total_passengers);

    for (int i = 0; i < total_passengers; i++) {
        passengers[i].id       = i+1;
       
        passengers[i].weight   = (rand() % (max_baggage + 15)) + 1;
        passengers[i].gender   = (rand()%2 == 0)? 'M':'F';
        passengers[i].isVIP    = (i % 5 == 0); 
        passengers[i].yields   = 0;
        passengers[i].isChecked= false;

        if (pthread_create(&passThreads[i], NULL, passenger_thread, &passengers[i]) != 0) {
            perror("pthread_create passenger");
        }
    }

    pthread_t capThread;
    CaptainParams *cp = malloc(sizeof(CaptainParams));
    cp->plane      = &globalPlane;
    cp->flightTime = 3; 
    cp->T1         = 10;
cp->totalPlanes= total_planes;
    if (pthread_create(&capThread, NULL, captain_thread, cp) != 0) {
        perror("pthread_create captain");
        free(cp);
        exit(EXIT_FAILURE);
    }

  
    pthread_t dispThread;
    DispatcherParams dp;
    dp.planesCount = total_planes;
    dp.T1          = 10;

    if (pthread_create(&dispThread, NULL, dispatcher_thread, &dp) != 0) {
        perror("pthread_create dispatcher");
    }

   
    for (int i=0; i<total_passengers; i++) {
        pthread_join(passThreads[i], NULL);
    }
free( passThreads);
    free(passengers);

    
    pthread_join(capThread, NULL);
    free(cp);

    pthread_join(dispThread, NULL);

    
    remove_msg_queue(msgqid);
    
sem_destroy(&stairsSem);

    printf("\n--- Symulacja zakończona ---\n\n");
    return 0;
}
