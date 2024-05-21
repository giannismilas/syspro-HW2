#pragma once
typedef struct node* nodeptr;
typedef struct Q* queueptr;
struct node{
    int jobid;      //unique increasing jobid for each job
    char* job;     //job arguments in a array of strings
    nodeptr next;   //pointer to next node
};

struct Q{
    nodeptr front;  //first job of the queue
    nodeptr rear;   //last job of queue
    int size;       //number of jobs in queue
};



nodeptr createNode(int, char*, int);
queueptr initQueue();
int isEmpty(queueptr);
nodeptr enqueue(queueptr, int, char*,int);
nodeptr dequeue(queueptr);
void freeQueue(queueptr);