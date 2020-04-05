#include "symbolTableDef.h"
#include <stdio.h>
#include <stdlib.h>

typedef enum{
    T_INTEGER, T_REAL, T_BOOLEAN, T_ERROR
} dataType;

dataType getExpressionType(ASTNode *ptr) {
    switch(ptr->gs) {

        // Arithemetic operators, only binary forms called here
        case g_PLUS:
        case g_MINUS:
        case g_MUL:
        case g_DIV:
        {
            dataType t1 = getExpressionType(ptr->child);
            dataType t2 = getExpressionType(ptr->child->next);

            if (t1 == T_INTEGER && t2 == T_INTEGER)
                return T_INTEGER;

            if (t1 == T_REAL && t2 == T_REAL)
                return T_REAL;

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
            dataType t1 = getExpressionType(ptr->child);
            dataType t2 = getExpressionType(ptr->child->next);

            if (t1 == T_INTEGER && t2 == T_INTEGER)
                return T_BOOLEAN;

            if (t1 == T_REAL && t2 == T_REAL)
                return T_BOOLEAN;

            return T_ERROR;
        }

        // Logical operators
        case g_AND:
        case g_OR:
        {
            dataType t1 = getExpressionType(ptr->child);
            dataType t2 = getExpressionType(ptr->child->next);

            if (t1 == T_BOOLEAN && t2 == T_BOOLEAN)
                return T_BOOLEAN;

            return T_ERROR;
        }

        case g_u:
        {
            // t1 will be g_unary_op
            dataType t2 = getExpressionType(ptr->child->next); // g_ of either arithmetic, relational or logical operators.

            // Cannot have either boolean or error type expression with unary operator.
            if(t2 == T_BOOLEAN || t2 == T_ERROR)
                return T_ERROR;
        }

        case g_NUM:
            return T_INTEGER;

        case g_RNUM:
            return T_REAL;

        case g_TRUE:
        case g_FALSE:
            return T_BOOLEAN;

        case g_ID:
            // TODO: get type from symbol table!

    }
}

void checkTypeAssignmentStmt(ASTNode* rt) {
    rt = rt->child;

    if(rt->gs == g_lvalueIDStmt) {
        rt = rt->child; // g_ASSIGNOP

        dataType t1 = getExpressionType(rt->child); // g_ID
        dataType t2 = getExpressionType(rt->child->next);  // g_expression

        if(t1 == T_ERROR || t2 == T_ERROR || t1 != t2)
            printf("\n ERROR: The types of left and right hand side of assignment operator are not same at line %d.", rt->tkinfo->lno);
    }
    else { // g_lvalueARRStmt
        rt = rt->child; // g_ASSIGNOP

        dataType t1 = getExpressionType(rt->child); // g_ID
        dataType t2 = getExpressionType(rt->child->next);  // g_index
        dataType t3 = getExpressionType(rt->child->next->next);  // g_expression

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

