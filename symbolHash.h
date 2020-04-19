// Group Number: 31
// MADHUR PANWAR   2016B4A70933P
// TUSSANK GUPTA   2016B3A70528P
// SALMAAN SHAHID  2016B4A70580P
// APURV BAJAJ     2016B3A70549P
// HASAN NAQVI     2016B5A70452P

#ifndef SYMBOLHASH_H
#define SYMBOLHASH_H

#include "symbolTableDef.h"
#include "hash.h"


//Add the given function/variable details to the symbol table in st and return the address to the entry's union
union funcVar *stAdd(char *lexeme, union funcVar fvinfo , symbolTable *st);

//Check if the given key is in the current symbol table or not
symTableNode *stSearchCurrent(char *lexeme, symbolTable *st);

//Check if the given key is in the current scope or not
// (check current ST and all the parent STs. Don't search in function input and output params)
//take help of stSearchCurrent(...)
symTableNode *stSearch(char *lexeme, symbolTable *st);

//return the address of the data element after finding the required entry in the table
// return NULL if not found
symFuncInfo *stGetFuncInfo(char *lexeme, symbolTable *funcTable);
symVarInfo *stGetVarInfoCurrent(char *lexeme, symbolTable *st);
symVarInfo *stGetVarInfo(char *lexeme, symbolTable *st);

//UTILISE THE SYMBOLHASHFUNCTION from the hash.h header

// initialize entries to NULL
void initSymbolTable(symbolTable *st);

//create new symbol table and initialize entries to NULL
symbolTable *createSymbolTable();

#endif //SYMBOLHASH_H
