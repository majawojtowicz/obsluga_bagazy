#include "passenger.h"
#include "stdlib.h"
#include <mqueue.h>

void *passenger_thread(void *arg)
{
    int departureWaitTime=0;
    int securityWaitTime=0;
    bool notSatisfied=false;
    Passenger* passenger = (Passenger *) arg;


    //zeby nie bylo konfliktow na wspoldzielonej pamieci
    sem_t* semptr = sem_open(SemaphoreName, /* name */
               O_CREAT,       /* create the semaphore */
               AccessPerms,   /* protection perms */
               0);            /* initial value */
    //if (semptr == (void*) -1) printf("sem_open\n");
    sem_post(semptr);


    //printf("Hello! I'm a passenger %d.\n", passenger->id);

    char buf[3];

    //while(1){
    //    sleep(1);
    //}; //test empty loop

    while (passenger->status!=6) {
        if (!sem_wait(semptr)) {
            if (passenger->status==3 || passenger->status==31) departureWaitTime++;
            if (passenger->status==2) {
                securityWaitTime++;
                //printf("P %d: waiting for security %d\n", passenger->id, securityWaitTime);
            }
            if (departureWaitTime>5) {
                printf("I'm still waiting for departure :( %d Escalating to service!\n",passenger->id);
                //try to find the gate!
                mqd_t mqd = mq_open ("/DeparturesMQ", O_CREAT | O_WRONLY | O_NONBLOCK,  0600, NULL);
                //perror("Trying to send myself");
                snprintf(buf, 3, "%d", passenger->id);
                mq_send(mqd, buf, 3, 10);
                //perror("Tried to send myself");
                departureWaitTime=0;
            }
        
            if (securityWaitTime>PASSENGER_ANGRY && !notSatisfied) {
                printf("I'm extremely dissatisfied by the security check lines! %d I quit!\n", passenger->id);
                passenger->status=-3;
                notSatisfied=true;
            }
            if (notSatisfied) passenger->status=-3; //trzymamy ten status do konca
            sem_post(semptr);
        }

        sleep(PASSENGER_SLEEP);
    }
    printf("Passenger %d happily landed!\n", passenger->id);
    pthread_exit(NULL);
}