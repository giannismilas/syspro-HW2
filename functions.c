#include "functions.h"
#include "Qimplementation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

extern queueptr myqueue;

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}



void *controller_thread(void *arg) {
    int clientSocket = *(int *)arg;

    // Read command from client
    char jobCommanderInputCommand[BUFFER_SIZE];
    memset(jobCommanderInputCommand, 0, BUFFER_SIZE);
    int n = read(clientSocket, jobCommanderInputCommand, BUFFER_SIZE);
    if (n < 0) 
        error("ERROR reading from socket");
    printf("%s\n",jobCommanderInputCommand);
    
    // Process command and handle job
    char command[BUFFER_SIZE];
    sscanf(jobCommanderInputCommand, "%s ", command);

    char response[BUFFER_SIZE];
    //sprintf(response, "Command processed");
    if(!strcmp(command,"issueJob")){
        nodeptr temp=enqueue(myqueue,jobCommanderInputCommand,clientSocket);
        sprintf(response,"JOB <job_%d,%s> SUBMITTED",temp->jobid,temp->job);
    }
    else if(!strcmp(command,"setConcurrency")){

    }
    else if(!strcmp(command,"stop")){
        
    }
    else if(!strcmp(command,"poll")){
        
    }
    else if(!strcmp(command,"exit")){
        
    }







    n = write(clientSocket, response, strlen(response));
    if (n < 0)
        error("ERROR writing to socket");

    // Close client socket
    close(clientSocket);
    pthread_exit(NULL);
}

void *worker_thread(void *arg) {
    
    pthread_exit(NULL);
}
