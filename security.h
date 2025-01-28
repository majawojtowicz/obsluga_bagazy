#ifndef SECURITY_H
#define SECURITY_H

#include "common.h"

typedef struct {
    pthread_mutex_t stationLock;
    pthread_cond_t  stationCond;
    int occupantCount;   
    char occupantGender; 
} SecurityStation;


typedef struct QueueNode {
    Passenger passenger;
    struct QueueNode *next;
} QueueNode;

typedef struct {
    pthread_mutex_t queueLock;
    pthread_cond_t  queueCond;
    QueueNode *head;
    QueueNode *tail;
    int size;            
} PassengerQueue;


extern SecurityStation securityStations[MAX_STATIONS];


void init_security_stations(void);


int security_check(int passenger_id, char gender);


void leave_station(int station_id, int passenger_id);


void init_passenger_queue(PassengerQueue *queue);


bool enqueue_passenger(PassengerQueue *queue, Passenger pass);


Passenger dequeue_passenger(PassengerQueue *queue);

#endif // SECURITY_H