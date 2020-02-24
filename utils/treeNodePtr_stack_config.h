// Group Number: 31
// MADHUR PANWAR   2016B4A70933P
// TUSSANK GUPTA   2016B3A70528P
// SALMAAN SHAHID  2016B4A70580P
// APURV BAJAJ     2016B3A70549P
// HASAN NAQVI     2016B5A70452P

#ifndef COMPILER_CONSTRUCTION_TREENODEPTR_STACK_CONFIG_H
#define COMPILER_CONSTRUCTION_TREENODEPTR_STACK_CONFIG_H


#include <stdbool.h>
#include <stdio.h>

#define treeNodePtr_STACK_MAX 100

void treeNodePtr_del_val(treeNode* ptr); // (does nothing for integer type but) for a struct type this will involve freeing the memory allocated to 'val'
bool treeNodePtr_equals(treeNode* ptr1, treeNode* ptr2); // equality check between two integers. Again, actually useful only when a struct type 'val' is involved
void treeNodePtr_print(treeNode*, FILE* fp); // prints 'a' in file stream fp


#endif //COMPILER_CONSTRUCTION_TREENODEPTR_STACK_CONFIG_H
