// Group Number: 31
// MADHUR PANWAR   2016B4A70933P
// TUSSANK GUPTA   2016B3A70528P
// SALMAAN SHAHID  2016B4A70580P
// APURV BAJAJ     2016B3A70549P
// HASAN NAQVI     2016B5A70452P

#ifndef PARSERDEF_H
#define PARSERDEF_H

#include "lexerDef.h"

typedef enum{
    #define X(a,b) g_ ## a,
    #define K(a,b,c) g_ ## a,
    #include "keywords.txt"
    #include "tokens.txt"

    g_EPS,
    g_EOS,
    #include "nonTerminals.txt"

    #undef K
    #undef X
    g_numSymbols
} gSymbol;
//g_EPS is for Epsilon
//g_EOS is end of string (it marks the end of input stream)

typedef struct{
    int start;
    int end;
} ruleRange;

struct rhsNode{
    gSymbol s;
    struct rhsNode *next;
};
typedef struct rhsNode rhsNode;

typedef struct{
    int gRuleIndex; // 0-based index of the grammar rule used. 
    gSymbol lhs;    //non terminal symbol
    rhsNode *head;
} grammarNode;

struct treeNode{
    int gRuleIndex; // 0-based index of the grammar rule used.
    gSymbol gs;                     //ENUM Value
    tokenInfo *tkinfo;              // Info for Terminals
    struct treeNode *next;     //My Sibling on right
    struct treeNode *child;    //My first child
    struct treeNode *parent;   //My Parent
};
typedef struct treeNode treeNode;


#endif //PARSERDEF_H
