CC = gcc
CFLAGS = -Wall -O2

all: dyspozytor pasazer plane

dyspozytor: dyspozytor.c operacje.h
	$(CC) $(CFLAGS) -o dyspozytor dyspozytor.c

pasazer: pasazer.c operacje.h
	$(CC) $(CFLAGS) -o pasazer pasazer.c

plane: plane.c operacje.h
	$(CC) $(CFLAGS) -o plane plane.c

clean:
	rm -f dyspozytor pasazer plane
	