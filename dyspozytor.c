
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dyspozytor.h"

int create_dispatcher(Dispatcher *dispatcher, char *name, int id, int capacity, int baggage_capacity) {
    dispatcher->name = name;
    dispatcher->id = id;
    dispatcher->capacity = capacity;
    dispatcher->baggage_capacity = baggage_capacity;
    if (pthread_create(&dispatcher->thread, NULL, dispatcher_start, (void *)dispatcher) != 0) {
        perror("pthread_create");
        return -1;
    }
    return 0;
}
