//
//  KER_stack.h
//  empty
//
//  Created by Emmanuel Mera on 09/06/2023.
//

#ifndef KER_stack_h
#define KER_stack_h

typedef struct STACK_lst {
    void *data;
    struct STACK_lst *next;
} STACK_lst;

typedef struct {
    STACK_lst *stack;
} Stack;

extern Stack *STACK_create(void);
extern int STACK_is_empty(Stack *s);
extern void STACK_push(Stack *s, void *data);
extern void *STACK_pop(Stack *s);

#endif /* KER_stack_h */
