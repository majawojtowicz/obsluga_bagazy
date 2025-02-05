#include "dispatcher.h"
#include <signal.h>

extern volatile sig_atomic_t noMoreCheckIn;
extern int maxPlanesOperating;
extern volatile int currentPlanesOperating;
extern volatile PassengerStatus* status;
extern int total_passengers;
extern volatile int passengersDelivered;
extern volatile int passengersDeclined;

void *dispatcher_thread(void *arg)
{
    DispatcherParams *dp = ( DispatcherParams*)arg;

    printf("[DISPATCHER] Thread started. planesCount=%d\n", dp->planesCount);

    
    int forced = 0;
    while (forced < 2) {
    sleep( 5 );
        printf("[DISPATCHER] Sending SIGUSR1 (force_departure)\n");
    kill( getpid(), SIGUSR1);
        forced++;
}

    sleep(5);
    printf("[DISPATCHER] Sending SIGUSR2 => no more check-in.\n");
    kill(getpid(), SIGUSR2);

    printf("[DISPATCHER] Finished.\n");
    return NULL;
}

void increasePlanesOnDeparture(void) {
    if (passengersDelivered+passengersDeclined<total_passengers && currentPlanesOperating < maxPlanesOperating-1) 
        {
            currentPlanesOperating++;
            printf("[DISPATCHER] Bringing in new plane, so now %d operating\n",currentPlanesOperating);
        }
}

void printPassengersStatusOnLand(void) {
    printf("Passengers status:\n");

    for (int i=0;i<total_passengers;i++) 
        {
        if (status[i].overweight)
            printf("o");
            else printf(" ");
        }
        printf("\n");
    
    for (int i=0;i<total_passengers;i++) 
        {
        if (status[i].assignedPlaneNumber>=0)
            printf("%d", status[i].assignedPlaneNumber);
            else printf("-");
        }
        printf("\n");

    for (int i=0;i<total_passengers;i++) 
        {
            if (status[i].onStairs) printf("#"); else printf(" ");
        }
        printf("\n");
    for (int i=0;i<total_passengers;i++) 
        {
            if (!status[i].overweight)
                {
                    if (status[i].delivered) printf("+"); else if (status[i].airborne) printf("~");
                }
                else printf(" ");
        }
        printf("\n");
        printf("Passengers declined (overweight):%d\n", passengersDeclined);
        printf("Passengers delivered:%d/%d\n", passengersDelivered, total_passengers);
        printf("\n");
}

