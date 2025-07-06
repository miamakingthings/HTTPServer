/*********************************************************************************
* Mia T
* queue.c
* Queue ADT
*********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "queue.h"

typedef struct qNodeObj {
    int value;
    struct qNodeObj *next;
    struct qNodeObj *prev;
} qNodeObj;

typedef qNodeObj *qNode;

qNode newqNode(int value) {
    qNode N = malloc(sizeof(qNodeObj));
    N->value = value;
    N->next = NULL;
    N->prev = NULL;
    return (N);
}

void freeqNode(qNode *pN) {
    if (pN != NULL && *pN != NULL) {
        free(*pN);
        *pN = NULL;
    }
    return;
}

typedef struct QueueObj {
    qNode front;
    qNode back;
    int length;
    int capacity;
} QueueObj;

Queue newQueue(int capacity) {
    Queue Q = malloc(sizeof(QueueObj));
    Q->front = NULL;
    Q->back = NULL;
    Q->length = 0;
    Q->capacity = capacity;
    return (Q);
}

void freeQueue(Queue *pQ) {
    if (pQ != NULL && *pQ != NULL) {
        if ((*pQ)->length != 0) {
            qclear(*pQ);
        }
        free(*pQ);
        *pQ = NULL;
    }
    return;
}

int isEmpty(Queue Q) {
    assert(Q);

    return (Q->length == 0);
}

int isFull(Queue Q) {
    assert(Q);

    return (Q->length == Q->capacity);
}

int qfront(Queue Q) {
    assert(Q);

    if (Q == NULL) {
        printf("Queue Error: calling qfront() on NULL Queue reference\n");
        exit(1);
    }

    if (Q->length <= 0) {
        printf("Queue Error: calling qfront() on Queue of length <= 0\n");
        exit(1);
    }

    return (Q->front->value);

    return -1;
}

int qback(Queue Q) {
    assert(Q);

    if (Q == NULL) {
        printf("Queue Error: calling qback() on NULL Queue reference\n");
        exit(1);
    }

    if (Q->length <= 0) {
        printf("Queue Error: calling qback() on Queue of length <= 0\n");
        exit(1);
    }

    return (Q->back->value);

    return -1;
}

void enqueue(Queue Q, int x) {
    assert(Q);

    if (Q == NULL) {
        printf("Queue Error: calling enqueue() on NULL Queue reference\n");
        exit(1);
    }

    if (!isFull(Q)) {
        qNode N = newqNode(x);
        if (Q->length > 0) {
            N->prev = Q->back;
            Q->back->next = N;
            Q->back = N;
        } else {
            Q->front = Q->back = N;
        }
        Q->length++;
    }
    return;
}

int dequeue(Queue Q) {
    assert(Q);

    if (Q == NULL) {
        printf("Queue Error: calling dequeue() on NULL Queue reference\n");
        exit(1);
    }

    if (Q->length <= 0) {
        printf("Queue Error: calling dequeue() on Queue of length <= 0\n");
        exit(1);
    }

    int front = Q->front->value;

    qNode N = NULL;
    N = Q->front;
    if (Q->length > 1) {
        Q->front = Q->front->next;
    } else {
        Q->back = NULL;
    }
    Q->front->prev = NULL;
    Q->length--;
    freeqNode(&N);

    return front;
}

void qclear(Queue Q) {
    assert(Q);

    if (Q == NULL) {
        printf("Queue Error: calling qclear() on NULL Queue reference\n");
        exit(1);
    }

    qNode N = Q->front;
    while (N != NULL) {
        qNode M = N->next;
        freeqNode(&N);
        N = M;
    }
    Q->front = NULL;
    Q->back = NULL;
    Q->length = 0;
    return;
}
