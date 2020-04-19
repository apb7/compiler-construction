#include "astDef.h"
#include "lexerDef.h"
#include "parserDef.h"
#include "symbolTableDef.h"

#include <stdio.h>
#include <stdlib.h>

extern char *inverseMappingTable[];

ASTNode* buildASTTree(parseNode* parseNodeRoot);


ASTNode* createASTNode(parseNode *parseNode) {
    ASTNode *newNode = malloc(sizeof(ASTNode));

    newNode->gs = parseNode->gs;

    // TODO : copy token if its being freed
    newNode->tkinfo = parseNode->tkinfo;
    newNode->start_line_no = -1;
    newNode->end_line_no = -1;
    newNode->next = NULL;
    newNode->child = NULL;
    newNode->parent = NULL;
    newNode->stNode = NULL;
}


ASTNode* create_self_node_with_single_child(parseNode *parseNodeRoot) {

    ASTNode *newNode = createASTNode(parseNodeRoot);

    ASTNode *AST_child = buildASTTree(parseNodeRoot->child);
    // Beware of SegFault. Use this function carefully.
    AST_child->parent = newNode;
    newNode->child = AST_child;

    return newNode;
}

// Use for the rules of the following format:
// <N4> -> <op1> <term> <N41>
ASTNode* createASTNodeForRightRecursiveRule(parseNode *parseNodeRoot, gSymbol g_op1, gSymbol g_op2) {
    gSymbol g_t = parseNodeRoot->child->next->gs; // Either g_anyTerm, g_term or g_factor.

    ASTNode* AST_N1 = buildASTTree(parseNodeRoot->child->next->next); // <N1>

    ASTNode* ASTOp = buildASTTree(parseNodeRoot->child);
    ASTNode* ASTt1 = (parseNodeRoot->parent->child->gs == g_t) ? buildASTTree(parseNodeRoot->parent->child) : NULL;
    ASTNode* ASTt2 = buildASTTree(parseNodeRoot->child->next);

    if(ASTt1) {
        ASTOp->child = ASTt1;
        ASTt1->parent = ASTOp;
        ASTt1->next = ASTt2;
        ASTt2->parent = ASTOp;
    }
    else {
        ASTOp->child = ASTt2;
        ASTt2->parent = ASTOp;
    }

    if(AST_N1 == NULL)
        return ASTOp;

    ASTNode* cur = AST_N1;

    // Go to the lowest level where an operator's children are not already filled.
    // Example, b := 11 - 10 + (12 - 1);
    while((cur->child->gs == g_op1 || cur->child->gs == g_op2) && (cur->child->next != NULL)) {
        cur = cur->child;
    }

    ASTOp->next = cur->child;
    ASTOp->parent = cur;
    cur->child = ASTOp;

    return AST_N1;
}

// Rules of form <elems> -> <elem> <elems> 
// stop_condition is rule no for rule of form <elems> -> EPS 
ASTNode* simple_recursive_list_non_empty(parseNode *parseNodeRoot, int stop_condition) {

    parseNode *parse_child = parseNodeRoot->child; // <elem>

    ASTNode *AST_child = buildASTTree(parse_child);
    
    // elem exists, can't return NULL 
    ASTNode *newNode = createASTNode(parseNodeRoot);
    AST_child->parent = newNode;
    newNode->child = AST_child;

    parse_child = parse_child->next; // <elems>

    // List of all elems until EPS encountered
    while(parse_child->gRuleIndex + 2 != stop_condition) {

        parse_child = parse_child->child; // <elem>

        AST_child->next = buildASTTree(parse_child);
        AST_child = AST_child->next;
        AST_child->parent = newNode;

        parse_child = parse_child->next; // <elems>
    }

    return newNode;
}


ASTNode* buildASTTree(parseNode* parseNodeRoot) {

    if (parseNodeRoot == NULL) {
        printf("NULL pointer provided to buildASTTree(..).\n");
        return NULL;
    }

    // printf("Rule %d %s\n", parseNodeRoot->gRuleIndex + 2, inverseMappingTable[ parseNodeRoot->gs ]);

    switch(parseNodeRoot->gRuleIndex + 2) {

        // leaf node, either terminal or EPS
        case 1:
        {
            if(parseNodeRoot->gs == g_EPS)
                return NULL;

            ASTNode *newNode = createASTNode(parseNodeRoot);
            return newNode;
        }

        // <program> -> <moduleDeclarations> <otherModules> <driverModule> <otherModules>
        case 2:
        {
            ASTNode *AST_child;
            parseNode *parse_child = parseNodeRoot->child;

            do {
                AST_child = buildASTTree(parse_child);
                parse_child = parse_child->next;
            } while(AST_child == NULL && parse_child != NULL);

            // Empty program !
            if (parse_child == NULL)
                return NULL;

            ASTNode *newNode = createASTNode(parseNodeRoot);
            AST_child->parent = newNode;
            newNode->child = AST_child;

            while(parse_child != NULL) {
                ASTNode *AST_sibling = buildASTTree(parse_child);
                
                if (AST_sibling != NULL) {
                    AST_child->next = AST_sibling;
                    AST_sibling->parent = newNode;
                    AST_child = AST_child->next;
                }

                parse_child = parse_child->next;
            }

            return newNode;
        }

        // <moduleDeclarations> -> <moduleDeclaration> <moduleDeclarations>
        // <moduleDeclarations> -> EPS
        case 3:
            return simple_recursive_list_non_empty(parseNodeRoot, 4);

        // <moduleDeclarations> -> EPS
        case 4:
            return NULL;
        
        // <moduleDeclaration> -> DECLARE MODULE ID SEMICOL
        case 5:
        {
            parseNode *parse_child = parseNodeRoot->child->next->next; // <ID>

            return buildASTTree(parse_child);
        }

        // <otherModules> -> <module> <otherModules>
        // <otherModules> -> EPS
        case 6:
            return simple_recursive_list_non_empty(parseNodeRoot, 7);

        // <otherModules> -> EPS
        case 7:
            return NULL;

        // <driverModule> -> DRIVERDEF DRIVER PROGRAM DRIVERENDDEF <moduleDef>
        case 8:
        {
            parseNode *parse_child = parseNodeRoot->child;

            while(parse_child->next != NULL) {
                parse_child = parse_child->next;
            }

            ASTNode *AST_child = buildASTTree(parse_child);
            
            if(AST_child == NULL)
                return NULL;
            
            ASTNode *newNode = createASTNode(parseNodeRoot->child->next); // DRIVER
            AST_child->parent = newNode;
            newNode->child = AST_child;

            return newNode;
        }

        // <module> -> DEF MODULE ID ENDDEF TAKES INPUT SQBO <input_plist> SQBC SEMICOL <ret> <moduleDef>
        case 9:
        {
            ASTNode *newNode = createASTNode(parseNodeRoot);

            parseNode *parse_child = parseNodeRoot->child->next->next; // <ID> 1

            ASTNode *AST_child = buildASTTree(parse_child);
            AST_child->parent = newNode;
            newNode->child = AST_child;

            parse_child = parse_child->next->next->next->next->next; // <input_plist>

            ASTNode *AST_sibling = buildASTTree(parse_child);
            AST_sibling->parent = newNode;
            AST_child->next = AST_sibling;
            AST_child = AST_child->next;
            
            parse_child = parse_child->next->next->next; // <ret>

            AST_sibling = buildASTTree(parse_child);

            if(AST_sibling) {
                AST_sibling->parent = newNode;
                AST_child->next = AST_sibling;
                AST_child = AST_child->next;
            }

            parse_child = parse_child->next; // <moduleDef>

            AST_sibling = buildASTTree(parse_child);

            if(AST_sibling) {
                AST_sibling->parent = newNode;
                AST_child->next = AST_sibling;
                AST_child = AST_child->next;
            }            
            
            return newNode;
        }

        // <ret> -> RETURNS SQBO <output_plist> SQBC SEMICOL
        case 10:
        {
            parseNode *parse_child = parseNodeRoot->child->next->next; // <output_plist> 
            return buildASTTree(parse_child);
        }

        // <ret> -> ε
        case 11:
            return NULL;

        // <input_plist> -> ID COLON <dataType> <N1>
        // <N1> -> COMMA ID COLON <dataType> <N1>
        // <N1> -> ε
        case 12:
        {
            ASTNode *newNode = createASTNode(parseNodeRoot);

            parseNode *parse_child = parseNodeRoot->child; // ID
            ASTNode *AST_child = buildASTTree(parse_child);
            AST_child->parent = newNode;
            newNode->child = AST_child;

            parse_child = parse_child->next->next; // <dataType>
            ASTNode *AST_sibling = buildASTTree(parse_child);
            AST_sibling->parent = newNode;
            AST_child->next = AST_sibling;
            
            AST_child = AST_child->next;

            parse_child = parse_child->next; // <N1>

            // List of all (ID  <dataType>) pairs until EPS encountered
            while(parse_child->gRuleIndex + 2 != 14) {

                parse_child = parse_child->child->next; // ID

                AST_child->next = buildASTTree(parse_child);
                AST_child = AST_child->next;
                AST_child->parent = newNode;

                parse_child = parse_child->next->next; // <dataType>

                AST_child->next = buildASTTree(parse_child);
                AST_child = AST_child->next;
                AST_child->parent = newNode;

                parse_child = parse_child->next; // <N1>

                // TO REMOVE!!!
                if(parse_child == NULL) {
                    printf("Error within rule 57\n");
                    return NULL;
                }
            }
            return newNode;
        }

        // <output_plist> -> ID COLON <type> <N2>
        // <N2> -> COMMA ID COLON <type> <N2>
        // <N2> -> ε
        case 15:
        {
            ASTNode *newNode = createASTNode(parseNodeRoot);

            parseNode *parse_child = parseNodeRoot->child; // ID
            ASTNode *AST_child = buildASTTree(parse_child);
            AST_child->parent = newNode;
            newNode->child = AST_child;

            parse_child = parse_child->next->next; // <type>
            ASTNode *AST_sibling = buildASTTree(parse_child);
            AST_sibling->parent = newNode;
            AST_child->next = AST_sibling;
            
            AST_child = AST_child->next;

            parse_child = parse_child->next; // <N2>

            // List of all (ID  <type>) pairs until EPS encountered
            while(parse_child->gRuleIndex + 2 != 17) {

                parse_child = parse_child->child->next; // ID

                AST_child->next = buildASTTree(parse_child);
                AST_child = AST_child->next;
                AST_child->parent = newNode;

                parse_child = parse_child->next->next; // <type>

                AST_child->next = buildASTTree(parse_child);
                AST_child = AST_child->next;
                AST_child->parent = newNode;

                parse_child = parse_child->next; // <N2>

                // TO REMOVE!!!
                if(parse_child == NULL) {
                    printf("Error within rule 57\n");
                    return NULL;
                }
            }
            return newNode;
        }

        // <dataType> -> INTEGER
        case 18:
        // <dataType> -> REAL
        case 19:
        // <dataType> -> BOOLEAN
        case 20:
            return create_self_node_with_single_child(parseNodeRoot);

        // <dataType> -> ARRAY SQBO <range_arrays> SQBC OF <type>
        case 21:
        {
            ASTNode *newNode = createASTNode(parseNodeRoot);

            parseNode *parse_child = parseNodeRoot->child->next->next; // <range_arrays>

            ASTNode *AST_child = buildASTTree(parse_child);
            AST_child->parent = newNode;
            newNode->child = AST_child;

            parse_child = parse_child->next->next->next; // <type>

            ASTNode *AST_sibling = buildASTTree(parse_child);
            AST_sibling->parent = newNode;
            AST_child->next = AST_sibling;

            return newNode;
        }
        
        // <range_arrays> -> <index> RANGEOP <index>
        case 22:
        {
            ASTNode *newNode = createASTNode(parseNodeRoot);

            parseNode *parse_child = parseNodeRoot->child; // <index> 1

            ASTNode *AST_child = buildASTTree(parse_child);
            AST_child->parent = newNode;
            newNode->child = AST_child;

            parse_child = parse_child->next->next; // <index> 2

            ASTNode *AST_sibling = buildASTTree(parse_child);
            AST_sibling->parent = newNode;
            AST_child->next = AST_sibling;

            return newNode;
        }

        // <type> -> INTEGER
        case 23:
        // <type> -> REAL
        case 24:
        // <type> -> BOOLEAN
        case 25:
            return buildASTTree(parseNodeRoot->child);

        // Doubt(apb7): Do we need a node for <moduleDef> ? Yes, according to Ma'am.
        // <moduleDef> -> START <statements> END 
        case 26:
        {
            parseNode *parse_child = parseNodeRoot->child->next; // <statements>

            ASTNode *AST_grandchild = buildASTTree(parse_child);

            if(AST_grandchild == NULL)
                return NULL;

            ASTNode *newNode = createASTNode(parseNodeRoot);
            parse_child = parseNodeRoot->child; // START

            ASTNode *AST_child = buildASTTree(parse_child);

            AST_child->parent = newNode;
            newNode->child = AST_child;
            AST_child->start_line_no = AST_child->tkinfo->lno;
            AST_child->end_line_no = parse_child->next->next->tkinfo->lno;

            AST_grandchild->parent = AST_child;
            AST_child->child = AST_grandchild;

            return newNode;
        }

        // <statements> -> <statement> <statements>
        // <statements> -> EPS
        case 27:
            return simple_recursive_list_non_empty(parseNodeRoot, 28);

        // <statements> -> EPS
        case 28:
            return NULL;

        // <statement> -> <ioStmt>
        case 29:
        // <statement> -> <simpleStmt>
        case 30:
        // <statement> -> <declareStmt>
        case 31:
        // <statement> -> <conditionalStmt>
        case 32:
        // <statement> -> <iterativeStmt>
        case 33:
            return buildASTTree(parseNodeRoot->child);

        // For design uniformity in <statement>, create <ioStmt> node.
        // We retain the GET_VALUE node for ioStmt.
        // <ioStmt> -> GET_VALUE BO ID BC SEMICOL
        case 34:
        {
            ASTNode *newNode = createASTNode(parseNodeRoot);

            parseNode *parse_child = parseNodeRoot->child; // GET_VALUE

            ASTNode *AST_child = buildASTTree(parse_child);
            AST_child->parent = newNode;
            newNode->child = AST_child;

            parse_child = parse_child->next->next; // ID
            AST_child->next = buildASTTree(parse_child);
            AST_child = AST_child->next;
            AST_child->parent = newNode;

            return newNode;
        }

        // <ioStmt> -> PRINT BO <var> BC SEMICOL
        case 35:
        {
            ASTNode *newNode = createASTNode(parseNodeRoot);

            parseNode *parse_child = parseNodeRoot->child; // PRINT

            ASTNode *AST_child = buildASTTree(parse_child);
            AST_child->parent = newNode;
            newNode->child = AST_child;

            parse_child = parse_child->next->next; // <var>
            AST_child->next = buildASTTree(parse_child);
            AST_child = AST_child->next;
            AST_child->parent = newNode;

            return newNode;
        }

        case 36: // <boolConstt> -> TRUE
        case 37: // <boolConstt> -> FALSE
            return buildASTTree(parseNodeRoot->child);

        // <var_id_num> -> ID <whichId>
        case 38:
        {
            ASTNode *newNode = createASTNode(parseNodeRoot);

            parseNode *parse_child = parseNodeRoot->child;

            ASTNode *AST_child = buildASTTree(parse_child);
            AST_child->parent = newNode;
            newNode->child = AST_child;

            ASTNode *AST_sibling = buildASTTree(parse_child->next);

            if (AST_sibling) {
                AST_sibling->parent = newNode;
                AST_child->next = AST_sibling;
            }

            return newNode;
        }

        // Keep <var_id_num> node for uniformity.
        case 39: // <var_id_num> -> NUM
        case 40: // <var_id_num> -> RNUM
            return create_self_node_with_single_child(parseNodeRoot);

        case 41: // <var> -> <var_id_num>
        case 42: // <var> -> <boolConstt>
            return buildASTTree(parseNodeRoot->child);

        // <whichId> -> SQBO <index> SQBC
        case 43:
            return buildASTTree(parseNodeRoot->child->next);

        // <whichId> -> ε
        case 44:
            return NULL;
        
        // <simpleStmt> -> <assignmentStmt>
        case 45:
        // <simpleStmt> -> <moduleReuseStmt>
        case 46:
            return create_self_node_with_single_child(parseNodeRoot);

        // <assignmentStmt> -> ID <whichStmt>
        // <whichStmt> -> <lvalueIDStmt>
        // <whichStmt> -> <lvalueARRStmt>
        // <lvalueIDStmt> -> ASSIGNOP <expression> SEMICOL
        // <lvalueARRStmt> -> SQBO <index> SQBC ASSIGNOP <expression> SEMICOL
        case 47:
        {
            ASTNode *newNode = createASTNode(parseNodeRoot);
            parseNode *parse_child = parseNodeRoot->child; // ID

            ASTNode *AST_lhs = buildASTTree(parse_child);

            parse_child = parse_child->next; // <whichStmt>

            ASTNode *AST_child = buildASTTree(parse_child); // either <lvalueIDStmt> or <lvalueARRStmt>
            newNode->child = AST_child;
            AST_child->parent = newNode;

            ASTNode* AST_grandgrandchild = AST_child->child->child;
            AST_child->child->child = AST_lhs;
            AST_lhs->parent = AST_child->child;
            AST_lhs->next = AST_grandgrandchild;
            
            return newNode;
        }

        // <whichStmt> -> <lvalueIDStmt>
        case 48:
        // <whichStmt> -> <lvalueARRStmt>
        case 49:
            return buildASTTree(parseNodeRoot->child);

        // <lvalueIDStmt> -> ASSIGNOP <expression> SEMICOL
        case 50:
        {
            ASTNode *newNode = createASTNode(parseNodeRoot);

            parseNode *parse_child = parseNodeRoot->child; // ASSIGNOP

            ASTNode *AST_child = buildASTTree(parse_child);
            AST_child->parent = newNode;
            newNode->child = AST_child;

            parse_child = parse_child->next; // <expression>
            
            ASTNode *AST_grandchild = buildASTTree(parse_child);

            AST_child->child = AST_grandchild;
            AST_grandchild->parent = AST_child;

            return newNode;
        }

        // <lvalueARRStmt> -> SQBO <index> SQBC ASSIGNOP <expression> SEMICOL
        case 51:
        {
            ASTNode *newNode = createASTNode(parseNodeRoot);

            parseNode *parse_child = parseNodeRoot->child->next->next->next; // <ASSIGNOP>

            ASTNode *AST_child = buildASTTree(parse_child);
            AST_child->parent = newNode;
            newNode->child = AST_child;

            parse_child = parseNodeRoot->child->next; // <index>
            
            ASTNode *AST_grandchild = buildASTTree(parse_child);
            AST_child->child = AST_grandchild;
            AST_grandchild->parent = AST_child;

            parse_child = parse_child->next->next->next; // <expression>
            
            AST_grandchild->next = buildASTTree(parse_child);
            AST_grandchild = AST_grandchild->next;
            AST_grandchild->parent = AST_child;

            return newNode;
        }

        // <index> -> NUM
        case 52:
        // <index> -> ID
        case 53:
            return buildASTTree(parseNodeRoot->child);

        // <moduleReuseStmt> -> <optional> USE MODULE ID WITH PARAMETERS <idList> SEMICOL
        // <optional> -> SQBO <idList> SQBC ASSIGNOP
        // <optional> -> ε
        case 54:
        {
            ASTNode *newNode = createASTNode(parseNodeRoot);
            parseNode *parseChild = parseNodeRoot->child; // <optional>

            // <optional> -> ε
            if(parseChild->gRuleIndex + 2 == 56) {

                // Skip USE MODULE
                parseChild = (((parseChild->next)->next)->next); // ID
                ASTNode *ASTChild = buildASTTree(parseChild);
                ASTChild->parent = newNode;
                newNode->child = ASTChild;

                // Skip WITH PARAMETERS
                parseChild = (((parseChild->next)->next)->next); // <idList>
                ASTChild->next = buildASTTree(parseChild);
                ASTChild = ASTChild->next;
                ASTChild->parent = newNode;

            }
            else { // <optional> -> SQBO <idList> SQBC ASSIGNOP

                parseNode *parseGrandChild = parseChild->child;
                // Skip SQBO
                parseGrandChild = (parseGrandChild->next); // <idList>
                ASTNode *ASTGrandChild = buildASTTree(parseGrandChild);

                // Skip SQBC
                parseGrandChild = (parseGrandChild->next)->next; // ASSIGNOP
                ASTNode *ASTChild = buildASTTree(parseGrandChild);

                ASTChild->parent = newNode;
                newNode->child = ASTChild;

                ASTGrandChild->parent = ASTChild;
                ASTChild->child = ASTGrandChild;

                // Skip USE MODULE
                parseChild = (((parseChild->next)->next)->next); // ID
                ASTGrandChild->next = buildASTTree(parseChild);
                ASTGrandChild = ASTGrandChild->next;
                ASTGrandChild->parent = ASTChild;

                // Skip WITH PARAMETERS
                parseChild = (((parseChild->next)->next)->next); // <idList>
                ASTGrandChild->next = buildASTTree(parseChild);
                ASTGrandChild = ASTGrandChild->next;
                ASTGrandChild->parent = ASTChild;
            }

            return newNode;
        }

        // <idList> -> ID <N3>
        // <N3> -> COMMA ID <N3>
        // <N3> -> ε
        case 57:
        {
            ASTNode *newNode = createASTNode(parseNodeRoot);

            parseNode *parse_child = parseNodeRoot->child; // ID
            ASTNode *AST_child = buildASTTree(parse_child);

            AST_child->parent = newNode;
            newNode->child = AST_child;

            parse_child = parse_child->next; // N3

            // List of all IDS until EPS encountered
            while(parse_child->gRuleIndex + 2 != 59) {

                parse_child = parse_child->child->next; //ID

                AST_child->next = buildASTTree(parse_child);
                AST_child = AST_child->next;
                AST_child->parent = newNode;

                parse_child = parse_child->next; // N3
                
                // TO REMOVE!!!
                if(parse_child == NULL) {
                    printf("Error within rule 57\n");
                    return NULL;
                }
            }

            return newNode;
        }

        // <expression> -> <arithmeticOrBooleanExpr>
        case 60:
        // <expression> -> <U>
        case 61:
            return buildASTTree(parseNodeRoot->child);

        // <U> -> <unary_op> <new_NT>
        case 62:
        {
            ASTNode *newNode = createASTNode(parseNodeRoot);

            parseNode *parseChild = parseNodeRoot->child; // <unary_op>
            ASTNode *ASTChild = buildASTTree(parseChild);

            ASTChild->parent = newNode;
            newNode->child = ASTChild;

            parseChild = parseChild->next; // <new_NT>
            ASTChild->next = buildASTTree(parseChild);
            ASTChild = ASTChild->next;
            ASTChild->parent = newNode;

            return newNode;
        }

        // <new_NT> -> BO <arithmeticExpr> BC
        case 63:
            return buildASTTree((parseNodeRoot->child)->next);

        case 64: // <new_NT> ->  <var_id_num>
        case 65: // <unary_op> -> PLUS
        case 66: // <unary_op> -> MINUS
            return buildASTTree(parseNodeRoot->child);


        case 67: // <arithmeticOrBooleanExpr> -> <AnyTerm> <N7>
        case 70: // <AnyTerm> -> <arithmeticExpr> <N8>
        case 74: // <arithmeticExpr> -> <term> <N4>
        case 77: // <term> -> <factor> <N5>

            // Check if AST node of <Nx> is NULL.
            // If yes, then return AST node of <term>
        {
            ASTNode *ASTNx = buildASTTree((parseNodeRoot->child)->next);
            return ASTNx ? ASTNx : buildASTTree(parseNodeRoot->child);
        }

        case 68: // <N7> -> <logicalOp> <AnyTerm> <N71>
            return createASTNodeForRightRecursiveRule(parseNodeRoot, g_AND, g_OR);

        case 75: // <N4> -> <op1> <term> <N41>
            return createASTNodeForRightRecursiveRule(parseNodeRoot, g_PLUS, g_MINUS);

        case 78: // <N5> -> <op2> <factor> <N51>
            return createASTNodeForRightRecursiveRule(parseNodeRoot, g_MUL, g_DIV);


        case 69: // <N7> -> ε
        case 73: // <N8> -> ε
        case 76: // <N4> -> ε
        case 79: // <N5> -> ε
            return NULL;

        case 71: // <AnyTerm> -> <boolConstt>
            return buildASTTree(parseNodeRoot->child);

        case 72: // <N8> -> <relationalOp> <arithmeticExpr>
        {
            ASTNode* ASTRelationalOp = buildASTTree(parseNodeRoot->child); // <relationalOp>
            ASTNode* ASTt1 = buildASTTree(parseNodeRoot->parent->child); // <arithmeticExpr1>
            ASTNode* ASTt2 = buildASTTree(parseNodeRoot->child->next); // <arithmeticExpr2>

            ASTRelationalOp->child = ASTt1;
            ASTt1->parent = ASTRelationalOp;
            ASTt1->next = ASTt2;
            ASTt2->parent = ASTRelationalOp;

            return ASTRelationalOp;
        }

        case 80: // <factor> -> BO <arithmeticOrBooleanExpr> BC
            return buildASTTree(parseNodeRoot->child->next);

        case 81: // <factor> -> <var_id_num>
        case 82: // <op1> -> PLUS
        case 83: // <op1> -> MINUS
        case 84: // <op2> -> MUL
        case 85: // <op2> -> DIV
        case 86: // <logicalOp> -> AND
        case 87: // <logicalOp> -> OR
        case 88: // <relationalOp> -> LT
        case 89: // <relationalOp> -> LE
        case 90: // <relationalOp> -> GT
        case 91: // <relationalOp> -> GE
        case 92: // <relationalOp> -> EQ
        case 93: // <relationalOp> -> NE
            return buildASTTree(parseNodeRoot->child);



        // <declareStmt> -> DECLARE <idList> COLON <dataType> SEMICOL
        case 94:
        {
            ASTNode *newNode = createASTNode(parseNodeRoot);

            parseNode *parse_child = parseNodeRoot->child;
            parse_child = parse_child->next; // <idList>

            ASTNode *AST_child = buildASTTree(parse_child);
            AST_child->parent = newNode;
            newNode->child = AST_child;

            parse_child = parse_child->next->next; // <dataType>
            ASTNode *AST_sibling = buildASTTree(parse_child);

            AST_child->next = AST_sibling;
            AST_sibling->parent = newNode;

            return newNode;
        }

        // <conditionalStmt> -> SWITCH BO ID BC START <caseStmts> <default> END
        case 95:
        {
            ASTNode *newNode = createASTNode(parseNodeRoot);

            parseNode *parse_child = parseNodeRoot->child;
            parse_child = parse_child->next->next; // ID

            ASTNode *AST_child = buildASTTree(parse_child);
            AST_child->parent = newNode;
            newNode->child = AST_child;

            parse_child = parse_child->next->next; // START

            ASTNode *AST_sibling = buildASTTree(parse_child);
            AST_sibling->parent = newNode;
            AST_sibling->start_line_no = AST_sibling->tkinfo->lno;
            AST_sibling->end_line_no = parse_child->next->next->next->tkinfo->lno;
            AST_child->next = AST_sibling;

            AST_child = AST_child->next;

            parse_child = parse_child->next; // <caseStmts>

            ASTNode *AST_grandchild = buildASTTree(parse_child);
            AST_grandchild->parent = AST_child;
            AST_child->child = AST_grandchild;

            parse_child = parse_child->next; // <default>
            AST_grandchild->next = buildASTTree(parse_child);

            if (AST_grandchild->next) {
                AST_grandchild = AST_grandchild->next;
                AST_grandchild->parent = AST_child;
            }

            return newNode;
        }

        // <caseStmts> -> CASE <value> COLON <statements> BREAK SEMICOL <N9>
        // <N9> -> CASE <value> COLON <statements> BREAK SEMICOL <N9>
        // <N9> -> EPS
        case 96:
        {
            ASTNode *newNode = createASTNode(parseNodeRoot); // <caseStmts>

            parseNode *parse_child = parseNodeRoot->child->next; // <value>
            ASTNode *AST_child = buildASTTree(parse_child);
            
            AST_child->parent = newNode;
            newNode->child = AST_child;

            parse_child = parse_child->next->next; // <statements>
            ASTNode *AST_grandchild = buildASTTree(parse_child);

            if(AST_grandchild) {
                AST_child->child = AST_grandchild;
                AST_grandchild->parent = AST_child;
            }

            parse_child = parse_child->next->next->next; //<N9>

            // List of all IDS until EPS encountered
            while(parse_child->gRuleIndex + 2 != 98) {

                parse_child = parse_child->child->next; // <value>

                AST_child->next = buildASTTree(parse_child);
                AST_child = AST_child->next;
                AST_child->parent = newNode;

                parse_child = parse_child->next->next; // <statements>
                AST_grandchild = buildASTTree(parse_child);

                if(AST_grandchild) {
                    AST_child->child = AST_grandchild;
                    AST_grandchild->parent = AST_child;
                }

                parse_child = parse_child->next->next->next; //<N9>

                // TO REMOVE!!!
                if(parse_child == NULL) {
                    printf("Error within rule 98\n");
                    return NULL;
                }
            }

            return newNode;
        }

        // <value> -> NUM
        case 99:
        // <value> -> TRUE
        case 100:
        // <value> -> FALSE
        case 101:
            return buildASTTree(parseNodeRoot->child);

        // <default> -> DEFAULT COLON <statements> BREAK SEMICOL
        // Keeping a <default> node will help in distinguishing it later.
        case 102:
        {
            ASTNode *newNode = createASTNode(parseNodeRoot->child); //DEFAULT

            parseNode *parse_child = parseNodeRoot->child;
            parse_child = parse_child->next->next; // <statements>

            ASTNode *AST_child = buildASTTree(parse_child);

            if (AST_child) {
                AST_child->parent = newNode;
                newNode->child = AST_child;
            }

            return newNode;
        }

        // <default> -> ε
        case 103:
            return NULL;

        // <iterativeStmt> -> FOR BO ID IN <range> BC START <statements> END
        case 104:
        // TODO
        {
            ASTNode *newNode = createASTNode(parseNodeRoot);

            parseNode *parse_child = parseNodeRoot->child; // FOR

            ASTNode *AST_child = buildASTTree(parse_child);
            AST_child->parent = newNode;
            newNode->child = AST_child;

            parse_child = parse_child->next->next; // ID

            ASTNode *AST_sibling = buildASTTree(parse_child);
            AST_sibling->parent = newNode;
            AST_child->next = AST_sibling;

            AST_child = AST_child->next;
            parse_child = parse_child->next->next; // <range>

            AST_sibling = buildASTTree(parse_child);
            AST_sibling->parent = newNode;
            AST_child->next = AST_sibling;

            AST_child = AST_child->next;
            parse_child = parse_child->next->next; // START

            AST_sibling = buildASTTree(parse_child);
            AST_sibling->parent = newNode;
            AST_sibling->start_line_no = AST_sibling->tkinfo->lno;
            AST_sibling->end_line_no = parse_child->next->next->tkinfo->lno;
            AST_child->next = AST_sibling;

            parse_child = parse_child->next; // <statements>

            ASTNode *AST_grandchild = buildASTTree(parse_child);
            
            if (AST_grandchild != NULL)
                AST_grandchild->parent = AST_sibling;
    
            AST_sibling->child = AST_grandchild;

            return newNode;
        }

        // <iterativeStmt> -> WHILE BO <arithmeticOrBooleanExpr> BC START <statements> END
        case 105:
        {
            ASTNode *newNode = createASTNode(parseNodeRoot);

            parseNode *parse_child = parseNodeRoot->child; // WHILE

            ASTNode *AST_child = buildASTTree(parse_child);
            AST_child->parent = newNode;
            newNode->child = AST_child;

            parse_child = parse_child->next->next; // <arithmeticOrBooleanExpr>

            ASTNode *AST_sibling = buildASTTree(parse_child);
            AST_sibling->parent = newNode;
            AST_child->next = AST_sibling;

            AST_child = AST_child->next;
            parse_child = parse_child->next->next; // START

            AST_sibling = buildASTTree(parse_child);
            AST_sibling->parent = newNode;
            AST_sibling->start_line_no = AST_sibling->tkinfo->lno;
            AST_sibling->end_line_no = parse_child->next->next->tkinfo->lno;
            AST_child->next = AST_sibling;

            parse_child = parse_child->next; // <statements>

            ASTNode *AST_grandchild = buildASTTree(parse_child);

            if (AST_grandchild != NULL)
               AST_grandchild->parent = AST_sibling;
               
            AST_sibling->child = AST_grandchild;

            return newNode;
        }

        // <range> -> NUM RANGEOP NUM
        case 106:
        {
            ASTNode *newNode = createASTNode(parseNodeRoot);

            parseNode *parse_child = parseNodeRoot->child; // NUM 1

            ASTNode *AST_child = buildASTTree(parse_child);
            AST_child->parent = newNode;
            newNode->child = AST_child;

            parse_child = parse_child->next->next; // NUM 2

            ASTNode *AST_sibling = buildASTTree(parse_child);
            AST_sibling->parent = newNode;
            AST_child->next = AST_sibling;

            return newNode;
        }

        default:
        {
            printf("%d entered here\n", parseNodeRoot->gRuleIndex + 2);
            printf("E\nE\nE\nE\nE\n");
            return NULL;
        }
    }
}

void print_ASTTree(ASTNode *ASTNodeRoot) {

    if(ASTNodeRoot == NULL) {
        printf("NULL\n");
        return;
    }

    if (ASTNodeRoot->child == NULL)
        return;

    printf("%s \n", inverseMappingTable[ASTNodeRoot->gs]);

    ASTNode *child = ASTNodeRoot->child;

    while(child!=NULL) {
        printf("%s\t", inverseMappingTable[child->gs]);
        child = child->next;
    }
    printf("\n\n");

    child = ASTNodeRoot->child;

    while(child!=NULL) {
        print_ASTTree(child);
        child = child->next;
    }
}

void print_ParseTree(parseNode *parseNodeRoot) {

    if(parseNodeRoot == NULL) {
        printf("NULL\n");
        return;
    }

    if(parseNodeRoot->child == NULL)
        return;

    printf("%s \n", inverseMappingTable[parseNodeRoot->gs]);

    parseNode *child = parseNodeRoot->child;

    while(child!=NULL) {
        printf("%s\t", inverseMappingTable[child->gs]);
        child = child->next;
    }
    printf("\n\n");

    child = parseNodeRoot->child;

    while(child!=NULL) {
        print_ParseTree(child);
        child = child->next;
    }
}

int count_nodes_ASTTree(ASTNode *root) {
    int count = 1;

    ASTNode *child = root->child;

    while(child != NULL) {
        count += count_nodes_ASTTree(child);
        child = child->next;
    }

    return count;
}

int count_nodes_parseTree(parseNode *root) {
    int count = 1;

    parseNode *child = root->child;

    while(child != NULL) {
        count += count_nodes_parseTree(child);
        child = child->next;
    }

    return count;
}

void destroyAST(ASTNode *root){
    if(root == NULL)
        return;
    ASTNode* child=root->child;
    while(child != NULL) {
        ASTNode* tmp = child->next;
        destroyAST(child);
        child=tmp;
    }
    free(root);
}
