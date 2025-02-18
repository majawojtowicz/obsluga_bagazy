CC = gcc
CFLAGS = -Wall -Wextra -pthread -lrt

OBJS = main.o terminal.o dispatcher.o security.o departures.o gates.o captain.o passenger.o sec_check.o

all: projekt

projekt: $(OBJS)
	$(CC) $(CFLAGS) -o projekt $(OBJS)

main.o: main.c common.h terminal.h dispatcher.h security.h departures.h passenger.h
	$(CC) $(CFLAGS) -c main.c

terminal.o: terminal.h
	$(CC) $(CFLAGS) -c terminal.c

dispatcher.o: dispatcher.h
	$(CC) $(CFLAGS) -c dispatcher.c

security.o: security.h sec_check.h
	$(CC) $(CFLAGS) -c security.c

sec_check.o: sec_check.h
	$(CC) $(CFLAGS) -c sec_check.c

departures.o: departures.h gates.h captain.h
	$(CC) $(CFLAGS) -c departures.c

gates.o: gates.h
	$(CC) $(CFLAGS) -c gates.c

captain.o: captain.h
	$(CC) $(CFLAGS) -c captain.c

passenger.o: passenger.h
	$(CC) $(CFLAGS) -c passenger.c	

clean:
	rm -f *.o projekt simulation.log
