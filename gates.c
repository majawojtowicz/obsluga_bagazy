#include <mqueue.h>
#include "gates.h"
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

extern volatile bool forceDepartureStairs[]; //uzywane do przeslania informacji o force depart do schodow, zeby je zwolnic
extern volatile bool forceDeparturePlanes[]; //uzywane przez schody do wyslania samolotow
extern volatile bool airbornePlanes[]; //uzywane do przechowania status samolotow (0 - przy gate; 1 - w powietrzu)
extern volatile int currentPlanes; //ile obecnie samolotow w powietrzu
extern volatile int planesOnboarded[];

extern volatile bool evacuate;

extern const char* s2d;

extern int total_planes;
extern int stairs_capacity;
extern int plane_capacity; 
extern int total_passengers;


void *gate_thread(void *arg)
{
    int shmid;
    Passenger *passengers;
    sem_t* semptr;

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


    //int gateNumber = (int)(uintptr_t)arg; 
    GateStairs* stairs = (GateStairs *) arg;
    char planemq[10];
    snprintf(planemq, 12, "/Stairs%d", stairs->gateNumber);

    printf("\t\t\t\tStartujemy gate'a:%d!\n",stairs->gateNumber);
    // odbieramy pasazerow z terminalu
    mqd_t mqd = mq_open ("/DeparturesMQ", O_RDONLY | O_NONBLOCK);
    assert (mqd != -1);

    // wysylamy pasazerow na schody
    // wyczysc przed utworzeniem
    if (mq_unlink(planemq)) {
        perror("\t\t\t\tCleanup of old stairs queue failed");
    }
    mqd_t mqs = mq_open(planemq, O_CREAT | O_EXCL | O_RDWR | O_NONBLOCK,  0600, NULL);
    assert (mqs != -1);

    /* Get the message queue attributes */
    struct mq_attr attr;
    assert (mq_getattr (mqd, &attr) != -1);

    char *buffer = calloc (attr.mq_msgsize, 1);
    assert (buffer != NULL);

    while(1) {
        while (!(stairs->gateNumber<currentPlanes)) {sleep(1);} //czekamy na nasza kolej
        /* Retrieve message from the queue and get its priority level */
        unsigned int priority = 0;
        mq_getattr(mqs, &attr);
        // jesli jest miejsce na schodach, i nie ma forced departure, to pobiez z hallu
        if (!evacuate && !airbornePlanes[stairs->gateNumber]&&!forceDepartureStairs[stairs->gateNumber] 
            && attr.mq_curmsgs<stairs_capacity && attr.mq_curmsgs+planesOnboarded[stairs->gateNumber]<plane_capacity) {            
            if ((mq_receive (mqd, buffer, attr.mq_msgsize, &priority)) != -1)
            {
            //printf ("\t\t\t\tGate %d received [priority %u]: '%s'\n", stairs->gateNumber, priority, buffer);
            if (!sem_wait(semptr)) {
                passengers[atoi(buffer)].status=4;
                passengers[atoi(buffer)].assignedPlaneNumber = stairs->gateNumber;
                mq_send(mqs, buffer, 3, 10);
                sem_post(semptr);
            }
            if (STAIRS_ECHO) printf ("\t\t\t\tGate %d send passenger '%s' to stairs\n", stairs->gateNumber, buffer);
            }
        }
        else {
            if (airbornePlanes[stairs->gateNumber]) {
                //printf ("\t\t\t\tStairs %d locked as plane is airborne\n", stairs->gateNumber);
            }
            if (forceDepartureStairs[stairs->gateNumber]) {
                //wyczysc schody, cofnij pasazerow do departureHall/departuresMQ
                printf ("\t\t\t\tStairs %d clear for forced plane departure\n", stairs->gateNumber);
                // wyczysc schody z pasazerow
                //mqd_t mqback = mq_open ("/DeparturesBack", O_CREAT | O_EXCL | O_WRONLY,  0600, NULL);
                //perror("Opening DeparturesBack");
                //assert (mqback != -1);
                while (mq_receive (mqs, buffer, attr.mq_msgsize, &priority) != -1) {
                    if (!sem_wait(semptr)) {
                            printf("\t\t\t\tFD: %d, moved passenger %s back to hall.\n", stairs->gateNumber, buffer);
                            //mq_send(mqback, buffer, 3, 20);
                            //perror("FD result");
                            passengers[atoi(buffer)].status=31; //31 as pushed back
                            passengers[atoi(buffer)].assignedPlaneNumber = 0;
                            sem_post(semptr);                    
                    }
                }  
                //mq_close(mqback);
                // ustaw force plane (ktory zostanie usuniety przez kapitana), ale tylko jesli nie ma go juz w powietrzu
                if (!airbornePlanes[stairs->gateNumber]) forceDeparturePlanes[stairs->gateNumber]=true;
                // poczekaj na samolot az airborne
                while (!airbornePlanes[stairs->gateNumber]) {};
                // wyczysc force stairs
                forceDepartureStairs[stairs->gateNumber]=false;
            }
            if (attr.mq_curmsgs>stairs_capacity) {
                //printf ("\t\t\t\tStairs %d reached max capacity.\n", stairs->gateNumber);
            }

        }
        usleep(DEPARTURES_GATES+rand()%DEPARTURES_GATES);
        //printf("Gate %d is going to sleep...\n", stairs->gateNumber);
    }

    pthread_exit(NULL);
}