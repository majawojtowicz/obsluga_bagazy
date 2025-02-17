CC = gcc
CFLAGS = -Wall -Wextra -pthread

OBJS = main.o terminal.o dispatcher.o security.o departures.o

all: projekt

projekt: $(OBJS)
	$(CC) $(CFLAGS) -o projekt $(OBJS)

main.o: main.c common.h terminal.h dispatcher.h security.h departures.h
	$(CC) $(CFLAGS) -c main.c

terminal.o: terminal.h
	$(CC) $(CFLAGS) -c terminal.c

dispatcher.o: dispatcher.h
	$(CC) $(CFLAGS) -c dispatcher.c

security.o: security.h
	$(CC) $(CFLAGS) -c security.c

departures.o: departures.h
	$(CC) $(CFLAGS) -c departures.c

clean:
	rm -f *.o projekt simulation.log
