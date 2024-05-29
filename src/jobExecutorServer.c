#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include "functions.h"
#include "Qimplementation.h"

extern queueptr myqueue;                                    //global variable for the queue that holds jobs that are to be executed
volatile sig_atomic_t is_running = 1;                       //variable to break server loop when exit command is sent
int sockfd;

void custom_signal_handler(){                               //custom signal handler to shut server down and break loop
    is_running=0;
    shutdown(sockfd, SHUT_RDWR);
}

int main(int argc, char *argv[]){
    if (argc != 4)                                          //check for args
        return 1;
    
    signal(SIGUSR1, custom_signal_handler);                 //use custom handler for SIGUSR1
    int port_num = atoi(argv[1]);
    int bufferSize = atoi(argv[2]);
    int threadPoolSize = atoi(argv[3]);
    myqueue=initQueue(bufferSize);                          //initialize the queue

    sockfd = socket(AF_INET, SOCK_STREAM, 0);               //initialize socket
    if (sockfd < 0) 
        error("ERROR opening socket");


    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port_num);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)  //bind socket to the server so that commanders can send their jobs
        error("ERROR on binding");

    
    listen(sockfd, 5);                                      //Listen for connections

    pthread_t worker_threads[threadPoolSize];               //create the requested amount of workers that will wait until a job is available on the queue      
    for (int i = 0; i < threadPoolSize; i++) 
        pthread_create(&worker_threads[i], NULL, worker_thread, NULL);
    


    while (1) {                                             //loop that accepts commanders connections and creates a controller thread for each one so that the command of the user is identified and executed
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int newsockfd = accept(sockfd, (struct sockaddr *) &client_addr, &client_len);
        if(!is_running)
            break;
        
        if (newsockfd < 0) 
            error("ERROR on accept");

        pthread_t controller_thread_id;                     //create the controller thead
        pthread_create(&controller_thread_id, NULL, controller_thread, &newsockfd);
    }

    pthread_mutex_lock(&myqueue->mtx);                      //inform workers that server needs to shutdown so that they finish their last job and return
    myqueue->worker_exit = 1;
    for (int i = 0; i < threadPoolSize; i++) 
        pthread_cond_signal(&myqueue->job_available); 
    
    pthread_mutex_unlock(&myqueue->mtx);

    for (int i = 0; i < threadPoolSize; i++) {              //wait for all workers
        pthread_join(worker_threads[i], NULL);
    }
    close(sockfd);                                          //close and release socket
    return 0;
}