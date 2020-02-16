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

typedef enum {
    NT_PROGRAM
} nonTerminalType;

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
