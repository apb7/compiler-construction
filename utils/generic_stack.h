#ifndef COMPILER_CONSTRUCTION_GENERIC_STACK_H
#define COMPILER_CONSTRUCTION_GENERIC_STACK_H

#include <stdio.h>
#include <stdbool.h>

typedef struct NAME_TYPE(stack_node){ // any stack node
    TYPE val;
    struct NAME_TYPE(stack_node) *next;
}NAME_TYPE(stack_node);

typedef struct NAME_TYPE(stack){ // head of the stack
    int size;
    NAME_TYPE(stack_node)* top;
}NAME_TYPE(stack);

NAME_TYPE(stack)* (NAME_TYPE(stack_create))();// creates an empty stack and initializes it. returns pointer to its head
TYPE NAME_TYPE(stack_top)(NAME_TYPE(stack) *stk); //returns the top of the stack
TYPE NAME_TYPE(stack_pop)(NAME_TYPE(stack) *stk); //deletes and returns the top of the stack
NAME_TYPE(stack_node)* NAME_TYPE(make_stack_node)(TYPE value, NAME_TYPE(stack_node)* next); // makes (mallocs) a stack node with passed arguments and returns it
bool NAME_TYPE(stack_isEmpty)(NAME_TYPE(stack) *stk); // is the stack empty or not
void NAME_TYPE(stack_init)(NAME_TYPE(stack) *stk); // initialize the stack with 0 'size' and NULL 'next'
void NAME_TYPE(stack_del_nodes)(NAME_TYPE(stack) *stk); //given the top, it deletes all the nodes in the stack and initializes the stack by NAME_TYPE(stack_init)
void NAME_TYPE(stack_del_head)(NAME_TYPE(stack) *stk); // deletes all the nodes ( by a call to NAME_TYPE(stack_del_nodes) ) and then deletes the head of the stack
void NAME_TYPE(stack_push)(NAME_TYPE(stack)* stk, TYPE val); // pushes val on top of the stack
void NAME_TYPE(stack_print)(NAME_TYPE(stack)* stk, FILE* fp); // prints all the nodes' 'val's top to end of stack on the specified stream (viz. stdin, stderr, or in a file)

// All the methods involved with deletions free the memory wherever possible. 'delete' is basically a synonym for free as far as current context is concerned

#endif //COMPILER_CONSTRUCTION_GENERIC_STACK_H
