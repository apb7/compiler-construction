typedef enum{
    T_INTEGER, T_REAL, T_BOOLEAN, T_ERROR
} dataType;

dataType get_expression_type(ASTNode *ptr) {
    switch(ptr->gs) {

        // Arithemetic operators, only binary forms called here
        case g_PILUS:
        case g_MINUS:
        case g_MUL:
        case g_DIV:
        {
            dataType t1 = get_expression_type(ptr->child);
            dataType t2 = get_expression_type(ptr->child->next);

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
            dataType t1 = get_expression_type(ptr->child);
            dataType t2 = get_expression_type(ptr->child->next);

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
            dataType t1 = get_expression_type(ptr->child);
            dataType t2 = get_expression_type(ptr->child->next);

            if (t1 == T_BOOLEAN && t2 == T_BOOLEAN)
                return T_BOOLEAN;
            
            return T_ERROR;
        }

        case g_NUM:
            return T_INTEGER;

        case g_RNUM:
            return T_REAL;

        case g_TRUE:
        case g_FALSE:
            return T_BOOLEAN;

        // TODO: get type from symbol table!

    }
}

