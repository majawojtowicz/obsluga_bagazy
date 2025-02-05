#include "passenger.h"
#include "operacje.h"


extern sem_t stairsSem[];
extern Plane globalPlanes[];
extern volatile sig_atomic_t noMoreCheckIn;
extern volatile int currentPlanesOperating;
extern volatile PassengerStatus* status;
extern volatile int passengersDeclined;
extern int stairs_capacity;

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

    if (p->weight > globalPlanes[0].maxBaggage) {
        printf("[PASSENGER %d] Overweight: %d > %d. Denied.\n",
               p-> id, p->weight, globalPlane[0].maxBaggage);
            status[p->id].overweight=true;
        passengersDeclined++; 
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

    
    //przydzielmy losowo samolot, w zależności od tego ile obecnie samolotow w powietrzu/operacyjnie
    me.assignedPlaneNumber = rand() % (currentPlanesOperating-1 + 1 -0); //za rand() % (max_number + 1 - minimum_number) + minimum_number - rand od min do max, wlacznie
    status[me.id].assignedPlaneNumber=me.assignedPlaneNumber;
    printf("[PASSENGER %d] assigned plane [%d]\n", me.id, me.assignedPlaneNumber);
    
    printf("[PASSENGER %d] Waiting for stairs [%d]\n", me.id, me.assignedPlaneNumber);
    sem_wait(&stairsSem[me.assignedPlaneNumber]);

    me.onStairs=true;
    status[me.id].onStairs=true;

    printf("[PASSENGER %d] Entering stairs [%d]\n", me.id, me.assignedPlaneNumber);
    sleep(1);

    printf("[PASSENGER %d] Leavinng stairs [%d]\n", me.id, me.assignedPlaneNumber);
    sem_post(&stairsSem[me.assignedPlaneNumber]);

    //printf("[PASSENGER %d] attempting to board plane [%d] at address[%p]\n", me.id, me.assignedPlaneNumber, &globalPlanes[me.assignedPlaneNumber]);
    printf("[PASSENGER %d] attempting to board plane [%d]\n", me.id, me.assignedPlaneNumber);
    board_passenger(&globalPlanes[me.assignedPlaneNumber], me.id);
    status[me.id].onStairs=false;
    status[me.id].airborne=true;

    printf("[PASSENGER %d] Finished.\n", me.id);
    status[me.id].airborne=false;
    status[me.id].delivered=true;

    pthread_exit(NULL);
}
