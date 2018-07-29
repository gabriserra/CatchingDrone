#--------------------------------------------------- 
# Target file to be compiled by default
#---------------------------------------------------
MAIN = main
#---------------------------------------------------
# CC will be the compiler to use
#---------------------------------------------------
CC = gcc
#---------------------------------------------------
# CFLAGS will be the options passed to the compiler
#---------------------------------------------------
CFLAGS = -Wall -lpthread -lrt
#---------------------------------------------------
# LDFLAGS will be the modules loaded
#---------------------------------------------------
LDFLAGS = -L/usr/lib/x86_64-linux-gnu -lalleg -lm -pthread
#--------------------------------------------------- 
# Dependencies 
#---------------------------------------------------
$(MAIN): $(MAIN).o ptask.o physics.o userpanel.o udp.o
	$(CC) $(CFLAGS) -o $(MAIN) $(MAIN).o ptask.o physics.o userpanel.o udp.o $(LDFLAGS)
	
$(MAIN).o: $(MAIN).c 
	$(CC) -c $(MAIN).c

ptask.o: ptask.c
	$(CC) -c ptask.c
	
physics.o: physics.c
	$(CC) -c physics.c

userpanel.o: userpanel.c
	$(CC) -c userpanel.c
	
udp.o: udp.c
	$(CC) -c udp.c