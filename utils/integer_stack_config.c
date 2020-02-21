#include <stdbool.h>
#include <stdio.h>

void int_del_val(int val){
    /* do nothing since int is primitive and no dynamic memory allocation is involved */
}

bool int_equals(int a, int b){
    return a == b;
}

void int_print(int a, FILE* fp){
    fprintf(fp, "%d", a);
}