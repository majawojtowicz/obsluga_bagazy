#include "security.h"

SecurityStation securityStations[MAX_STATIONS];


static PassengerQueue securityQueue;


void init_security_stations(void)
{
    for (int i=0; i<MAX_STATIONS; i++) {
        pthread_mutex_init(&securityStations[i].stationLock, NULL);
        pthread_cond_init(&securityStations[i].stationCond, NULL);
        securityStations[i].occupantCount  = 0;
        securityStations[i].occupantGender = ' ';
    }

    init_passenger_queue(&securityQueue);
}


int security_check(int passenger_id, char gender)
{
    while (1) {
        for (int i = 0; i < MAX_STATIONS; i++) {
            pthread_mutex_lock(&securityStations[i].stationLock);

            if (securityStations[i].occupantCount < STATION_CAPACITY) {
                
                if (securityStations[i].occupantGender == ' ' ||
                    securityStations[i].occupantGender == gender)
                {
                    if (securityStations[i].occupantCount == 0) {
                        securityStations[i].occupantGender = gender;
                    }
                    securityStations[i].occupantCount++;
                    int station_id = i;

                    printf("[SECURITY] Passenger %d at station %d (gender=%c)\n",
                           passenger_id, i, gender);

                    pthread_mutex_unlock(&securityStations[i].stationLock);
                    return station_id;
                }
            }

            pthread_mutex_unlock(&securityStations[i].stationLock);
        }

        
        sleep(1);
    }
}


void leave_station(int station_id, int passenger_id)
{
    pthread_mutex_lock(&securityStations[station_id].stationLock);

    securityStations[station_id].occupantCount--;
    if (securityStations[station_id].occupantCount <= 0) {
        securityStations[station_id].occupantCount  = 0;
        securityStations[station_id].occupantGender = ' ';
    }

    pthread_cond_broadcast(&securityStations[station_id].stationCond);
    pthread_mutex_unlock(&securityStations[station_id].stationLock);

    printf("[SECURITY] Passenger %d left station %d\n", passenger_id, station_id);
}



void init_passenger_queue(PassengerQueue *queue)
{
    pthread_mutex_init(&queue->queueLock, NULL);
    pthread_cond_init(&queue->queueCond, NULL);
    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
}


bool enqueue_passenger(PassengerQueue *queue, Passenger pass)
{
    pthread_mutex_lock(&queue->queueLock);

    
    if (!pass.isVIP && queue->size > 3) {
        printf("[SECURITY QUEUE] Passenger %d frustrated (queue size=%d). Leaves.\n",
               pass.id, queue->size);
        pthread_mutex_unlock(&queue->queueLock);
        return false;
    }

    
    QueueNode *newNode = (QueueNode *)malloc(sizeof(QueueNode));
    newNode->passenger = pass;
    newNode->next      = NULL;

    
    if (pass.isVIP) {
        newNode->next = queue->head;
        queue->head   = newNode;
        if (queue->tail == NULL) {
            queue->tail = newNode;
        }
    } else {
        
        if (queue->tail == NULL) {
            queue->head = newNode;
            queue->tail = newNode;
        } else {
            queue->tail->next = newNode;
            queue->tail       = newNode;
        }
    }
    queue->size++;

    pthread_cond_broadcast(&queue->queueCond);
    pthread_mutex_unlock(&queue->queueLock);

    return true;
}

Passenger dequeue_passenger(PassengerQueue *queue)
{
    pthread_mutex_lock(&queue->queueLock);

    while (queue->head == NULL) {
        pthread_cond_wait(&queue->queueCond, &queue->queueLock);
    }

    QueueNode *node = queue->head;
    queue->head = node->next;
    if (queue->head == NULL) {
        queue->tail = NULL;
    }
    queue->size--;

    pthread_mutex_unlock(&queue->queueLock);

    Passenger p = node->passenger;
    free(node);
    return p;
}