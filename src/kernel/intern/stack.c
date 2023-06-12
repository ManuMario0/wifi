//
//  stack.c
//  empty
//
//  Created by Emmanuel Mera on 09/06/2023.
//

#include "stack.h"

#include "MEM_alloc.h"

Stack *STACK_create(void) {
    Stack *s = MEM_calloc(sizeof(Stack), __func__);
    return s;
}

int STACK_is_empty(Stack *s) {
    return !s->stack;
}

void STACK_push(Stack *s, void *data) {
    STACK_lst *lst = MEM_calloc(sizeof(STACK_lst), __func__);
    
    lst->data = data;
    lst->next = s->stack;
    s->stack = lst;
}

void *STACK_pop(Stack *s) {
    STACK_lst *lst = s->stack;
    if (lst == NULL)
        return NULL;
    
    s->stack = lst->next;
    void *data = lst->data;
    MEM_free(lst);
    return data;
}
