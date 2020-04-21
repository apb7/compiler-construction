// Group Number: 31
// MADHUR PANWAR   2016B4A70933P
// TUSSANK GUPTA   2016B3A70528P
// SALMAAN SHAHID  2016B4A70580P
// APURV BAJAJ     2016B3A70549P
// HASAN NAQVI     2016B5A70452P

#ifndef ASTDEF_H
#define ASTDEF_H

#include "parserDef.h"
#include "symbolTableDef.h"

typedef treeNode parseNode;

unsigned int number_of_ASTNodes;

struct ASTNode {
    gSymbol gs;
    tokenInfo *tkinfo;
    struct symTableNode *stNode;
    int start_line_no; // Only for START Nodes
    int end_line_no; 
    struct ASTNode *next; // Sibling of the ASTNode.
    struct ASTNode *child; // Child of the ASTNode.
    struct ASTNode *parent; // Parent of the ASTNode.
};

typedef struct ASTNode ASTNode;

#endif
