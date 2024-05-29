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

extern queueptr myqueue;

FILE* create_file();
void send_output(int pid, int jobid, int clientSocket);
void issueJob_command(char *args,int clientSocket);
void setConcurrency_command(char *args,int clientSocket);
void stop_command(char *args,int clientSocket);
void poll_command(int clientSocket);

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void *controller_thread(void *arg) {                                                //controller thread which read the user command from the socket and executes it accordingly
    int clientSocket = *(int *)arg;
    char jobCommanderInputCommand[BUFFER_SIZE];
    memset(jobCommanderInputCommand, 0, BUFFER_SIZE);
    int n = read(clientSocket, jobCommanderInputCommand, BUFFER_SIZE);              //read the command
    if (n < 0) 
        error("ERROR reading from socket");
    printf("%s\n",jobCommanderInputCommand);
    
    char command[BUFFER_SIZE];
    char args[BUFFER_SIZE];

    sscanf(jobCommanderInputCommand, "%s %[^\n]", command, args);                   //identify command and call the corresponding function for each one
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
    else if(!strcmp(command,"exit")){                                               //in case of exit empty the queue and update commander with message
        empty_queue_and_inform(myqueue);
        sprintf(response,"SERVER TERMINATED");
        n = write(clientSocket, response, strlen(response));
        if (n < 0)
            error("ERROR writing to socket");
        close(clientSocket);
        pthread_kill(pthread_self(), SIGUSR1);                                      //send signal to server to break the loop and wait for workers to finish
    }
    pthread_exit(NULL);
}



void *worker_thread(void *arg) {                                                    //the worker thread is based on a loop constantly removing items from the queue and executes them using fork and exec
    while(1) {
        if(myqueue->worker_exit)                                                    //exit case
            break;
        nodeptr temp = lock_dequeue(myqueue);                                       //wait until item from queue is available and get pointer to it
        if(myqueue->worker_exit || temp==NULL)
            break;
        int clientSocket = temp->clientSocket;                                      //get the clientsocket
        pid_t pid;
        int status;
        pid = fork();                                                               //create new process to execute job
        if (pid < 0) 
            error("Fork failed");
        else if (pid == 0) {
            FILE *output_file=create_file();                                        //create file and redirect output to that file
            dup2(fileno(output_file), STDOUT_FILENO);
            char *args[BUFFER_SIZE];
            char *token = strtok(temp->job, " ");
            int i = 0;
            while (token != NULL && i < BUFFER_SIZE - 1) {                          //get args to a 2d array
                args[i++] = token;
                token = strtok(NULL, " ");
            }
            args[i] = NULL;
            execvp(args[0], args);                                                  //execute job
            error("execvp");
        } 
        else {
            waitpid(pid, &status, 0);                                               //wait for kid to finish
            send_output(pid, temp->jobid, clientSocket);                            //read output from the file and send to clientsocket
            pthread_mutex_lock(&myqueue->mtx);
            myqueue->currently_running--;                                           //update running counter
            pthread_mutex_unlock(&myqueue->mtx);
            free(temp);                                                             //free node
        }
    }
    printf("goodbye\n");
    pthread_exit(NULL);
}



void issueJob_command(char *args,int clientSocket){                                 //issueJob function which gets the job from the args and does enqueue creating a new node
    nodeptr temp;
    char response[BUFFER_SIZE];
    temp=enqueue(myqueue,args,clientSocket);
    sprintf(response,"JOB <job_%d,%s> SUBMITTED",temp->jobid,temp->job);            //send message to commander after submission
    int n = write(clientSocket, response, strlen(response));
    if (n < 0)
        error("ERROR writing to socket");
}


void setConcurrency_command(char *args,int clientSocket){                           //change the concurrency level of the program
    char response[BUFFER_SIZE];
    pthread_mutex_lock(&myqueue->mtx);
    if(atoi(args)>myqueue->concurrency)
        pthread_cond_signal(&myqueue->job_available);
    myqueue->concurrency=atoi(args); 
    pthread_mutex_unlock(&myqueue->mtx);
    sprintf(response,"CONCURRENCY SET AT %d",atoi(args));                           //send response back to commander
    int n = write(clientSocket, response, strlen(response));
    if (n < 0)
        error("ERROR writing to socket");
    close(clientSocket);
}


void stop_command(char *args,int clientSocket){                                     //stop command function which deletes the requested job from the queue
    char response[BUFFER_SIZE];
    nodeptr temp;
    int id;
    sscanf(args, "job_%d", &id);
    if((temp=deleteJobID(myqueue,id))==NULL)                                        //check if job exists in the queue and write the correct message to a buffer 
        sprintf(response,"JOB <job_%d> NOTFOUND",id);
    else
        sprintf(response,"JOB <job_%d> REMOVED",id);
    int n = write(clientSocket, response, strlen(response));                        //send response to commander
    if (n < 0)
        error("ERROR writing to socket");
    close(clientSocket);
    if(temp!=NULL)                                                                  //free node
        free(temp);
}



void poll_command(int clientSocket){                                                //poll command for the queue 
    char response[BUFFER_SIZE];
    write_queue_to_buffer(myqueue, response);                                       //write the queue into a buffer and then write the buffer to the clientSocket for the commander to read
    int n = write(clientSocket, response, strlen(response));
    printf("%s\n\n\n",response);
    if (n < 0)
        error("ERROR writing to socket");
    close(clientSocket);
}


FILE* create_file(){                                                                //create file with pid name
    char filename[20];
    sprintf(filename, "%d.output", getpid());
    FILE *output_file = fopen(filename, "w");                                       //open for writing
    if (output_file == NULL) 
        error("Error opening output file");
    return output_file;
}



void send_output(int pid, int jobid, int clientSocket) {                            //read the output file and send the output to the commander
    char filename[20];
    sprintf(filename, "%d.output", pid);                                            //get name

    FILE *file = fopen(filename, "r");                                              //open for reading
    if (file == NULL)
        error("Error opening output file");

    char start_message[BUFFER_SIZE];
    sprintf(start_message, "-----job_%d output start-----", jobid);

    char end_message[BUFFER_SIZE];
    sprintf(end_message, "-----job_%d output end-----", jobid);

    
    if (write(clientSocket, start_message, strlen(start_message)) < 0)              //send start message
        error("ERROR writing to socket");

    char response[BUFFER_SIZE];                                                     //send output in a loop because it might not fit into buffer
    size_t read_bytes;
    while ((read_bytes = fread(response, 1, BUFFER_SIZE, file)) > 0) {
        if (write(clientSocket, response, read_bytes) < 0)
            error("ERROR writing to socket");
    }

    if (write(clientSocket, end_message, strlen(end_message)) < 0)                  //send end message
        error("ERROR writing to socket");

    fclose(file);
    remove(filename);
    close(clientSocket);
}