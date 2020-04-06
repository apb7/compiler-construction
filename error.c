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
        case E_SEMANTIC:
            //TODO: MAKE SEMANTIC ERRORS DESCRIPTIVE
            fprintf(stderr, "Line %u: SEMANTIC ERROR: ",e.lno);
            switch(e.edata.seme.etype){
                case SEME_UNDECLARED:
                    fprintf(stderr,"'%s' undeclared.\n",(e.edata.seme.errStr1));
                    break;
                case SEME_UNASSIGNED:
                    fprintf(stderr,"'%s' was not assigned any value.\n",e.edata.seme.errStr1);
                    break;
                case SEME_REDUNDANT_DECLARATION:
                    fprintf(stderr,"Found redundant declaration for module '%s'.\n",e.edata.seme.errStr1);
                    break;
                case SEME_REDECLARATION:
                    fprintf(stderr,"'%s' was already declared above.\n",e.edata.seme.errStr1);
                    break;
                case SEME_OUT_OF_BOUNDS:
                    fprintf(stderr,"Array index out of bounds for array '%s'.\n",e.edata.seme.errStr1);
                    break;
                case SEME_REDEFINITION:
                    fprintf(stderr,"Previous definition was here. Found redefinition of the module '%s'.\n",e.edata.seme.errStr1);
                    break;
                case SEME_RECURSION:
                    fprintf(stderr,"The module '%s' cannot call itself.\n",e.edata.seme.errStr1);
                    break;
                case SEME_INVALID_BOUNDS:
                    fprintf(stderr,"The start index of the array '%s' should not be greater than the end index.\n",e.edata.seme.errStr1);
                    break;
                default:
                    fprintf(stderr,"\n");
                    break;
            }
            break;
    }
}

void foundNewError(error e){

    if(STACK_ENABLED){
        error *newError = (error *)(malloc(sizeof(error)));
        *newError = e;
        if(e.errType == E_SEMANTIC){
            //this will only happen if there were no errors of syntax or lexical kind
            errorPtr_stack_push(errorStack,newError);
            return;
        }
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
           if(e.errType == LEXICAL){
               free(e.edata.le.errTk);
           }
           else if(e.errType == SYNTAX_NTT || e.errType == SYNTAX_TT){
               free(e.edata.se.tkinfo);
           }
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