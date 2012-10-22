CC = gcc
CFLAGS = -Wall -O2
DEBUG_CFLAGS = -Wall -O2 -g
APP = httpc

all:
	$(CC) $(CFLAGS)  httpc.c -o $(APP)

debug:
	$(CC) $(DEBUG_CFLAGS) httpc.c -o $(APP)

clean:
	rm -f *.o *.core a.out $(APP)
