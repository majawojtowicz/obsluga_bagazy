#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "common.h"

typedef struct {
    int planesCount; 
    int T1;          
} DispatcherParams;

void *dispatcher_thread(void *arg);

#endif // DISPATCHER_H