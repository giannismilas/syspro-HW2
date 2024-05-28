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
#include <signal.h>
#include <sys/wait.h>

FILE* create_file();
extern queueptr myqueue;

void send_output(int pid, int jobid, int clientSocket);
void issueJob_command(char *args,int clientSocket);
void setConcurrency_command(char *args,int clientSocket);
void stop_command(char *args,int clientSocket);
void poll_command(int clientSocket);

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
    char response[BUFFER_SIZE];
    if(!strcmp(command,"issueJob")){
        issueJob_command(args,clientSocket);
    }
    else if(!strcmp(command,"setConcurrency")){
        setConcurrency_command(args,clientSocket);
    }
    else if(!strcmp(command,"stop")){
        stop_command(args,clientSocket);
    }
    else if(!strcmp(command,"poll")){
        poll_command(clientSocket);
    }
    else if(!strcmp(command,"exit")){
        empty_queue_and_inform(myqueue);
        sprintf(response,"SERVER TERMINATED");
        n = write(clientSocket, response, strlen(response));
        if (n < 0)
            error("ERROR writing to socket");
        close(clientSocket);
        pthread_kill(pthread_self(), SIGUSR1);
    }
    pthread_exit(NULL);
}



void *worker_thread(void *arg) {
    while(1) {
        if(myqueue->worker_exit)
            break;
        nodeptr temp = lock_dequeue(myqueue);
        if(myqueue->worker_exit || temp==NULL)
            break;
        int clientSocket = temp->clientSocket;
        pid_t pid;
        int status;
        pid = fork();
        if (pid < 0) 
            error("Fork failed");
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
            pthread_mutex_lock(&myqueue->mtx);
            myqueue->currently_running--; 
            pthread_mutex_unlock(&myqueue->mtx);
        }
    }
    pthread_exit(NULL);
}



void issueJob_command(char *args,int clientSocket){
    nodeptr temp;
    char response[BUFFER_SIZE];
    temp=enqueue(myqueue,args,clientSocket);
    sprintf(response,"JOB <job_%d,%s> SUBMITTED",temp->jobid,temp->job);
    int n = write(clientSocket, response, strlen(response));
    if (n < 0)
        error("ERROR writing to socket");
}


void setConcurrency_command(char *args,int clientSocket){
    char response[BUFFER_SIZE];
    pthread_mutex_lock(&myqueue->mtx);
    myqueue->concurrency=atoi(args); 
    pthread_mutex_unlock(&myqueue->mtx);
    sprintf(response,"CONCURRENCY SET AT %d",atoi(args));
    int n = write(clientSocket, response, strlen(response));
    if (n < 0)
        error("ERROR writing to socket");
    close(clientSocket);
}


void stop_command(char *args,int clientSocket){
    char response[BUFFER_SIZE];
    nodeptr temp;
    int id;
    sscanf(args, "job_%d", &id);
    if((temp=deleteJobID(myqueue,id))==NULL)
        sprintf(response,"JOB <job_%d> NOTFOUND",id);
    else
        sprintf(response,"JOB <job_%d> REMOVED",id);
    int n = write(clientSocket, response, strlen(response));
    if (n < 0)
        error("ERROR writing to socket");
    close(clientSocket);
}



void poll_command(int clientSocket){
    char response[BUFFER_SIZE];
    write_queue_to_buffer(myqueue, response);
    int n = write(clientSocket, response, strlen(response));
    printf("%s\n\n\n",response);
    if (n < 0)
        error("ERROR writing to socket");
    close(clientSocket);
}


FILE* create_file(){
    char filename[20];
    sprintf(filename, "%d.output", getpid());
    FILE *output_file = fopen(filename, "w");
    if (output_file == NULL) 
        error("Error opening output file");
    return output_file;
}



void send_output(int pid, int jobid, int clientSocket) {
    char filename[20];
    sprintf(filename, "%d.output", pid);

    FILE *file = fopen(filename, "r");
    if (file == NULL)
        error("Error opening output file");

    char start_message[BUFFER_SIZE];
    sprintf(start_message, "-----job_%d output start-----", jobid);

    char end_message[BUFFER_SIZE];
    sprintf(end_message, "-----job_%d output end-----", jobid);

    // Send start message
    if (write(clientSocket, start_message, strlen(start_message)) < 0)
        error("ERROR writing to socket");

    char response[BUFFER_SIZE];
    size_t read_bytes;
    while ((read_bytes = fread(response, 1, BUFFER_SIZE, file)) > 0) {
        if (write(clientSocket, response, read_bytes) < 0)
            error("ERROR writing to socket");
    }

    // Send end message
    if (write(clientSocket, end_message, strlen(end_message)) < 0)
        error("ERROR writing to socket");

    fclose(file);
    remove(filename);
    close(clientSocket);
}