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



//this will store the type of the variable
//sidx and eidx will be valid only when isArray is true

typedef struct symTableNode symTableNode;
typedef struct symbolTable symbolTable;
typedef struct symFuncInfo symFuncInfo;

typedef enum{
    VARIABLE, STAT_ARR, DYN_L_ARR, DYN_R_ARR, DYN_ARR
}varOrArr;

typedef enum {
    NOT_FOR, FOR_IN, FOR_OUT
}forInfo;

union numOrId {
    unsigned int vt_num;
    symTableNode *vt_id;
};

struct varType{
    gSymbol baseType;
    varOrArr vaType;
    union numOrId si;
    union numOrId ei;
    int width;
};
typedef struct varType varType;


//symbol table entry for a variable
struct symVarInfo{
    unsigned int lno;
    varType vtype;
    int offset;
    bool isAssigned;
    forInfo forLoop;
    int whileLevel;
    bool isIOlistVar;
};
typedef struct symVarInfo symVarInfo;


typedef struct ASTNode ASTNode;

//for making a linked list of AST nodes
struct ASTNodeListNode{
    ASTNode *astNode;
    symbolTable *currST;
    symFuncInfo *callerFuncInfo;
    struct ASTNodeListNode *next;
};
typedef struct ASTNodeListNode ASTNodeListNode;

typedef symTableNode paramInpNode;
typedef symTableNode paramOutNode;

//for function's current status
typedef enum{
    F_DECLARED, F_DECLARATION_VALID, F_DEFINED
} funcStatus;

typedef enum{
    INP_PLIST, OUT_PLIST
}pListType;

//symbol table entry for a function
struct symFuncInfo{
    funcStatus status;
    char funcName[30];
    unsigned int lno;
    paramInpNode *inpPListHead;
    paramOutNode *outPListHead;
    symbolTable *st;
    ASTNodeListNode *pendingCallListHead;
    int arSize;
};


//when a function is only declared and you see a call, then put the moduleReuseStatement Node to the beginning
//of the pendingCallList after checking all IDs for their validity
//these pending calls will be fulfilled at the time of processing of function definition


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
    symbolTable *headChild;  //nestedTablesHead --linked list of all nested scope symbol tables
    symbolTable *lastChild;  //nestedTablesTail --last node of the above list (for easy last node insertions)
    symbolTable *parent; //parentTable
    symbolTable *next;  //parallel scope tables
    ASTNode *startNode; //start node from AST
    char funcName[30];
    int scopeSize;
};

typedef struct whileVarList whileVarList;
struct whileVarList {
    symTableNode* node;
    whileVarList* next;
};


#endif //SYMBOLTABLEDEF_H
