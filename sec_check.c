#include <mqueue.h>
#include "sec_check.h"
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>


extern int total_passengers;
extern char* securityQueue;
extern const char* s2d;
extern volatile int mfd;
extern volatile bool secevacuate;



void *check_thread(void *arg)
{
    int shmid;
    Passenger *passengers;
    sem_t* semptr;

    // dobierzmy sie do pamieci wspoldzielonej z danymi pasazerow...
    key_t kls = ftok(".", 'X');
    shmid = shmget(kls, total_passengers*sizeof(Passenger), IPC_CREAT|0660);
    if ((int) shmid == -1) {
       perror("\t\tSG: Error occured during shmid() call\n");
       exit(1);
    }
    passengers = (Passenger *) shmat (shmid, 0, 0);

    if ((int) passengers == -1) {
       perror("\t\tSG: tError occured during shmat() call\n");
       exit(1);
    }

    semptr = sem_open(SemaphoreName, /* name */
                            O_CREAT, /* create the semaphore */
                            AccessPerms, /* protection perms */
                            0); /* initial value */
    if (semptr == (void*) -1) printf("sem_open");


    SecGate* myGate = (SecGate *) arg;

    //polacz z kolejka z priorytetami
    // otworz kolejke z poczekalnia pasazerow z ktorej beda brani na schody
    mqd_t mqsec = mq_open (securityQueue, O_RDWR | O_NONBLOCK,  0600, NULL);
    /* Ensure the creation was successful */
    if (mqsec == -1)
    {
        perror ("\t\tProblems with security queue");
        exit (1);
    }

    /* Get the message queue attributes */
    struct mq_attr attr;
    assert (mq_getattr (mqsec, &attr) != -1);

    char *buffer = calloc (attr.mq_msgsize, 1);
    assert (buffer != NULL);

    unsigned int priority = 0; // dla obslugi VIP

    //otworz korytarz z security gate do departures
    //int mfd = open(s2d, O_CREAT | O_WRONLY); /* open as write-only */
    
    //int mfd = open(s2d, O_WRONLY); /* open as write-only */
    //perror("Security FIFO pipe open issue");
    if (mfd < 0) /* can't go on */
    {
        printf("\t\tSecurity passage from gate: %d to departures not functioning. Exiting\n", myGate->gateNumber);
        exit(1); /* no point in continuing */       
    }


    printf ("\t\tStarting sec gate:%d\n", myGate->gateNumber);

    int passId1;
    bool pass1Gender;
    bool pass2Gender;
    int passId2;
    char buf[3];

    while (1)
    {
        if ((mq_receive (mqsec, buffer, attr.mq_msgsize, &priority)) != -1)
        {
            if (!secevacuate) {
                if (SEC_GATES_ECHO) printf ("\t\t SecGate %d received p:%s [pri %u]\n", myGate->gateNumber, buffer, priority);
                //doing sec check...
                passId1 = atoi(buffer);
                pass1Gender = passengers[passId1].isFemale;
                // probujemy odprawic dwoje pasazerow jesli tej samej plci
                if ((mq_receive (mqsec, buffer, attr.mq_msgsize, &priority)) != -1) {
                    passId2 = atoi(buffer); 
                    if (SEC_GATES_ECHO) printf ("\t\t SecGate %d checking genders of p's:%d, %d\n", myGate->gateNumber, passId1, passId2); 
                    pass2Gender = passengers[passId2].isFemale;
                    if (pass1Gender==pass2Gender) {
                        //pchnijmy pasazera 2
                        write(mfd, &passId2, sizeof(int));
                        if (SEC_GATES_ECHO) printf ("\t\t SecGate %d served two same-sex p's: %d, %d\n", myGate->gateNumber, passId1, passId2);
                }
                else {
                    //wrzucmy z powrotem do kolejki ale na wysokim priority
                    snprintf(buf, 3, "%d", passId2);
                    mq_send(mqsec, buf, 3, 64); // przesylamy do stanowisk...
                    if (SEC_GATES_ECHO) printf("\t\t SecGate %d send back to queue (top) diff-sex p: %d\n", myGate->gateNumber, passId2);
                    }
                }
                write(mfd,&passId1, sizeof(int));
                //if (SEC_GATES_ECHO) perror("\t\t Issues in sending?");
            }
            else {
                //ewakuacja przy stanowiskach security
                passengers[passId1].status=0; 
                if (SEC_GATES_ECHO) printf ("\t\t SecGate %d evacuated p:%s\n", myGate->gateNumber, passId1);
            }
        }
        usleep(SEC_CHECK_WAIT);
    }

    pthread_exit(NULL);
}