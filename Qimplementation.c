#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "functions.h"
#include "Qimplementation.h"


nodeptr createNode(int jobid, char* job,int clientSocket) {    //simple newnode function that allocates space for a new queue node to add job info and returns pointer 
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
    return newQueue;
}


int isEmpty(queueptr q) {                                                       //check if queue is Empty
    return (q->front == NULL);
}


nodeptr enqueue(queueptr q,  char* job, int clientSocket) {          //insert a node to the rear of the queue
    pthread_mutex_lock(&q->mtx);
    while (q->size == q->max_items)
        pthread_cond_wait(&q->room_available, &q->mtx);
    
    nodeptr newNode;
    if (isEmpty(q)) {                                                           //first node so front and rear pointer show to the new node
        newNode = createNode(q->cur_jobid, job,clientSocket);
        q->front = newNode;
        q->rear = newNode;
    } 
    else {                                                                      //insert to the rear with increased position
        newNode = createNode(q->cur_jobid, job,clientSocket);
        q->rear->next = newNode;
        q->rear = newNode;
    }
    q->size++;                                                                  //increase the size of the queue
    q->cur_jobid++;
    pthread_cond_signal(&q->job_available);
    pthread_mutex_unlock(&q->mtx);
    return newNode;
}


nodeptr dequeue(queueptr q) {                                                   //remove a job from the front of the queue and modify all qpositions of the rest of the nodes
    pthread_mutex_lock(&q->mtx);
    while (q->size == 0)
        pthread_cond_wait(&q->job_available, &q->mtx);
    nodeptr temp = q->front;
    q->front = q->front->next;
    q->size--;
    pthread_cond_signal(&q->room_available);
    pthread_mutex_unlock(&q->mtx);
    return temp;
}



void freeQueue(queueptr q) {                                                    //free the whole structure of the queue 
    while (!isEmpty(q))
        dequeue(q);
    free(q);
}


nodeptr deleteJobID(queueptr q, int jobID) {                                    //search and delete a job with a specific jobID
    if (isEmpty(q))
        return NULL;
    nodeptr current = q->front;
    nodeptr prev = NULL;
    while (current != NULL && current->jobid != jobID) {                        //traverse the queue to find the node
        prev = current;
        current = current->next;
    }
    if (current == NULL)                                                        //not found
        return NULL;
    if (prev == NULL)                                                           //found in front position so can use deque function
        return dequeue(q);
    else {                                                                      //found somewhere else so remove node and update all the Qpositions of the node on the right
        prev->next = current->next;
    }
    if (current == q->rear)                                                     //if found in the rear position no need to update Qpositions   
        q->rear = prev;
    q->size--;
    return current;
}


void write_queue_to_buffer(queueptr q, char* buffer) {
    // Acquire the mutex lock to ensure thread safety
    pthread_mutex_lock(&q->mtx);
    
    // Pointer to traverse the queue
    nodeptr current = q->front;
    
    // Initialize buffer to an empty string
    buffer[0] = '\0';
    
    // Traverse the queue and append each job to the buffer
    while (current != NULL) {
        char temp[256];
        snprintf(temp, sizeof(temp), "<%d, %s>\n", current->jobid, current->job);
        // Ensure not to overflow the buffer
        if (strlen(buffer) + strlen(temp) + 1 < BUFFER_SIZE) {
            strcat(buffer, temp);
        } else {
            // Handle buffer overflow (optional: you can log an error or break loop)
            break;
        }
        current = current->next;
    }
    
    // Release the mutex lock
    pthread_mutex_unlock(&q->mtx);
}