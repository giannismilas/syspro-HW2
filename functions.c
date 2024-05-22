#include "functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>




void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}



void *controller_thread(void *arg) {
    int clientSocket = *(int *)arg;

    // Read command from client
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    int n = read(clientSocket, buffer, BUFFER_SIZE);
    if (n < 0) 
        error("ERROR reading from socket");
    printf("%s\n",buffer);
    // Process command and handle job







    char response[BUFFER_SIZE];
    sprintf(response, "Command processed");

    n = write(clientSocket, response, strlen(response));
    if (n < 0)
        error("ERROR writing to socket");

    // Close client socket
    close(clientSocket);
    pthread_exit(NULL);
}

void *worker_thread(void *arg) {
    // Worker thread logic goes here
    pthread_exit(NULL);
}
