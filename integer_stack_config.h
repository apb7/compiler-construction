#ifndef COMPILER_CONSTRUCTION_INTEGER_STACK_CONFIG_H
#define COMPILER_CONSTRUCTION_INTEGER_STACK_CONFIG_H

#include <stdbool.h>
#include <stdio.h>

#define int_STACK_MAX 100

void int_del_val(int val); // (does nothing for integer type but) for a struct type this will involve freeing the memory allocated to 'val'
bool int_equals(int a, int b); // equality check between two integers. Again, actually useful only when a struct type 'val' is involved
void int_print(int a, FILE* fp); // prints 'a' in file stream fp

#endif //COMPILER_CONSTRUCTION_INTEGER_STACK_CONFIG_H
