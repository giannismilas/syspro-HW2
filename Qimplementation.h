#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
typedef struct node* nodeptr;
typedef struct Q* queueptr;
struct node{
    int jobid;      //unique increasing jobid for each job
    char* job;     //job arguments in a array of strings
    int clientSocket;
    nodeptr next;   //pointer to next node
};

struct Q{
    nodeptr front;  //first job of the queue
    nodeptr rear;   //last job of queue
    pthread_mutex_t mtx;
    pthread_cond_t job_available; 
    pthread_cond_t room_available; 
    int max_items;
    int concurrency;
    int cur_jobid;
    int size;       
};



nodeptr createNode(int, char*,int);
queueptr initQueue(int);
int isEmpty(queueptr);
nodeptr enqueue(queueptr, int, char*,int);
nodeptr dequeue(queueptr);
void freeQueue(queueptr);
nodeptr deleteJobID(queueptr, int);