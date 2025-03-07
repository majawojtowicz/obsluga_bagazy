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
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <semaphore.h>


// ustawienia wypisywania na ekran symulacji
#define MAIN_ECHO          1
#define TERMINAL_ECHO      1
#define SECURITY_ECHO      1
#define SEC_GATES_ECHO     1
#define DEPARTURES_ECHO    0
#define STAIRS_ECHO        0
#define CAPTAIN_ECHO       0
#define PASSENGER_ECHO     0

// ustawienia czasow symulacji (poszczegolne stanowiska, szybkosc obslugi, itd)
#define PASSENGER_FLOW     100000
#define SECURITY_HALL_WAIT 250000 // usleep, randomizowane miedzy 0.25s a 0.50s
#define SEC_CHECK_WAIT     500000 // usleep, 0.5s
#define DEPARTURES_HALL    100000 // 0.2s
#define DEPARTURES_GATES   200000 // randomizowane miedzy 0.20s a 0.40s
#define CAPTAIN_WAIT       900000 // 0.9s pomiedzy pasazerami
#define FLIGHT_TIME        10     // 10s w kazda strone (czyli w sumie 20s od wylotu do powrotu na lotnisko)
#define PASSENGER_SLEEP    2      // budzimy sie co 2s
#define PASSENGER_ANGRY    7      // jak utkniemy przez x PASSENGER_SLEEP w kolejce w security, to opuszczamy lotnisko (!)

// ustawienia czestotliwosci eventow odpalanych przez dispatchera
#define FORCED_FQ          5
#define EVACUATE_FQ        15  
#define NOCHECKINS_FQ      30


// simulation capacity constants 
#define MAX_PLANES         10
#define MAX_STATIONS       3   
#define STATION_CAPACITY   2   

#define SECURITY_THREAT    95 // poziom bezpieczenistwa, im liczba nizsza tym wieksza szansa ze pasazer nie przejdze (95/100 -> co 20 odpada, 90/100 -> co 10, itd.)


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

#define MAX_SEC_GATES      3

#define SIG_NO_MORE_PASSENGERS SIGUSR2 


#define FTOK_PATH "/tmp"
#define FTOK_CHAR 'X'


typedef struct {
    int  id;
    int  weight;        
    bool isVIP;
    bool isFemale;      
    int  assignedPlaneNumber; // plane = stairs
    int status; //-3, dissatisfied, -2 - overweight, -1 - not sent, 0 - outside, 1 - terminal/check-in, 
                // 2 - security, 3 - departure hall, 4 - stairs, 5 - plane, 6 - delivered 
                // -4 - security check failed
} Passenger;

#define BackingFile "/shMemEx"
#define AccessPerms 0644
#define SemaphoreName "PassengersData"

#define key 12345



#endif // COMMON_H
