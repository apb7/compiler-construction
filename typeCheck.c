typedef enum{
    T_INTEGER, T_REAL, T_BOOLEAN, T_ERROR
} dataType;

dataType get_expression_type(ASTNode *ptr) {
    switch(ptr->gs) {

        // ALWAYS binary operators called here
        case g_PILUS:
        case g_MINUS:
        case g_MUL:
        case g_DIV:

        case g_LT:
        case g_LE:
        case g_GT:
        case g_GE:
        case g_EQ:
        case g_NE:
        {
            dataType t1 = get_expression_type(ptr->child);
            dataType t2 = get_expression_type(ptr->child->next);

            if (t1 == T_INTEGER && t2 == T_INTEGER)
                return T_INTEGER;

            if (t1 == T_REAL && t2 == T_REAL)
                return T_REAL;
            
            return T_ERROR;
        }

        case g_AND:
        case g_OR:
        {
            dataType t1 = get_expression_type(ptr->child);
            dataType t2 = get_expression_type(ptr->child->next);

            if (t1 == T_BOOLEAN && t2 == T_BOOLEAN)
                return T_BOOLEAN;
            
            return T_ERROR;
        }



    }
}

