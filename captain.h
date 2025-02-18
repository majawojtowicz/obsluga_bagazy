#ifndef CAPTAIN_H
#define CAPTAIN_H

#include "common.h"

typedef struct {
	int gateNumber; // aka numer samolotu
	int planeCapacity; // calkowita pojemnosc
	int onboarded; //ile obecnie
} Captain;


void *captain_thread(void *arg);

#endif // CAPTAIN_H