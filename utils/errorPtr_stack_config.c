#include <stdbool.h>
#include <stdio.h>
#include "../error.h"

void errorPtr_del_val(error *val){
    /* do nothing since int is primitive and no dynamic memory allocation is involved */
}

bool errorPtr_equals(error *a, error *b){
    return a == b;
}

void errorPtr_print(error *ptr, FILE* fp){
    fprintf(fp, "(%d,%d)", ptr->lno,ptr->errType);
}