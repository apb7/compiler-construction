// Group Number: 31
// MADHUR PANWAR   2016B4A70933P
// TUSSANK GUPTA   2016B3A70528P
// SALMAAN SHAHID  2016B4A70580P
// APURV BAJAJ     2016B3A70549P
// HASAN NAQVI     2016B5A70452P

#include "generic_stack.h"
#include <stdio.h>
#include <stdlib.h>

void NAME_TYPE(del_val)(TYPE val){
    /* do nothing since int is primitive and no dynamic memory allocation is involved */
}

bool NAME_TYPE(equals)(TYPE a, TYPE b){
    return a == b;
}

void NAME_TYPE(print)(TYPE ptr, FILE* fp){
    /* Do Nothing */
}

NAME_TYPE(stack)* (NAME_TYPE(stack_create))(){
    NAME_TYPE(stack)* stk = (NAME_TYPE(stack)*)malloc(sizeof(NAME_TYPE(stack)));
    NAME_TYPE(stack_init)(stk);
    return stk;
}

TYPE NAME_TYPE(stack_top)(NAME_TYPE(stack) *stk){
    if(stk->size == 0){
    fprintf(stderr,"Stack is empty! Can't return top.\n");
    // something of type TYPE has to be returned. Either we can define a default return value in the TYPE_stack_config.h
    // or we can agree to calling top only if the stack is not empty. The latter is better since having a default
    // return value just for this purpose is a waste of memory. So if at all we enter here, we exit(0);
    fprintf(stderr,"Can't return a value as expected. So, exiting...");
    exit(1);
    }
    return stk->top->val;
}

TYPE NAME_TYPE(stack_pop)(NAME_TYPE(stack)* stk){
        if(stk->size == 0){
            fprintf(stderr,"Stack is empty! Can't pop.\n");
            // something of type TYPE has to be returned. Either we can define a default return value in the TYPE_stack_config.h
            // or we can agree to calling top only if the stack is not empty. The latter is better since having a default
            // return value just for this purpose is a waste of memory. So if at all we enter here, we exit(0);
            fprintf(stderr,"Can't return a value as expected. So, exiting...");
            exit(1);
        }
        else{
            NAME_TYPE(stack_node)* curr_top = stk->top;
            stk->top = curr_top->next;
            TYPE val = curr_top->val;
            free(curr_top); // works because for all stacks, the nodes are malloced
            stk->size -= 1;
            return val;
        }
}

NAME_TYPE(stack_node)* NAME_TYPE(make_stack_node)(TYPE val, NAME_TYPE(stack_node)* next){
    NAME_TYPE(stack_node) * node = (NAME_TYPE(stack_node) *) malloc(sizeof(NAME_TYPE(stack_node)));
    node->val = val;
    node->next = next;
    return node;
}

bool NAME_TYPE(stack_isEmpty)(NAME_TYPE(stack) *stk){
    return (stk->size > 0) ? false : true;
}

void NAME_TYPE(stack_init)(NAME_TYPE(stack) *stk){
    stk->top = NULL;
    stk->size = 0;
}

void NAME_TYPE(stack_del_nodes)(NAME_TYPE(stack) *stk){
    NAME_TYPE(stack_node) *nav, *curr; // the pair of node pointers to navigate the linked list

    nav = stk->top; // nav starts at the top element
    curr = NULL; // curr will follow nav

    while(nav != NULL){ // until we encounter the end of list. NOTE: when nav hits NULL, curr points to last element
        // and that is the last deletion to be done. What 'nav' points to, will be freed in that iteration
        curr = nav;
        nav = nav->next; // 'nav' moves on
        // free all memory allocations of the node pointed to by 'curr'
        curr->next = NULL; // not really needed since we'll free it anyway but just for the sake of sanity
        NAME_TYPE(del_val)(curr->val); // if the 'val' in stack nodes is a non-primitive type, it will have a corresponding
        // malloc allocation too. This call frees that before we free the node container, else we'll have a memory leak.
        // Such functions, namely NAME_TYPE(del_val) are defined in the config files (TYPE_stack_config.h and .c files)
        free(curr); // now that 'val' has been freed (if needed to be), we can free the node container
    }
    NAME_TYPE(stack_init)(stk); // initialize the stack as it contains zero nodes now
}

void NAME_TYPE(stack_del_head)(NAME_TYPE(stack) *stk){
    NAME_TYPE(stack_del_nodes)(stk); // delete all the stack nodes
    free(stk); // free the head
}

void NAME_TYPE(stack_push)(NAME_TYPE(stack)* stk, TYPE val){
    if(stk->size >= NAME_TYPE(STACK_MAX)){
        fprintf(stderr,"Stack has reached size limit! Can't push further.\n");
        exit(1);
    }
    stk->top = NAME_TYPE(make_stack_node)(val, stk->top);
    stk->size += 1;
}

void NAME_TYPE(stack_print)(NAME_TYPE(stack)* stk, FILE* fp){
    NAME_TYPE(stack_node)* nav = stk->top;
    if(nav == NULL){
        fprintf(fp, "Stack is empty! Nothing to print.\n");
        return;
    }
    fprintf(fp, "[");
    // print the first element here and for all further prints, we print a comma before them
    NAME_TYPE(print)(nav->val, fp); // for integer 'val', printing is well defined, but for a generic 'val', we rely on
    // definitions in TYPE_stack_config.h and .c files
    nav = nav->next;
    while(nav != NULL)
    {
        fprintf(fp, ", ");
        NAME_TYPE(print)(nav->val, fp);
        nav = nav->next;
    }
    fprintf(fp, "]\n");
}



