#include <stdio.h> // for i/o operations
#include "treeNodePtr_stack.h" // TYPE_stack.h
#include "treeNodePtr_stack_config.h" // TYPE_stack_config.h
#include "treeNodePtr_Def.h"
int main(void){
    treeNodePtr_stack* stk = treeNodePtr_stack_create();
    printf("%d\n",stk->size);
    printf("%p\n",stk->top);
    /* cook your dish here*/
    return 0;
}
