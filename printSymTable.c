#include <stdio.h>
#include <stdlib.h>
#include "symbolHash.h"
#include "symbolTableDef.h"
#include "symbolTable.h"



void printSymbolTable(symbolTable* st){
    // prints the whole SymbolTable Structure by calling printCurrSymTable
    // TODO: add a call to printCurrSymTable(..) appropriately
    if(st == NULL)
        return;
    symTableNode *currST = NULL;
    printf("################################# SYMBOL TABLE #################################\n\n");
    for(int i=0; i<SYMBOL_TABLE_SIZE; i++){
        currST = (st->tb)[i];
        while(currST != NULL){
            printf("%s, slot %d\n",currST->lexeme,i);
            printCurrSymTable(currST);
            printf("\n\n");
            currST = currST->next;
        }
    }
    printf("##################################### ~ ** ~ #####################################\n\n");
}

void printCurrSymTable(symTableNode *stn){
    // prints just one symbol table and its hierarchies corresponding to a function scope
    // i.e. prints the whole scope structure of a function
    int level = 0;



}


void makeSampleSymTableForTest(symbolTable* funcTable){

}