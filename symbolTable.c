#include "symbolTable.h"
#include "symbolTableDef.h"
#include "symbolHash.h"
#include <stdio.h>
#include "astDef.h"
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "error.h"

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
//    TODO: use createSymbolTable()
    if(currST == NULL){
        currST = (symbolTable *) malloc(sizeof(symbolTable));
        initSymbolTable(currST);
        return currST;
    }
    if(currST->lastChild == NULL){
        currST->headChild = (symbolTable *) malloc(sizeof(symbolTable));
        initSymbolTable(currST->headChild);
        currST->lastChild = currST->headChild;
    }
    else{
        symbolTable *tail = currST->lastChild;
        tail->next = (symbolTable *) malloc(sizeof(symbolTable));
        initSymbolTable(tail->next);
        currST->lastChild = tail->next;
    }
    currST->lastChild->parent = currST;
    return currST->lastChild;
}

void checkModuleSignature(ASTNode *moduleReuseNode, symFuncInfo *funcInfo){
    //TODO: Check whether the type and order of input and output variables match. if not report an error
}

varType getVtype(ASTNode *dataTypeNode){
    //TODO: Construct the varType struct and return it
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
    //TODO: Write code similar to inpListSearchID(..)
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
    //TODO: Write code similar to createParamInpNode
}

paramOutNode *createParamOutList(ASTNode *outputPlistNode){
    //TODO: Write code similar to createParamInpList
}



//handles single module declaration
void handleModuleDeclaration(ASTNode *root){
    if(root == NULL){
        fprintf(stderr,"handleModuleDeclaration: Empty node received.\n");
        return;
    }
    //TODO: add root to the funcTable

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
            if(stSearch(idNode->tkinfo->lexeme,currST) == NULL){
                //not found in any of the symbol tables
                if(inpListSearchID(idNode,funcInfo) == NULL){
                    //not found in the input list as well
                    paramOutNode *tmp = outListSearchID(idNode,funcInfo);
                    if(tmp == NULL){
                        //not found anywhere
                        error e;
                        e.errType = E_SEMANTIC;
                        e.lno = idNode->tkinfo->lno;
                        strcpy(e.edata.seme.errStr,idNode->tkinfo->lexeme);
                        e.edata.seme.etype = SEME_UNDECLARED;
                        foundNewError(e);
                    }
                    else{
                        //finally found in output parameters list
                        setAssignedOutParam(tmp);
                    }
                }
            }
        }
            break;
        case g_PRINT:
            //TODO: Handle this section
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
            error e;
            e.errType = E_SEMANTIC;
            e.lno = outptr->lno;
            strcpy(e.edata.seme.errStr,outptr->lexeme);
            e.edata.seme.etype = SEME_UNASSIGNED;
            foundNewError(e);
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
        checkModuleSignature(pcptr->astNode,funcInfo);
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
        error e;
        e.errType = E_SEMANTIC;
        e.lno = finfo->lno;
        strcpy(e.edata.seme.errStr,idNode->tkinfo->lexeme);
        e.edata.seme.etype = SEME_REDUNDANT_DECLARATION;
        foundNewError(e);
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
