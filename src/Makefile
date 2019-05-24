CC=gcc
CFLAGS=-Wall
HFILES=matrix.h
OBJ=master.o worker.o

all: master worker

master: master.o
	$(CC) $(CFLAGS) -o master master.o

worker: worker.o
	$(CC) $(CFLAGS) -o worker worker.o

master.o: master.c $(HFILES)
	$(CC) $(CFLAGS) -c  master.c

worker.o: worker.c $(HFILES)
	$(CC) $(CFLAGS) -c worker.c

clean: 
	$(RM) matrix *.o *~
