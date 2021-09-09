#--------------------------------------------------- 
# Target file to be compiled by default
#---------------------------------------------------
MAIN = sim

#---------------------------------------------------
# CC will be the compiler to use
#---------------------------------------------------
CC = gcc

#---------------------------------------------------
# CFLAGS will be the options passed to the compiler
#---------------------------------------------------
CFLAGS = -Wall
DBGFLAGS = -g -O0

#---------------------------------------------------
# LDFLAGS will be the modules loaded
#---------------------------------------------------
LDFLAGS = -L/usr/lib/x86_64-linux-gnu -lalleg -lm -lretif -lpthread

#--------------------------------------------------- 
# Target-specific variables
#---------------------------------------------------
debug: CFLAGS += $(DBGFLAGS)

#--------------------------------------------------- 
# Dependencies 
#---------------------------------------------------

all: $(MAIN)

debug: all

$(MAIN): main.o task.o physics.o userpanel.o udp.o
	$(CC) $(CFLAGS) -o $(MAIN) main.o task.o physics.o userpanel.o udp.o $(LDFLAGS)
	
main.o: main.c 
	$(CC) $(CFLAGS) -I/usr/include/lib -c main.c

task.o: task.c
	$(CC) $(CFLAGS) -c task.c
	
physics.o: physics.c
	$(CC) $(CFLAGS) -c physics.c

userpanel.o: userpanel.c
	$(CC) $(CFLAGS) -c userpanel.c
	
udp.o: udp.c
	$(CC) $(CFLAGS) -c udp.c

clean:
	rm -rf *.o sim