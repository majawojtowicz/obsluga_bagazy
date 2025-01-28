#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <semaphore.h>


#define MAX_PLANES         10
#define MAX_STATIONS       3   
#define STATION_CAPACITY   2   


#define MIN_PASSENGERS     1
#define MAX_PASSENGERS     100

#define MIN_PLANES         1
#define MAX_PLANES_ALLOWED 10

#define MIN_STAIRS         1
#define MAX_STAIRS         10

#define MIN_CAPACITY       1
#define MAX_CAPACITY       50

#define MIN_BAGGAGE        1
#define MAX_BAGGAGE        100


#define SIG_NO_MORE_PASSENGERS SIGUSR2 


#define FTOK_PATH "/tmp"
#define FTOK_CHAR 'X'


typedef struct {
    int  id;
    int  weight;     
    char gender;     
    bool isVIP;      
    int  yields;     
    bool isChecked;  
} Passenger;



#endif // COMMON_H
