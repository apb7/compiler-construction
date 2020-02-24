// Group Number: 31
// MADHUR PANWAR   2016B4A70933P
// TUSSANK GUPTA   2016B3A70528P
// SALMAAN SHAHID  2016B4A70580P
// APURV BAJAJ     2016B3A70549P
// HASAN NAQVI     2016B5A70452P

#include "error.h"
#include "errorPtr_stack.h"
#include "stdlib.h"

extern errorPtr_stack *errorStack;
extern char *inverseMappingTable[];
extern char *enum2LexemeTable[];
int STACK_ENABLED = 0;

void initErrorStack(){
    errorStack = errorPtr_stack_create();
    STACK_ENABLED = 1;
}

void destroyErrorStack(){
    errorPtr_stack_del_head(errorStack);
    STACK_ENABLED = 0;
}

void printError(error e){
    switch (e.errType) {
        case LEXICAL:
            fprintf(stderr, "Line %u: LEXICAL ERROR: Invalid token '%s' found.\n", e.lno,
                    e.edata.le.errTk);
            break;
        case SYNTAX_TT:
            fprintf(stderr, "Line %u: SYNTAX ERROR in input: Expected '%s' but Found '%s'\n", e.lno,
                    enum2LexemeTable[e.edata.se.stackTopSymbol], ((e.edata).se.tkinfo)->lexeme);
            break;
        case SYNTAX_NTT:
            fprintf(stderr, "Line %u: SYNTAX ERROR: Stack top, '%s' cannot generate the token '%s' ('%s')\n",
                    e.lno, inverseMappingTable[e.edata.se.stackTopSymbol],
                    inverseMappingTable[((e.edata).se.tkinfo)->type], ((e.edata).se.tkinfo)->lexeme);
            break;
        case STACK_NON_EMPTY:
            fprintf(stderr, "SYNTAX ERROR: Stack Non-empty: End of input source file reached.\n");
            break;
    }
}

void foundNewError(error e){
    if(STACK_ENABLED){
        error *newError = (error *)(malloc(sizeof(error)));
        *newError = e;
        if(errorPtr_stack_isEmpty(errorStack)){
            errorPtr_stack_push(errorStack,newError);
            return;
        }
        else{
            error *topNode = errorPtr_stack_top(errorStack);
            if(e.lno > topNode->lno){
                errorPtr_stack_push(errorStack,newError);
                return;
            }
            else if(e.lno < topNode->lno){
                //this case does not arise
            }
            else{
                if(topNode->errType == LEXICAL){
                    errorPtr_stack_push(errorStack,newError);
                    return;
                }
                else if(e.errType == LEXICAL){
                    errorPtr_stack_pop(errorStack);
                    errorPtr_stack_push(errorStack,newError);
                    errorPtr_stack_push(errorStack,topNode);
                    return;
                }
                else{
                    if(topNode->errType == SYNTAX_NTT && e.errType == SYNTAX_TT){
                        free(errorPtr_stack_pop(errorStack));
                        errorPtr_stack_push(errorStack,newError);
                        return;
                    }
                }
            }
        }
        free(newError);
    }
    else{
           printError(e);
    }
}



void printAllErrors() {
    errorPtr_stack *tmpStack = errorPtr_stack_create();
//    errorPtr_stack_print(errorStack,stdout);
    while (!errorPtr_stack_isEmpty(errorStack)) {
        errorPtr_stack_push(tmpStack, errorPtr_stack_pop(errorStack));
    }
    while (!errorPtr_stack_isEmpty(tmpStack)) {
        error *topNode = errorPtr_stack_pop(tmpStack);
        printError(*topNode);
        if(topNode->errType == LEXICAL)
            free((topNode->edata).le.errTk);
        free(topNode);
    }
    errorPtr_stack_del_head(tmpStack);
}