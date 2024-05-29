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

extern queueptr myqueue;
volatile sig_atomic_t is_running = 1;
int sockfd;

void custom_signal_handler(){
    is_running=0;
    shutdown(sockfd, SHUT_RDWR);
}

int main(int argc, char *argv[]){
    if (argc != 4) 
        return 1;
    
    signal(SIGUSR1, custom_signal_handler);
    int port_num = atoi(argv[1]);
    int bufferSize = atoi(argv[2]);
    int threadPoolSize = atoi(argv[3]);
    myqueue=initQueue(bufferSize);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");


    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port_num);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR on binding");

    // Listen for connections
    listen(sockfd, 5);
    // Create worker threads
    pthread_t worker_threads[threadPoolSize];
    for (int i = 0; i < threadPoolSize; i++) 
        pthread_create(&worker_threads[i], NULL, worker_thread, NULL);
    


    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int newsockfd = accept(sockfd, (struct sockaddr *) &client_addr, &client_len);
        if(!is_running)
            break;
        
        if (newsockfd < 0) 
            error("ERROR on accept");

        pthread_t controller_thread_id;
        pthread_create(&controller_thread_id, NULL, controller_thread, &newsockfd);
    }

    pthread_mutex_lock(&myqueue->mtx);
    myqueue->worker_exit = 1;
    for (int i = 0; i < threadPoolSize; i++) 
        pthread_cond_signal(&myqueue->job_available); 
    
    pthread_mutex_unlock(&myqueue->mtx);

    for (int i = 0; i < threadPoolSize; i++) {
        pthread_join(worker_threads[i], NULL);
    }
    close(sockfd);
    return 0;
}