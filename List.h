/*********************************************************************************
* Mia T
* List.h
* LinkedList ADT
*********************************************************************************/

#ifndef __LIST_H__
#define __LIST_H__

#include <stdio.h>

typedef struct ListObj *List;

List newList(void);

void freeList(List *pL);

int length(List L);

int list_index(List L);

int front(List L);

int back(List L);

char *get_key(List L);

int get_val(List L);

int equals(List A, List B);

void clear(List L);

void set(List L, int x, char y[]);

void moveFront(List L);

void moveBack(List L);

void movePrev(List L);

void moveNext(List L);

void prepend(List L, int x, char y[]);

void append(List L, int x, char y[]);

void insertBefore(List L, int x, char y[]);

void insertAfter(List L, int x, char y[]);

void deleteFront(List L);

void deleteBack(List L);

void delete (List L);

void printList(FILE *out, List L);

List copyList(List L);

List concatList(List A, List B);

#endif
