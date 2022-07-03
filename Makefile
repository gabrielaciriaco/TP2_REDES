CC=gcc 
CFLAGS=-Wall -Wextra -g 
THREADFLAG=-pthread
EXEC_EQUIPMENT=./equipment 
EXEC_SERVER=./server

all: $(EXEC_EQUIPMENT) $(EXEC_SERVER)

$(EXEC_EQUIPMENT): equipment.c
	$(CC) $(CFLAGS) $(THREADFLAG) equipment.c -o $(EXEC_EQUIPMENT)

$(EXEC_SERVER): server.c 
	$(CC) $(CFLAGS) $(THREADFLAG) server.c  -o $(EXEC_SERVER)

clean:
	rm -rf *.o server equipment