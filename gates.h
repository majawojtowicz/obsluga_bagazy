#ifndef GATES_H
#define GATES_H

#include "common.h"

typedef struct {
	int gateNumber;
	int stairsCapacity;
} GateStairs;


void *gate_thread(void *arg);

#endif // GATES_H