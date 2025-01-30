#include "captain.h"
#include <signal.h>
#include <stdbool.h>

static volatile sig_atomic_t force_departure = 0;



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

    printf("[CAPTAIN] Thread started totalPlanes=%d\n", cp->totalPlanes);

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
        }

        if ( doDepart) {
            
            plane->isDeparting = true;
            pthread_cond_broadcast(&plane->planeCond);
        pthread_mutex_unlock(&plane->planeLock);
sem_wait(&stairsSem);
            


            
            printf("[CAPTAIN] Departing planeId=%d (onBoard=%d)\n",
                   plane->planeId, plane->currentOnBoard);
            sleep(cp->flightTime);

            printf("[CAPTAIN] Landed. Clearing plane...\n");
            clear_plane(plane);
                sem_post(&stairsSem);
            pthread_mutex_lock(&plane->planeLock);

           
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
    }

    printf("[CAPTAIN] Used all planes. Exiting.\n");
    return NULL;
}
