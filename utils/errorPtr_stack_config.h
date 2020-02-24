#ifndef ERROR_STACK_CONFIG_H
#define ERROR_STACK_CONFIG_H

#include <stdbool.h>
#include <stdio.h>

#define errorPtr_STACK_MAX 10000

void errorPtr_del_val(error *val); // (does nothing for integer type but) for a struct type this will involve freeing the memory allocated to 'val'
bool errorPtr_equals(error *a, error *b); // equality check between two integers. Again, actually useful only when a struct type 'val' is involved
void errorPtr_print(error *a, FILE* fp); // prints 'a' in file stream fp

#endif //ERROR_STACK_CONFIG_H