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






    printf("%d\n\n\n",clientSocket);
    n = write(clientSocket, response, strlen(response));
    if (n < 0)
        error("ERROR writing to socket");

    // // Close client socket
    // close(clientSocket);
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
            char filename[20];
            sprintf(filename, "%d.output", getpid());
            FILE *output_file = fopen(filename, "w");
            if (output_file == NULL) {
                perror("Error opening output file");
                exit(EXIT_FAILURE); 
            }
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
            perror("execvp");
            exit(EXIT_FAILURE);
        } else {
            waitpid(pid, &status, 0);
            char filename[20];
            sprintf(filename, "%d.output", pid);

            FILE *file = fopen(filename, "r");
            if (file == NULL) {
                perror("Error opening output file");
                pthread_exit(NULL);
            }
            char response[BUFFER_SIZE];
            size_t total_read = 0;
            size_t read_bytes;
            while ((read_bytes = fread(response + total_read, 1, BUFFER_SIZE - total_read, file)) > 0) {
                total_read += read_bytes;
                if (total_read >= BUFFER_SIZE - 1) {
                    break;
                }
            }
            fclose(file);
            response[total_read] = '\0';

            remove(filename);
            printf("%s\n\n\n",response);
            int n = write(clientSocket, response, strlen(response));
            if (n < 0)
                perror("ERROR writing to socket");
        }
    }
    pthread_exit(NULL);
}