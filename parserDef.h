#ifndef CCGIT_PARSERDEF_H
#define CCGIT_PARSERDEF_H

typedef enum {
    INTEGER, REAL, BOOLEAN, OF, ARRAY, START, END, DECLARE, MODULE, DRIVER,
    PROGRAM, DRIVERDEF, DRIVERENDDEF, GET_VALUE, PRINT, USE, WITH, PARAMETERS,
    TRUE, FALSE, TAKES, INPUT, RETURNS, AND, OR, FOR, IN, SWITCH, CASE, BREAK,
    DEFAULT, WHILE, PLUS, MINUS, MUL, DIV, LT, LE, GE, GT, EQ, NE, DEF, ENDDEF,
    COLON, RANGEOP, SEMICOL, COMMA, ASSIGNOP, SQBO, SQBC, BO, BC, COMMENTMARK,
    ID, NUM, RNUM
} terminalType;

typedef enum{
    #define X(a) g_ ## a,
    #include "data/tokens.txt"
    g_EOS,
    g_EPS,
    #include "data/nonTerminals.txt"
    #undef X
    g_numSymbols
} gSymbol;
//g_EPS is for Epsilon

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



typedef enum {
    S_T,S_NT
} symbolType;

typedef struct{
    symbolType tag;
    char str[25];
    union {
        terminalType t;
        nonTerminalType nt;
    } sym;
} mappingTable[131];    //113 around symbols



#endif //CCGIT_PARSERDEF_H
