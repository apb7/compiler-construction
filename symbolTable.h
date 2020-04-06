// Group Number: 31
// MADHUR PANWAR   2016B4A70933P
// TUSSANK GUPTA   2016B3A70528P
// SALMAAN SHAHID  2016B4A70580P
// APURV BAJAJ     2016B3A70549P
// HASAN NAQVI     2016B5A70452P
#include "astDef.h"
#include "symbolTableDef.h"
#include "error.h"

#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

//TODO: put all the declarations here once symbolTable.c is done
void handleStatements(ASTNode *statementsNode, symFuncInfo *funcInfo, symbolTable *currST);
void boundsCheckIfStatic(ASTNode *idNode, ASTNode *idOrNumNode, symFuncInfo *funcInfo, symbolTable *currST);
void throwSemanticError(unsigned int lno, char* errStr1, char *errStr2, SemanticErrorType errorType);
#endif //SYMBOLTABLE_H
