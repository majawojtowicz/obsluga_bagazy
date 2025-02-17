#include "common.h"

extern void terminalRun(void);
extern void securityRun(void);
extern void departuresRun(void);
extern void dispatcherRun(pid_t p1, pid_t p2, pid_t p3);

//korytarze (nazwane kolejki fifo) miedzy poszczegolnymi czesciami lotniska
const char* m2t = "./main2terminal";
const char* t2s = "./terminal2security";
const char* s2d = "./security2departures";


int main(int argc, char *argv[])
{
	//usunmy stre polaczenia, jesli istnieja

	unlink(m2t);
	unlink(t2s);
	unlink(s2d);

    mkfifo(m2t, 0666); /* read/write for user/group/others */
   	mkfifo(t2s, 0666); /* read/write for user/group/others */
   	mkfifo(s2d, 0666); /* read/write for user/group/others */

	int fd = open(m2t, O_CREAT | O_WRONLY); /* open as write-only */
	if (fd < 0) return -1; /* can't go on */

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
	while(passenger<100) {
		//printf("Airport simulation in progress...\n");
		printf("Sending passenger %d to the terminal\n", passenger);
		write(fd,&passenger, sizeof(int));
		passenger++;
		sleep(1);
    }	
    close(fd);
    unlink(m2t);
    printf("All passengers sent... Ending simulation");
    return(0);	
}

