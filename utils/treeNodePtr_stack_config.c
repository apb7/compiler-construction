#include <stdbool.h>
#include <stdio.h>
#include "../parser/parserDef.h"
#include "treeNodePtr_stack_config.h"

void treeNodePtr_del_val(treeNode* ptr){
   /**/ // TODO: this is bad 'cause still the memory corresponding to children and tkinfo will leak.
    }

bool treeNodePtr_equals(treeNode* ptr1, treeNode* ptr2){
    return ptr1 == ptr2;
}

void treeNodePtr_print(treeNode* ptr, FILE* fp){
    fprintf(fp, "%d", ptr->tk);
}