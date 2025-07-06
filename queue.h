/*********************************************************************************
* Mia T
* queue.h
* Queue ADT
*********************************************************************************/

#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <stdio.h>

typedef struct QueueObj *Queue;

Queue newQueue(int capacity);

void freeQueue(Queue *pQ);

int isEmpty(Queue Q);

int isFull(Queue Q);

int qfront(Queue Q);

int qback(Queue Q);

void enqueue(Queue Q, int x);

int dequeue(Queue Q);

void qclear(Queue Q);

#endif
