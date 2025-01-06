#include "pasazer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "plane.h"

int create_passenger(Passenger *passenger, char *name, int id, int baggage_amount, bool is_VIP) {
    passenger->name = name;
    passenger->id = id;
    passenger->baggage_amount = baggage_amount;
    passenger->is_VIP = is_VIP;
    if (pthread_create(&passenger->thread, NULL, passenger_start, (void *)passenger) != 0) {
        perror("pthread_create");
        return -1;
    }
    return 0;
}

void *passenger_start(void *arg) {
    Passenger *passenger = (Passenger *)arg;
    // podroz po lotnisku-etapy
    return NULL;
}

