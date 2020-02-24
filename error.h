// Group Number: 31
// MADHUR PANWAR   2016B4A70933P
// TUSSANK GUPTA   2016B3A70528P
// SALMAAN SHAHID  2016B4A70580P
// APURV BAJAJ     2016B3A70549P
// HASAN NAQVI     2016B5A70452P

#ifndef ERROR_H
#define ERROR_H

#include "parser/parserDef.h"
#include "lexer/lexerDef.h"

typedef enum {
    NO_ERROR, SYNTAX_TT, SYNTAX_NTT, LEXICAL,STACK_NON_EMPTY
} ErrorType;

typedef struct{
    tokenInfo *tkinfo;
    gSymbol stackTopSymbol;
} syntaxError;

typedef struct{
    char *errTk;
} lexicalError;


typedef struct{
    ErrorType errType;
    unsigned int lno;
    union {
        syntaxError se;
        lexicalError le;
    } edata;

} error;

void foundNewError(error e);
void initErrorStack();
void printAllErrors();
void destroyErrorStack();
void printError(error e);
#endif //ERROR_H
