#include "terminal.h"
#include <sys/prctl.h>
#include <signal.h>

extern const char* m2t;
extern const char* t2s;

const char * terminalName = "Terminal";

void evacuationReceived()
{
	//signal(SIGUSR1, evacuationReceived);
	printf("--- Terminal - received evacuation request! Proceed to the exits.\n");
}

void set_handler() {
	struct sigaction current; /* current setup */
	sigemptyset(&current.sa_mask); /* clear the signal set */
	current.sa_flags = 0; /* for setting sa_handler, not sa_action */
	current.sa_handler = evacuationReceived; /* specify a handler */
	sigaction(SIGUSR1, &current, NULL); /* register the handler */
}


void terminalRun(void) {

	// ustaw nazwe procesu, zeby bylo widoczne na ptree
    if (prctl(PR_SET_NAME, (unsigned long) terminalName) < 0)
    {
        perror("prctl()");
    }

    //zarejestruj sie na sygnaly
	//signal(SIGUSR1, evacuationReceived);
	set_handler();

	//zacznij nasluchiwac od wejscia glownego
	int mfd = open(m2t, O_RDONLY);
	if (mfd < 0) {
		printf("Terminal entry is broken, cannot handle passengers. Exiting\n");
		exit(1); /* no point in continuing */
	}

	//otworz korytarz z terminala do security
	//mkfifo(t2s, 0666); /* read/write for user/group/others */
	int fd = open(t2s, O_CREAT | O_WRONLY); /* open as write-only */
	if (fd < 0) /* can't go on */
	{
		printf("Terminal exit to security is broken, cannot handle passengers. Exiting\n");
		exit(1); /* no point in continuing */		
	}
	
	while (1)
	{
		int next;
		int i;
		ssize_t count = read(mfd, &next, sizeof(int));	
		if (count>0) {
			printf("Terminal operating and received passenger:%d\n", next);
			//wyslij dalej
			write(fd,&next, sizeof(int));
		}
		sleep(1+rand()%4);
	}
}