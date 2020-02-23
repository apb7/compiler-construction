// Group Number: 31
// MADHUR PANWAR   2016B4A70933P
// TUSSANK GUPTA   2016B3A70528P
// SALMAAN SHAHID  2016B4A70580P
// APURV BAJAJ     2016B3A70549P
// HASAN NAQVI     2016B5A70452P

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