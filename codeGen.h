#include "astDef.h"
#include "symbolTableDef.h"

void printLeaf(ASTNode* leaf, FILE* fp);
void getAptReg(char *regStr, int regno, int width);
void generateCode(ASTNode* root, symbolTable* symT, FILE* fp);
void genExpr(ASTNode *astNode, FILE *fp, bool firstCall, gSymbol expType);