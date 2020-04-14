#include "astDef.h"
#include "symbolTableDef.h"

typedef enum{
    T_INTEGER, T_REAL, T_BOOLEAN, T_ERROR, T_UNDEFINED
} primitiveDataType;

varType* getDataType(ASTNode *ptr);

primitiveDataType getExpressionPrimitiveType(ASTNode *ptr);

void checkTypeAssignmentStmt(ASTNode* rt);

void checkType(ASTNode *root);