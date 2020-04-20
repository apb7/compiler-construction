// Group Number: 31
// MADHUR PANWAR   2016B4A70933P
// TUSSANK GUPTA   2016B3A70528P
// SALMAAN SHAHID  2016B4A70580P
// APURV BAJAJ     2016B3A70549P
// HASAN NAQVI     2016B5A70452P

#include "astDef.h"
#include "symbolTableDef.h"

void getAptReg(char memToRegStr[][6], int regno, int width);
void generateCode(ASTNode* root, symbolTable* symT, FILE* fp);
void genExpr(ASTNode *astNode, FILE *fp, bool firstCall, gSymbol expType);