//
//  KER_list.h
//
//  Created by Emmanuel Mera on 16/03/2023.
//

#ifndef list_h
#define list_h

typedef struct List {
    void *data;
    struct List *next;
} List;

extern List *listCreateCell(void *data);
extern void listDestroyList(List *l, void (*f)(void *));
extern List *listCons(List *head, List *tail);
extern void *listHead(List *l);
extern List *listTail(List *l);
extern void listIter(List *l, void *(*f)(void *));
extern int listLength(List *l);

#endif /* list_h */
