#include "security.h"
#include <sys/prctl.h>
#include "sec_check.h"
#include <mqueue.h>
#include <assert.h>

extern const char* t2s;
extern const char* s2d;

extern int total_passengers;


const char * securityName = "SecurityChecks";
const char * securityQueue = "/SecurityMQ";

volatile int mfd; // to be used across the check gates
volatile bool secevacuate;

mqd_t mqsec;

void evacuationReceivedSec()
{
	//signal(SIGUSR1, evacuationReceived);
	printf("\t\tSecurity - received evacuation request! Proceed to the exits.\n");
	secevacuate = true;

	mqd_t mqd_evac = mq_open (securityQueue, O_RDONLY | O_NONBLOCK);
    assert (mqd_evac != -1);
    struct mq_attr attr;
    assert (mq_getattr (mqd_evac, &attr) != -1);

    char *buffer = calloc (attr.mq_msgsize, 1);
    assert (buffer != NULL);

	int priority;

	int shmid;
	Passenger *passengers;
	//int count = 100;
	sem_t* semptr;

	key_t kls = ftok(".", 'X');
    shmid = shmget(kls, total_passengers*sizeof(Passenger), IPC_CREAT|0660);
	if ((int) shmid == -1) {
       perror("\t\tError occured during shmid() call\n");
       exit(1);
  	}
	passengers = (Passenger *) shmat (shmid, 0, 0);

	if ((int) passengers == -1) {
       perror("\t\tError occured during shmat() call\n");
       exit(1);
  	}

	//clean departure hall queue
	while ((mq_receive (mqd_evac, buffer, attr.mq_msgsize, &priority))!= -1) {
		//perror("Evac mq_receive");
		if (SECURITY_ECHO) printf("\t\t Evacuated from security hall, p:%s\n", buffer);
		passengers[atoi(buffer)].status=0;
	}

	//clean FIFO pipe, brutal way
	//unlink(s2d);
   	//mkfifo(s2d, 0666); /* read/write for user/group/others */
	for (int i=0;i<total_passengers;i++) {
		if (passengers[i].status==2) {
			passengers[i].status=0;
			if (SECURITY_ECHO) printf("\t\t Evacuated from past-security corridors, p:%d\n", i);
		}
	}

	printf("\t\tSecurity hall evacuation completed!\n");
	secevacuate = false;	
}

void set_evac_handler() {
	struct sigaction current; /* current setup */
	sigemptyset(&current.sa_mask); /* clear the signal set */
	current.sa_flags = 0; /* for setting sa_handler, not sa_action */
	current.sa_handler = evacuationReceivedSec; /* specify a handler */
	sigaction(SIGUSR1, &current, NULL); /* register the handler */
}

void create_sec_queue() {
	printf("\t\tOpening security checks queue...\n");
	// usun ja najpierw, bo mogla zostac z poprzedniego uruchomienia
	if (!mq_unlink(securityQueue)) {
		perror("\t\t\tCleanup of security queue failed or not exist");
	}
	// otworz kolejke z poczekalnia pasazerow z ktorej beda brani na schody
	mqsec = mq_open (securityQueue, O_CREAT | O_EXCL | O_WRONLY,  0600, NULL);
	/* Ensure the creation was successful */
	if (mqsec == -1)
  	{
    	perror ("\t\tProblems with security queue");
    	exit (1);
  	}

}

void securityRun(void) {

	int shmid;
	Passenger *passengers;
	//int count = 100;
	sem_t* semptr;

	key_t kls = ftok(".", 'X');
    shmid = shmget(kls, total_passengers*sizeof(Passenger), IPC_CREAT|0660);
	if ((int) shmid == -1) {
       perror("\t\tError occured during shmid() call\n");
       exit(1);
  	}
	passengers = (Passenger *) shmat (shmid, 0, 0);

	if ((int) passengers == -1) {
       perror("\t\tError occured during shmat() call\n");
       exit(1);
  	}


	// ustaw nazwe procesu, zeby bylo widoczne na ptree

    if (prctl(PR_SET_NAME, (unsigned long) securityName) < 0)
    {
        perror("\t\tprctl()");
    }

    set_evac_handler();

  	semptr = sem_open(SemaphoreName, /* name */
							O_CREAT, /* create the semaphore */
							AccessPerms, /* protection perms */
							0); /* initial value */
	if (semptr == (void*) -1) printf("sem_open");

	//zacznij nasluchiwac od terminala
	int fd = open(t2s, O_RDONLY);
	if (fd < 0) {
		printf("\t\tSecurity entry is broken, cannot handle passengers. Exiting\n");
		exit(1); /* no point in continuing */
	}

	create_sec_queue();

	//stworz watki z security gates
	pthread_t *gateThreads = malloc(sizeof(pthread_t)* MAX_SEC_GATES);
	SecGate *gates  = malloc(sizeof(SecGate)* MAX_SEC_GATES);

  	for (int g=0;g<MAX_SEC_GATES;g++)
  		{
  			gates[g].gateNumber=g;
  			if (pthread_create(&gateThreads[g], NULL, check_thread, &gates[g])!=0) {
  				perror("\t\t\tproblem z utworzeniem watku security gate'a!");
  		}
  	}


	//i puszczaj ludzi dalej
	//otworz korytarz z terminala do security
	mfd = open(s2d, O_CREAT | O_WRONLY); /* open as write-only */
	if (mfd < 0) /* can't go on */
	{
		printf("\t\tSecurity passage to departure hall is non-functioning, cannot handle passengers. Exiting\n");
		exit(1); /* no point in continuing */		
	}

  	char buf[3];

	while (1)
	{
		int next;
		int i;
		int priority;
		if (!secevacuate) {
		ssize_t count = read(fd, &next, sizeof(int));	
		if (count>0) {
			if (passengers[next].status==1) { // tylko kiedy idzie z terminala'a, co moze nie byc spelnione przy ewakuacji
				if (SECURITY_ECHO) printf("\t\tSecurity received p:%d ", next);
				if (!sem_wait(semptr)) {
					passengers[next].status=2;
					if (passengers[next].isVIP) {
						if (SECURITY_ECHO) printf("(VIP)\n", next);
						priority=32; 
					} 
					else 
						{
							priority=0;
							if (SECURITY_ECHO) printf("\n");
						}
					sem_post(semptr);
				}
				snprintf(buf, 3, "%d", next);
				mq_send(mqsec, buf, 3, priority); // przesylamy do stanowisk...

				//write(mfd,&next, sizeof(int));
				}
			}
		}
		usleep(SECURITY_HALL_WAIT+rand()%SECURITY_HALL_WAIT);
	}
}