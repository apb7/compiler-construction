// Stack has a next pointer and a generic val. This val can be of any type (say int, float, struct foo, etc. The only caveat
// is that such a type must exist before you create the stack).
// Include the following wherever you want to use the generic stack: header file where definition of the type of 'val' exists,
// the corresponding TYPE_stack.h file (need to create this and TYPE_stack.c), and the TYPE_stack_config.h file (need to create
// this and the corresponding TYPE_stack_config.c file). All of these files already exist for an stack of integer values.
// This driver serves as a template to demonstrate how you can create your generic stack and use it.

// since integer is supported by C by default, we don't need to include any file where type definition of integer exists
#include <stdio.h> // for i/o operations
#include "utils/integer_stack.h" // TYPE_stack.h
#include "utils/integer_stack_config.h" // TYPE_stack_config.h

// need to check the following:
// response to pushing beyond int_STACK_MAX
// the following functions:
// [WAE == "Works as expected"]
//int_stack *int_stack_create();
//  does it create a stack as expected      -- WAE
//int int_stack_top(int_stack) *stk); //returns the top of the stack
//  1. can you get the top of a non empty stack,        -- WAE
//  2. of an empty stack        -- WAE
//int int_stack_pop(int_stack) *stk); //deletes and returns the top of the stack
//  1. can you pop a non empty stack,       -- WAE
//  2. an empty stack       -- WAE
//int_stack_node* int_make_stack_node(int value, int_stack_node)* next); // makes (mallocs) a stack node with passed arguments and returns it
//  can you make a stack node with given arguments      -- WAE
//bool int_stack_isEmpty(int_stack) *stk); // is the stack empty or not
//  is the status returned by this function as expected     -- WAE
//void int_stack_init(int_stack) *stk); // initialize the stack with 0 'size' and NULL 'next'
//  does it correctly initialize the stack      -- WAE
//void int_stack_del_nodes(int_stack) *stk); //given the top, it deletes all the nodes in the stack and initializes the stack by int_stack_init)
//  does it delete all nodes and free the memory as expected        -- WAE
//void int_stack_del_head(int_stack) *stk); // deletes all the nodes ( by a call to int_stack_del_nodes) ) and then deletes the head of the stack
//  does it delete the head correctly       -- WAE
//void int_stack_push(int_stack)* stk, int val); // pushes val on top of the stack
//  1. can it push as expected      -- WAE
//  2. does it push beyond int_STACK_MAX        -- WAE
//void int_stack_print(int_stack)* stk, FILE* fp);
//  1. does it print a non empty int_stack      -- WAE
//  2. what about printing an empty stack        -- WAE
//  3. does it print to a file      -- WAE

int main(void){
    int n = 5;
    int random_tokens[5] = {12,2,323,52,1};

    int_stack* stk = int_stack_create();
    printf("%d\n",stk->size);
    printf("%p\n",stk->top);

    printf("Is stack empty: %d\n", int_stack_isEmpty(stk));

    int_stack_print(stk, stdout);

//    int a = int_stack_top(stk);
//    printf("%d",a);

//    int a = int_stack_pop(stk);
//    printf("%d",a);

    printf("\n");

    // Pushing...
    for(int i=0; i<n; i++){
        int_stack_push(stk, random_tokens[i]);
        int_stack_print(stk, stdout);
        int p = int_stack_top(stk);
        printf("top: %d\n", p);
        printf("Is stack empty: %d\n", int_stack_isEmpty(stk));
    }

//    printf("Initializing stack...");
//    int_stack_init(stk);
//    printf("Is stack empty: %d\n", int_stack_isEmpty(stk));

    printf("\n");

    // Popping...
//    for(int i=0; i<n-1; i++){
//        int p = int_stack_pop(stk);
//        printf("popped: %d\n", p);
//        int_stack_print(stk, stdout);
//        printf("Is stack empty: %d\n", int_stack_isEmpty(stk));
//    }


    printf("\n");

    int_stack_node* nod = int_make_stack_node(24,stk->top);
    printf("nod_val: %d\n", nod->val);
    printf("nod_next_val: %d\n", nod->next->val);

    int p = int_stack_pop(stk);
    printf("popped: %d\n", p);
    int_stack_print(stk, stdout);
    printf("Is stack empty: %d\n", int_stack_isEmpty(stk));

    printf("Can I access a popped memory: %d\n", nod->next->val);

    int_stack_node* nd = stk->top->next->next;
//    printf("Value before freeing: %d\n", nd->val);

//
//    int_stack_del_nodes(stk);
//    printf("%d\n",stk->size);
//    printf("%p\n",stk->top);

//    printf("Value after freeing: %d", nd->val);

    int_stack_print(stk, stdout);
    int_stack_del_head(stk);
    int_stack_print(stk, stdout);

//    int_stack* b;
//    int_stack_print(stk, stdout);

    int_stack* b = int_stack_create();
    // Pushing...
    for(int i=0; i<99; i++){
        int_stack_push(b, i);
//        int_stack_print(stk, stdout);
        int p = int_stack_top(b);
        printf("top: %d\n", p);
        printf("size: %d\n", b->size);

//        printf("Is stack empty: %d\n", int_stack_isEmpty(stk));
    }
    FILE* fp = fopen("test_print_stack.txt","w");
    int_stack_print(b,fp);
    fclose(fp);

    return 0;
}
