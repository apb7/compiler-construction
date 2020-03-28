// Group Number: 31
// MADHUR PANWAR   2016B4A70933P
// TUSSANK GUPTA   2016B3A70528P
// SALMAAN SHAHID  2016B4A70580P
// APURV BAJAJ     2016B3A70549P
// HASAN NAQVI     2016B5A70452P

#ifndef AST_H
#define AST_H

#include "astDef.h"
#include <stdbool.h>

ASTNode* createASTNode(ASTNode *parent, treeNode *trNode);
ASTNode* buildASTTree(treeNode* trNodeRoot);

void print_ASTTree(ASTNode *root);


#endif
