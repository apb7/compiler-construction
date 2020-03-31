// Group Number: 31
// MADHUR PANWAR   2016B4A70933P
// TUSSANK GUPTA   2016B3A70528P
// SALMAAN SHAHID  2016B4A70580P
// APURV BAJAJ     2016B3A70549P
// HASAN NAQVI     2016B5A70452P

#ifndef SYMBOLTABLEDEF_H
#define SYMBOLTABLEDEF_H

#include <stdbool.h>
#include "lexerDef.h"
#include "config.h"

#define SYMBOL_TABLE_SIZE 101

//this will store the type of the variable
//sidx and eidx will be valid only when isArray is true
struct varType{
    gSymbol baseType;
    bool isArray;
    int sidx, eidx;
    int bytes;
};
typedef struct varType varType;

//for an input parameter node of a function
struct paramInpNode{
    char lexeme[30];
    varType vtype;
    unsigned int lno;    //line number
    int offset;
};

//for an output parameter node of a function
struct paramOutNode{
    char lexeme[30];
    varType vtype;
    unsigned int lno;
    int offset;
    bool isAssigned;
};

//for function's current status
typedef enum{
    F_DECLARED, F_DEFINED
} funcStatus;

typedef struct symbolTable symbolTable;

//symbol table entry for a function
struct symFuncInfo{
    funcStatus status;
    unsigned int lno;
    struct paramInpNode *inpPListHead;
    struct paramOutNode *outPListHead;
    symbolTable *st;
};

//symbol table entry for a variable
struct symVarInfo{
    unsigned int lno;
    varType vtype;
    int offset;
};

//structure for a symbol table node
struct symTableNode{
    bool isEmpty;   //to know if this position is empty in symbol table
    char lexeme[30];    //function or identifier name
    union{
        struct symFuncInfo func;
        struct symVarInfo var;
    } info;

};
typedef struct symTableNode symTableNode;

//a symbol table has an array and a linked list of nested scopes
struct symbolTable{
    symTableNode tb[SYMBOL_TABLE_SIZE]; //current scope symbol table
    symbolTable *nestedTablesHead;  //linked list of all parallel scope symbol tables
    symbolTable *parentTable;
};

symbolTable *funcTable;

#endif //SYMBOLTABLEDEF_H
