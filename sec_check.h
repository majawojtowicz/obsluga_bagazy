#ifndef SEC_CHECK_H
#define SEC_CHECK_H

#include "common.h"

typedef struct {
	int gateNumber; // aka numer samolotu
	bool isFemale; //ile obecnie
} SecGate;


void *check_thread(void *arg);

#endif // SEC_CHECK_H