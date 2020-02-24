// Group Number: 31
// MADHUR PANWAR   2016B4A70933P
// TUSSANK GUPTA   2016B3A70528P
// SALMAAN SHAHID  2016B4A70580P
// APURV BAJAJ     2016B3A70549P
// HASAN NAQVI     2016B5A70452P

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