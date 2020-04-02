#ifndef SYMBOLHASH_H
#define SYMBOLHASH_H

#include "symbolTableDef.h"
#include "hash.h"



//Add the given function/variable details to the symbol table in st and return the address to the entry's union
union funvar *stAdd(char *lexeme, union funvar fvinfo , symbolTable *st);

//Check if the given key is in the current symbol table or not
bool stSearchCurrent(char *lexeme, symbolTable *st);

//Check if the given key is in the current scope or not
// (check current ST and all the parent STs. Don't search in function input and output params)
//take help of stSearchCurrent(...)
bool stSearch(char *lexeme, symbolTable *st);

//return the address of the data element after finding the required entry in the table
symFuncInfo *stGetFuncInfo(char *lexeme, symbolTable *st);
symVarInfo *stGetVarInfo(char *lexeme, symbolTable *st);

//UTILISE THE SYMBOLHASHFUNCTION from the hash.h header

//create new symbol table and initialize entries to NULL
symbolTable *createSymbolTable();

#endif //SYMBOLHASH_H
