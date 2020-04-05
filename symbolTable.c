#include "symbolTable.h"
#include "symbolTableDef.h"
#include "symbolHash.h"
#include <stdio.h>
#include "astDef.h"
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "error.h"

//TODO: when the scope of the function ends, check if all its output parameters have been assigned.
//TODO: Use the error function to make all the errors rather than making them manually e.g. refactor boundsCheckIfStatic(..)
symbolTable funcTable;


void initSymFuncInfo(symFuncInfo *funcInfo, char *funcName){
    funcInfo->status = F_DECLARED;
    funcInfo->lno = -1;
    funcInfo->st = NULL;
    funcInfo->inpPListHead = NULL;
    funcInfo->outPListHead = NULL;
    funcInfo->pendingCallListHead = NULL;
    strcpy(funcInfo->funcName,funcName);
}

void setAssignedOutParam(paramOutNode *outNode){
    if(outNode == NULL)
        return;
    outNode->isAssigned = true;
}

symbolTable *newScope(symbolTable *currST){
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

void checkModuleSignature(ASTNode *moduleReuseNode, symFuncInfo *funcInfo, symbolTable *currST){
    //TODO: Check whether the type, number and order of input and output variables match. if not report an error
    if(moduleReuseNode == NULL || currST == NULL){
        fprintf(stderr,"checkModuleSignature: NULL error.\n");
        return;
    }
    paramInpNode *currInpListNode = funcInfo->inpPListHead;
    paramOutNode *currOutListNode = funcInfo->outPListHead;
    ASTNode *idOrList = moduleReuseNode->child;
    ASTNode *currListNode = NULL;
    char *currlexeme;
    symTableNode *currSymNode;

    if(idOrList->child != NULL){
        //idOrList is actually idList
        //match LHS list with outPList
        currListNode = idOrList->child;
        do{
            // match LHS list i.e. currListNode's type with currOutListNode
            currlexeme = currListNode->tkinfo->lexeme;
            currSymNode = stSearch(currlexeme, currST);
            // currSymNode is assured to be non-NULL since we check this in handleModuleReuse

            if(currOutListNode == NULL){
//                TODO: ERROR: too many return values expected from function
                    return;
            }
            //match the type of nodes
            if(currSymNode->info.var.vtype.baseType != currOutListNode->vtype.baseType){
//                TODO: ERROR; throw typeMismatchError
                    return;
            }
            currOutListNode = currOutListNode->next;
            currListNode = currListNode->next;
        }while(currListNode != NULL);
        if(currOutListNode != NULL){
//            TODO: ERROR; too few retuen values expected from the function
                return;
        }
        idOrList = idOrList->next; // now points to function name AST node i.e. ID after idList
    }
    //skip module name and go to RHS list
    idOrList = idOrList->next; // points to RHS List

    //match RHS List with InpPlist
    currListNode = idOrList->child;
    do{
        // match RHS list i.e. currListNode's type with currInpListNode
        currlexeme = currListNode->tkinfo->lexeme;
        currSymNode = stSearch(currlexeme, currST);
        // currSymNode is assured to be non-NULL since we check this in handleModuleReuse

        if(currInpListNode == NULL){
//                TODO: ERROR: too many arguments passed to the function
            return;
        }
        //match the type of nodes
        if(currSymNode->info.var.vtype.baseType != currInpListNode->vtype.baseType){
//                TODO: ERROR; throw typeMismatchError
            return;
        }
        currInpListNode = currInpListNode->next;
        currListNode = currListNode->next;
    }while(currListNode != NULL);
    if(currInpListNode != NULL){
//            TODO: ERROR; too few arguments passed to the function
        return;
    }
}

int getSizeByType(gSymbol gs){
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

varType getVtype(ASTNode *dataTypeNode){
    //TODO: Construct the varType struct and return it
//    typedef enum{
//        VARIABLE, STAT_ARR, DYN_L_ARR, DYN_R_ARR, DYN_ARR
//    }varOrArr;
//
//    union numOrId {
//        int vt_num;
//        symTableNode *vt_id;
//    };
//
//    struct varType{
//        gSymbol baseType;
//        varOrArr vaType;
//        union numOrId si;
//        union numOrId ei;
//        int bytes;
//    };
//    typedef struct varType varType;
    varType vt;
    ASTNode *rangeArrOrBType = dataTypeNode->child;
    ASTNode *bTypeOrNull = dataTypeNode->child->next;
    if(bTypeOrNull == NULL){
        // we're dealing with a variable
        vt.baseType = rangeArrOrBType->gs;
        vt.bytes = getSizeByType(vt.baseType);
        vt.vaType = VARIABLE;
    }
    else{
        // we're dealing with an array

    }
    vt.baseType = dataTypeNode->gs;


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

paramInpNode *createParamInpNode(ASTNode *idNode, ASTNode *dataTypeNode){
    if(idNode == NULL || dataTypeNode == NULL){
        fprintf(stderr,"createParamInpNode: NULL error.\n");
        return NULL;
    }
    paramInpNode *ptr = (paramInpNode *) (malloc(sizeof(paramInpNode)));
    if(idNode->tkinfo)
        ptr->lno = idNode->tkinfo->lno;
    strcpy(ptr->lexeme,idNode->tkinfo->lexeme);
    ptr->vtype = getVtype(dataTypeNode);
    //TODO: Offset computation
    ptr->next = NULL;
    return ptr;
}

paramInpNode *createParamInpList(ASTNode *inputPlistNode){
    if(inputPlistNode == NULL){
        return NULL;
    }
    ASTNode *curr = inputPlistNode->child;
    paramInpNode *head = NULL;
    paramInpNode *currInp = NULL;
    while(curr != NULL){
        if(head == NULL){
            head = createParamInpNode(curr,curr->next);
            currInp = head;
        }
        else{
            currInp->next = createParamInpNode(curr,curr->next);
            currInp = currInp->next;
        }
        if(curr->next)
            curr = curr->next->next;
        else
            curr = NULL;
    }
    return head;
}

paramOutNode *createParamOutNode(ASTNode *idNode, ASTNode *dataTypeNode){
    if(idNode == NULL || dataTypeNode == NULL){
        fprintf(stderr,"createParamOutNode: NULL error.\n");
        return NULL;
    }
    paramOutNode *ptr = (paramOutNode *) (malloc(sizeof(paramOutNode)));
    if(idNode->tkinfo)
        ptr->lno = idNode->tkinfo->lno;
    strcpy(ptr->lexeme,idNode->tkinfo->lexeme);
    ptr->vtype = getVtype(dataTypeNode);
    //TODO: Offset computation
    ptr->next = NULL;
    return ptr;
}

paramOutNode *createParamOutList(ASTNode *outputPlistNode){
    if(outputPlistNode == NULL){
        return NULL;
    }
    ASTNode *curr = outputPlistNode->child;
    paramOutNode *head = NULL;
    paramOutNode *currOut = NULL;
    while(curr != NULL){
        if(head == NULL){
            head = createParamOutNode(curr, curr->next);
            currOut = head;
        }
        else{
            currOut->next = createParamOutNode(curr,curr->next);
            currOut = currOut->next;
        }
        if(curr->next)
            curr = curr->next->next;
        else
            curr = NULL;
    }
    return head;
}

//____________________________________
//useless code just to compile
void boundsCheckIfStatic(ASTNode *idNode, ASTNode *idOrNumNode, symFuncInfo *funcInfo, symbolTable *currST){

}
//_____________________________________


//handles single module declaration
void handleModuleDeclaration(ASTNode *moduleIDNode){
    if(moduleIDNode == NULL){
        fprintf(stderr,"handleModuleDeclaration: Empty node received.\n");
        return;
    }
    //TODO: add moduleIDNode to the funcTable
    symFuncInfo *finfo = stGetFuncInfo(moduleIDNode->tkinfo->lexeme,&(funcTable));
    if(finfo == NULL){
        union funcVar fv;
        initSymFuncInfo(&(fv.func),moduleIDNode->tkinfo->lexeme);
        union funcVar *funcEntry = stAdd(moduleIDNode->tkinfo->lexeme, fv, &funcTable);
    }
    else{
        // ERROR: Redeclaration Error
        if(finfo->status == F_DECLARED){
            // no need to check this if condition but just for consistency
//            TODO: throw Redeclaration error
        }
    }
}

void throwSemanticError(unsigned int lno, char* errStr, SemanticErrorType errorType){
    error e;
    e.errType = E_SEMANTIC;
    e.lno = lno;
    strcpy(e.edata.seme.errStr, errStr);
    e.edata.seme.etype = errorType;
    foundNewError(e);
}

bool checkIDInScopesAndLists(ASTNode *idNode, symFuncInfo *funcInfo, symbolTable *currST, bool assign){
    if(stSearch(idNode->tkinfo->lexeme,currST) == NULL) {
        //not found in any of the symbol tables
        if (inpListSearchID(idNode, funcInfo) == NULL) {
            //not found in the input list as well
            paramOutNode *tmp = outListSearchID(idNode, funcInfo);
            if (tmp == NULL) {
                //not found anywhere
                return false;
            }
            else {
                if(assign) {
                    //finally found in output parameters list
                    setAssignedOutParam(tmp);
                }
            }
        }
    }
    return true;
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
            if(checkIDInScopesAndLists(idNode, funcInfo, currST, true) == false){
             //not found anywhere
             throwSemanticError(idNode->tkinfo->lno, idNode->tkinfo->lexeme, SEME_UNDECLARED);
             }
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
                    if (checkIDInScopesAndLists(idOrConst, funcInfo, currST, false) == false) {
//                      TODO: will this checkIDInScopesAndLists check suffice if ID is array; handle array index bound checking
                        throwSemanticError(idOrConst->tkinfo->lno, idOrConst->tkinfo->lexeme, SEME_UNDECLARED);
                        return;
                    }
                    if(idOrConst->next != NULL){
                        // if idOrConst->next is NUM, the following will do static bound checking
                        boundsCheckIfStatic(idOrConst, idOrConst->next, funcInfo, currST);
//                        PRINT var_id_num->ID, NUM -- handled upto last line
                        idOrConst = idOrConst->next;
                        if(idOrConst->gs == g_ID){
//                      PRINT var_id_num->ID, ID
                            if(checkIDInScopesAndLists(idOrConst, funcInfo, currST, false) == false){
//                            TODO: dynamic bounds check on array in previous if and index given by this ID
                                throwSemanticError(idOrConst->tkinfo->lno, idOrConst->tkinfo->lexeme, SEME_UNDECLARED);
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
    //TODO: First check whether called function is declared/defined or not
    //TODO: Then check all the IDs whether they are already declared
    //TODO: Then if function already defined, call checkModuleSignature(...) else add node to pendingCallList's head

}

void handleAssignmentStmt(ASTNode *assignmentStmtNode, symFuncInfo *funcInfo, symbolTable *currST){
    //TODO: Handle Assignment Statement
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
    //TODO: Handle Declare Statement
}

void handleConditionalStmt(ASTNode *conditionalStmtNode, symFuncInfo *funcInfo, symbolTable *currST){
    //TODO: Handle Conditional Statement
}

void handleIterativeStmt(ASTNode *declareStmtNode, symFuncInfo *funcInfo, symbolTable *currST){
    //TODO: Handle Iterative Statement
}

void handleStatements(ASTNode *statementsNode, symFuncInfo *funcInfo, symbolTable *currST){
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


void handleStartNode(ASTNode *startNode, symFuncInfo *funcInfo, symbolTable *currST){
    if(startNode == NULL || startNode->child == NULL){
        fprintf(stderr,"handleStartNode: Empty Start Node or its child received.\n");
        return;
    }
    symbolTable *newST = newScope(currST);
    switch(startNode->child->gs){
        case g_statements:
            handleStatements(startNode->child,funcInfo,newST);
            break;
        case g_caseStmts:
            //TODO: Write code for handleCaseStmts(...)
            break;
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
        if(!(outptr->isAssigned)){
            throwSemanticError(outptr->lno, outptr->lexeme, SEME_UNASSIGNED);
        }
        outptr = outptr->next;
    }
}



void handlePendingCalls(symFuncInfo *funcInfo){
    //this is to handle function calls which occurred in between the function declaration and definition
    if(funcInfo == NULL)
        return;
    ASTNodeListNode *pcptr = funcInfo->pendingCallListHead;
//    __________________________________________

//    useless code just to compile
    symbolTable *currST = NULL;
//    ___________________________________________


    while(pcptr != NULL){
        checkModuleSignature(pcptr->astNode,funcInfo, currST);
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
    if(finfo->status == F_DECLARED){
        //Error of a redundant declaration
        throwSemanticError(finfo->lno, idNode->tkinfo->lexeme, SEME_REDUNDANT_DECLARATION);
    }
    if(finfo->status == F_DECLARED) {
//        TODO: ...
    }

    finfo->lno = idNode->tkinfo->lno;
    finfo->status = F_DEFINED;
    finfo->inpPListHead = createParamInpList(inpListNode);
    finfo->outPListHead = createParamOutList(outListNode);
    handlePendingCalls(finfo);
    handleModuleDef(moduleDefNode->child,finfo);
}

void buildSymbolTable(ASTNode *root){
    if(root == NULL)
        return;
    switch(root->gs){
        case g_program:
        {
            ASTNode *ptr = root->child;
            while(ptr != NULL){
                buildSymbolTable(ptr);
                ptr = ptr->next;
            }
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
        case g_driverModule:
        {
            union funcVar fv;
            initSymFuncInfo(&(fv.func),"@driver");
            fv.func.status = F_DEFINED;
            //@driver is the special name for driver function
            union funcVar *funcEntry = stAdd("@driver", fv, &funcTable);
            handleModuleDef(root->child,&(funcEntry->func));
        }
            break;
        case g_otherModules:
            //this node will only occur when it has atleast one child
        {
            ASTNode *ptr = root->child;
            while(ptr != NULL){
                handleOtherModule(root->child);
                ptr = ptr->next;
            }
        }
            break;
    }
}
