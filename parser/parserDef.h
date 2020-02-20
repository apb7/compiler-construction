#ifndef CCGIT_PARSERDEF_H
#define CCGIT_PARSERDEF_H

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

#endif //CCGIT_PARSERDEF_H
