#include "captain.h"
#include <signal.h>
#include <stdbool.h>
#include "dispatcher.h"

extern volatile int passengersDelivered;
extern volatile int passengersDeclined;
extern volatile int currentPlanesOperating;
extern int total_passengers;

static volatile sig_atomic_t force_departure = 0;
static volatile sig_atomic_t noMoreCheckIn = 0;

static void captain_signal_handler(int signo)
{
    if (signo == SIGUSR1) {
        force_departure = 1;
    }
    if (signo == SIGUSR2) {
        noMoreCheckIn = 1;
    }
}

extern sem_t stairsSem; 

void *captain_thread(void *arg)
{
    CaptainParams *cp = (CaptainParams*)arg;
    Plane *plane      = cp->plane;

    struct sigaction sa;
    sa.sa_handler =captain_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);

     printf("[CAPTAIN %d] Thread started\n", plane->planeId);

    // oczekuj na zwiekszenie ilosci samolotow (dyspozytor)
    while (currentPlanesOperating<plane->planeId+1) {
        sleep(1);
    }
    // wyczysc poprzednie (zanim zaczelismy latac) force_departure
    force_departure=0;

printf("[CAPTAIN %d] plane summoned by the dispatcher.\n", plane->planeId);

    while (1) {
        bool doDepart = false;
pthread_mutex_lock(&plane->planeLock);

       
        int waited = 0;
        while (!plane->isFull && !force_departure && waited < cp->T1) {
            pthread_mutex_unlock(&plane->planeLock);
            sleep(1);
            waited++;
            pthread_mutex_lock(&plane->planeLock);
        }

        if (plane->isFull || force_departure || (waited >= cp->T1)) {
            doDepart = true;
                printf("[CAPTAIN] Plane %d ready to depart (onBoard=%d)\n",
                   plane->planeId, plane->currentOnBoard);
                   if (plane->isFull) printf(" being full\n");
                    else if (force_departure) printf(" forced by dispatcher\n");
                    else if (waited>=cp->T1) printf(" overtime\n");
        }

        if ( doDepart) {
            
            plane->isDeparting = true;
            pthread_cond_broadcast(&plane->planeCond);
        pthread_mutex_unlock(&plane->planeLock);
sem_wait(&stairsSem[plane->planeId]);
            


            
            printf("[CAPTAIN] Departing planeId=%d (onBoard=%d)\n",
                   plane->planeId, plane->currentOnBoard);

            //popros dyspozytora o podstawienie kolejnego samolotu 
            increasePlanesOnDeparture();

            pthread_mutex_lock(&plane->planeLock);
            //nie pozwalaj na disembark podczas lotu samolotu
            
            sleep(cp->flightTime);
            pthread_mutex_unlock(&plane->planeLock);


            printf("[CAPTAIN %d] Plane landed. Disembarked plane...\n", plane->planeId);
            clear_plane(plane);
                sem_post(&stairsSem)[plane->planeId];
            pthread_mutex_lock(&plane->planeLock);

           
           printPassengersStatusOnLand();
            if (plane->planeId > cp->totalPlanes) {
                pthread_mutex_unlock(&plane->planeLock);
                break;
            }

           
            force_departure = 0;
            pthread_mutex_unlock(&plane->planeLock);
    }
    else {
           
            pthread_mutex_unlock(&plane->planeLock);
        }
    if (passengersDelivered+passengersDeclined==total_passengers || noMoreCheckIn) break;
    }

    if (passengersDelivered+passengersDeclined==total_passengers) printf("[CAPTAIN %d] No more passangers to take. Exiting.\n", plane->planeId);
    else if (noMoreCheckIn) printf("[CAPTAIN %d] No more check-in today. Exiting\n", plane->planeId);
    pthread_exit(NULL);
}
