#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "plane.h"


//obsluga samolotu

int create_plane(Plane *plane, char *name, int id, int capacity, int baggage_capacity, char *capitan) {
    plane->name = name;
    plane->id = id;
    plane->capacity = capacity;
    plane->baggage_capacity = baggage_capacity;
    plane->capitan = capitan;
    if (pthread_create(&plane->thread, NULL, plane_start, (void *)plane) != 0) {
        perror("pthread_create");
        return -1;
    }
    return 0;
}

void *plane_start(void *arg) {
    Plane *plane = (Plane *)arg;
    // symulacje odlotow i przylotow i schody
}