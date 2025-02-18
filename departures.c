#include "departures.h"
#include "gates.h"
#include "captain.h"
#include <sys/prctl.h>
#include <mqueue.h>
#include <stdint.h>
#include <assert.h>

const char * departuresName = "DeparturesHall";
extern const char* s2d;

extern int total_planes;
extern int total_passengers;
extern int stairs_capacity;
extern int max_baggage;
extern int plane_capacity; 


//int totalStairs=10; // tyle maksymalnie schodow ile maksymalnie samolotow

mqd_t mqd;

volatile bool forceDepartureStairs[MAX_PLANES]; //uzywane do przeslania informacji o force depart do schodow, zeby je zwolnic
volatile bool forceDeparturePlanes[MAX_PLANES]; //uzywane przez schody do wyslania samolotow
volatile bool airbornePlanes[MAX_PLANES]; // uzywane do przechowania status samolotow (0 - przy gate; 1 - w powietrzu)
volatile int planesOnboarded[MAX_PLANES]; // do sprawdzania czy mozemy jeszcze dac pasazerow na schody vs. samolot przy gate

volatile bool forceDeparture;
volatile bool evacuate;
volatile int currentPlanes;


void evacuationReceivedDep()
{
	mqd_t mqd_evac = mq_open ("/DeparturesMQ", O_RDONLY | O_NONBLOCK);
    assert (mqd_evac != -1);
    struct mq_attr attr;
    assert (mq_getattr (mqd_evac, &attr) != -1);

    char *buffer = calloc (attr.mq_msgsize, 1);
    assert (buffer != NULL);

	int priority;

	printf("\t\t\tDeparture hall - received evacuation request! Proceed to the exits.\n");
	evacuate = true;

	//printf("\t\t\tDeparture MQ size:%d", attr.mq_curmsgs);

	int shmid;
	Passenger *passengers;
	//int count = 100;
	sem_t* semptr;

	key_t kls = ftok(".", 'X');
    shmid = shmget(kls, total_passengers*sizeof(Passenger), IPC_CREAT|0660);
	if ((int) shmid == -1) {
       perror("\t\t\tError occured during shmid() call\n");
       exit(1);
  	}
	passengers = (Passenger *) shmat (shmid, 0, 0);

	if ((int) passengers == -1) {
       perror("\t\t\tError occured during shmat() call\n");
       exit(1);
  	}

	//clean departure hall queue
	while ((mq_receive (mqd_evac, buffer, attr.mq_msgsize, &priority))!= -1) {
		//perror("Evac mq_receive");
		if (DEPARTURES_ECHO) printf("\t\t\t Evacuated from departures hall, p:%s\n", buffer);
		passengers[atoi(buffer)].status=0;
	}

	printf("\t\t\tDepartures hall evacuation completed!\n");
	evacuate = false;
}

void forceReceivedDep()
{
	forceDeparture=true;
	//signal(SIGUSR1, evacuationReceived);
	printf("\t\t\tDeparture hall - forced departure! All planes preparing to takeoff.\n");
	for (int i=0;i<currentPlanes;i++) {
		if (!airbornePlanes[i]) {
			//printf("\t\t\tFD: %d start.\n",i);
			forceDepartureStairs[i]=true; // ale tylko te ktore juz nie leca
			//while(forceDepartureStairs[i]) {} //dajmy im sie zrzucic
			//printf("\t\t\tFD: %d done.\n",i);
		}
	}
	sleep(3);
	// parse through pushed back (from stairs)

	int shmid;
	Passenger *passengers;
	//int count = 100;
	sem_t* semptr;

	key_t kls = ftok(".", 'X');
    shmid = shmget(kls, total_passengers*sizeof(Passenger), IPC_CREAT|0660);
	if ((int) shmid == -1) {
       perror("\t\t\tError occured during shmid() call\n");
       exit(1);
  	}
	passengers = (Passenger *) shmat (shmid, 0, 0);

	if ((int) passengers == -1) {
       perror("\t\t\tError occured during shmat() call\n");
       exit(1);
  	}

	char buf[3];
	for (int i = 0; i < total_passengers; i++)
		{
			//if (!sem_wait(semptr)) {
				if (passengers[i].status==31) {	
					passengers[i].status==30;
					snprintf(buf, 3, "%d", i);
					mq_send(mqd, buf, 3, 10);
					printf("\t\t\tPushed back %i comes to departure hall\n",i);
				}
			//sem_post(semptr)
		}			

	forceDeparture=false;
}

void set_evachandler() {
	struct sigaction current; /* current setup */
	sigemptyset(&current.sa_mask); /* clear the signal set */
	current.sa_flags = 0; /* for setting sa_handler, not sa_action */
	current.sa_handler = evacuationReceivedDep; /* specify a handler */
	sigaction(SIGUSR1, &current, NULL); /* register the handler */
	current.sa_handler = forceReceivedDep;
	sigaction(SIGUSR2, &current, NULL);
}

void departuresRun(void) {
	// ustaw nazwe procesu, zeby bylo widoczne na ptree
    if (prctl(PR_SET_NAME, (unsigned long) departuresName) < 0)
    {
        perror("\t\t\tprctl()");
    }

    set_evachandler();

    forceDeparture=false; 
    evacuate = false;
    currentPlanes=1;

	int shmid;
	Passenger *passengers;
	//int count = 100;
	sem_t* semptr;

	key_t kls = ftok(".", 'X');
    shmid = shmget(kls, total_passengers*sizeof(Passenger), IPC_CREAT|0660);
	if ((int) shmid == -1) {
       perror("\t\t\tError occured during shmid() call\n");
       exit(1);
  	}
	passengers = (Passenger *) shmat (shmid, 0, 0);

	if ((int) passengers == -1) {
       perror("\t\t\tError occured during shmat() call\n");
       exit(1);
  	}

  	semptr = sem_open(SemaphoreName, /* name */
							O_CREAT, /* create the semaphore */
							AccessPerms, /* protection perms */
							0); /* initial value */
	if (semptr == (void*) -1) printf("sem_open");

    //zacznij nasluchiwac od terminala
	int fd = open(s2d, O_RDONLY);
	if (fd < 0) {
		printf("\t\t\tDepartures hall entry is broken, cannot handle passengers. Exiting\n");
		exit(1); /* no point in continuing */
	}

	printf("\t\t\tOpening departures terminal...\n");
	// usun ja najpierw, bo mogla zostac z poprzedniego uruchomienia
	if (!mq_unlink("/DeparturesMQ")) {
		perror("\t\t\tCleanup of departures queue failed");
	}


	struct mq_attr attr;
	attr.mq_maxmsg=128;
	attr.mq_msgsize=4096;
	attr.mq_flags = 0;

	// otworz kolejke z poczekalnia pasazerow z ktorej beda brani na schody
	mqd = mq_open ("/DeparturesMQ", O_CREAT | O_EXCL | O_WRONLY,  0600, NULL);
	/* Ensure the creation was successful */
	if (mqd == -1)
  	{
    	perror ("\t\t\tdepartures mq_open");
    	exit (1);
  	}
/*
  	assert (mq_getattr (mqd, &attr) != -1);
    printf("MQ curmsgs:%d\n", attr.mq_curmsgs);
    printf("MQ max_msg:%d\n", attr.mq_maxmsg);
    printf("MQ flags:%d\n", attr.mq_flags);
    printf("MQ msgsize:%d\n", attr.mq_msgsize);
*/




  	//stworz watki ze schodami/gatem
	pthread_t *gateThreads = malloc(sizeof(pthread_t)* total_planes);
	GateStairs *gateStairs  = malloc(sizeof(GateStairs)* total_planes);

  	for (int g=0;g<total_planes;g++)
  		{
  			gateStairs[g].gateNumber=g;
  			//void *ptr = (void *)(uintptr_t)g;
  			if (pthread_create(&gateThreads[g], NULL, gate_thread, &gateStairs[g])!=0) {
  				perror("\t\t\tproblem z utworzeniem watku gate'a!");
  		}
  	}

  	sleep(5); 
  	//stworz watki ze schodami/gatem
	pthread_t *captainThreads = malloc(sizeof(pthread_t)* total_planes);
	Captain *captains  = malloc(sizeof(Captain)* total_planes);

  	for (int g=0;g<total_planes;g++)
  		{
  			captains[g].gateNumber=g;
  			captains[g].planeCapacity=plane_capacity;
  			//void *ptr = (void *)(uintptr_t)g;
  			if (pthread_create(&captainThreads[g], NULL, captain_thread, &captains[g])!=0) {
  				perror("\t\t\tproblem z utworzeniem watku kapitana'a!");
  		}
  	}


  	printf("\t\t\tDepartures open. Handling passengers\n");

  	char buf[3];

	while (1)
	{
		int next;
		if (!forceDeparture && !evacuate) {
			ssize_t count = read(fd, &next, sizeof(int));	
			if (count>0) {
				if (!sem_wait(semptr)) {
					//odpusc zirytowanych pasazerow
					if (passengers[next].status!=-3) {
						if (DEPARTURES_ECHO) printf("\t\t\tDepartures received p:%d\n", next);
						snprintf(buf, 3, "%d", next);
						mq_send(mqd, buf, 3, 10);
						passengers[next].status=3;
						if (DEPARTURES_ECHO) printf("\t\t\t Departures added p:%d\n", next);
						//struct mq_attr attr;
						//mq_getattr (mqd, &attr);
						//if (DEPARTURES_ECHO) printf("\t\t\t Departures MQ size:%d\n", attr.mq_curmsgs);
						sem_post(semptr);
					}
				}
			}
		}
		usleep(DEPARTURES_HALL);
	}
}