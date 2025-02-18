#include <mqueue.h>
#include "captain.h"
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

extern volatile bool forceDepartureStairs[]; //uzywane do przeslania informacji o force depart do schodow, zeby je zwolnic
extern volatile bool forceDeparturePlanes[]; //uzywane przez schody do wyslania samolotow
extern volatile bool airbornePlanes[]; //uzywane do przechowania status samolotow (0 - przy gate; 1 - w powietrzu)
extern volatile int planesOnboarded[];

extern volatile int currentPlanes;

extern int total_planes;
extern int total_passengers;
//extern int stairs_capacity;
//extern int max_baggage;
//extern int plane_capacity; 


void *captain_thread(void *arg)
{
    //int gateNumber = (int)(uintptr_t)arg; 
    Captain* captain = (Captain *) arg;

    int shmid;
    Passenger *passengers;
    sem_t* semptr;
    int passengersList[100];

    // dobierzmy sie do pamieci wspoldzielonej z danymi pasazerow...
    key_t kls = ftok(".", 'X');
    shmid = shmget(kls, total_passengers*sizeof(Passenger), IPC_CREAT|0660);
    if ((int) shmid == -1) {
       perror("\t\t\t\tError occured during shmid() call\n");
       exit(1);
    }
    passengers = (Passenger *) shmat (shmid, 0, 0);

    if ((int) passengers == -1) {
       perror("\t\t\t\tError occured during shmat() call\n");
       exit(1);
    }

    semptr = sem_open(SemaphoreName, /* name */
                            O_CREAT, /* create the semaphore */
                            AccessPerms, /* protection perms */
                            0); /* initial value */
    if (semptr == (void*) -1) printf("sem_open");

    printf("\t\t\t\t\tSamolot przy gate:%d!\n",captain->gateNumber);

    char planemq[10];
    snprintf(planemq, 12, "/Stairs%d", captain->gateNumber);

    // odbieramy pasazerow ze schodow
    mqd_t mqs = mq_open (planemq, O_RDONLY | O_NONBLOCK);
    assert (mqs != -1);

    /* Get the message queue attributes */
    struct mq_attr attr;
    assert (mq_getattr (mqs, &attr) != -1);

    char *buffer = calloc (attr.mq_msgsize, 1);
    assert (buffer != NULL);


    planesOnboarded[captain->gateNumber]=0;

    while(1) {
        while (!(captain->gateNumber<currentPlanes)) {sleep(1);} //czekamy na nasza kolej
        // odbieramy pasazerow ze schodow jedynie jesli nie jestesmy pelni lub nie ma forced departure
        if (!forceDeparturePlanes[captain->gateNumber] && captain->onboarded<captain->planeCapacity) 
        {
            unsigned int priority = 0;
            if ((mq_receive (mqs, buffer, attr.mq_msgsize, &priority)) != -1) {
                //odebralismy, to zwiekszym current o 1
                passengersList[captain->onboarded]=atoi(buffer); // dodajmy pasazera do list
                captain->onboarded++;  
                planesOnboarded[captain->gateNumber]++;              

                if (!sem_wait(semptr)) {
                    passengers[atoi(buffer)].status=5;
                    passengers[atoi(buffer)].assignedPlaneNumber = captain->gateNumber;
                    sem_post(semptr);
                }

                printf ("\t\t\t\t\tPlane %d onboarded '%s' (%d/%d).\n", captain->gateNumber, buffer, captain->onboarded, captain->planeCapacity);
            }
        }
        else {
            if (forceDeparturePlanes[captain->gateNumber]) {
                printf("\t\t\t\t\tPlane %d is forced to depart!\n", captain->gateNumber);
                forceDeparturePlanes[captain->gateNumber]=false;
            }
            if (captain->onboarded>=captain->planeCapacity) {
                printf("\t\t\t\t\tPlane %d is full. Departing!\n", captain->gateNumber);   
            }
            airbornePlanes[captain->gateNumber]=true;
            //zwieksz ilosc samolotow, jesli mozesz
            if (currentPlanes<total_planes) {
                currentPlanes++;
                printf("Added new plane:%d\n", currentPlanes-1);
            }
            sleep(FLIGHT_TIME);
            printf("\t\t\t\t\tPlane %d has landed in its destination.\n", captain->gateNumber);
            //rozladuj podroznych
            if (!sem_wait(semptr)) {
                for (int i=0;i<captain->onboarded;i++) passengers[passengersList[i]].status=6;
                sem_post(semptr);
            }
            captain->onboarded=0;
            planesOnboarded[captain->gateNumber]=0;
            
            sleep(FLIGHT_TIME);
            printf("\t\t\t\t\tPlane %d is back.\n", captain->gateNumber);            
            airbornePlanes[captain->gateNumber]=false;
        }
        usleep(CAPTAIN_WAIT);
        //printf("Gate %d is going to sleep...\n", stairs->gateNumber);
    }

    pthread_exit(NULL);
}