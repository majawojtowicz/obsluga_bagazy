#include "plane.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

void init_plane(Plane  *plane, int  id, int capacity, int maxBaggage)
{
    plane->planeId  = id;
    plane->capacity = capacity;
    plane->maxBaggage    = maxBaggage;
    plane->currentOnBoard= 0;

    pthread_mutex_init(&plane->planeLock, NULL);
    pthread_cond_init(&plane->planeCond, NULL);

    plane->isFull= false;
    plane->isDeparting  = false;
}

void board_passenger(Plane *plane, int passenger_id)
{
RETRY:
    pthread_mutex_lock(&plane->planeLock);

    
    while (plane->isDeparting) {
    pthread_cond_wait(&plane->planeCond, &plane->planeLock);
        
    }

  
    if (plane->currentOnBoard < plane->capacity) {
       
        plane->currentOnBoard++;
        printf("[PLANE %d] Passenger %d boarded (onBoard=%d/%d)\n",
               plane-> planeId,
               passenger_id,
               plane->currentOnBoard,
               plane->capacity);

        
        if (plane->currentOnBoard == plane->capacity) {
            plane->isFull = true;
          
        pthread_cond_broadcast(&plane->planeCond);
        }

        pthread_mutex_unlock(&plane->planeLock);
    }
    else {
        
        int oldId = plane->planeId;
        printf("[PLANE %d] Passenger %d f ound plane FULL => waiting for next plane...\n",
               plane->planeId, passenger_id);

       
        while (plane->planeId == oldId) {
        pthread_cond_wait(&plane->planeCond, &plane->planeLock);
        }

        
        
        pthread_mutex_unlock(&plane->planeLock);

        
        goto RETRY;
    }
}


void clear_plane(Plane *plane)
{
    pthread_mutex_lock(&plane->planeLock);

plane->currentOnBoard = 0;
plane->maxBaggage = (rand() % 10) + 5;
    plane-> planeId++;
    plane->isFull = false;
    plane->isDeparting = false;

    printf("[PLANE %d] Landed & cleared. Baggage limit=%d\n",
           plane-> planeId, plane->maxBaggage);

    pthread_cond_broadcast(&plane->planeCond);
    pthread_mutex_unlock(&plane->planeLock);
}
