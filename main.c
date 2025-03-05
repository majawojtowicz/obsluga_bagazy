#include "common.h"
#include <stdint.h>
#include "passenger.h"

extern void terminalRun(void);
extern void securityRun(void);
extern void departuresRun(void);
extern void dispatcherRun(pid_t p1, pid_t p2, pid_t p3);

//korytarze (nazwane kolejki fifo) miedzy poszczegolnymi czesciami lotniska
const char* m2t = "./main2terminal";
const char* t2s = "./terminal2security";
const char* s2d = "./security2departures";

//to bedzie wspoldzielone miedzy procesami?
int shmid;
Passenger *passengers;
int count = 100;
sem_t* semptr;

// z pewnymi domyslnymi
int total_planes=5;
int total_passengers= 90;
int stairs_capacity = 5;
int max_baggage = 90;
int plane_capacity = 20; 

//kolejki fifo, deskryptory
int fd; 

pthread_t *passengerThreads;

void setup_passengers_shm() {
	printf("allocating shared memory...\n");
	key_t kls = ftok(".", 'X');
	shmid = shmget(kls, total_passengers*sizeof(Passenger), IPC_CREAT|0660);
	if ((int) shmid == -1) {
       perror("Error occured during shmid() call\n");
       exit(1);
  	}
	passengers = (Passenger *) shmat (shmid, 0, 0);

	passengerThreads = malloc(sizeof(pthread_t)* total_passengers);

	if ((int) passengers == -1) {
       perror("Error occured during shmat() call\n");
       exit(1);
  	}
  	/* semahore code to lock the shared mem */
  	semptr = sem_open(SemaphoreName, /* name */
			   O_CREAT,       /* create the semaphore */
			   AccessPerms,   /* protection perms */
			   0);            /* initial value */
  	if (semptr == (void*) -1) printf("sem_open\n");
  	int i;
  	for (i=0;i<total_passengers;i++) {
  		passengers[i].status =0 ; //all passengers at the airport entry
  		passengers[i].id=i;
  		passengers[i].weight = 50+rand()%50;
  		if (rand()%10>5) passengers[i].isFemale=true; else passengers[i].isFemale=false;
  		if (i%5==0) passengers[i].isVIP=true; else passengers[i].isVIP=false;
  		if (pthread_create(&passengerThreads[i], NULL, passenger_thread, &passengers[i])!=0) {
  				perror("\t\t\tproblem z utworzeniem watku pasazera!");
  		}
	}

  /* increment the semaphore so that memreader can read */
  if (sem_post(semptr) < 0) printf("sem_post\n");
}


void simulationConfig() {
    while (1) {
        printf("Podaj liczb pasazerow (%d..%d): ", MIN_PASSENGERS, MAX_PASSENGERS);
        fflush(stdout);
        if (scanf("%d", &total_passengers) != 1) {
            fprintf(stderr, "Blad wczytywania.\n");
            fseek(stdin,0,SEEK_END);
            continue;
        }
        if ( total_passengers < MIN_PASSENGERS || total_passengers > MAX_PASSENGERS) {
            fprintf(stderr, "Niewlasciwa liczba pasazerow.\n");
            continue;
    }
        break;
    }

    while (1) {
        printf("Podaj liczbe samolotow (%d..%d): ", MIN_PLANES, MAX_PLANES_ALLOWED);
        fflush(stdout);
        if (scanf("%d", &total_planes) != 1) {
            fprintf(stderr, "Blad wczytywania.\n");
            fseek( stdin,0, SEEK_END );
            continue;
        }
        if (total_planes < MIN_PLANES || total_planes > MAX_PLANES_ALLOWED) {
            fprintf(stderr, "Niewlasciwa liczba samolotow.\n");
        continue;
    }
    break;
    }


    while (1) {
        printf("Podaj pojemnosc schodow K (%d..%d): ", MIN_STAIRS, MAX_STAIRS);
        fflush(stdout);
        if (scanf("%d", &stairs_capacity) != 1) {
            fprintf(stderr, "Blad wczytywania.\n");
            fseek(stdin,0,SEEK_END);
            continue;
        }
        if (stairs_capacity < MIN_STAIRS || stairs_capacity > MAX_STAIRS) {
            fprintf(stderr, "Niewlasciwa pojemnosc schodow.\n");
            continue;
        }
        break;
    }

    while (1) {
        printf("Podaj pojemnosc samolotu P (%d..%d): ", MIN_CAPACITY, MAX_CAPACITY);
        fflush(stdout);
        if (scanf("%d", &plane_capacity) != 1) {
            fprintf(stderr, "Blad wczytywania.\n");
            fseek(stdin,0,SEEK_END);
            continue;
        }
        if (plane_capacity < MIN_CAPACITY || plane_capacity > MAX_CAPACITY) {
            fprintf(stderr, "Niewlasciwa pojemnosc.\n");
            continue;
        }
        break;
    }

    while (1) {
        printf("Podaj dopuszczalna wage bagazu Md (%d..%d): ", MIN_BAGGAGE, MAX_BAGGAGE);
        fflush(stdout);
        if (scanf("%d", &max_baggage) != 1) {
            fprintf(stderr, "Blad wczytywania.\n");
            fseek(stdin,0,SEEK_END);
            continue;
        }
        if (max_baggage < MIN_BAGGAGE || max_baggage > MAX_BAGGAGE) {
            fprintf(stderr, "Niewlasciwa waga.\n");
            continue;
        }
        break;
    }

}

void endReceivedMain()
{
    printf("Cleaning the airport. See you again!\n");
    close(fd);
    unlink(m2t); // zamykamy przejscia miedzy salami
    unlink(t2s);
    unlink(s2d);
    exit(0);    
}


void set_endhandler() {
    struct sigaction current; /* current setup */
    sigemptyset(&current.sa_mask); /* clear the signal set */
    current.sa_flags = 0; /* for setting sa_handler, not sa_action */
    current.sa_handler = endReceivedMain;
    sigaction(SIGTERM, &current, NULL);
}


int main(int argc, char *argv[])
{
    set_endhandler(); // bedziemy czyscic kolejki na koncu symulacji
	simulationConfig();
	setup_passengers_shm();
	//usunmy stre polaczenia, jesli istnieja
	unlink(m2t);
	unlink(t2s);
	unlink(s2d);

    mkfifo(m2t, 0666); /* read/write for user/group/others */
   	mkfifo(t2s, 0666); /* read/write for user/group/others */
   	mkfifo(s2d, 0666); /* read/write for user/group/others */

	fd = open(m2t, O_CREAT | O_WRONLY); /* open as write-only */
	if (fd < 0) return -1; /* can't go on */

	int dfd = open(s2d, O_CREAT | O_WRONLY); /* open as write-only */
	if (dfd < 0) return -1; /* can't go on */

	// odpalamy procesy dla poszczegolnych czesci lotniska
	pid_t p1;
    p1 = fork();
    if(p1<0)
    {
      perror("terminal fork fail");
      exit(1);
    }
    // child process because return value zero
    else if (p1 == 0) {
    	//odpalamy terminal
    	terminalRun();
    }
    pid_t p2;
    p2 = fork();
    if (p2<0) {
    	perror("security fork fail");
    	exit(1);
    }
    else if (p2==0) {
    	sleep(2);
    	securityRun();
    }
    pid_t p3;
    p3 = fork();
    if (p3<0) {
    	perror("departures fork fail");
    	exit(1);
    }
    else if (p3==0) {
    	sleep(2);
    	departuresRun();
    }
   	
   	pid_t p;
    p = fork();
    if (p<0) {
    	perror("dispatcher fork fail");
    	exit(1);
    }
    else if (p==0) {
    	dispatcherRun(p1, p2, p3);
    }

    int passenger = 0;

   	// glowna petla symulacji
	while(passenger<total_passengers) {
		//printf("Airport simulation in progress...\n");
		if (MAIN_ECHO) printf("Sending passenger %d to the terminal\n", passenger);
		write(fd,&passenger, sizeof(int));
		passenger++;
		usleep(PASSENGER_FLOW);
    }	


    // na czas development, do not die...
    while(1) {
    	//dosylaj tych ktorzy np. wrocili z ewakuacji....
    	for (int i=0;i<total_passengers;i++) {
			if (!sem_wait(semptr)) {
				//ktos wrocil przed terminal (ewakuacja), przeslij ponownie
				if (passengers[i].status==0) {
					if (MAIN_ECHO) printf("Resending p:%d\n", i);
					passengers[i].status==0;
					write(fd,&i, sizeof(int));
				}
				sem_post(semptr);
			}
			usleep(PASSENGER_FLOW);
    	}
    }

    close(fd);
    unlink(m2t);
    shmdt((void *) passengers);
    printf("All passengers sent... Ending simulation");
    return(0);	
}

