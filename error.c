#include "error.h"
#include "utils/errorPtr_stack.h"
#include "utils/errorPtr_stack_config.h"
#include "stdlib.h"

extern errorPtr_stack *errorStack;
extern char *inverseMappingTable[];
extern char *enum2LexemeTable[];

void initErrorStack(){
    errorStack = errorPtr_stack_create();
}

void destroyErrorStack(){
    errorPtr_stack_del_head(errorStack);
}

void foundNewError(error e){
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

void printAllErrors() {
    errorPtr_stack *tmpStack = errorPtr_stack_create();
//    errorPtr_stack_print(errorStack,stdout);
    while (!errorPtr_stack_isEmpty(errorStack)) {
        errorPtr_stack_push(tmpStack, errorPtr_stack_pop(errorStack));
    }
    while (!errorPtr_stack_isEmpty(tmpStack)) {
        error *topNode = errorPtr_stack_pop(tmpStack);
        switch (topNode->errType) {
            case LEXICAL:
                fprintf(stderr, "Line %u: LEXICAL ERROR: Invalid token '%s' found.\n", topNode->lno,
                        topNode->edata.le.errTk);
                break;
            case SYNTAX_TT:
                fprintf(stderr, "Line %u: SYNTAX ERROR in input: Expected '%s' but Found '%s'\n", topNode->lno,
                        enum2LexemeTable[topNode->edata.se.stackTopSymbol], ((topNode->edata).se.tkinfo)->lexeme);
                break;
            case SYNTAX_NTT:
                fprintf(stderr, "Line %u: SYNTAX ERROR: Stack top, '%s' cannot generate the token '%s' ('%s')\n",
                        topNode->lno, inverseMappingTable[topNode->edata.se.stackTopSymbol],
                        inverseMappingTable[((topNode->edata).se.tkinfo)->type], ((topNode->edata).se.tkinfo)->lexeme);
                break;
            case STACK_NON_EMPTY:
                fprintf(stderr, "SYNTAX ERROR: Stack Non-empty: End of input source file reached.\n");
                break;
        }
        free(topNode);
    }
}