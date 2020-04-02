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

void initSymbolTable(symbolTable *st){
    (st->parentTable) = NULL;
    (st->nestedTablesHead) = NULL;
    (st->nestedTablesTail) = NULL;
    (st->next) = NULL;
    for(int i=0; i<SYMBOL_TABLE_SIZE; i++){
        (st->tb)[i] = NULL;
    }
}

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
        currST = (symbolTable *) malloc(sizeof(symbolTable));
        initSymbolTable(currST);
        return currST;
    }
    if(currST->nestedTablesTail == NULL){
        currST->nestedTablesHead = (symbolTable *) malloc(sizeof(symbolTable));
        initSymbolTable(currST->nestedTablesHead);
        currST->nestedTablesTail = currST->nestedTablesHead;
    }
    else{
        symbolTable *tail = currST->nestedTablesTail;
        tail->next = (symbolTable *) malloc(sizeof(symbolTable));
        initSymbolTable(tail->next);
        currST->nestedTablesTail = tail->next;
    }
    currST->nestedTablesTail->parentTable = currST;
    return currST->nestedTablesTail;
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
        fprintf(stderr,"createParamInpList: NULL error.\n");
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
            if(!stSearch(idNode->tkinfo->lexeme,currST)){
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

void handleSimpleStmt(ASTNode *simpleStmtNode, symFuncInfo *funcInfo, symbolTable *currST){
    //TODO: Handle Simple Statement
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
            case g_condionalStmt:
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

void handleOtherModule(ASTNode *moduleNode){
    if(moduleNode == NULL){
        fprintf(stderr,"handleOtherModule: Empty module Node received.\n");
        return;
    }
    //TODO: add this module to symbol table, fill its inputlist using createParamInpList
    // and outputlist using createParamOutList, call handleModuleDef to do further handling


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
            union funvar fv;
            initSymFuncInfo(&(fv.func),"@driver");
            fv.func.status = F_DEFINED;
            //@driver is the special name for driver function
            union funvar *funcEntry = stAdd("@driver",fv,&funcTable);
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
