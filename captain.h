#ifndef CAPTAIN_H
#define CAPTAIN_H

#include "common.h"
#include "plane.h"

typedef struct {
    Plane *plane;
    int   flightTime;   
    int   T1;           
    int   totalPlanes;  
} CaptainParams;

void *captain_thread(void *arg);

#endif // CAPTAIN_H