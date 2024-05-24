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
    newQueue->currently_running=0;
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
    while (q->currently_running >= q->concurrency)
        pthread_cond_wait(&q->job_available, &q->mtx);
    nodeptr temp = q->front;
    q->front = q->front->next;
    q->size--;
    q->currently_running++;
    pthread_cond_signal(&q->room_available);
    pthread_mutex_unlock(&q->mtx);
    return temp;
}



void freeQueue(queueptr q) {                                                    //free the whole structure of the queue 
    while (!isEmpty(q))
        dequeue(q);
    free(q);
}


nodeptr deleteJobID(queueptr q, int jobID) {
    // Acquire the mutex lock to ensure thread safety
    pthread_mutex_lock(&q->mtx);
    
    if (isEmpty(q)) {
        // Release the mutex lock before returning
        pthread_mutex_unlock(&q->mtx);
        return NULL;
    }
    
    nodeptr current = q->front;
    nodeptr prev = NULL;
    
    while (current != NULL && current->jobid != jobID) {
        prev = current;
        current = current->next;
    }
    
    if (current == NULL) {
        // Release the mutex lock before returning
        pthread_mutex_unlock(&q->mtx);
        return NULL;
    }
    
    if (prev == NULL) {
        // Release the mutex lock before calling dequeue
        pthread_mutex_unlock(&q->mtx);
        return dequeue(q);
    } else {
        prev->next = current->next;
    }
    
    if (current == q->rear)
        q->rear = prev;
    
    q->size--;
    
    // Release the mutex lock before returning
    pthread_mutex_unlock(&q->mtx);
    
    return current;
}



void write_queue_to_buffer(queueptr q, char* buffer) {
    pthread_mutex_lock(&q->mtx);
    nodeptr current = q->front;
    buffer[0] = '\0';
    while (current != NULL) {
        char temp[256];
        snprintf(temp, sizeof(temp), "<%d, %s>\n", current->jobid, current->job);
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