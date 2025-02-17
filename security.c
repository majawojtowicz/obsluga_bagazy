#include "security.h"
#include <sys/prctl.h>

extern const char* t2s;
extern const char* s2d;

const char * securityName = "SecurityChecks";


void securityRun(void) {

	// ustaw nazwe procesu, zeby bylo widoczne na ptree

    if (prctl(PR_SET_NAME, (unsigned long) securityName) < 0)
    {
        perror("prctl()");
    }

	//zacznij nasluchiwac od terminala
	int fd = open(t2s, O_RDONLY);
	if (fd < 0) {
		printf("Security entry is broken, cannot handle passengers. Exiting\n");
		exit(1); /* no point in continuing */
	}

	//i puszczaj ludzi dalej
	//otworz korytarz z terminala do security
	int mfd = open(s2d, O_CREAT | O_WRONLY); /* open as write-only */
	if (mfd < 0) /* can't go on */
	{
		printf("Security passage to departure hall is non-functioning, cannot handle passengers. Exiting\n");
		exit(1); /* no point in continuing */		
	}


	while (1)
	{
		int next;
		int i;
		ssize_t count = read(fd, &next, sizeof(int));	
		if (count>0) {
			printf("Security received passenger:%d\n", next);
			//wyslij dalej
			write(mfd,&next, sizeof(int));
		}
		sleep(1+rand()%5);
	}
}