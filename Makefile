CC = gcc
CFLAGS = -Wall -Wextra -pthread

OBJS = main.o plane.o passenger.o security.o captain.o dispatcher.o operacje.o

all: projekt

projekt: $(OBJS)
        $(CC) $(CFLAGS) -o projekt $(OBJS)

main.o: main.c common.h plane.h passenger.h captain.h dispatcher.h security.h operacje.h
        $(CC) $(CFLAGS) -c main.c

plane.o: plane.c plane.h common.h
        $(CC) $(CFLAGS) -c plane.c

passenger.o: passenger.c passenger.h common.h plane.h security.h operacje.h
        $(CC) $(CFLAGS) -c passenger.c

security.o: security.c security.h common.h
        $(CC) $(CFLAGS) -c security.c

captain.o: captain.c captain.h plane.h common.h
        $(CC) $(CFLAGS) -c captain.c

dispatcher.o: dispatcher.c dispatcher.h common.h
        $(CC) $(CFLAGS) -c dispatcher.c

operacje.o: operacje.c operacje.h common.h
        $(CC) $(CFLAGS) -c operacje.c

clean:
        rm -f *.o projekt simulation.log