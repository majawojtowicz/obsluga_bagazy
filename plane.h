#ifndef plane_h
#define plane_h

typedef struct Plane {
    char *name;
    int id;
    int capacity;
    int baggage_capacity;
    char *capitan
} Plane;

int create_plane(Plane *plane, char *name, int id, int capacity, int baggage_capacity, char *capitan);

void *plane_start(void *arg);


#endif