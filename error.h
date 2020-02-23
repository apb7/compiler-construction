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

#endif //ERROR_H
