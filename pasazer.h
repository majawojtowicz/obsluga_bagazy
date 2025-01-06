#ifndef pasazer_h
#define pasazer_h


typedef struct Passenger {
    char *name;
    int id;
    int baggage_amount;
    bool is_VIP;
} Passenger;

int create_passenger(Passenger *passenger, char *name, int id, int baggage_amount, bool is_VIP);

void *passenger_start(void *arg);

#endif

