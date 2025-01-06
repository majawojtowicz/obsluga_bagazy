#ifndef dyspozytor_h
#define dyspozytor_h


typedef struct Dispatcher {
    char *name;
    int id;
    int capacity;
    int baggage_capacity;
} Dispatcher;

int create_dispatcher(Dispatcher *dispatcher, char *name, int id, int capacity, int baggage_capacity);

void *dispatcher_start(void *arg);

#endif