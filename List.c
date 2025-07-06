/*********************************************************************************
* Mia T
* List.c
* Linked List ADT
*********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "List.h"

typedef struct NodeObj {
    char *key;
    int value;
    struct NodeObj *next;
    struct NodeObj *prev;
} NodeObj;

typedef NodeObj *Node;

Node newNode(int value, char key[]) {
    Node N = malloc(sizeof(NodeObj));
    N->key = malloc(strlen(key) + 1);
    memcpy(N->key, key, strlen(key) + 1);
    N->value = value;
    N->next = NULL;
    N->prev = NULL;
    return (N);
}

void freeNode(Node *pN) {
    if (pN != NULL && *pN != NULL) {
        free((*pN)->key);
        free(*pN);
        *pN = NULL;
    }
    return;
}

typedef struct ListObj {
    Node front;
    Node back;
    Node cursor;
    int list_index;
    int length;
} ListObj;

List newList(void) {
    List L = malloc(sizeof(ListObj));
    L->front = NULL;
    L->back = NULL;
    L->cursor = NULL;
    L->list_index = -1;
    L->length = 0;
    return (L);
}

void freeList(List *pL) {
    if (pL != NULL && *pL != NULL) {
        if ((*pL)->length != 0) {
            clear(*pL);
        }
        free(*pL);
        *pL = NULL;
    }
    return;
}

int length(List L) {
    assert(L);

    if (L == NULL) {
        printf("List Error: calling length() on NULL List reference\n");
        exit(1);
    }

    return (L->length);
}

int list_index(List L) {
    assert(L);

    if (L == NULL) {
        printf("List Error: calling list_index() on NULL List reference\n");
        exit(1);
    }

    if (L->cursor) {
        return (L->list_index);
    }
    return -1;
}

int front(List L) {
    assert(L);

    if (L == NULL) {
        printf("List Error: calling front() on NULL List reference\n");
        exit(1);
    }

    if (L->length <= 0) {
        printf("List Error: calling front() on List of length <= 0\n");
        exit(1);
    }

    return (L->front->value);

    return -1;
}

int back(List L) {
    assert(L);

    if (L == NULL) {
        printf("List Error: calling back() on NULL List reference\n");
        exit(1);
    }

    if (L->length <= 0) {
        printf("List Error: calling back() on List of length <= 0\n");
        exit(1);
    }

    return (L->back->value);

    return -1;
}

char *get_key(List L) {
    assert(L);

    if (L == NULL) {
        printf("List Error: calling get_key() on NULL List reference\n");
        exit(1);
    }

    if (L->length <= 0) {
        printf("List Error: calling get_key() on List of length <= 0\n");
        exit(1);
    }

    if (L->list_index < 0) {
        printf("List Error: calling get_key() on List with list_index < 0\n");
        exit(1);
    }

    if (L->cursor == NULL) {
        return NULL;
    }

    return (L->cursor->key);
}

int get_val(List L) {
    assert(L);

    if (L == NULL) {
        printf("List Error: calling get_val() on NULL List reference\n");
        exit(1);
    }

    if (L->length <= 0) {
        printf("List Error: calling get_val() on List of length <= 0\n");
        exit(1);
    }

    if (L->list_index < 0) {
        printf("List Error: calling get_val() on List with list_index < 0\n");
        exit(1);
    }

    if (L->cursor == NULL) {
        return -1;
    }

    return (L->cursor->value);
}

int equals(List A, List B) {
    assert(A);
    assert(B);

    int eq = 0;
    Node N = NULL;
    Node M = NULL;

    if (A == NULL || B == NULL) {
        printf("List Error: calling equals() on NULL List reference\n");
        exit(1);
    }

    eq = (A->length == B->length);
    N = A->front;
    M = B->front;
    while (eq && N != NULL) {
        eq = (N->value == M->value && N->key == M->key);
        N = N->next;
        M = M->next;
    }
    return (eq);
}

void clear(List L) {
    assert(L);

    if (L == NULL) {
        printf("List Error: calling clear() on NULL List reference\n");
        exit(1);
    }

    Node N = L->front;
    while (N != NULL) {
        Node M = N->next;
        freeNode(&N);
        N = M;
    }
    L->front = NULL;
    L->back = NULL;
    L->cursor = NULL;
    L->list_index = -1;
    L->length = 0;
    return;
}

void set(List L, int x, char y[]) {
    assert(L);

    if (L == NULL) {
        printf("List Error: calling set() on NULL List reference\n");
        exit(1);
    }

    if (L->length <= 0) {
        printf("List Error: calling set() on List of length <= 0\n");
        exit(1);
    }

    if (L->list_index < 0) {
        printf("List Error: calling set() on List with list_index < 0\n");
        exit(1);
    }

    L->cursor->value = x;
    L->cursor->key = y;

    return;
}

void moveFront(List L) {
    assert(L);

    if (L == NULL) {
        printf("List Error: calling moveFront() on NULL List reference\n");
        exit(1);
    }

    if (L->length != 0) {
        L->cursor = L->front;
        L->list_index = 0;
    }
    return;
}

void moveBack(List L) {
    assert(L);

    if (L == NULL) {
        printf("List Error: calling moveBack() on NULL List reference\n");
        exit(1);
    }

    if (L->length != 0) {
        L->cursor = L->back;
        L->list_index = L->length - 1;
    }
    return;
}

void movePrev(List L) {
    assert(L);

    if (L == NULL) {
        printf("List Error: calling movePrev() on NULL List reference\n");
        exit(1);
    }

    if (L->list_index != -1) {
        L->cursor = L->cursor->prev;
        L->list_index--;
    }
    return;
}

void moveNext(List L) {
    assert(L);

    if (L == NULL) {
        printf("List Error: calling moveNext() on NULL List reference\n");
        exit(1);
    }

    if (L->list_index == -1) {
        return;
    } else if (L->list_index == L->length) {
        L->list_index = -1;
    } else {
        L->cursor = L->cursor->next;
        L->list_index++;
    }
    return;
}

void prepend(List L, int x, char y[]) {
    assert(L);

    if (L == NULL) {
        printf("List Error: calling prepend() on NULL List reference\n");
        exit(1);
    }

    Node N = newNode(x, y);
    if (L->length > 0) {
        N->next = L->front;
        L->front->prev = N;
        L->front = N;
    } else {
        L->front = L->back = N;
    }
    L->length++;
    L->list_index++;
    return;
}

void append(List L, int x, char y[]) {
    assert(L);

    if (L == NULL) {
        printf("List Error: calling append() on NULL List reference\n");
        exit(1);
    }

    Node N = newNode(x, y);
    if (L->length > 0) {
        N->prev = L->back;
        L->back->next = N;
        L->back = N;
    } else {
        L->front = L->back = N;
    }
    L->length++;
    return;
}

void insertBefore(List L, int x, char y[]) {
    assert(L);

    if (L == NULL) {
        printf("List Error: calling insertBefore() on NULL List reference\n");
        exit(1);
    }

    if (L->length <= 0) {
        printf("List Error: calling insertBefore() on List of length <= 0\n");
        exit(1);
    }

    if (L->list_index < 0) {
        printf("List Error: calling insertBefore() on List with list_index < 0\n");
        exit(1);
    }

    if (L->cursor == L->front) {
        prepend(L, x, y);
        return;
    }

    Node N = newNode(x, y);

    N->next = L->cursor;
    N->prev = L->cursor->prev;
    N->prev->next = N;
    L->cursor->prev = N;
    L->list_index++;
    L->length++;

    return;
}

void insertAfter(List L, int x, char y[]) {
    assert(L);

    if (L == NULL) {
        printf("List Error: calling insertAfter() on NULL List reference\n");
        exit(1);
    }

    if (L->length <= 0) {
        printf("List Error: calling insertAfter() on List of length <= 0\n");
        exit(1);
    }

    if (L->list_index < 0) {
        printf("List Error: calling insertAfter() on List with list_index < 0\n");
        exit(1);
    }

    if (L->cursor == L->back) {
        append(L, x, y);
        return;
    }

    Node N = newNode(x, y);

    N->prev = L->cursor;
    N->next = L->cursor->next;
    N->next->prev = N;
    L->cursor->next = N;
    L->length++;

    return;
}

void deleteFront(List L) {
    assert(L);

    if (L == NULL) {
        printf("List Error: calling deleteFront() on NULL List reference\n");
        exit(1);
    }

    if (L->length <= 0) {
        printf("List Error: calling deleteFront() on List of length <= 0\n");
        exit(1);
    }

    if (L->cursor == L->front) {
        L->cursor = NULL;
        L->list_index = -1;
    }

    Node N = NULL;
    N = L->front;
    if (L->length > 1) {
        L->front = L->front->next;
    } else {
        L->back = NULL;
    }
    L->front->prev = NULL;
    L->length--;
    if (L->list_index >= 0) {
        L->list_index--;
    }
    freeNode(&N);

    return;
}

void deleteBack(List L) {
    assert(L);

    if (L == NULL) {
        printf("List Error: calling deleteBack() on NULL List reference\n");
        exit(1);
    }

    if (L->length <= 0) {
        printf("List Error: calling deleteBack() on List of length <= 0\n");
        exit(1);
    }

    if (L->cursor == L->back) {
        L->cursor = NULL;
        L->list_index = -1;
    }

    Node N = NULL;
    N = L->back;
    if (L->length > 1) {
        L->back = L->back->prev;
    } else {
        L->front = NULL;
    }
    L->back->next = NULL;
    L->length--;
    freeNode(&N);

    return;
}

void delete (List L) {
    assert(L);

    if (L == NULL) {
        printf("List Error: calling delete() on NULL List reference\n");
        exit(1);
    }

    if (L->length <= 0) {
        printf("List Error: calling delete() on List of length <= 0\n");
        exit(1);
    }

    if (L->list_index < 0) {
        printf("List Error: calling delete() on List with list_index < 0\n");
        exit(1);
    }

    if (L->cursor == L->front) {
        deleteFront(L);
        return;
    }

    if (L->cursor == L->back) {
        deleteBack(L);
        return;
    }

    L->cursor->prev->next = L->cursor->next;
    L->cursor->next->prev = L->cursor->prev;

    L->cursor = NULL;
    L->list_index = -1;
    L->length--;

    return;
}

void printList(FILE *out, List L) {
    assert(L);

    if (L == NULL) {
        printf("List Error: calling printList() on NULL List reference\n");
        exit(1);
    }

    Node N = NULL;
    for (N = L->front; N != NULL; N = N->next) {
        fprintf(out, "%s %d ", N->key, N->value);
    }
    return;
}

List copyList(List L) {
    assert(L);

    if (L == NULL) {
        printf("List Error: calling copyList() on NULL List reference\n");
        exit(1);
    }

    List copy = newList();
    Node N = NULL;
    for (N = L->front; N != NULL; N = N->next) {
        append(copy, N->value, N->key);
    }
    return (copy);
}

List concatList(List A, List B) {
    assert(A);
    assert(B);

    if (A == NULL && B == NULL) {
        printf("List Error: calling concatList() on NULL List reference\n");
        exit(1);
    }

    List concat = newList();
    Node N = NULL;
    Node M = NULL;
    for (N = A->front; N != NULL; N = N->next) {
        append(concat, N->value, N->key);
    }
    for (M = B->front; M != NULL; M = M->next) {
        append(concat, M->value, N->key);
    }
    return (concat);
}
