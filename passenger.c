#include "passenger.h"
#include "operacje.h"


extern sem_t stairsSem;
extern Plane globalPlane;
extern volatile sig_atomic_t noMoreCheckIn;


static PassengerQueue controlQueue;
static pthread_once_t queueOnce = PTHREAD_ONCE_INIT;

static void init_local_queue(void)
{
    init_passenger_queue(&controlQueue);
}

void *passenger_thread(void *arg)
{

    pthread_once(&queueOnce, init_local_queue);

    Passenger *p = (Passenger *)arg;

    if (noMoreCheckIn) {
        printf("[PASSENGER %d] No more check-in allowed. Exiting.\n", p->id);
        pthread_exit(NULL);
    }

    if (p->weight > globalPlane.maxBaggage) {
        printf("[PASSENGER %d] Overweight: %d > %d. Denied.\n",
               p->id, p->weight, globalPlane.maxBaggage);
        pthread_exit(NULL);
    }

    
    bool enqueued = enqueue_passenger(&controlQueue, *p);
    if (!enqueued) {
        
        pthread_exit(NULL);
    }

   
    Passenger me = dequeue_passenger(&controlQueue);
    
    int station = security_check(me.id, me.gender);
    sleep(1 + rand()%2); 
    leave_station(station, me.id);

    
    printf("[PASSENGER %d] Waiting for stairs...\n", me.id);
    sem_wait(&stairsSem);

    printf("[PASSENGER %d] Entering stairs...\n", me.id);
    sleep(1);

    printf("[PASSENGER %d] Leaving stairs.\n", me.id);
    sem_post(&stairsSem);

    
    board_passenger(&globalPlane, me.id);

    printf("[PASSENGER %d] Finished.\n", me.id);
    pthread_exit(NULL);
}