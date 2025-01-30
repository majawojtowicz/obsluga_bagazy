#ifndef PLANE_H
#define PLANE_H

#include "common.h"


typedef struct Plane {
    int planeId;
    int capacity;       
    int maxBaggage;     
    int currentOnBoard; 
    
}  Plane;


void init_plane( Plane *plane, int id, int capacity , int maxBaggage);
void plane_board_passenger(Plane *plane);  
void plane_clear( Plane *plane);            
void *plane_thread(void *arg);
#endif