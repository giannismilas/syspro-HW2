#include <stdio.h>
#include <stdlib.h>
#include "Qimplementation.h"


nodeptr createNode(int jobid, char* job) {    //simple newnode function that allocates space for a new queue node to add job info and returns pointer 
    nodeptr newNode = (nodeptr)malloc(sizeof(struct node));
    if (newNode == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    newNode->jobid = jobid;
    newNode->job = job;
    newNode->next = NULL;
    return newNode;
}


queueptr initQueue() {                                                          //allocates space for the structure that holds info for the queue and initializes pointers to NULL and size to 0
    queueptr newQueue = (queueptr)malloc(sizeof(struct Q));
    if (newQueue == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    newQueue->front = NULL;
    newQueue->rear = NULL;
    newQueue->size = 0;
    return newQueue;
}


int isEmpty(queueptr q) {                                                       //check if queue is Empty
    return (q->front == NULL);
}


nodeptr enqueue(queueptr q, int jobid, char* job) {          //insert a node to the rear of the queue
    nodeptr newNode;
    if (isEmpty(q)) {                                                           //first node so front and rear pointer show to the new node
        newNode = createNode(jobid, job);
        q->front = newNode;
        q->rear = newNode;
    } 
    else {                                                                      //insert to the rear with increased position
        newNode = createNode(jobid, job);
        q->rear->next = newNode;
        q->rear = newNode;
    }
    q->size++;                                                                  //increase the size of the queue
    return newNode;
}


nodeptr dequeue(queueptr q) {                                                   //remove a job from the front of the queue and modify all qpositions of the rest of the nodes
    if (isEmpty(q)) {
        printf("Queue is empty, cannot dequeue\n");
        return NULL;
    }
    nodeptr temp = q->front;
    q->front = q->front->next;
    q->size--;
    return temp;
}



void freeQueue(queueptr q) {                                                    //free the whole structure of the queue 
    while (!isEmpty(q))
        dequeue(q);
    free(q);
}