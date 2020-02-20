#ifndef COMPILER_CONSTRUCTION_TREENODEPTR_DEF_H
#define COMPILER_CONSTRUCTION_TREENODEPTR_DEF_H
#include "parser/parserDef.h"

struct treeNode{
    gSymbol tk;                     //ENUM Value
//    tokenInfo *tkinfo;              // Info for Terminals
    struct treeNode *next;     //My Sibling on right
    struct treeNode *child;    //My first child
    struct treeNode *parent;   //My Parent
};
typedef struct treeNode treeNode;



#endif //COMPILER_CONSTRUCTION_TREENODEPTR_DEF_H
