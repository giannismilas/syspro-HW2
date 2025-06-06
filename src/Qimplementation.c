#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "functions.h"
#include "Qimplementation.h"


nodeptr createNode(int jobid, char* job,int clientSocket) {                                 //simple newnode function that allocates space for a new queue node to add job info and returns pointer 
    nodeptr newNode = (nodeptr)malloc(sizeof(struct node));
    if (newNode == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    newNode->jobid = jobid;
    newNode->job = malloc(BUFFER_SIZE * sizeof(char));
    strcpy(newNode->job,job);
    newNode->next = NULL;
    newNode->clientSocket=clientSocket;
    return newNode;
}


queueptr initQueue(int max_items) {                                                          //allocates space for the structure that holds info for the queue and initializes pointers to NULL and size to 0
    queueptr newQueue = (queueptr)malloc(sizeof(struct Q));
    if (newQueue == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    pthread_mutex_init(&newQueue->mtx, NULL);
    pthread_cond_init(&newQueue->job_available, NULL);
    pthread_cond_init(&newQueue->room_available, NULL);
    newQueue->front = NULL;
    newQueue->rear = NULL;
    newQueue->size = 0;
    newQueue->max_items=max_items;
    newQueue->concurrency=1;
    newQueue->cur_jobid=0;
    newQueue->currently_running=0;
    newQueue->worker_exit=0;
    return newQueue;
}


int isEmpty(queueptr q) {                                                                   //check if queue is Empty
    return (q->front == NULL);
}


nodeptr enqueue(queueptr q,  char* job, int clientSocket) {                                 //insert a node to the rear of the queue
    pthread_mutex_lock(&q->mtx);                                                            //access to queue
    while (q->size == q->max_items)                                                         //wait until there is space available
        pthread_cond_wait(&q->room_available, &q->mtx);
    
    nodeptr newNode;    
    if (isEmpty(q)) {                                                                       //first node so front and rear pointer show to the new node
        newNode = createNode(q->cur_jobid, job,clientSocket);
        q->front = newNode;
        q->rear = newNode;
    } 
    else {                                                                                  //insert to the rear with increased position
        newNode = createNode(q->cur_jobid, job,clientSocket);
        q->rear->next = newNode;
        q->rear = newNode;
    }
    q->size++;                                                                              //increase the size of the queue
    q->cur_jobid++;
    pthread_cond_signal(&q->job_available);                                                 //inform for new job
    pthread_mutex_unlock(&q->mtx);                                                          //unlock
    return newNode;
}


nodeptr lock_dequeue(queueptr q) {                                                          //remove a job from the front of the queue and modify all qpositions of the rest of the nodes
    pthread_mutex_lock(&q->mtx);                                                            //lock        
    while (!q->worker_exit && q->size == 0)                                                 //wait until a job is available
        pthread_cond_wait(&q->job_available, &q->mtx);
    if (q->worker_exit) {                                                                   //if server needs to shut down return
        pthread_mutex_unlock(&q->mtx);
        return NULL;
    }
    while (q->currently_running >= q->concurrency)                                          //wait until concurrency allows execution
        pthread_cond_wait(&q->job_available, &q->mtx);
    nodeptr temp = q->front;
    if(temp==NULL || q->worker_exit)
        return NULL;
    q->front = q->front->next;
    q->size--;
    q->currently_running++;
    pthread_cond_signal(&q->room_available);
    pthread_mutex_unlock(&q->mtx);
    return temp;
}


nodeptr dequeue(queueptr q) {                                                               //remove a job from the front of the queue and modify all qpositions of the rest of the nodes
    if(q->size==q->max_items)
        return NULL;
    nodeptr temp = q->front;
    q->front = q->front->next;
    q->size--;
    return temp;
}


void freeQueue(queueptr q) {                                                                //free the whole structure of the queue 
    while (!isEmpty(q))
        dequeue(q);
    free(q);
}


nodeptr deleteJobID(queueptr q, int jobID) {                                                //search for a job with the specific id asked and remove from the queue
    if (isEmpty(q)) {
        return NULL;
    }
    nodeptr current = q->front;
    nodeptr prev = NULL;
    while (current != NULL && current->jobid != jobID) {
        prev = current;
        current = current->next;
    }
    if (current == NULL) 
        return NULL;
    if (prev == NULL) 
        return dequeue(q);
    else 
        prev->next = current->next;
    if (current == q->rear)
        q->rear = prev;
    q->size--;
    return current;
}


void write_queue_to_buffer(queueptr q, char* buffer) {                                      //write the queue to a buffer to print for poll command
    pthread_mutex_lock(&q->mtx);
    nodeptr current = q->front;
    buffer[0] = '\0';
    while (current != NULL) {
        char temp[256];
        snprintf(temp, sizeof(temp), "<job_%d, %s>\n", current->jobid, current->job);
        if (strlen(buffer) + strlen(temp) + 1 < BUFFER_SIZE) {
            strcat(buffer, temp);
        } 
        else {
            break;
        }
        current = current->next;
    }
    pthread_mutex_unlock(&q->mtx);
}


void empty_queue_and_inform(queueptr q){                                                    //empty the queue and inform the processes that were waiting with message when server shuts down
    char response[BUFFER_SIZE];
    sprintf(response,"SERVER TERMINATED BEFORE EXECUTION");
    while(!isEmpty(q)){
        nodeptr temp=dequeue(q);
        int n = write(temp->clientSocket, response, strlen(response));
        if (n < 0)
            error("ERROR writing to socket");
        close(temp->clientSocket);
        free(temp);
    }
}