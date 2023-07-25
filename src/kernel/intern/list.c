//
//  list.c
//
//  Created by Emmanuel Mera on 16/03/2023.
//

#include <stdlib.h>

#include "../KER_list.h"

List *listCreateCell(void *data) {
    List *l = malloc(sizeof(List));
    l->data = data;
    l->next = NULL;
    return l;
}

// USELESS ! see next function (I was very tired when I wrote this piece of code LOL)
void listDestroyCell(List *l) {
    free(l);
}

void listDestroyList(List *l, void (*f)(void *)) {
    if (l) {
        if (f)
            f(l->data);
        listDestroyList(l->next, f);
        listDestroyCell(l);
    }
}

// TODO: stupid !!!!!! I will do it better, don't need any listCreateCell stuff IDIOT
List *listCons(List *head, List *tail) {
    head->next = tail;
    return head;
}

void *listHead(List *l) {
    return l->data;
}

List *listTail(List *l) {
    return l->next;
}

void listIter(List *l, void *(*f)(void *)) {
    if (l) {
        l->data = f(l->data);
        listIter(l->next, f);
    }
}

int listLength(List *l) {
    if (l->next)
        return 1+listLength(l->next);
    return 1;
}
