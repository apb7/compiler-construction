#ifndef PARSERDEF_H
#define PARSERDEF_H

#include "../lexer/lexerDef.h"

typedef enum{
    #define X(a,b) g_ ## a,
    #define K(a,b,c) g_ ## a,
    #include "../data/keywords.txt"
    #include "../data/tokens.txt"

    g_EPS,
    g_EOS,
    #include "../data/nonTerminals.txt"

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
    gSymbol lhs;    //non terminal symbol
    rhsNode *head;
} grammarNode;

struct treeNode{
    gSymbol tk;                     //ENUM Value
    tokenInfo *tkinfo;              // Info for Terminals
    struct treeNode *next;     //My Sibling on right
    struct treeNode *child;    //My first child
    struct treeNode *parent;   //My Parent
};
typedef struct treeNode treeNode;

//typedef enum {
//    TT, NTT, NO_ERROR
//} syntaxErrorType;
//typedef struct{
//    syntaxErrorType errType;
//    tokenInfo *tkinfo;
//    gSymbol stackTopSymbol;
//} syntaxError;

#endif //PARSERDEF_H
