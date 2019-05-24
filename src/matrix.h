#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

#define MAX_VAL 10
#define MIN_VAL 1
#define PORTNUM 8606

typedef int MATRIX;

struct arg_struct {
	MATRIX** rowOfA;
	MATRIX** B;
	MATRIX** C;
	int numOfRows;
	int socket;
	int dimension;
	int startIndex;
	int endIndex;
};

// Master
MATRIX** generateMatrix(int dimension);
void initSocket(MATRIX **A, MATRIX** B, MATRIX** C, int dimension, int numOfWorkers);
void printMatrix(MATRIX** C, int dimension);
void  partitionMatrix(MATRIX** A, struct arg_struct *args);
void* sendData(void* args);

// Worker
void initConnection();
void calculateMatrix(int socket, int numOfRows, int dimension, MATRIX** rowOfA, MATRIX** B);

// Both
void error(char *msg);