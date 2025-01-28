#ifndef PLANE_H
#define PLANE_H

#include "common.h"
#include <pthread.h>
#include <stdbool.h>

typedef struct {
    int  planeId;          
    int  capacity;         
    int  maxBaggage;       
    int  currentOnBoard;   

    pthread_mutex_t planeLock;
    pthread_cond_t  planeCond;

    bool isFull;           
    bool isDeparting;      
} Plane;

void init_plane(Plane *plane, int id, int capacity, int maxBaggage);
void board_passenger(Plane *plane, int passenger_id);
void clear_plane(Plane *plane);

#endif // PLANE_H