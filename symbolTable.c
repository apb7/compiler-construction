#include "symbolTable.h"
#include "symbolTableDef.h"
#include "symbolHash.h"
#include <stdio.h>
#include "astDef.h"
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "error.h"
#include "lexerDef.h"
#include "typeCheck.h"

//DONE: when the scope of the function ends, check if all its output parameters have been assigned.
//DONE: Use the error function to make all the errors rather than making them manually e.g. refactor boundsCheckIfStatic(..)
//DONE: check while reading the input list for the first time that its arrays are STAT_ARR (can use getVType).
//DONE: redeclaration of variables in input and output lists should be error. What if an inp list var is redeclared in output list.
    //i will then assume that, that input var is now shadowed by this output var. (therefore changed the order of search everywhere)
//TODO: if dyn arrays allowed in input list : having a dynamic array in input list is no longer an error as long as its indices are pre declared in the same list. perform static checks (base type match and static bounds check)
//TODO: add this at suitable place: printf("Input source code is semantically correct...........\n"); -- do this after code gen when all semantic checks have been performed
//TODO: at least one of the variables involved in boolean expression of WHILE loop condition must be the LHS of an assignment statement inside the loop
//DONE: Complete the function handleUndefinedModules(...) -- subject to change if the following is an error: module was declared, not called and not defined. Currently this is not considered as an error.

symbolTable funcTable;
int nextGlobalOffset;
bool haveSemanticErrors;

void initSymFuncInfo(symFuncInfo *funcInfo, char *funcName) {
    funcInfo->status = F_DECLARED;
    funcInfo->lno = -1;
    funcInfo->st = NULL;
    funcInfo->inpPListHead = NULL;
    funcInfo->outPListHead = NULL;
    funcInfo->pendingCallListHead = NULL;
    (funcName != NULL) ? strcpy(funcInfo->funcName,funcName) :  memset(funcInfo->funcName, 0, sizeof(funcInfo->funcName));
}

void initSymVarInfo(symVarInfo *varInfo) {
    varInfo->isAssigned = false;
    varInfo->lno = -1;
    varInfo->isLoopVar = false;
    varInfo->offset = -1;
}

int getSizeByType(gSymbol gs) {
    switch(gs){
        case g_INTEGER:
            return SIZE_INTEGER;
            break;
        case g_REAL:
            return SIZE_REAL;
            break;
        case g_BOOLEAN:
            return SIZE_BOOLEAN;
            break;
        default: return SIZE_INTEGER;
    }
}

void throwSemanticError(unsigned int lno, char *errStr1, char *errStr2, SemanticErrorType errorType) {
    haveSemanticErrors = true;
    error e;
    e.errType = E_SEMANTIC;
    e.lno = lno;
    (errStr1 != NULL) ? strcpy(e.edata.seme.errStr1, errStr1) : memset(e.edata.seme.errStr1, 0, ERROR_STRING_LENGTH);
    (errStr2 != NULL) ? strcpy(e.edata.seme.errStr2, errStr2) : memset(e.edata.seme.errStr2, 0, ERROR_STRING_LENGTH);
    e.edata.seme.etype = errorType;
    foundNewError(e);
}

void setAssignedOutParam(paramOutNode *outNode) {
    if(outNode == NULL)
        return;
    (outNode->info).var.isAssigned = true;
}

symbolTable *newScope(symbolTable *currST) {
    if(currST == NULL){
        currST = createSymbolTable();
        return currST;
    }
    if(currST->lastChild == NULL){
        currST->headChild = createSymbolTable();
        currST->lastChild = currST->headChild;
    }
    else{
        symbolTable *tail = currST->lastChild;
        tail->next = createSymbolTable();
        currST->lastChild = tail->next;
    }
    currST->lastChild->parent = currST;
    return currST->lastChild;
}

bool matchStaticBounds(symTableNode *passedParam, paramInpNode *inplistNode, unsigned int lno) {
    // match the static bounds between inplistNode and passedParam
    // inplist is assumed to be STAT_ARR type
    // passedParam is assumed to be an array type
    // if passedParam is DYN_ARR, nothing is to be done, else one or both of the static types have to be matched
    if(passedParam == NULL || inplistNode == NULL){
        fprintf(stderr, "matchStaticBounds: Received a NULL Node.\n");
        return false;
    }
    switch(inplistNode->info.var.vtype.vaType){
        case STAT_ARR: {
            unsigned int lb = (inplistNode->info).var.vtype.si.vt_num;
            unsigned int rb = (inplistNode->info).var.vtype.ei.vt_num;
            switch(passedParam->info.var.vtype.vaType){
                case STAT_ARR:
                    if(passedParam->info.var.vtype.si.vt_num != lb){
//                        TODO: throw arrayLeftRangeMismatchError
                        throwSemanticError(lno, inplistNode->lexeme, passedParam->lexeme, SEME_PARAM_PASS_ARR_LBOUND_MISMATCH);
                        return false;
                    }
                    if(passedParam->info.var.vtype.ei.vt_num != rb){
//                        TODO: throw arrayRightRangeMismatchError
                        throwSemanticError(lno, inplistNode->lexeme, passedParam->lexeme, SEME_PARAM_PASS_ARR_RBOUND_MISMATCH);
                       return false;
                    }
                    return true;
                    break;
                case DYN_L_ARR:
                    if(passedParam->info.var.vtype.ei.vt_num != rb){
//                        TODO: throw arrayRightRangeMismatchError
                        throwSemanticError(lno, inplistNode->lexeme, passedParam->lexeme, SEME_PARAM_PASS_ARR_RBOUND_MISMATCH);
                        return false;
                    }
                    return true;
                    break;
                case DYN_R_ARR:
                    if(passedParam->info.var.vtype.si.vt_num != lb){
//                        TODO: throw arrayLeftRangeMismatchError
                        throwSemanticError(lno, inplistNode->lexeme, passedParam->lexeme, SEME_PARAM_PASS_ARR_LBOUND_MISMATCH);
                        return false;
                    }
                    return true;
                    break;
                case DYN_ARR:
                    // can't do anything here
                    return true;
                    break;
                default:
                    fprintf(stderr, "matchStaticBounds: Received a non array type passedParam Node.\n");
                    return false;
            }
            break;
        }
        default:
            fprintf(stderr, "matchStaticBounds: Received a non STAT_ARR type paramInpNode.\n");
            return false;
    }
}

bool matchDataType(symTableNode *passedOrGot, unsigned int lno, symTableNode *plistNode, pListType pt) {
    // plistNode must be of type paramInpNode or paramOutNode
    // cannot get an array returned from a function
    // can pass an array to a function
    // inputPlist can have only static arrays
    /* implications:
     * checks that all nodes of outPlist must be primitive types
     * while dealing with inpPlist nodes, match all available indices of the array with the statically available ones in the input list array
     *
     */
    if(passedOrGot == NULL || plistNode == NULL){
        fprintf(stderr, "matchDataType: Received a NULL Node.\n");
        return false;
    }
    switch(pt)
    {
        case INP_PLIST:
            switch(plistNode->info.var.vtype.vaType)
            {
                case VARIABLE: // inplist has variable
                    switch(passedOrGot->info.var.vtype.vaType)
                    {
                        case VARIABLE: // passed a variable
                            if(passedOrGot->info.var.vtype.baseType != plistNode->info.var.vtype.baseType){
                                throwSemanticError(lno, plistNode->lexeme, passedOrGot->lexeme, SEME_PARAM_PASS_VAR_BASE_TYPE_MISMATCH);
//                                TODO: throw FuncVariableBaseTypeMismatchError
                                return false;
                            }
                            return true;
                            break;
                        default: // passed an array
                            throwSemanticError(lno, plistNode->lexeme, passedOrGot->lexeme, SEME_PARAM_PASS_ARR_TO_VAR);
//                          TODO: throw PasssedArrayToAVariable
                            return false;
                    }
                    break;
                case STAT_ARR:
                    switch(passedOrGot->info.var.vtype.vaType){
                        case VARIABLE:
                            throwSemanticError(lno, plistNode->lexeme, passedOrGot->lexeme, SEME_PARAM_PASS_VAR_TO_ARR);
//                          TODO: throw PasssedVariableToAnArray
                            return false;
                            break;
                        default:
                            // passedOrGot is an array; gotta match static bounds
                            if(plistNode->info.var.vtype.baseType != passedOrGot->info.var.vtype.baseType){
                                throwSemanticError(lno, plistNode->lexeme, passedOrGot->lexeme, SEME_PARAM_PASS_ARR_BASE_TYPE_MISMATCH);
//                                TODO throw FuncVariableBaseTypeMismatchError
                                return false;
                            }
                            if(!matchStaticBounds(passedOrGot, plistNode, lno)){
//                                TODO: throw staticBoundsMismatchError -- thrown in matchStaticBounds(...)
                                return false;
                            }
                            return true;
                    }
                    break;
                default:
                    throwSemanticError(lno, plistNode->lexeme, passedOrGot->lexeme, SEME_UNEXPECTED_DYN_ARR_IN_INP_LIST);
//                TODO: throw UnExpectedDynamicArrayTypeInInputList
                    return false;
            }
            break;
        case OUT_PLIST:
            // match the base type of out list node with the type of node that the value is assigned to; no arrays allowed
            if(!(passedOrGot->info.var.vtype.vaType == VARIABLE && plistNode->info.var.vtype.vaType == VARIABLE)){
                // the second condition in 'if' above is redundant 'cause a valid output list node cannot syntactically be an array type
                // so the falsity of the above means that the variable receiving the value is an array.
//                TODO: throw ReceivedVarInArr
                throwSemanticError(lno, plistNode->lexeme, passedOrGot->lexeme, SEME_PARAM_RECV_VAR_IN_ARR);
            }
            if(passedOrGot->info.var.vtype.baseType != plistNode->info.var.vtype.baseType){
                throwSemanticError(lno, plistNode->lexeme, passedOrGot->lexeme, SEME_PARAM_RECV_VAR_BASE_TYPE_MISMATCH);
//                TODO throw FuncVariableBaseTypeMismatchError
                return false;
            }
            return true;
            break;
        default: return false;
    }

}

ASTNode *getIDFromModuleReuse(ASTNode *moduleReuseNode){
    ASTNode *idOrAssOp = moduleReuseNode->child;
    switch (idOrAssOp->gs){
        case g_ID:
            return idOrAssOp;
        case g_ASSIGNOP:
            return idOrAssOp->child->next;
        default:
            return NULL;
    }
}

void checkModuleSignature(ASTNode *moduleReuseNode, symFuncInfo *funcInfo, symbolTable *currST) {
    if(moduleReuseNode == NULL || funcInfo == NULL || currST == NULL){
        fprintf(stderr, "checkModuleSignature: Received a NULL Node.\n");
        return;
    }
    //TODO: Check whether the type, number and order of input and output variables match. if not report an error

    ASTNode *idNode = getIDFromModuleReuse(moduleReuseNode);
    symFuncInfo *finfo = stGetFuncInfo(idNode->tkinfo->lexeme, &funcTable);
    paramInpNode *currInpListNode = finfo->inpPListHead;
    paramOutNode *currOutListNode = finfo->outPListHead;
    ASTNode *idOrAssOp = moduleReuseNode->child;
    ASTNode *idOrList = NULL;
    ASTNode *currListNode = NULL;
    char *currlexeme;
    symTableNode *currSymNode;


    if(idOrAssOp->gs == g_ASSIGNOP){
        //idOrAssOp is actually ASSIGNOP
        //match LHS list with outPList
        idOrList = idOrAssOp->child;
        // idOrList is actually idList
        currListNode = idOrList->child;
        do{
            // match LHS list i.e. currListNode's type with currOutListNode
            currSymNode = checkIDInScopesAndLists(currListNode, funcInfo, currST, true);
            // currSymNode is assured to be non-NULL since we check this in handleModuleReuse

            if(currOutListNode == NULL){
                throwSemanticError(currListNode->tkinfo->lno, finfo->funcName, NULL, SEME_PARAM_RECV_TOO_MANY_RET_VALS_EXPECTED);
//                TODO: ERROR: too many return values expected from function
                return;
            }
            //match the type of nodes
            if(!matchDataType(currSymNode, currListNode->tkinfo->lno, currOutListNode, OUT_PLIST)){
                return;

            }

            currOutListNode = currOutListNode->next;
            currListNode = currListNode->next;
        }while(currListNode != NULL);
        if(currOutListNode != NULL){
            throwSemanticError(idOrList->next->tkinfo->lno, finfo->funcName, NULL, SEME_PARAM_RECV_TOO_FEW_RET_VALS_EXPECTED);
//            TODO: ERROR; too few return values expected from the function
            return;
        }
        idOrList = idOrList->next; // now points to function name AST node i.e. ID after idOrList
    }
    if(idOrAssOp->gs == g_ID){
        // idOrAssOp is actually ID
        // if function returns values but they are not received then it is an error
        // we are here means, we haven't captured any values
        // now we must check if they were returned or not
        // Note that this is not necessary to be checked in case of input list, because syntactically it is enforced that the parameter list
        // passed while calling a function cannot be empty
        if(currOutListNode != NULL){
            throwSemanticError(idOrAssOp->tkinfo->lno, idOrAssOp->tkinfo->lexeme, NULL, SEME_RETURN_VALUES_NOT_CAPTURED);
            return;
        }
        idOrList = idOrAssOp; // now points to function name AST node i.e. ID after idOrList
    }
    //skip module name and go to RHS list
    idOrList = idOrList->next; // points to RHS List

    //match RHS List with InpPlist
    currListNode = idOrList->child;
    do{
        // match RHS list i.e. currListNode's type with currInpListNode
        currSymNode = checkIDInScopesAndLists(currListNode, funcInfo, currST, false);
        // currSymNode is assured to be non-NULL since we check this in handleModuleReuse

        if(currInpListNode == NULL){
            throwSemanticError(currListNode->tkinfo->lno, finfo->funcName, NULL, SEME_PARAM_PASS_TOO_MANY_ARGS_PASSED);
//                TODO: ERROR: too many arguments passed to the function
            return;
        }
        //match the type of nodes
        if(!matchDataType(currSymNode, currListNode->tkinfo->lno, currInpListNode, INP_PLIST)){
            return;
        }
        currInpListNode = currInpListNode->next;
        currListNode = currListNode->next;
    }while(currListNode != NULL);
    if(currInpListNode != NULL){
        throwSemanticError(idOrList->parent->tkinfo->lno, finfo->funcName, NULL, SEME_PARAM_PASS_TOO_FEW_ARGS_PASSED);
//            TODO: ERROR; too few arguments passed to the function
        return;
    }
}

void initVarType(varType *vt) {
    // these are just randomly chosen values since we don't have a default and ned to initialise it with something
    vt->baseType = g_INTEGER;
    vt->vaType = VARIABLE;
    vt->si.vt_id = NULL;
    vt->ei.vt_id = NULL;
    vt->bytes = SIZE_INTEGER;
}

varType getVtype(ASTNode *typeOrDataTypeNode, symFuncInfo *funcInfo, symbolTable *currST) {
    varType vt;
    initVarType(&vt);

    if(typeOrDataTypeNode == NULL){
        fprintf(stderr, "getVType: Received a NULL Node.\n");
        return vt;
    }
    //TODO: Construct the varType struct and return it
    switch(typeOrDataTypeNode->gs)
    {
        case g_INTEGER:
        case g_BOOLEAN:
        case g_REAL:
        {
            vt.baseType = typeOrDataTypeNode->gs;
            vt.bytes = getSizeByType(vt.baseType);
            vt.vaType = VARIABLE;
            vt.si.vt_id = NULL;
            vt.ei.vt_id = NULL;
        }
            break;
        case g_dataType:
        {   // actually a 'dataType' node
            ASTNode *rangeArrOrBaseType = typeOrDataTypeNode->child;
            ASTNode *baseTypeOrNull = rangeArrOrBaseType->next;

            if (baseTypeOrNull == NULL)
            {
                // we're dealing with a variable
                vt.baseType = rangeArrOrBaseType->gs;
                vt.bytes = getSizeByType(vt.baseType);
                vt.vaType = VARIABLE;
                vt.si.vt_id = NULL;
                vt.ei.vt_id = NULL;
            }
            else
            {
                // we're dealing with an array
                // for a dynamic array, vt.bytes will store the size occupied by single element of its base type
                vt.baseType = baseTypeOrNull->gs;
                vt.bytes = getSizeByType(vt.baseType); // size occupied by single element
                ASTNode *numOrId = rangeArrOrBaseType->child;
                switch(numOrId->gs)
                {   // check the left bound
                    case g_NUM:
                    {
                        unsigned int lb = numOrId->tkinfo->value.num;
                        switch(numOrId->next->gs)
                        {   // check the right bound
                            case g_NUM:{
                                unsigned int rb = numOrId->next->tkinfo->value.num;
                                vt.vaType = STAT_ARR;
                                vt.si.vt_num = lb;
                                vt.ei.vt_num = rb;
                                vt.bytes = vt.bytes*(rb - lb + 1);
                                break;
                            }
                            case g_ID:
                                vt.vaType = DYN_R_ARR;
                                vt.si.vt_num = lb;
                                vt.ei.vt_id = checkIDInScopesAndLists(numOrId->next, funcInfo, currST, false);
                                if(vt.ei.vt_id == NULL)
                                    throwSemanticError(numOrId->next->tkinfo->lno,numOrId->next->tkinfo->lexeme,NULL,SEME_UNDECLARED);
                                else if(vt.ei.vt_id->info.var.vtype.baseType != g_INTEGER)
                                    throwSemanticError(numOrId->next->tkinfo->lno,numOrId->next->tkinfo->lexeme,NULL,SEME_ARR_IDX_NOT_INT);
                                else
                                    numOrId->next->stNode = vt.ei.vt_id;
                                // can't statically get 'bytes' and 'ei.vt_num' (as NUM) fields
                                break;
                            default:
                                fprintf(stderr, "getVType: Unexpected ASTNode found representing right bound of array.\n");
                        }

                        break;
                    }
                    case g_ID:
                        switch(numOrId->next->gs)
                        {   // check the right bound
                            case g_NUM: {
                                unsigned int rb = numOrId->next->tkinfo->value.num;
                                vt.vaType = DYN_L_ARR;
                                vt.si.vt_id = checkIDInScopesAndLists(numOrId, funcInfo, currST, false);
                                if(vt.si.vt_id == NULL)
                                    throwSemanticError(numOrId->tkinfo->lno,numOrId->tkinfo->lexeme,NULL,SEME_UNDECLARED);
                                else if(vt.si.vt_id->info.var.vtype.baseType != g_INTEGER)
                                    throwSemanticError(numOrId->tkinfo->lno,numOrId->tkinfo->lexeme,NULL,SEME_ARR_IDX_NOT_INT);
                                else
                                    numOrId->stNode = vt.si.vt_id;
                                vt.ei.vt_num = rb;
                                // can't statically get 'bytes' and 'si.vt_num' fields
                                break;
                            }
                            case g_ID:
                                vt.vaType = DYN_ARR;
                                vt.si.vt_id = checkIDInScopesAndLists(numOrId, funcInfo, currST, false);
                                if(vt.si.vt_id == NULL)
                                    throwSemanticError(numOrId->tkinfo->lno,numOrId->tkinfo->lexeme,NULL,SEME_UNDECLARED);
                                else if(vt.si.vt_id->info.var.vtype.baseType != g_INTEGER)
                                    throwSemanticError(numOrId->tkinfo->lno,numOrId->tkinfo->lexeme,NULL,SEME_ARR_IDX_NOT_INT);
                                else
                                    numOrId->stNode = vt.si.vt_id;
                                vt.ei.vt_id = checkIDInScopesAndLists(numOrId->next, funcInfo, currST, false);
                                if(vt.ei.vt_id == NULL)
                                    throwSemanticError(numOrId->next->tkinfo->lno,numOrId->next->tkinfo->lexeme,NULL,SEME_UNDECLARED);
                                else if(vt.ei.vt_id->info.var.vtype.baseType != g_INTEGER)
                                    throwSemanticError(numOrId->next->tkinfo->lno,numOrId->next->tkinfo->lexeme,NULL,SEME_ARR_IDX_NOT_INT);
                                else
                                    numOrId->next->stNode = vt.ei.vt_id;
                                // can't statically get 'bytes', 'si.vt_num' and 'ei.vt_num' fields
                                break;
                            default:
                                fprintf(stderr, "getVType: Unexpected ASTNode found representing right bound of array.\n");
                        }
                        break;
                    default:
                        fprintf(stderr, "getVType: Unexpected ASTNode found representing left bound of array.\n");
                }
            }
            break;
        }
        default:
            // can't handle this node
            fprintf(stderr, "getVType: Invoked on invalid node.\n");
    }
    return vt;
}

paramInpNode *inpListSearchID(ASTNode *idNode, symFuncInfo *funcInfo){
    //search for this ID in the input list and return pointer to the node if found else return NULL
    if(idNode == NULL || funcInfo == NULL || idNode->tkinfo == NULL)
        return NULL;
    char *lexeme = idNode->tkinfo->lexeme;
    paramInpNode *ptr = funcInfo->inpPListHead;
    while(ptr != NULL){
        if(equals(ptr->lexeme,lexeme))
            return ptr;
        ptr = ptr->next;
    }
    return NULL;
}

paramOutNode *outListSearchID(ASTNode *idNode, symFuncInfo *funcInfo){
    //search for this ID in the output list and return pointer to the node if found else return NULL
    if(idNode == NULL || funcInfo == NULL || idNode->tkinfo == NULL)
        return NULL;
    char *lexeme = idNode->tkinfo->lexeme;
    paramOutNode *ptr = funcInfo->outPListHead;
    while(ptr != NULL){
        if(equals(ptr->lexeme,lexeme))
            return ptr;
        ptr = ptr->next;
    }
    return NULL;
}

paramInpNode *createParamInpNode(ASTNode *idNode, ASTNode *dataTypeNode, symFuncInfo *funcInfo){
    if(idNode == NULL || dataTypeNode == NULL){
        fprintf(stderr,"createParamInpNode: NULL error.\n");
        return NULL;
    }
    paramInpNode *prevDeclaration = inpListSearchID(idNode,funcInfo);
    if(prevDeclaration != NULL){
        throwSemanticError(idNode->tkinfo->lno,idNode->tkinfo->lexeme,NULL,SEME_REDECLARATION);
        return NULL;
    }
    else{
        paramInpNode *ptr = (paramInpNode *) (malloc(sizeof(paramInpNode)));
        initSymVarInfo(&((ptr->info).var));
        if(idNode->tkinfo)
            (ptr->info).var.lno = idNode->tkinfo->lno;
        strcpy(ptr->lexeme,idNode->tkinfo->lexeme);
        (ptr->info).var.vtype = getVtype(dataTypeNode, funcInfo, NULL);
        //TODO: check Offset computation
        (ptr->info).var.offset = nextGlobalOffset;
        nextGlobalOffset += (ptr->info).var.vtype.bytes;
        ptr->next = NULL;
        idNode->stNode = ptr;
        return ptr;
    }
}

paramInpNode *createParamInpList(ASTNode *inputPlistNode) {
    if(inputPlistNode == NULL){
        return NULL;
    }
    ASTNode *curr = inputPlistNode->child;
    paramInpNode *head = NULL;
    paramInpNode *currInp = NULL;
    symFuncInfo dummy_finfo;
    initSymFuncInfo(&dummy_finfo,"@dummy");
    dummy_finfo.inpPListHead = head;
    while(curr != NULL){
        if(head == NULL){
            head = createParamInpNode(curr,curr->next,&dummy_finfo);
            currInp = head;
            dummy_finfo.inpPListHead = head;
        }
        else{
            currInp->next = createParamInpNode(curr,curr->next,&dummy_finfo);
            if(currInp->next != NULL)   //so that we only include non NULL nodes
                currInp = currInp->next;
        }
        if(curr->next)
            curr = curr->next->next;
        else
            curr = NULL;
    }
    return head;
}

paramOutNode *createParamOutNode(ASTNode *idNode, ASTNode *dataTypeNode, symFuncInfo *funcInfo){
    if(idNode == NULL || dataTypeNode == NULL){
        fprintf(stderr,"createParamOutNode: NULL error.\n");
        return NULL;
    }
    paramOutNode *prevDeclaration = outListSearchID(idNode,funcInfo);
    if(prevDeclaration != NULL){
        throwSemanticError(idNode->tkinfo->lno,idNode->tkinfo->lexeme,NULL,SEME_REDECLARATION);
        return NULL;
    }
    else{
        paramOutNode *ptr = (paramOutNode *) (malloc(sizeof(paramOutNode)));
        initSymVarInfo(&((ptr->info).var));
        if(idNode->tkinfo)
            (ptr->info).var.lno = idNode->tkinfo->lno;
        strcpy(ptr->lexeme,idNode->tkinfo->lexeme);
        (ptr->info).var.vtype = getVtype(dataTypeNode,NULL, NULL);
        (ptr->info).var.isAssigned = false;
        //TODO: check Offset computation
        (ptr->info).var.offset = nextGlobalOffset;
        nextGlobalOffset += (ptr->info).var.vtype.bytes;
        ptr->next = NULL;
        idNode->stNode = ptr;
        return ptr;
    }
}

paramOutNode *createParamOutList(ASTNode *outputPlistNode){
    if(outputPlistNode == NULL){
        return NULL;
    }
    ASTNode *curr = outputPlistNode->child;
    paramOutNode *head = NULL;
    paramOutNode *currOut = NULL;
    symFuncInfo dummy_finfo;
    initSymFuncInfo(&dummy_finfo,"@dummy");
    dummy_finfo.outPListHead = head;
    while(curr != NULL){
        if(head == NULL){
            head = createParamOutNode(curr, curr->next,&dummy_finfo);
            currOut = head;
            dummy_finfo.outPListHead = head;
        }
        else{
            currOut->next = createParamOutNode(curr,curr->next,&dummy_finfo);
            if(currOut->next != NULL)
                currOut = currOut->next;
        }
        if(curr->next)
            curr = curr->next->next;
        else
            curr = NULL;
    }
    return head;
}

symTableNode* findType(ASTNode* node, symbolTable* currST, symFuncInfo* funcInfo, int* isVar, gSymbol* ty) {
    symTableNode* varNode = stSearch(node->tkinfo->lexeme,currST);;
    if(varNode == NULL)
        varNode = outListSearchID(node,funcInfo);
    if(varNode == NULL)
        varNode = inpListSearchID(node,funcInfo);
    if(varNode!=NULL) {
        *ty = varNode->info.var.vtype.baseType;
        if(varNode->info.var.vtype.vaType==VARIABLE)
            *isVar=1;
        else
            *isVar = 0;
    }
    return varNode;
}

bool assignIDinScope(ASTNode *idNode, symFuncInfo *funcInfo, symbolTable *currST){
    if((idNode->stNode = stSearch(idNode->tkinfo->lexeme,currST)) == NULL){
        //not found in any of the symbol tables
        paramOutNode *tmp;
        if((tmp = outListSearchID(idNode,funcInfo)) == NULL){
            //not found in the output list as well
            if((idNode->stNode = inpListSearchID(idNode,funcInfo)) == NULL){
                //not found anywhere
                throwSemanticError(idNode->tkinfo->lno, idNode->tkinfo->lexeme, NULL, SEME_UNDECLARED);
                return false;
            }
            //finally found in input parameters list
        }
        else{
            idNode->stNode = tmp;
            setAssignedOutParam(tmp);
        }
    }
    gSymbol ty; int isVar = 0;
    symTableNode* varNode = findType(idNode,currST,funcInfo,&isVar,&ty);
    if(varNode->info.var.isLoopVar) {
        throwSemanticError(idNode->tkinfo->lno, idNode->tkinfo->lexeme, NULL, SEME_LOOP_VAR_REDEFINED);
        // TODO: ERROR loop var redefined
        return false;
    }
    return true;
}

bool useIDinScope(ASTNode *idNode, symFuncInfo *funcInfo, symbolTable *currST){
    if((idNode->stNode = stSearch(idNode->tkinfo->lexeme,currST)) == NULL){
        //not found in any of the symbol tables
        if(((idNode->stNode = outListSearchID(idNode,funcInfo))) == NULL){
            //not found in the output list as well
            if((idNode->stNode = inpListSearchID(idNode,funcInfo)) == NULL){
                //not found anywhere
                throwSemanticError(idNode->tkinfo->lno, idNode->tkinfo->lexeme, NULL, SEME_UNDECLARED);
                return false;
            }
            //finally found in input parameters list
        }
    }
    return true;
}

//handles single module declaration
void handleModuleDeclaration(ASTNode *moduleIDNode){
    if(moduleIDNode == NULL){
        fprintf(stderr,"handleModuleDeclaration: Empty node received.\n");
        return;
    }
    symFuncInfo *finfo = stGetFuncInfo(moduleIDNode->tkinfo->lexeme,&(funcTable));
    if(finfo == NULL){
        union funcVar fv;
        initSymFuncInfo(&(fv.func),moduleIDNode->tkinfo->lexeme);
        fv.func.lno = moduleIDNode->tkinfo->lno;
        union funcVar *funcEntry = stAdd(moduleIDNode->tkinfo->lexeme, fv, &funcTable);
        moduleIDNode->stNode = stSearchCurrent(moduleIDNode->tkinfo->lexeme,&funcTable);
    }
    else{
        // ERROR: Redeclaration Error
        if(finfo->status == F_DECLARED){
            // no need to check this if condition but just for consistency
            throwSemanticError(moduleIDNode->tkinfo->lno, moduleIDNode->tkinfo->lexeme, NULL, SEME_MODULE_REDECLARED);
//            TOCHECK: throw Redeclaration error
        }
    }
}

symTableNode *checkIDInScopesAndLists(ASTNode *idNode, symFuncInfo *funcInfo, symbolTable *currST, bool assign){
    symTableNode* st = NULL;
    st = stSearch(idNode->tkinfo->lexeme,currST);
    if(st == NULL) {
        //not found in any of the symbol tables
        st = outListSearchID(idNode, funcInfo);
        if (st == NULL) {
            //not found in the input list as well
            st = inpListSearchID(idNode, funcInfo);
            if (st == NULL) {
                //not found anywhere
                return NULL;
            }
        }
        else {
            if(assign) {
                //finally found in output parameters list
                setAssignedOutParam(st);
            }
        }
    }
    return st;
}

void handleIOStmt(ASTNode *ioStmtNode, symFuncInfo *funcInfo, symbolTable *currST){
    if(ioStmtNode == NULL){
        fprintf(stderr,"handleIOStmt: Error, ioStmtNode found NULL.\n");
        return;
    }
    ASTNode *opNode = ioStmtNode->child;
    switch(opNode->gs){
        case g_GET_VALUE:{
            ASTNode *idNode = opNode->next;
            assignIDinScope(idNode,funcInfo,currST);
        }
        break;
        case g_PRINT: {
            //TODO: Handle this section
//                PRINT TRUE
//                PRINT FALSE
//                PRINT var_id_num->ID // print(k); ID can be array also i.e. print(arr);
//                PRINT var_id_num->ID, NUM // print(a[5]);
//                PRINT var_id_num->ID, ID // print(a[b]);
//                PRINT var_id_num->NUM // print(4);
//                PRINT var_id_num->RNUM // print(2.58E-25);

            ASTNode *varOrBoolConst = opNode->next;
            if (varOrBoolConst->child != NULL) {
//                PRINT TRUE -- nothing to handle
//                PRINT FALSE -- nothing to handle
                // varOrBoolConst is actually var_id_num
                ASTNode *idOrConst = varOrBoolConst->child;
                if(idOrConst->gs == g_ID) {
//                PRINT var_id_num->NUM -- nothing to handle
//                PRINT var_id_num->RNUM -- nothing to handle
//                    PRINT var_id_num->ID
                    // idOrConst is actually ID
                    //adding symTableNode pointer in ASTNode
                    if ((idOrConst->stNode = checkIDInScopesAndLists(idOrConst, funcInfo, currST, false)) == NULL) {
//                      TODO: will this checkIDInScopesAndLists check suffice if ID is array; handle array index bound checking
                        throwSemanticError(idOrConst->tkinfo->lno, idOrConst->tkinfo->lexeme, NULL, SEME_UNDECLARED);
                        return;
                    }
                    if(idOrConst->next != NULL){

                        // if idOrConst->next is NUM, the following will do static bound checking
                        boundsCheckIfStatic(idOrConst, idOrConst->next, funcInfo, currST);
//                        PRINT var_id_num->ID, NUM -- handled upto last line
                        idOrConst = idOrConst->next;
                        if(idOrConst->gs == g_ID){
//                      PRINT var_id_num->ID, ID
                            if((idOrConst->stNode = checkIDInScopesAndLists(idOrConst, funcInfo, currST, false)) == NULL){
//                            TODO: dynamic bounds check on array in previous if and index given by this ID
                                throwSemanticError(idOrConst->tkinfo->lno, idOrConst->tkinfo->lexeme, NULL, SEME_UNDECLARED);
                                return;
                            }
                        }

                    }
                }
            }
        }
        break;
    }
}

void handleModuleReuse(ASTNode *moduleReuseNode, symFuncInfo *funcInfo, symbolTable *currST){
    if(moduleReuseNode == NULL || moduleReuseNode->child == NULL){
        fprintf(stderr,"handleModuleReuse: NULL node found.\n");
        return;
    }
    ASTNode *idListNode1 = NULL;
    ASTNode *moduleIdNode = NULL;
    ASTNode *idListNode2 = NULL;
    if(moduleReuseNode->child->gs == g_ASSIGNOP){
        idListNode1 = moduleReuseNode->child->child;
        moduleIdNode = idListNode1->next;
        idListNode2 = moduleIdNode->next;
    }
    else{
        moduleIdNode = moduleReuseNode->child;
        idListNode2 = moduleIdNode->next;
    }
    //adding symTableNode pointer in ASTNode
    moduleIdNode->stNode = stSearchCurrent(moduleIdNode->tkinfo->lexeme,&funcTable);
    symFuncInfo *finfo;
    if(moduleIdNode->stNode != NULL)
        finfo = &(moduleIdNode->stNode->info.func);
    else
        finfo = NULL;

    if(finfo == NULL){
        //No such function
        throwSemanticError(moduleIdNode->tkinfo->lno, moduleIdNode->tkinfo->lexeme, NULL, SEME_FUNCTION_NOT_FOUND);
        return;
    }
    else if(finfo->status == F_DEFINED) {
        if (equals(funcInfo->funcName, finfo->funcName)) {
            //report recursion error
            throwSemanticError(moduleIdNode->tkinfo->lno, moduleIdNode->tkinfo->lexeme, NULL, SEME_RECURSION);
            return;
        }
    }
    bool error_free = true;
    if(idListNode1){
        ASTNode *idNode = idListNode1->child;
        while(idNode != NULL){
            error_free = assignIDinScope(idNode, funcInfo, currST) ? error_free : false;
            idNode = idNode->next;
        }
    }
    if(idListNode2){
        ASTNode *idNode = idListNode2->child;
        while(idNode != NULL){
            error_free = useIDinScope(idNode, funcInfo, currST) ? error_free : false;
            idNode = idNode->next;
        }
    }
    if(!error_free)
        return; //discard this call
    if(finfo->status == F_DECLARED){
        finfo->status = F_DECLARATION_VALID;
        finfo->pendingCallListHead = (ASTNodeListNode *) malloc(sizeof(ASTNodeListNode));
        finfo->pendingCallListHead->next = NULL;
        finfo->pendingCallListHead->astNode = moduleReuseNode;
        finfo->pendingCallListHead->currST = currST;
        finfo->pendingCallListHead->callerFuncInfo = funcInfo;
    }
    else if(finfo->status == F_DECLARATION_VALID){
        ASTNodeListNode *anode = (ASTNodeListNode *) malloc(sizeof(ASTNodeListNode));
        anode->next = finfo->pendingCallListHead;
        anode->astNode = moduleReuseNode;
        anode->currST = currST;
        anode->callerFuncInfo = funcInfo;
        finfo->pendingCallListHead = anode;
    }
    if(finfo->status == F_DEFINED){
        checkModuleSignature(moduleReuseNode,funcInfo,currST);
    }
}

void boundsCheckIfStatic(ASTNode *idNode, ASTNode *idOrNumNode, symFuncInfo *funcInfo, symbolTable *currST){
    symTableNode *arrinfoEntry = checkIDInScopesAndLists(idNode,funcInfo,currST,false);
    //adding symTableNode pointer in ASTNode
    idNode->stNode = arrinfoEntry;
    symVarInfo *arrinfo = NULL;
    if(arrinfoEntry != NULL)
        arrinfo = &(arrinfoEntry->info.var);
    else{
        throwSemanticError(idNode->tkinfo->lno,idNode->tkinfo->lexeme,NULL,SEME_UNDECLARED);
        return;
    }
    if((arrinfo->vtype).vaType == STAT_ARR && idOrNumNode->gs == g_NUM){
        int idx = (idOrNumNode->tkinfo->value).num;
        if(!((idx >= (arrinfo->vtype).si.vt_num) && (idx <= (arrinfo->vtype).ei.vt_num))){
            //out of bounds
            throwSemanticError(idNode->tkinfo->lno, idNode->tkinfo->lexeme, NULL,  SEME_OUT_OF_BOUNDS);
        }
    }
    else if(idOrNumNode->gs == g_ID){
        //do type checking at compile time for array index when it is ID
        symTableNode *stn = checkIDInScopesAndLists(idOrNumNode,funcInfo,currST,false);
        //adding symTableNode pointer in ASTNode
        idOrNumNode->stNode = stn;
        if(stn == NULL){
            throwSemanticError(idOrNumNode->tkinfo->lno,idOrNumNode->tkinfo->lexeme,NULL,SEME_UNDECLARED);
        }
        else if(stn->info.var.vtype.baseType != g_INTEGER){
            throwSemanticError(idOrNumNode->tkinfo->lno,idOrNumNode->tkinfo->lexeme,NULL,SEME_ARR_IDX_NOT_INT);
        }
    }
}

void handleExpression(ASTNode *someNode, symFuncInfo *funcInfo, symbolTable *currST){
    if(someNode == NULL){
        return;
    }

    if(someNode->gs == g_var_id_num){
        if(someNode->child->gs == g_ID){
            ASTNode *idNode = someNode->child;
            useIDinScope(idNode, funcInfo, currST);

            if(idNode->next != NULL){
                //array
                boundsCheckIfStatic(idNode, idNode->next, funcInfo, currST);
            }
        }
        handleExpression(someNode->next,funcInfo,currST);
    }
    else{
        handleExpression(someNode->child,funcInfo,currST);
        handleExpression(someNode->next,funcInfo,currST);
    }
}

void handleAssignmentStmt(ASTNode *assignmentStmtNode, symFuncInfo *funcInfo, symbolTable *currST){
    if(assignmentStmtNode == NULL || assignmentStmtNode->child == NULL){
        fprintf(stderr,"handleAssignmentStmt: NULL node found.\n");
        return;
    }

    varType *vt1, *vt2;

    switch(assignmentStmtNode->child->gs){
        case g_lvalueIDStmt:{
            ASTNode *idNode = assignmentStmtNode->child->child->child;
            assignIDinScope(idNode, funcInfo, currST);
            vt1 = getDataType(idNode);

            handleExpression(idNode->next,funcInfo,currST);
            vt2 = getDataType(idNode->next);

            if (vt1 != NULL && vt2 != NULL) {
                if(vt1->baseType == vt2->baseType && vt1->vaType == vt2->vaType) {
                    if(vt1->vaType == VARIABLE)
                        return; // No error
                    else {
                        printf("TODO: handle arrays assignemnt\n");
                        return;
                    }
                }

                else {
                    printf("LHS AND RHS DONT MATCH!( %d %d %d %d )line no %d\n", vt1->baseType, vt2->baseType, vt1->vaType, vt2->vaType, idNode->tkinfo->lno);
                }
            }
        }
        break;

        case g_lvalueARRStmt:{
            ASTNode *idNode = assignmentStmtNode->child->child->child;
            useIDinScope(idNode, funcInfo, currST);
            vt1 = getDataType(idNode);

            boundsCheckIfStatic(idNode, idNode->next, funcInfo, currST);
            // TODO: use bound check result!

            handleExpression(idNode->next->next,funcInfo,currST);
            vt2 = getDataType(idNode->next->next);

            if (vt1 != NULL && vt2 != NULL) {
                if(vt1->baseType == vt2->baseType && VARIABLE == vt2->vaType)
                    return; // No error
                else 
                    printf("LHS AND RHS DONT MATCH! line no %d\n", idNode->tkinfo->lno);
            }
        }
        break;
    }
}

void handleSimpleStmt(ASTNode *simpleStmtNode, symFuncInfo *funcInfo, symbolTable *currST){
    if(simpleStmtNode == NULL || simpleStmtNode->child == NULL){
        fprintf(stderr,"handleSimpleStmt: NULL node found.\n");
        return;
    }
    switch(simpleStmtNode->child->gs){
        case g_assignmentStmt:
            handleAssignmentStmt(simpleStmtNode->child,funcInfo,currST);
            break;
        case g_moduleReuseStmt:
            handleModuleReuse(simpleStmtNode->child,funcInfo,currST);
            break;
    }
}

void handleDeclareStmt(ASTNode *declareStmtNode, symFuncInfo *funcInfo, symbolTable *currST){
    if(declareStmtNode == NULL || declareStmtNode->child == NULL){
        fprintf(stderr,"handleDeclareStmt: NULL node found.\n");
        return;
    }
    ASTNode *idListNode = declareStmtNode->child;
    ASTNode *dataTypeNode = idListNode->next;
    ASTNode *idNode = idListNode->child;
    varType vtype = getVtype(dataTypeNode, funcInfo, currST);
    while(idNode != NULL){
        //check for already existing definition in current scope

        //loop variable redeclare check
        gSymbol ty; int isVar=0;
        symTableNode* varNode = findType(idNode,currST,funcInfo,&isVar,&ty);
        if(varNode!=NULL && varNode->info.var.isLoopVar) {
            throwSemanticError(idNode->tkinfo->lno, idNode->tkinfo->lexeme, NULL, SEME_LOOP_VAR_REDECLARED);
            // TOCHECK: ERROR Loop variable redeclared
        }
        else{
            symVarInfo *vinfo = stGetVarInfoCurrent(idNode->tkinfo->lexeme,currST);
            if(vinfo == NULL){
                if(currST->parent == NULL && (outListSearchID(idNode,funcInfo)) != NULL){
                    //highest scope & redeclaration
                    throwSemanticError(idNode->tkinfo->lno, idNode->tkinfo->lexeme, NULL, SEME_REDECLARATION);
                }
                else{
                    if(vtype.vaType == STAT_ARR && vtype.si.vt_num > vtype.ei.vt_num){
                       //invalid si and ei
                       throwSemanticError(idNode->tkinfo->lno,idNode->tkinfo->lexeme,NULL,SEME_INVALID_BOUNDS);
                    }
                    else{
                        //safe for declaration
                        union funcVar fv;
                        initSymVarInfo(&(fv.var));
                        fv.var.lno = idNode->tkinfo->lno;
                        fv.var.vtype = vtype;
                        fv.var.isLoopVar=false;
                        //TODO: check Offset Calculation
                        if(vtype.vaType == VARIABLE || vtype.vaType == STAT_ARR){
                            fv.var.offset = nextGlobalOffset;
                            nextGlobalOffset += fv.var.vtype.bytes;
                        }
                        stAdd(idNode->tkinfo->lexeme,fv,currST);
                        //adding symTableNode pointer in ASTNode
                        idNode->stNode = stSearchCurrent(idNode->tkinfo->lexeme,currST);
                    }

                }
            }
            else{
                //redeclaration
                throwSemanticError(idNode->tkinfo->lno, idNode->tkinfo->lexeme, NULL, SEME_REDECLARATION);
            }
        }
        idNode = idNode->next;
    }
}

void handleConditionalStmt(ASTNode *conditionalStmtNode, symFuncInfo *funcInfo, symbolTable *currST){
    if(conditionalStmtNode==NULL || conditionalStmtNode->child==NULL) {
        fprintf(stderr,"handleConditionalStmt: NULL node found.\n");
        return;
    }
    gSymbol ty;
    int isVar=0; // assume that the switch variable is non-array by default
    ASTNode *idNode = conditionalStmtNode->child;
    ASTNode *ptr = idNode; //on ID
    symTableNode* varNode = findType(ptr,currST,funcInfo,&isVar,&ty);
    if(varNode == NULL) {
        throwSemanticError(ptr->tkinfo->lno, ptr->tkinfo->lexeme, NULL, SEME_SWITCH_VAR_UNDECLARED);
        return; // no use to go ahead if we cannot find the variable in scope. we need to know its data type (integer or boolean) to proceed.
        // TOCHECK: ERROR handle undeclared case statement var error
    }
    else if(!isVar){
        // TOCHECK: ERROR handle switch variable is an array
        throwSemanticError(ptr->tkinfo->lno, ptr->tkinfo->lexeme, NULL, SEME_SWITCH_VAR_TYPE_ARR);
        return;
    }
    else if(ty!=g_BOOLEAN && ty!=g_INTEGER) {
        throwSemanticError(ptr->tkinfo->lno, ptr->tkinfo->lexeme, NULL, SEME_SWITCH_VAR_TYPE_INVALID);
        // TOCHECK: ERROR handle not valid data type
        return;
    }
    //adding symTableNode pointer in ASTNode
    ptr->stNode = varNode;
    if(ptr->next->gs == g_START)
        currST = newScope(currST);
    ptr=ptr->next->child; //on caseStmts
    if(ty == g_BOOLEAN) {
        if(ptr->next!=NULL) {
            throwSemanticError(ptr->next->tkinfo->lno, NULL, NULL, SEME_DEFAULT_IN_BOOLEAN_SWITCH);
            // TOCHECK: ERROR handle default in g_BOOLEAN
        }
        int true_count=0, false_count=0;
        ptr=ptr->child; //on TRUE/FALSE
        ASTNode* it = ptr;
        while(it!=NULL) {
            if(it->gs==g_TRUE)
                true_count++;
            else if(it->gs==g_FALSE)
                false_count++;
            else {
                throwSemanticError(it->tkinfo->lno, it->tkinfo->lexeme, NULL, SEME_NON_BOOLEAN_IN_SWITCH);
                // TOCHECK: ERROR handle non boolean
            }
            it=it->next;
        }
        if(true_count*false_count!=1) {
            throwSemanticError(idNode->tkinfo->lno, NULL, NULL, SEME_TOO_MANY_BOOLEAN_CASES_IN_SWITCH);
            // TOCHECK: ERROR both T/F don't occur
        }
        while(ptr!=NULL) {
            handleStatements(ptr->child,funcInfo,currST);
            ptr=ptr->next;
        }
    } else {// INTEGER type switch
        if(ptr->next==NULL) {
            throwSemanticError(idNode->tkinfo->lno, NULL, NULL, SEME_MISSING_DEFAULT_IN_INTEGER_SWITCH);
            // TOCHECK: ERROR handle no default in g_INTEGER
        }
        ptr=ptr->child; //on NUM
        ASTNode* it=ptr;
        while(it!=NULL) {
            if(it->gs!=g_NUM) {
                throwSemanticError(it->tkinfo->lno, it->tkinfo->lexeme, NULL, SEME_NON_INTEGER_IN_SWITCH);
                // TOCHECK: ERROR not NUM
            }
            it=it->next;
        }
        ASTNode *default_node = (ptr != NULL && ptr->parent != NULL) ? ptr->parent->next : NULL;
        while(ptr!=NULL) {
            handleStatements(ptr->child,funcInfo,currST);
            ptr=ptr->next;
        }
        if(default_node != NULL){
            handleStatements(default_node->child,funcInfo,currST);
        }
    }

}

void handleIterativeStmt(ASTNode *iterativeStmtNode, symFuncInfo *funcInfo, symbolTable *currST){
    if(iterativeStmtNode==NULL || iterativeStmtNode->child==NULL) {
        fprintf(stderr,"handleIterativeStmt: NULL node found.\n");
        return;
    }
    ASTNode* ptr = iterativeStmtNode;
    ptr=ptr->child; //on FOR/WHILE
    if(ptr->gs==g_FOR) {
        ptr=ptr->next; //on ID
        gSymbol ty;
        int isVar=0;
        symTableNode* varNode = findType(ptr,currST,funcInfo,&isVar,&ty);
        //adding symTableNode pointer in ASTNode
        ptr->stNode = varNode;
        if(varNode == NULL) {
            throwSemanticError(ptr->tkinfo->lno, ptr->tkinfo->lexeme, NULL, SEME_FOR_VAR_UNDECLARED);
            // TOCHECK: ERROR handle undeclared iterative statement var error
            return;
        }

        else if(!isVar) {
            throwSemanticError(ptr->tkinfo->lno, ptr->tkinfo->lexeme, NULL, SEME_FOR_VAR_TYPE_ARR);
            // TOCHECK: ERROR handle for variable is an array
            return;
        }
        else if(ty!=g_INTEGER){
            throwSemanticError(ptr->tkinfo->lno, ptr->tkinfo->lexeme, NULL, SEME_FOR_VAR_TYPE_INVALID);
            return;
            // TOCHECK: ERROR handle not valid data type
        }

        varNode->info.var.isLoopVar=true;
        ptr=ptr->next->next;
        if(ptr->gs==g_START)
            currST=newScope(currST);
        ptr=ptr->child;
        handleStatements(ptr,funcInfo,currST);
        varNode->info.var.isLoopVar=false;
        return;
    }
    else if(ptr->gs==g_WHILE) {
        ptr=ptr->next;
        handleExpression(ptr,funcInfo,currST);  //to do existence checking for all its IDs
        // TODO: verify typeof(ptr)
        ptr=ptr->next;
        if(ptr->gs==g_START)
            currST=newScope(currST);
        ptr=ptr->child;
        handleStatements(ptr,funcInfo,currST);
        return;
    }
}

void handleStatements(ASTNode *statementsNode, symFuncInfo *funcInfo, symbolTable *currST){
    if(statementsNode == NULL || statementsNode->child == NULL)
        return;
    ASTNode *ptr = statementsNode->child; //the first statement
    while(ptr != NULL){
        switch(ptr->gs){
            case g_ioStmt:
                handleIOStmt(ptr,funcInfo,currST);
                break;
            case g_simpleStmt:
                handleSimpleStmt(ptr,funcInfo,currST);
                break;
            case g_declareStmt:
                handleDeclareStmt(ptr,funcInfo,currST);
                break;
            case g_conditionalStmt:
                handleConditionalStmt(ptr,funcInfo,currST);
                break;
            case g_iterativeStmt:
                handleIterativeStmt(ptr,funcInfo,currST);
                break;
        }
        ptr = ptr->next;
    }
}

void handleModuleDef(ASTNode *startNode, symFuncInfo *funcInfo){
    if(startNode == NULL){
        fprintf(stderr,"handleModuleDef: Empty Start Node received.\n");
        return;
    }
    if(startNode->tkinfo)
        funcInfo->lno = startNode->tkinfo->lno;
    funcInfo->st = newScope(NULL);
    if(startNode->child == NULL)
        return; //no statements inside
    handleStatements(startNode->child,funcInfo,funcInfo->st);
    //In the end check that all variables in output list are assigned
    paramOutNode *outptr = funcInfo->outPListHead;
    while(outptr != NULL){
        if(!((outptr->info).var.isAssigned)){
            throwSemanticError((outptr->info).var.lno, outptr->lexeme, NULL, SEME_UNASSIGNED);
        }
        outptr = outptr->next;
    }
}



void handlePendingCalls(symFuncInfo *funcInfo){
    //this is to handle function calls which occurred in between the function declaration and definition
    if(funcInfo == NULL)
        return;
    ASTNodeListNode *pcptr = funcInfo->pendingCallListHead;
    while(pcptr != NULL){
        checkModuleSignature(pcptr->astNode,pcptr->callerFuncInfo,pcptr->currST);
        ASTNodeListNode *tmp = pcptr;
        pcptr = pcptr->next;
        free(tmp);
    }
    funcInfo->pendingCallListHead = NULL;   //handles all pending calls
}

void handleOtherModule(ASTNode *moduleNode){
    if(moduleNode == NULL){
        fprintf(stderr,"handleOtherModule: Empty module Node received.\n");
        return;
    }
    ASTNode *idNode = moduleNode->child;
    ASTNode *inpListNode = NULL;
    ASTNode *outListNode = NULL;
    ASTNode *moduleDefNode = NULL;
    ASTNode *tmp = idNode;
    if(tmp->next != NULL && tmp->next->gs == g_input_plist){
        inpListNode = tmp->next;
        tmp = tmp->next;
    }
    if(tmp->next != NULL && tmp->next->gs == g_output_plist){
        outListNode = tmp->next;
        tmp = tmp->next;
    }
    if(tmp->next != NULL && tmp->next->gs == g_moduleDef){
        moduleDefNode = tmp->next;
    }
    symFuncInfo *finfo = stGetFuncInfo(idNode->tkinfo->lexeme,&(funcTable));
    if(finfo == NULL){
        //first appearance of this function name
        union funcVar fv;
        initSymFuncInfo(&(fv.func),idNode->tkinfo->lexeme);
        fv.func.status = F_DEFINED;
        union funcVar *funcEntry = stAdd(idNode->tkinfo->lexeme, fv, &funcTable);
        finfo = &(funcEntry->func);
    }
    else if(finfo->status == F_DECLARED){
        //Error of a redundant declaration
        throwSemanticError(finfo->lno, idNode->tkinfo->lexeme, NULL, SEME_REDUNDANT_DECLARATION);
    }
    else if(finfo->status == F_DEFINED) {
        //redefinition
        char val[30];
        sprintf(val,"%d",finfo->lno);
        throwSemanticError(idNode->tkinfo->lno, idNode->tkinfo->lexeme, val, SEME_REDEFINITION);
        return;
    }

    finfo->lno = idNode->tkinfo->lno;
    finfo->status = F_DEFINED;
    finfo->inpPListHead = createParamInpList(inpListNode);
    finfo->outPListHead = createParamOutList(outListNode);
    //adding symTableNode pointer in ASTNode
    idNode->stNode = stSearchCurrent(idNode->tkinfo->lexeme,&funcTable);
    handlePendingCalls(finfo);
    handleModuleDef(moduleDefNode->child,finfo);
}

void handleUndefinedModules(){
    //Original intention: Traverses and checks all functions to know if there is any function which is still left in the state DECLARED or DECLARATION_VALID
    //What it does now: currently we only throw an error if a function was declared, called and not defined. A function that was declared, never used and not defined is not an error
    symTableNode *currFunc = NULL;
    for(int i = 0; i<SYMBOL_TABLE_SIZE; i++){
        // traverse all slots one by one
        currFunc = funcTable.tb[i];
        while(currFunc != NULL){
            //traverse all functions hashed in current slot
            if(currFunc->info.func.status == F_DECLARATION_VALID){
                throwSemanticError(currFunc->info.func.lno, currFunc->info.func.funcName, NULL, SEME_MODULE_USED_NOT_DEFINED);
            }
            currFunc = currFunc->next;
        }
    }

}

void buildSymbolTable(ASTNode *root){
    if(root == NULL)
        return;
    switch(root->gs){
        case g_program:
        {
            nextGlobalOffset = 0;
            haveSemanticErrors = false;
            ASTNode *ptr = root->child;
            while(ptr != NULL){
                buildSymbolTable(ptr);
                ptr = ptr->next;
            }
            handleUndefinedModules();
        }
            break;
        case g_moduleDeclarations:
        {
            ASTNode *ptr = root->child;
            while(ptr != NULL){
                handleModuleDeclaration(ptr);
                ptr = ptr->next;
            }
        }
            break;
        case g_DRIVER:
        {
            union funcVar fv;
            initSymFuncInfo(&(fv.func),"@driver");
            fv.func.status = F_DEFINED;
            //@driver is the special name for driver function
            union funcVar *funcEntry = stAdd("@driver", fv, &funcTable);
            //adding symTableNode pointer in ASTNode
            root->stNode = stSearchCurrent("@driver",&funcTable);
            nextGlobalOffset = 0;
            funcEntry->func.lno = root->tkinfo->lno;
            handleModuleDef(root->child->child,&(funcEntry->func));
        }
            break;
        case g_otherModules:
            //this node will only occur when it has atleast one child
        {
            ASTNode *ptr = root->child;
            while(ptr != NULL){
                nextGlobalOffset = 0;
                handleOtherModule(ptr);
                ptr = ptr->next;
            }
        }
            break;
    }
}
