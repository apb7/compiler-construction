#include "symbolTable.h"
#include "symbolTableDef.h"
#include <stdio.h>


symbolTable funcTable;

void initSymbolTable(symbolTable *st){
    (st->parentTable) = NULL;
    (st->nestedTablesHead) = NULL;
    for(int i=0; i<SYMBOL_TABLE_SIZE; i++){
        (st->tb)[i] = NULL;
    }
}

