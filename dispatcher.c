#include "dispatcher.h"
#include <sys/prctl.h>

const char * dispatcherName = "AirportDispatcher";

void dispatcherRun(pid_t terminalPid, pid_t securityPid, pid_t departuresPid) {

	// ustaw nazwe procesu, zeby bylo widoczne na ptree
    if (prctl(PR_SET_NAME, (unsigned long) dispatcherName) < 0)
    {
        perror("prctl()");
    }

	while (1)
	{
		sleep(1+rand()%5);

		// i wylosujmy jakas operacje
		int operation = rand()%3;
		printf("Dispatcher operation in progress:%d\n", operation);

		switch (operation) {
			case EVACUATE:
				printf ("ATTENTION! All passengers request to evacuate from the airport...\n");
				kill(terminalPid, SIGUSR1);
				kill(securityPid, SIGUSR1);
				kill(departuresPid, SIGUSR1);
			break;
			case FORCED_DEPART:
				printf ("ATTENTION! All airplanes requested to depart...\n");
			break;
			case NO_CHECKIN:
				printf ("ATTENTION! No more checkins...\n");
			break;
			default:
			printf("Dispatch operation not supported...\n");
		}
	}
}