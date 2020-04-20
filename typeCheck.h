// Group Number: 31
// MADHUR PANWAR   2016B4A70933P
// TUSSANK GUPTA   2016B3A70528P
// SALMAAN SHAHID  2016B4A70580P
// APURV BAJAJ     2016B3A70549P
// HASAN NAQVI     2016B5A70452P

#include "astDef.h"
#include "symbolTableDef.h"

typedef enum{
    T_INTEGER, T_REAL, T_BOOLEAN, T_ERROR, T_UNDEFINED
} primitiveDataType;

varType* getDataType(ASTNode *ptr);
primitiveDataType getExpressionPrimitiveType(ASTNode *ptr);
