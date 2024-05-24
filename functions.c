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
#include <sys/wait.h>

FILE* create_file();
void send_output(int pid, int jobid, int clientSocket);

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
    char args[BUFFER_SIZE];

    // Extract the command and the rest of the input
    sscanf(jobCommanderInputCommand, "%s %[^\n]", command, args);
    nodeptr temp;
    char response[BUFFER_SIZE];
    if(!strcmp(command,"issueJob")){
        temp=enqueue(myqueue,args,clientSocket);
        sprintf(response,"JOB <job_%d,%s> SUBMITTED",temp->jobid,temp->job);
    }
    else if(!strcmp(command,"setConcurrency")){

    }
    else if(!strcmp(command,"stop")){
        int id;
        sscanf(args, "job_%d", &id);
        if((temp=deleteJobID(myqueue,id))==NULL){
            sprintf(response,"JOB <job_%d> NOTFOUND",id);
        }
        else{
            sprintf(response,"JOB <job_%d> REMOVED",id);
        }
    }
    else if(!strcmp(command,"poll")){
        write_queue_to_buffer(myqueue, response);
    }
    else if(!strcmp(command,"exit")){
        
    }
    n = write(clientSocket, response, strlen(response));
    if (n < 0)
        error("ERROR writing to socket");

    // Close client socket
    if(strcmp(command,"issueJob"))
        close(clientSocket);
    pthread_exit(NULL);
}




void *worker_thread(void *arg) {
    while(1) {
        nodeptr temp = dequeue(myqueue);
        int clientSocket = temp->clientSocket;
        pid_t pid;
        int status;
        pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            pthread_exit(NULL);
        } 
        else if (pid == 0) {
            FILE *output_file=create_file();
            dup2(fileno(output_file), STDOUT_FILENO);
            char *args[BUFFER_SIZE];
            char *token = strtok(temp->job, " ");
            int i = 0;
            while (token != NULL && i < BUFFER_SIZE - 1) {
                args[i++] = token;
                token = strtok(NULL, " ");
            }
            args[i] = NULL;
            execvp(args[0], args);
            error("execvp");
        } 
        else {
            waitpid(pid, &status, 0);
            send_output(pid, temp->jobid, clientSocket);
        }
    }
    pthread_exit(NULL);
}




FILE* create_file(){
    char filename[20];
    sprintf(filename, "%d.output", getpid());
    FILE *output_file = fopen(filename, "w");
    if (output_file == NULL) {
        perror("Error opening output file");
        exit(EXIT_FAILURE); 
    }
    return output_file;
}


void send_output(int pid, int jobid, int clientSocket) {
    char filename[20];
    sprintf(filename, "%d.output", pid);

    FILE *file = fopen(filename, "r");
    if (file == NULL)
        error("Error opening output file");

    char response[BUFFER_SIZE];
    size_t total_read = 0;
    size_t read_bytes;
    char start_delimiter[BUFFER_SIZE];
    sprintf(start_delimiter, "-----job_%d output start-----\n", jobid);

    strcpy(response, start_delimiter);
    while ((read_bytes = fread(response + strlen(response), 1, BUFFER_SIZE - total_read - strlen(response), file)) > 0) {
        total_read += read_bytes;
        if (total_read >= BUFFER_SIZE - strlen(response) - 1) {
            break;
        }
    }
    fclose(file);

    char end_delimiter[BUFFER_SIZE];
    sprintf(end_delimiter, "\n-----job_%d output end-----", jobid);
    strcat(response, end_delimiter);
    response[BUFFER_SIZE - 1] = '\0';

    remove(filename);

    int n = write(clientSocket, response, strlen(response));
    if (n < 0)
        error("ERROR writing to socket");

    close(clientSocket);
}