#include "symbolHash.h"
#include <stdlib.h>
#include <string.h>


symTableNode* makeSymTableNode(char* lexeme, union funcVar fvinfo){
    symTableNode *newNode = (symTableNode*)(malloc(sizeof(symTableNode)));
    strcpy(newNode->lexeme, lexeme);
    newNode->info = fvinfo;
    newNode->next = NULL;
    return newNode;
}

//Add the given function/variable details to the symbol table in st and return the address to the entry's union
union funcVar *stAdd(char *lexeme, union funcVar fvinfo , symbolTable *st){
    int slot = symbolTableHashFunction(lexeme);
    symTableNode *hashNode = (st->tb)[slot];
    symTableNode *infoNode = makeSymTableNode(lexeme, fvinfo);
    if(hashNode != NULL){
        while(hashNode->next != NULL){
            hashNode = hashNode->next;
        }
        hashNode->next = infoNode;
    }
    else{
        (st->tb)[slot] = infoNode;
    }
    return &(infoNode->info);
}

//Check if the given key is in the current symbol table or not
symTableNode *stSearchCurrent(char *lexeme, symbolTable *st){
    // assuming st is not NULL
    if(st == NULL){
        return NULL;
    }
    int slot = symbolTableHashFunction(lexeme);
    symTableNode *curr_node = (st->tb)[slot];
    while(curr_node != NULL){
        if(strcmp(curr_node->lexeme, lexeme) == 0){
            return curr_node;
        }
        curr_node = curr_node->next;
    }
    return NULL;
}

//Check if the given key is in the current scope or not
// (check current ST and all the parent STs. Don't search in function input and output params)
//take help of stSearchCurrent(...)
symTableNode *stSearch(char *lexeme, symbolTable *st){
    // assuming st is not NULL
    if(st == NULL){
        return NULL;
    }
    symTableNode *curr = NULL;
    while(st != NULL){
        curr = stSearchCurrent(lexeme, st);
        if(curr){
            return curr;
        }
        st = st->parent;
    }
    return NULL;
}

//return the address of the data element after finding the required entry in the table
// return NULL if not found
symVarInfo *stGetVarInfoCurrent(char *lexeme, symbolTable *st){
    symTableNode *symInfoNode = stSearchCurrent(lexeme, st);
    if(symInfoNode){
        return &(symInfoNode->info.var);
    }
    return NULL;
}


symFuncInfo *stGetFuncInfo(char *lexeme, symbolTable *st){
// assumption: 'st' is the funcTable declared globally in symbolTable.c
    symTableNode *symInfoNode = stSearchCurrent(lexeme, st);
    if(symInfoNode){
        return &(symInfoNode->info.func);
    }
    return NULL;
}

symVarInfo *stGetVarInfo(char *lexeme, symbolTable *st){
    symTableNode *symInfoNode = stSearch(lexeme, st);
    if(symInfoNode){
        return &(symInfoNode->info.var);
    }
    return NULL;
}

//UTILISE THE SYMBOLHASHFUNCTION from the hash.h header

void initSymbolTable(symbolTable *st){
    (st->parent) = NULL;
    (st->headChild) = NULL;
    (st->lastChild) = NULL;
    (st->next) = NULL;
    for(int i=0; i<SYMBOL_TABLE_SIZE; i++){
        (st->tb)[i] = NULL;
    }
}

//create new symbol table and initialize entries to NULL
symbolTable *createSymbolTable(){
    symbolTable *st = (symbolTable*)(malloc(sizeof(symbolTable)));
    initSymbolTable(st);
    return st;
}