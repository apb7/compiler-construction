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
#include "astDef.h"

#define SYMBOL_TABLE_SIZE 101

//this will store the type of the variable
//sidx and eidx will be valid only when isArray is true

typedef struct symTableNode symTableNode;

typedef enum{
    VARIABLE, STAT_ARR, DYN_L_ARR, DYN_R_ARR, DYN_ARR
}varOrArr;

union numOrId {
    int vt_num;
    symTableNode *vt_id;
};

struct varType{
    gSymbol baseType;
    varOrArr vaType;
    union numOrId si;
    union numOrId ei;
    int bytes;
};
typedef struct varType varType;

//for making a linked list of AST nodes
struct ASTNodeListNode{
    ASTNode *astNode;
    struct ASTNodeListNode *next;
};
typedef struct ASTNodeListNode ASTNodeListNode;

//for an input parameter node of a function
struct paramInpNode{
    char lexeme[30];
    varType vtype;
    unsigned int lno;    //line number
    int offset;
    struct paramInpNode *next;
};
typedef struct paramInpNode paramInpNode;

//for an output parameter node of a function
struct paramOutNode{
    char lexeme[30];
    varType vtype;
    unsigned int lno;
    int offset;
    bool isAssigned;
    struct paramOutNode *next;
};
typedef struct paramOutNode paramOutNode;

//for function's current status
typedef enum{
    F_DECLARED, F_DECLARATION_VALID, F_DEFINED
} funcStatus;

typedef struct symbolTable symbolTable;

//symbol table entry for a function
struct symFuncInfo{
    funcStatus status;
    char funcName[30];
    unsigned int lno;
    struct paramInpNode *inpPListHead;
    struct paramOutNode *outPListHead;
    symbolTable *st;
    ASTNodeListNode *pendingCallListHead;
};
typedef struct symFuncInfo symFuncInfo;

//when a function is only declared and you see a call, then put the moduleReuseStatement Node to the beginning
//of the pendingCallList after checking all IDs for there validity
//these pending calls will be fulfilled at the time of processing of function definition


//symbol table entry for a variable
struct symVarInfo{
    unsigned int lno;
    varType vtype;
    int offset;
};
typedef struct symVarInfo symVarInfo;

union funcVar{
    struct symFuncInfo func;
    struct symVarInfo var;
};

//structure for a symbol table node
struct symTableNode{
    char lexeme[30];    //function or identifier name
    union funcVar info;
    struct symTableNode *next;
};

//a symbol table has an array and a linked list of nested scopes
struct symbolTable{
    symTableNode *tb[SYMBOL_TABLE_SIZE]; //current scope symbol table
    symbolTable *headChild;  //nestedTablesHead --linked list of all parallel scope symbol tables
    symbolTable *lastChild;  //nestedTablesTail --last node of the above list (for easy last node insertions)
    symbolTable *parent; //parentTable
    symbolTable *next;  //parallel scope tables
};


#endif //SYMBOLTABLEDEF_H