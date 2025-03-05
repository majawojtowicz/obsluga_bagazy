#include "terminal.h"
#include <sys/prctl.h>
#include <signal.h>

extern const char* m2t;
extern const char* t2s;

extern int total_planes;
extern int total_passengers;
extern int stairs_capacity;
extern int max_baggage;
extern int plane_capacity; 


const char * terminalName = "Terminal";

volatile bool noCheckins;
volatile bool terevacuate;

int mtfd; //przejscie: main -> terminal
int tfd; // przejscie: terminal -> security

void evacuationReceived()
{
	//signal(SIGUSR1, evacuationReceived);
	printf("\tTerminal - received evacuation request! Proceed to the exits.\n");
	terevacuate = true;

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


	for (int i=0;i<total_passengers;i++) {
		if (passengers[i].status==1) {
			passengers[i].status=0;
			if (TERMINAL_ECHO) printf("\t Evacuated from terminal, p:%d\n", i);
		}
	}
	printf("\tTerminal evacuation completed.\n");
	terevacuate=false;

}

void noCheckinsReceived() 
{
	printf("\tTerminal - no more checkins allowed!\n");
	noCheckins=true;
}

void endReceivedTerminal()
{
    printf("\tCleaning the terminal. See you again!\n");
    close(mtfd);
    unlink(m2t); // zamykamy przejscia miedzy salami

    close(tfd);
    unlink(t2s);

    exit(0);    
}

void set_handler() {
	struct sigaction current; /* current setup */
	sigemptyset(&current.sa_mask); /* clear the signal set */
	current.sa_flags = 0; /* for setting sa_handler, not sa_action */
	current.sa_handler = evacuationReceived; /* specify a handler */
	sigaction(SIGUSR1, &current, NULL); /* register the handler */
	current.sa_handler = noCheckinsReceived; /* specify a handler */
	sigaction(SIGRTMIN+1, &current, NULL); /* register the handler */
	current.sa_handler = endReceivedTerminal; /* specify a handler */
	sigaction(SIGTERM, &current, NULL); /* register the handler */
}


void terminalRun(void) {

	int shmid;
	Passenger *passengers;
	int count = 100;
	sem_t* semptr;

	noCheckins = false;
	terevacuate = false;

	key_t kls = ftok(".", 'X');
    shmid = shmget(kls, total_passengers*sizeof(Passenger), IPC_CREAT|0660);
	if ((int) shmid == -1) {
       perror("\tError occured during shmid() call\n");
       exit(1);
  	}
	passengers = (Passenger *) shmat (shmid, 0, 0);

	if ((int) passengers == -1) {
       perror("\tError occured during shmat() call\n");
       exit(1);
  	}

  	semptr = sem_open(SemaphoreName, /* name */
							O_CREAT, /* create the semaphore */
							AccessPerms, /* protection perms */
							0); /* initial value */
	if (semptr == (void*) -1) printf("sem_open");

	// ustaw nazwe procesu, zeby bylo widoczne na ptree
    if (prctl(PR_SET_NAME, (unsigned long) terminalName) < 0)
    {
        perror("\tterminal prctl()");
    }

    //zarejestruj sie na sygnaly
	//signal(SIGUSR1, evacuationReceived);
	set_handler();

	//zacznij nasluchiwac od wejscia glownego
	mtfd = open(m2t, O_RDONLY);
	if (mtfd < 0) {
		printf("\tTerminal entry is broken, cannot handle passengers. Exiting\n");
		exit(1); /* no point in continuing */
	}

	//otworz korytarz z terminala do security
	//mkfifo(t2s, 0666); /* read/write for user/group/others */
	tfd = open(t2s, O_CREAT | O_WRONLY); /* open as write-only */
	if (tfd < 0) /* can't go on */
	{
		printf("\tTerminal exit to security is broken, cannot handle passengers. Exiting\n");
		exit(1); /* no point in continuing */		
	}
	
	while (1)
	{
		int next;
		int i;
		if (!noCheckins && !terevacuate) {
			ssize_t count = read(mtfd, &next, sizeof(int));	
			if (count>0) {
				bool passedCheckin=false;
				if (TERMINAL_ECHO) printf("\tTerminal received p:%d\n", next);
				if (!sem_wait(semptr)) {
					if (passengers[next].weight<max_baggage) {
						passengers[next].status=1;
						passedCheckin=true;
					}
					else {
						passengers[next].status=-2;
						if (TERMINAL_ECHO) printf("\t-Overweight p:%d\n",next);
					}
				sem_post(semptr);
				}
				//jesli przeszedl kontrole wagi - wyslij dalej
				if (passedCheckin) write(tfd,&next, sizeof(int));
			}
		}
		usleep(200000);
	}
}