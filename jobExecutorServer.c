#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "functions.h"


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

    // Close client socket
    close(clientSocket);
    pthread_exit(NULL);
}

void *worker_thread(void *arg) {
    // Worker thread logic goes here
    pthread_exit(NULL);
}


int main(int argc, char *argv[]){
    if (argc != 4) {
        return 1;
    }
    int port_num = atoi(argv[1]);
    int bufferSize = atoi(argv[2]);
    int threadPoolSize = atoi(argv[3]);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");


    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port_num);

    // Bind socket to address
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR on binding");

    // Listen for connections
    listen(sockfd, 5);




    // Create worker threads
    pthread_t worker_threads[threadPoolSize];
    for (int i = 0; i < threadPoolSize; i++) {
        pthread_create(&worker_threads[i], NULL, worker_thread, NULL);
    }


    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int newsockfd = accept(sockfd, (struct sockaddr *) &client_addr, &client_len);
        if (newsockfd < 0) 
            error("ERROR on accept");

        // Create controller thread for each connection
        pthread_t controller_thread_id;
        pthread_create(&controller_thread_id, NULL, controller_thread, &newsockfd);
    }

    close(sockfd);
    return 0;







    return 0;
}