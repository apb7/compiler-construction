#include "symbolTableDef.h"
#include "symbolTable.h"
#include "astDef.h"
#include <stdio.h>
#include <stdlib.h>

typedef enum{
    T_INTEGER, T_REAL, T_BOOLEAN, T_ERROR, T_UNDEFINED
} primitiveDataType;

extern char* inverseMappingTable[];

primitiveDataType getExpressionPrimitiveType(ASTNode *ptr) {
    switch(ptr->gs) {

        // Arithemetic operators, only binary forms called here
        case g_PLUS:
        case g_MINUS:
        case g_MUL:
        case g_DIV:
        {
            primitiveDataType t1 = getExpressionPrimitiveType(ptr->child);
            primitiveDataType t2 = getExpressionPrimitiveType(ptr->child->next);

            if (t1 == T_INTEGER && t2 == T_INTEGER)
                return T_INTEGER;

            if (t1 == T_REAL && t2 == T_REAL)
                return T_REAL;
            
            if(t1 == T_UNDEFINED || t2 == T_UNDEFINED)
                return T_UNDEFINED;
            
            return T_ERROR;
        }

        // Relational operators
        case g_LT:
        case g_LE:
        case g_GT:
        case g_GE:
        case g_EQ:
        case g_NE:
        {
            primitiveDataType t1 = getExpressionPrimitiveType(ptr->child);
            primitiveDataType t2 = getExpressionPrimitiveType(ptr->child->next);

            if (t1 == T_INTEGER && t2 == T_INTEGER)
                return T_BOOLEAN;

            if (t1 == T_REAL && t2 == T_REAL)
                return T_BOOLEAN;

            if(t1 == T_UNDEFINED || t2 == T_UNDEFINED)
                return T_UNDEFINED;

            return T_ERROR;
        }

        // Logical operators
        case g_AND:
        case g_OR:
        {
            primitiveDataType t1 = getExpressionPrimitiveType(ptr->child);
            primitiveDataType t2 = getExpressionPrimitiveType(ptr->child->next);

            if (t1 == T_BOOLEAN && t2 == T_BOOLEAN)
                return T_BOOLEAN;

            if(t1 == T_UNDEFINED || t2 == T_UNDEFINED)
                return T_UNDEFINED;

            return T_ERROR;
        }

        case g_u:
        {
            // t1 will be g_unary_op
            primitiveDataType t2 = getExpressionPrimitiveType(ptr->child->next); // g_ of either arithmetic, relational or logical operators.

            // Can only have real or integer expression with unary operator.
            if(t2 == T_REAL || t2 == T_INTEGER || t2 == T_UNDEFINED)
                return t2;

            return T_ERROR;
        }

        case g_var_id_num:
            return getExpressionPrimitiveType(ptr->child);

        case g_NUM:
            return T_INTEGER;

        case g_RNUM:
            return T_REAL;

        case g_TRUE:
        case g_FALSE:
            return T_BOOLEAN;

        case g_ID:
        {
            if (ptr->stNode == NULL)
                return T_UNDEFINED;
                
            if (ptr->stNode->info.var.vtype.baseType == g_INTEGER)
                return T_INTEGER;
            
            if (ptr->stNode->info.var.vtype.baseType == g_REAL)
                return T_REAL;
            
            if (ptr->stNode->info.var.vtype.baseType == g_BOOLEAN)
                return T_BOOLEAN;
        }
    }
}

// TODO: handle memory leaks
varType* getDataType(ASTNode *ptr) {
    switch(ptr->gs) {
        case g_ID:
        {
            if (ptr->stNode == NULL)
                return NULL;

            return &(ptr->stNode->info.var.vtype);
        }

        default:
        {
            primitiveDataType expressionType = getExpressionPrimitiveType(ptr);

            if (expressionType == T_UNDEFINED)
                return NULL;

            if(expressionType == T_ERROR) {
                printf("Invalid expression type! at line no %d \n", ptr->tkinfo->lno);
                return NULL;
            }

            varType *vt = malloc(sizeof(varType));

            if(expressionType == T_INTEGER)
                vt->baseType = g_INTEGER;
            
            if(expressionType == T_REAL)
                vt->baseType = g_REAL;
                
            if(expressionType == T_BOOLEAN)
                vt->baseType = g_BOOLEAN;

            vt->bytes = getSizeByType(vt->baseType);
            vt->vaType = VARIABLE;
            vt->si.vt_id = NULL;
            vt->ei.vt_id = NULL;

            return vt;
        }
    }
}
/*
void checkTypeAssignmentStmt(ASTNode* rt) {
    rt = rt->child;

    if(rt->gs == g_lvalueIDStmt) {
        rt = rt->child; // g_ASSIGNOP

        varType *t1 = getDataType(rt->child); // g_ID
        varType *t2 = getDataType(rt->child->next);  // g_expression

        if(t2 == NULL) {
            printf(" TYPE ERROR: Expression has type error at line %d.\n", rt->tkinfo->lno);
            return;    
        }

        if(t1->baseType == t2->baseType && t1->vaType == t2->vaType) {
            if(t1->vaType == VARIABLE)
                return; // No error
            else
                printf("to do other vatypes\n");
            return;
        }

        else {
            printf(" TYPE ERROR: LHS and RHS tpes don't match at line %d.\n", rt->tkinfo->lno);
        }
    }

    else { // g_lvalueARRStmt
        rt = rt->child; // g_ASSIGNOP

        primitiveDataType t1 = getExpressionPrimitiveType(rt->child); // g_ID
        primitiveDataType t2 = getExpressionPrimitiveType(rt->child->next);  // g_index
        primitiveDataType t3 = getExpressionPrimitiveType(rt->child->next->next);  // g_expression

        // TODO: Additional checks needed for array A[k]
        if(t1 == T_ERROR || t2 == T_ERROR || t1 != t2)
            printf("\n ERROR: The types of left and right hand side of assignment operator are not same at line %d.", rt->tkinfo->lno);
    }
}

void checkType(ASTNode *root) {
    if(root == NULL)
        return;

    switch(root->gs)
    {
        case g_assignmentStmt:
            checkTypeAssignmentStmt(root);

        // No break because we need to check all nodes.
        default:
        {
            checkType(root->child);
            checkType(root->next);
        }
    }
}
*/
