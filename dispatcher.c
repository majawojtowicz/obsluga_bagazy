#include "dispatcher.h"
#include <sys/prctl.h>


extern int total_passengers;

const char * dispatcherName = "AirportDispatcher";

int dshmid;
int period=0; // co 10 sekund force depart
Passenger *dpassengers;
int operation=-1;
int dcount = 100;
sem_t* dsemptr;

bool noCheckinsRaised;

void printAirportStatus(pid_t terminalPid, pid_t securityPid, pid_t departuresPid) {

		int declined = 0;
		int delivered =0;
		int waiting = 0;
		if (!sem_wait(dsemptr)) {
			// print status z shared memory
			printf("outside:\t");
			for (int i=0;i<total_passengers;i++) {
				if (dpassengers[i].status==0) {
					printf("+"); 
					waiting++;
				}
				else if (dpassengers[i].status==-2) {
					printf("-"); 
					declined++;
				}
				else if (dpassengers[i].status==-3) {
					printf("!");
					declined++;
				}
				else if (dpassengers[i].status==-4) {
					printf("#");
					declined++;
				}
				else printf(" ");
			}
			printf("\n");
			printf("termina:\t");
			for (int i=0;i<total_passengers;i++) {
				if (dpassengers[i].status==1) printf("T"); else printf(" ");
			}
			printf("\n");
			printf("securit:\t");
			for (int i=0;i<total_passengers;i++) {
				if (dpassengers[i].status==2) printf("S"); else printf(" ");
			}
			printf("\n");
			printf("departu:\t");
			for (int i=0;i<total_passengers;i++) {
				if (dpassengers[i].status==3) printf("D"); 
				else if (dpassengers[i].status==31) printf("x");
				else printf(" ");
			}
			printf("\n");
			printf("stairs :\t");
			for (int i=0;i<total_passengers;i++) {
				if (dpassengers[i].status==4) printf("%d",dpassengers[i].assignedPlaneNumber); else printf(" ");
			}
			printf("\n");
			printf("planes :\t");
			for (int i=0;i<total_passengers;i++) {
				if (dpassengers[i].status==5) printf("%d",dpassengers[i].assignedPlaneNumber); else printf(" ");
			}
			printf("\n");
			printf("landed :\t");
			for (int i=0;i<total_passengers;i++) {
				if (dpassengers[i].status==6) {
					printf("+");
					delivered++;
				} else printf(" ");
			}
			printf("\n");
			sem_post(dsemptr);
		}
		if (declined+delivered>=total_passengers || (noCheckinsRaised && declined+delivered+waiting>=total_passengers)) {
			//finish simulation
			printf ("Closing simulation...\n");
			kill(terminalPid, SIGTERM);
			kill(securityPid, SIGTERM);
			kill(departuresPid, SIGTERM);
			kill(getppid(), SIGTERM);
			//czyscimy pamiec wspoldzielona
			shmctl(dshmid, IPC_RMID, NULL);
			exit(0);
		}

}

void dispatcherRun(pid_t terminalPid, pid_t securityPid, pid_t departuresPid) {

	noCheckinsRaised = false;
	// ustaw nazwe procesu, zeby bylo widoczne na ptree
    if (prctl(PR_SET_NAME, (unsigned long) dispatcherName) < 0)
    {
        perror("prctl()");
    }

    key_t kls = ftok(".", 'X');
    dshmid = shmget(kls, total_passengers*sizeof(Passenger), IPC_CREAT|0660);
	if ((int) dshmid == -1) {
       perror("Error occured during shmid() call\n");
       exit(1);
  	}
	dpassengers = (Passenger *) shmat (dshmid, 0, 0);

	if ((int) dpassengers == -1) {
       perror("Error occured during shmat() call\n");
       exit(1);
  	}

  	dsemptr = sem_open(SemaphoreName, /* name */
							O_CREAT, /* create the semaphore */
							AccessPerms, /* protection perms */
							0); /* initial value */
	if (dsemptr == (void*) -1) printf("sem_open");


	while (1)
	{
		sleep(2);
		period++;
		printf("Dispatcher cycle:%d\n", period);
		// i wylosujmy jakas operacje
		if (period%FORCED_FQ==0) operation = FORCED_DEPART;
		if (period%EVACUATE_FQ==0) operation = EVACUATE;
		if (period%NOCHECKINS_FQ==0) {
			operation = NO_CHECKIN;
			noCheckinsRaised=true;
		}

		switch (operation) {
			case EVACUATE:
				printf ("ATTENTION! All passengers request to evacuate from the airport...\n");
				kill(terminalPid, SIGUSR1);
				kill(securityPid, SIGUSR1);
				kill(departuresPid, SIGUSR1);
				operation = -1;
			break;
			case FORCED_DEPART:
				printf ("ATTENTION! All airplanes requested to depart...\n");
				kill(departuresPid, SIGUSR2);
				operation = -1;
			break;
			case NO_CHECKIN:
				printf ("ATTENTION! No more checkins...\n");
				kill(terminalPid, SIGRTMIN+1);
			break;
			default:
			//printf("Dispatch operation not supported...\n");
		}
		operation=-1;
		if (period%2==0) {
			printAirportStatus(terminalPid, securityPid, departuresPid);
		}
	}
}