#include "departures.h"
#include <sys/prctl.h>

const char * departuresName = "DeparturesHall";

extern const char* s2d;

void evacuationReceivedDep()
{
	//signal(SIGUSR1, evacuationReceived);
	printf("--- Departure hall - received evacuation request! Proceed to the exits.\n");
}

void set_evachandler() {
	struct sigaction current; /* current setup */
	sigemptyset(&current.sa_mask); /* clear the signal set */
	current.sa_flags = 0; /* for setting sa_handler, not sa_action */
	current.sa_handler = evacuationReceivedDep; /* specify a handler */
	sigaction(SIGUSR1, &current, NULL); /* register the handler */
}

void departuresRun(void) {
	// ustaw nazwe procesu, zeby bylo widoczne na ptree
    if (prctl(PR_SET_NAME, (unsigned long) departuresName) < 0)
    {
        perror("prctl()");
    }

    set_evachandler();

    //zacznij nasluchiwac od terminala
	int fd = open(s2d, O_RDONLY);
	if (fd < 0) {
		printf("Departures hall entry is broken, cannot handle passengers. Exiting\n");
		exit(1); /* no point in continuing */
	}


	while (1)
	{
		int next;
		int i;
		ssize_t count = read(fd, &next, sizeof(int));	
		if (count>0) {
			printf("Departures received passenger:%d\n", next);
		}
		sleep(1+rand()%5);
	}
}