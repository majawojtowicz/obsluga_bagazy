#include "dispatcher.h"
#include <signal.h>

extern volatile sig_atomic_t noMoreCheckIn;

void *dispatcher_thread(void *arg)
{
    DispatcherParams *dp = (DispatcherParams*)arg;

    printf("[DISPATCHER] Thread started. planesCount=%d\n", dp->planesCount);

    
    int forced = 0;
    while (forced < 2) {
        sleep(5);
        printf("[DISPATCHER] Sending SIGUSR1 (force_departure)\n");
        kill(getpid(), SIGUSR1);
        forced++;
    }

    sleep(5);
    printf("[DISPATCHER] Sending SIGUSR2 => no more check-in.\n");
    kill(getpid(), SIGUSR2);

    printf("[DISPATCHER] Finished.\n");
    return NULL;
}