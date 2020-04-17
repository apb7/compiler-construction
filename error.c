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
        case E_TYPE_MISMATCH:
            fprintf(stderr, "Line %u: TYPE ERROR: Types of LHS and RHS do not match.\n", e.lno);
            break;
        case E_EXPRESSION_ERROR:
            fprintf(stderr, "Line %u: TYPE ERROR: Expression evaluates to error type.\n", e.lno);
            break;        
        case E_SEMANTIC:
            //TODO: MAKE SEMANTIC ERRORS DESCRIPTIVE
            fprintf(stderr, "Line %u: SEMANTIC ERROR: ",e.lno);
            switch(e.edata.seme.etype){
                case SEME_UNDECLARED:
                    fprintf(stderr,"'%s' undeclared.\n",(e.edata.seme.errStr1));
                    break;
                case SEME_FUNCTION_NOT_FOUND:
                    fprintf(stderr,"The module '%s' was neither declared nor defined before use.\n",e.edata.seme.errStr1);
                    break;
                case SEME_UNASSIGNED:
                    fprintf(stderr,"'%s' was not assigned any value.\n",e.edata.seme.errStr1);
                    break;
                case SEME_REDUNDANT_DECLARATION:
                    fprintf(stderr,"Found redundant declaration for module '%s'.\n",e.edata.seme.errStr1);
                    break;
                case SEME_REDECLARATION:
                    fprintf(stderr,"'%s' was already declared before.\n",e.edata.seme.errStr1);
                    break;
                case SEME_OUT_OF_BOUNDS:
                    fprintf(stderr,"Array index out of bounds for array '%s'.\n",e.edata.seme.errStr1);
                    break;
                case SEME_REDEFINITION:
                    fprintf(stderr,"Previous definition was at line %s. Found redefinition of the module '%s'.\n",e.edata.seme.errStr2, e.edata.seme.errStr1);
                    break;
                case SEME_RECURSION:
                    fprintf(stderr,"The module '%s' cannot call itself.\n",e.edata.seme.errStr1);
                    break;
                case SEME_INVALID_BOUNDS:
                    fprintf(stderr,"The start index of the array '%s' should not be greater than the end index.\n",e.edata.seme.errStr1);
                    break;
                case SEME_PARAM_PASS_ARR_LBOUND_MISMATCH:
                    fprintf(stderr, "Left array bounds of formal parameter '%s' and actual parameter '%s' do not match.\n",e.edata.seme.errStr1,e.edata.seme.errStr2);
                    break;
                case SEME_PARAM_PASS_ARR_RBOUND_MISMATCH:
                    fprintf(stderr, "Right array bounds of formal parameter '%s' and actual parameter '%s' do not match.\n",e.edata.seme.errStr1,e.edata.seme.errStr2);
                    break;
                case SEME_PARAM_PASS_VAR_BASE_TYPE_MISMATCH:
                    fprintf(stderr,"The primitive types of formal parameter '%s' and actual parameter '%s' do not match.\n",e.edata.seme.errStr1,e.edata.seme.errStr2);
                    break;
                case SEME_PARAM_PASS_ARR_TO_VAR:
                    fprintf(stderr, "The formal parameter '%s' is a primitive type variable while the passed parameter '%s' is an array.\n",e.edata.seme.errStr1,e.edata.seme.errStr2);
                    break;
                case SEME_PARAM_PASS_VAR_TO_ARR:
                    fprintf(stderr, "The formal parameter '%s' is an array while the passed parameter '%s' is a primitive type variable.\n",e.edata.seme.errStr1,e.edata.seme.errStr2);
                    break;
                case SEME_PARAM_PASS_ARR_BASE_TYPE_MISMATCH:
                    fprintf(stderr,"The primitive types of formal parameter array '%s' and actual parameter array '%s' do not match.\n",e.edata.seme.errStr1,e.edata.seme.errStr2);
                    break;
                case SEME_PARAM_RECV_VAR_IN_ARR:
                    fprintf(stderr,"Returned primitive type variable '%s' is assigned to a variable '%s' of array type.\n",e.edata.seme.errStr1,e.edata.seme.errStr2);
                    break;
                case SEME_PARAM_RECV_VAR_BASE_TYPE_MISMATCH:
                    fprintf(stderr,"The primitive types of returned variable '%s' and the variable '%s' that it is assigned to do not match.\n",e.edata.seme.errStr1,e.edata.seme.errStr2);
                    break;
                case SEME_PARAM_RECV_TOO_MANY_RET_VALS_EXPECTED:
                    fprintf(stderr,"Too many return values expected from the module '%s'.\n",e.edata.seme.errStr1);
                    break;
                case SEME_PARAM_RECV_TOO_FEW_RET_VALS_EXPECTED:
                    fprintf(stderr,"Too few return values expected from the module '%s'.\n",e.edata.seme.errStr1);
                    break;
                case SEME_PARAM_PASS_TOO_MANY_ARGS_PASSED:
                    fprintf(stderr,"Too many arguments passed to the module '%s'.\n",e.edata.seme.errStr1);
                    break;
                case SEME_PARAM_PASS_TOO_FEW_ARGS_PASSED:
                    fprintf(stderr,"Too few arguments passed to the module '%s'.\n",e.edata.seme.errStr1);
                    break;
                case SEME_LOOP_VAR_REDEFINED:
                    fprintf(stderr,"The loop variable '%s' is assigned here.\n",e.edata.seme.errStr1);
                    break;
                case SEME_MODULE_REDECLARED:
                    fprintf(stderr,"The module '%s' is redeclared here.\n",e.edata.seme.errStr1);
                    break;
                case SEME_LOOP_VAR_REDECLARED:
                    fprintf(stderr, "The loop variable '%s' is redeclared here.\n", e.edata.seme.errStr1);
                    break;
                case SEME_SWITCH_VAR_UNDECLARED:
                    fprintf(stderr, "The switch statement variable '%s' used here is undeclared.\n", e.edata.seme.errStr1);
                    break;
                case SEME_SWITCH_VAR_TYPE_ARR:
                    fprintf(stderr, "The switch statement variable '%s' is of array type.\n", e.edata.seme.errStr1);
                    break;
                case SEME_SWITCH_VAR_TYPE_INVALID:
                    fprintf(stderr, "The switch statement variable '%s' has invalid primitive type ('integer' or 'boolean' type expected).\n", e.edata.seme.errStr1);
                    break;
                case SEME_DEFAULT_IN_BOOLEAN_SWITCH:
                    fprintf(stderr, "The boolean switch statement cannot have a default.\n");
                    break;
                case SEME_NON_BOOLEAN_IN_SWITCH:
                    fprintf(stderr, "The boolean switch statement cannot have non-boolean case values. Unexpected case value '%s' found.\n", e.edata.seme.errStr1);
                    break;
                case SEME_TOO_MANY_BOOLEAN_CASES_IN_SWITCH:
                    fprintf(stderr, "The boolean switch statement must have exactly one case each for 'true' and 'false'.\n");
                    break;
                case SEME_MISSING_DEFAULT_IN_INTEGER_SWITCH:
                    fprintf(stderr, "The integer switch statement must have a 'default'.\n");
                    break;
                case SEME_NON_INTEGER_IN_SWITCH:
                    fprintf(stderr, "The integer switch statement cannot have non-integer case values. Unexpected case value '%s' found.\n", e.edata.seme.errStr1);
                    break;
                case SEME_FOR_VAR_UNDECLARED:
                    fprintf(stderr, "The for loop counter variable '%s' used here is undeclared.\n", e.edata.seme.errStr1);
                    break;
                case SEME_FOR_VAR_TYPE_ARR:
                    fprintf(stderr, "The for loop counter variable '%s' is of array type.\n", e.edata.seme.errStr1);
                    break;
                case SEME_FOR_VAR_TYPE_INVALID:
                    fprintf(stderr, "The for loop counter variable '%s' has invalid primitive type ('integer' type expected).\n", e.edata.seme.errStr1);
                    break;
                case SEME_RETURN_VALUES_NOT_CAPTURED:
                    fprintf(stderr, "Return value(s) of the module '%s' must be captured while calling it.\n", e.edata.seme.errStr1);
                    break;
                case SEME_ARR_IDX_NOT_INT:
                    fprintf(stderr,"'%s' is not an integer. Array index should be an integer.\n",e.edata.seme.errStr1);
                    break;
                case SEME_MODULE_USED_NOT_DEFINED:
                    fprintf(stderr,"Module '%s' is called but missing definition.\n",e.edata.seme.errStr1);
                    break;
                case SEME_NOT_A_ARRAY:
                    fprintf(stderr,"'%s' is not a array to be indexed.\n",e.edata.seme.errStr1);
                    break;
                case SEME_WHILE_COND_TYPE_MISMATCH:
                    fprintf(stderr,"WHILE loop condition type is not 'boolean'.\n");
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
        if(e.errType == E_SEMANTIC || e.errType == E_TYPE_MISMATCH || e.errType == E_EXPRESSION_ERROR){
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