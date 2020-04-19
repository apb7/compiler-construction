// Group Number: 31
// MADHUR PANWAR   2016B4A70933P
// TUSSANK GUPTA   2016B3A70528P
// SALMAAN SHAHID  2016B4A70580P
// APURV BAJAJ     2016B3A70549P
// HASAN NAQVI     2016B5A70452P

#include "symbolTableDef.h"
#include "symbolTable.h"
#include "astDef.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include "typeCheck.h"
#include "symbolTable.h"


bool rhsBoundsCheckIfStatic(ASTNode *idNode, ASTNode *idOrNumNode) {
    symTableNode *arrinfoEntry = idNode->stNode ;
    symVarInfo *arrinfo = &(arrinfoEntry->info.var);

    if((arrinfo->vtype).vaType == STAT_ARR && idOrNumNode->gs == g_NUM){
        int idx = (idOrNumNode->tkinfo->value).num;
        if(!((idx >= (arrinfo->vtype).si.vt_num) && (idx <= (arrinfo->vtype).ei.vt_num))){
            //out of bounds
            return false;
        }
    }

    else if((arrinfo->vtype).vaType == VARIABLE) 
        return false;

    else if(idOrNumNode->gs == g_ID){
        symTableNode *stn = idOrNumNode->stNode;
      
        if(stn == NULL)
            return false;
        else if(stn->info.var.vtype.baseType != g_INTEGER)
            return false;
    }
    return true;
}

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

            // array[index]         
            if(ptr->next != NULL) {
                if (rhsBoundsCheckIfStatic(ptr, ptr->next) == false)
                    return T_UNDEFINED;
            }

            primitiveDataType result;
                
            if (ptr->stNode->info.var.vtype.baseType == g_INTEGER)
                result = T_INTEGER;
            else if (ptr->stNode->info.var.vtype.baseType == g_REAL)
                result =  T_REAL;
            else if (ptr->stNode->info.var.vtype.baseType == g_BOOLEAN)
                result = T_BOOLEAN;

            // ID should either be a variable or it should it indexed.
            if (ptr->stNode->info.var.vtype.vaType == VARIABLE ^ ptr->next != NULL)
                return result;
            
            return T_ERROR;            
        }
    }
}

// DONE: handle memory leaks
varType* getDataType(ASTNode *ptr) {
    switch(ptr->gs) {
        case g_ID:
        {
            if (ptr->stNode == NULL)
                return NULL;
            varType *vt = malloc(sizeof(varType));
            *vt = (ptr->stNode->info.var.vtype);
            return vt;
        }

        case g_var_id_num:
        {
            if(ptr->child->gs == g_ID) {
                if (ptr->child->stNode == NULL){
                    return NULL;
                }


                // Return array type for case A:=B
                if (ptr->child->next == NULL){
                    varType *vt = malloc(sizeof(varType));
                    *vt = (ptr->child->stNode->info.var.vtype);
                    return vt;
                }

            }
            // otherwise let it fall through the case stmt
        }

        default:
        {
            primitiveDataType expressionType = getExpressionPrimitiveType(ptr);

            if (expressionType == T_UNDEFINED)
                return NULL;

            if(expressionType == T_ERROR) {
                throwTypeError(E_EXPRESSION_ERROR, ptr->tkinfo->lno);

                return NULL;
            }

            varType *vt = malloc(sizeof(varType));

            if(expressionType == T_INTEGER)
                vt->baseType = g_INTEGER;
            
            if(expressionType == T_REAL)
                vt->baseType = g_REAL;
                
            if(expressionType == T_BOOLEAN)
                vt->baseType = g_BOOLEAN;

            vt->width = getSizeByType(vt->baseType);
            vt->vaType = VARIABLE;
            vt->si.vt_id = NULL;
            vt->ei.vt_id = NULL;

            return vt;
        }
    }
}
