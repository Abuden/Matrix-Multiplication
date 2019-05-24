#include "matrix.h"

int main() {
	initConnection();
}

void initConnection() {
	struct sockaddr_in dest;
	memset(&dest, 0, sizeof(dest));
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = inet_addr("127.0.0.1");
	dest.sin_port = htons(PORTNUM);

	printf("\nInitiating connection..\n");
	int mysocket = socket(AF_INET, SOCK_STREAM, 0);
	if (mysocket < 0) 
		error("Connection failed\n");

	printf("Connecting..\n");
	int length = connect(mysocket, (struct sockaddr*)&dest, sizeof(struct sockaddr)); // HERE
	printf("Length: %d\n", length);
	if (length < 0)
		error("Connection failed\n");
	printf("Connection successful\n");

	int numOfRows = 0;
	int dimension = 0;

	printf("\nReceiving number of workers..\n");
	if (recv(mysocket, &numOfRows, sizeof(numOfRows), 0) < 0)
		error("Receiving failed..\n");
	printf("Received numOfRows: %d\n", numOfRows);

	printf("\nReceiving dimension..\n");
	if (recv(mysocket, &dimension, sizeof(dimension), 0) < 0)
		error("Receiving failed..\n");
	printf("Received dimension: %d\n", dimension);

	MATRIX** rowOfA = malloc(sizeof(MATRIX *) * numOfRows);

	printf("Receiving a slice of matrix A..\n");
	for (int i=0; i<numOfRows; i++) {
		rowOfA[i] = malloc(sizeof(MATRIX) * dimension);
		if (recv(mysocket, rowOfA[i], sizeof(MATRIX) * dimension, 0) < 0)
			error("Receiving of a slice of matrix A failed\n");
	}
	printf("Received successfully\n");	
	for (int i=0; i<numOfRows; i++) {
		for (int j=0; j<dimension; j++) {
			printf("[%d] ", rowOfA[i][j]);
		}
		printf("\n");
	}

	MATRIX** B = malloc(sizeof(MATRIX *) * dimension);
	
	printf("Receiving matrix B..\n");
	for (int i=0; i<dimension; i++) {
		B[i] = malloc(sizeof(MATRIX) * dimension);
		if (recv(mysocket, B[i], sizeof(MATRIX) * dimension, 0) < 0)
			error("Receiving of Matrix  B failed\n");
	}
	printf("Received successfully\n");
	
	for (int i=0; i<dimension; i++) {
		for (int j=0; j<dimension; j++) {
			printf("[%d] ", B[i][j]);
		}
		printf("\n");
	}
	calculateMatrix(mysocket, numOfRows, dimension, rowOfA, B);

	close(mysocket);
}

void calculateMatrix(int socket, int numOfRows, int dimension, MATRIX** rowOfA, MATRIX** B) {

	MATRIX** rowOfC = malloc(sizeof(MATRIX *) * numOfRows);
	
	for (int i=0; i<numOfRows; i++) {
		rowOfC[i] = malloc(sizeof(MATRIX) * dimension);
	}

	int sum = 0;

	for (int i=0; i<numOfRows; i++) { // Rows of Matrix A
		for (int j=0; j<dimension; j++) { // Column of Matrix B
			for (int k=0; k<dimension; k++) { // Rows of Matrix B
				sum += rowOfA[i][k] * B[k][j];
			}

			rowOfC[i][j] = sum;
			sum = 0;
		}
	}

	printf("Printing matrix C..\n");
	for (int i=0; i<numOfRows; i++) {
		for (int j=0; j<dimension; j++) {
			printf("[%d] ", rowOfC[i][j]);
		}
		printf("\n");
	}

	for (int i=0; i<numOfRows; i++) {
		if (send(socket, rowOfC[i], sizeof(MATRIX) * dimension, 0) < 0)
			error("Sending of Matrix C failed..\n");
	}
}

void error(char *msg) {
	perror(msg);
	exit(1);
}