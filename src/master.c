#include "matrix.h"

int main(int argc, char **argv) {

	int dimension = 0;
	int workers = 0;

	if (argc != 3)
		error("Expected 2 arguments. Usage: ./master dimension workers\n");
	else {
		dimension = atoi(argv[1]);
		workers = atoi(argv[2]);

		if (dimension % workers != 0)
			error("Number of workers must evenly divide into the dimension and must be equal\n");
	}

	printf("Matrix A\n");
	MATRIX** A = generateMatrix(dimension);
	
	printf("Matrix B\n");
	MATRIX** B = generateMatrix(dimension);
	
	MATRIX** C = malloc(dimension * sizeof(MATRIX*) * dimension);

	for (int i=0; i<dimension; i++)
		C[i] = malloc(sizeof(MATRIX) * dimension);
		
	initSocket(A, B, C, dimension, workers);
}

MATRIX** generateMatrix(int dimension) {
	MATRIX** matrix = malloc(dimension * sizeof(MATRIX*));

	// Allocate memory
	for (int i=0; i<dimension; i++)
		matrix[i] = malloc(dimension * sizeof(MATRIX));

	srand(time(NULL));
	// Generate random elements
	for (int i=0; i<dimension; i++)
		for (int j=0; j<dimension; j++)
			matrix[i][j] = rand() % MAX_VAL + MIN_VAL;

	// Print matrix
	for (int i=0; i<dimension; i++) {
		for (int j=0; j<dimension; j++) {
			printf("[%d] ", matrix[i][j]);
		}

		printf("\n");
	}

	printf("\n");

	return matrix;
}

void initSocket(MATRIX **A, MATRIX** B, MATRIX** C, int dimension, int numOfWorkers) {
	
	unsigned int clientLength = 0;
	
	struct sockaddr_in dest, serv;
	memset(&serv, 0, sizeof(serv));
	serv.sin_family = AF_INET;
	serv.sin_addr.s_addr = INADDR_ANY;
	serv.sin_port = htons(PORTNUM);
		
	printf("Receiving connection..\n");
	int mysocket = socket(AF_INET, SOCK_STREAM, 0);
	if (mysocket < 0)
		error("Connection failed\n");

	printf("Binding socket to address..\n");
	if (bind(mysocket, (struct sockaddr*)&serv, sizeof(struct sockaddr)) < 0)
		error("Binding failed\n");

	if (listen(mysocket, 1) != 0)
		error("Listening failed..\n");
	printf("Listened successfully\n");

	clientLength = sizeof(dest);

	printf("Taking request from queue..\n");
	
	int consocket = 0;
	long t = 0;
	pthread_t threads[numOfWorkers];
	struct arg_struct args;
	args.numOfRows = dimension/numOfWorkers;
	args.dimension = dimension;
	args.B = B; // May need to initialise here
	args.C = C;
	args.dimension = dimension;

	args.rowOfA = malloc(sizeof(MATRIX *) * args.numOfRows);

	for (int i=0; i<args.numOfRows; i++) {
		args.rowOfA[i] = malloc(sizeof(MATRIX) * args.dimension);
	}	

	int i = 0;
	
	while (i < numOfWorkers) { // HERE
		if ((consocket = accept(mysocket, (struct sockaddr*)&dest, &clientLength)) < 0)
			error("Accepting failed\n");
		args.socket = consocket;
		args.startIndex = args.numOfRows * (int) t;
		args.endIndex = args.numOfRows * ((int) t + 1);
		partitionMatrix(A, &args);
		
		if (pthread_create(&threads[t], NULL, &sendData, (void *)&args) < 0)
			error("Could not create thread\n");

		printf("Handler assigned\n");
		
		t++;
		i++;	
	}	

	printf("Exited loop\n");

	if (consocket < 0)
		error("Accept failed\n");

	for (long t=0; t<numOfWorkers; t++) {
		pthread_join(threads[t], NULL);
	}
	
	printf("\nAll data successfully sent\n");
	printf("Printing matrix C..\n");
	printMatrix(C, dimension);
	close(mysocket);
}

void* sendData(void* arguments) {
	struct arg_struct *args = arguments;
	int sock = args -> socket;
	int rows = args -> numOfRows;
	int dimension = args -> dimension;
	int startIndex = args -> startIndex;
	int endIndex = args -> endIndex;

	printf("Sending numOfRows..\n");
	if (send(sock, &rows, sizeof(rows), 0) < 0)
		error("Sending of numOfRows failed\n"); 
	printf("numOfRows sent successfully\n");

	printf("Sending dimension of the matrices..\n");
	if (send(sock, &dimension, sizeof(dimension), 0) < 0)
		error("Sending of dimension failed\n");
	printf("Dimension sent successfully\n");

	printf("Sending a slice of matrix A..\n");
	for (int i=startIndex; i<endIndex; i++) {
		printf("args -> rowOfA[i] %d\n", args -> rowOfA[i][0]);
		if (send(sock, args -> rowOfA[i], sizeof(MATRIX) * dimension, 0) < 0)
			error("Sending a slice of A failed\n");
	}

	for (int i=0; i<rows; i++) {
		for (int j=0; j<dimension; j++) {
			printf("[%d] ", args -> rowOfA[i][j]);
		}
		printf("\n");
	}
	printf("\nSlice of A sent successfully\n");

	printf("Sending matrix B..\n");
	for (int i=0; i<dimension; i++) {
		if (send(sock, args -> B[i], sizeof(MATRIX) * dimension, 0) < 0)
			error("Sending matrix B failed\n");
	}

	for (int i=0; i<dimension; i++) {
		for (int j=0; j<dimension; j++) {
			printf("[%d] ", args -> B[i][j]);
		}
		printf("\n");
	}
	printf("\nMatrix B sent successfully\n");


	printf("\nReceiving Matrix C..\n");
	for (int i=startIndex; i<endIndex; i++) {
		if (recv(sock, args -> C[i], sizeof(MATRIX) * dimension, 0) < 0)
			error("Receiving of C failed\n");
	}
	printf("\nReceived Matrix C successfully\n");

	printf("Printing matrix C..\n");
	for (int i=startIndex; i<endIndex; i++) {
		for (int j=0; j<dimension; j++) {
			printf("[%d] ", args -> C[i][j]);
		}
		printf("\n");
	}

	close(sock);

	pthread_exit(NULL);
}

void partitionMatrix(MATRIX** A, struct arg_struct *args) {
	int startIndex = args -> startIndex;
	int endIndex = args -> endIndex;

	for (int i=startIndex; i<endIndex; i++) {
		args -> rowOfA[i] = A[i];
	}
}

void printMatrix(MATRIX** C, int dimension) {
	for (int i=0; i<dimension; i++) {
		for (int j=0; j<dimension; j++) {
			printf("[%d] ", C[i][j]);
		}
		printf("\n");
	}
}

void error(char *msg) {
	perror(msg);
	exit(1);
}