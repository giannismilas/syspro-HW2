#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
typedef struct node* nodeptr;
typedef struct Q* queueptr;
struct node{
    int jobid;                      //unique increasing jobid for each job
    char* job;                      //job arguments in a array of strings
    int clientSocket;               //clientsocket of the commander that send the job
    nodeptr next;                   //pointer to next node
};

struct Q{
    nodeptr front;                  //first job of the queue
    nodeptr rear;                   //last job of queue
    pthread_mutex_t mtx;            //mutex to request exclusive access to queue
    pthread_cond_t job_available;   //condition variable indicating a job is available for dequeue
    pthread_cond_t room_available;  //condition variable indicating there is space available for enqueue
    int worker_exit;                //if exit is sent this variable goes to 1 so that workers can exit
    int max_items;                  //maximum size of the queue
    int concurrency;                //concurrency
    int cur_jobid;                  //job id for next enqueue
    int size;                       //current size of queue
    int currently_running;          //number of jobs running
};



nodeptr createNode(int, char*,int);
queueptr initQueue(int);
int isEmpty(queueptr);
nodeptr enqueue(queueptr,char*,int);
nodeptr lock_dequeue(queueptr);
nodeptr dequeue(queueptr q);
void freeQueue(queueptr);
nodeptr deleteJobID(queueptr, int);
void write_queue_to_buffer(queueptr, char*);
void empty_queue_and_inform(queueptr);