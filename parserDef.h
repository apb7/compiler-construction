#ifndef CCGIT_PARSERDEF_H
#define CCGIT_PARSERDEF_H

typedef enum{
    #define X(a) g_ ## a,
    #include "data/tokens.txt"
    g_EOS,
    #include "data/nonTerminals.txt"
    #undef X
    g_numSymbols
} gSymbol;
//g_EPS is for Epsilon
//g_EOS is end of string

typedef enum {
#define X(a) nt_ ## a,
#include "data/nonTerminals.txt"
#undef X
    nt_numNonTerminals
} nonTerminalType;

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

#endif //CCGIT_PARSERDEF_H
