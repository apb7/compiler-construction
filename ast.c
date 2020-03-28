#include "astDef.h"
#include "lexerDef.h"
#include "parserDef.h"

#include <stdio.h>
#include <stdlib.h>

extern char *inverseMappingTable[];

ASTNode* buildASTTree(parseNode* parseNodeRoot);


ASTNode* createASTNode(parseNode *parseNode){
    ASTNode *newNode = malloc(sizeof(ASTNode));

    newNode->gs = parseNode->gs;

    // TODO : copy token if its being freed
    newNode->tkInfo = parseNode->tkinfo;
    newNode->next = NULL;
    newNode->child = NULL;
    newNode->parent = NULL;
}


ASTNode* create_self_node_with_single_child(parseNode *parseNodeRoot) {

    ASTNode *newNode = createASTNode(parseNodeRoot);

    ASTNode *AST_child = buildASTTree(parseNodeRoot->child);
    // Beware of SegFault. Use this function carefully.
    AST_child->parent = newNode;
    newNode->child = AST_child;

    return newNode;
}


ASTNode* buildASTTree(parseNode* parseNodeRoot) {

    if (parseNodeRoot == NULL) {
        printf("NULL ptr provided\n");
        return NULL;
    }

    printf("Rule %d %s\n", parseNodeRoot->gRuleIndex + 2, inverseMappingTable[ parseNodeRoot->gs ]);

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

            parse_child = parse_child->next;

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

/* 3 .. 7 */

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
            
            ASTNode *newNode = createASTNode(parseNodeRoot);
            AST_child->parent = newNode;
            newNode->child = AST_child;

            return newNode;
        }

/* 9 .. 17 */

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

        // Doubt(apb7): Do we need a node for <moduleDef> ?
        // <moduleDef> -> START <statements> END 
        case 26:
        {
            parseNode *parse_child = parseNodeRoot->child;
            parse_child = parse_child->next;

            ASTNode *AST_child = buildASTTree(parse_child);

            if(AST_child == NULL)
                return NULL;

            ASTNode *newNode = createASTNode(parseNodeRoot);
            AST_child->parent = newNode;
            newNode->child = AST_child;

            return newNode;
        }

        // <statements> -> <statement> <statements>
        case 27:
        {
            parseNode *parse_child = parseNodeRoot->child;

            ASTNode *AST_child = buildASTTree(parse_child);
            
            // Statement can't return NULL 
            ASTNode *newNode = createASTNode(parseNodeRoot);
            AST_child->parent = newNode;
            newNode->child = AST_child;

            parse_child = parse_child->next;

            // List of all statements until EPS encountered
            while(parse_child->gRuleIndex + 2 != 28) {

                //printf("LOOP1 %s %d \n", inverseMappingTable[ parse_child->gs], parse_child->gRuleIndex);

                parse_child = parse_child->child;

                //printf("LOOP2 %s %d \n", inverseMappingTable[ parse_child->gs], parse_child->gRuleIndex);

                AST_child->next = buildASTTree(parse_child);
                AST_child = AST_child->next;
                AST_child->parent = newNode;

                parse_child = parse_child->next;
                
                // TO REMOVE!!!
                if(parse_child == NULL) {
                    printf("Error within rule 27\n");
                    return NULL;
                }
            }

            return newNode;
        }

        // <statements> -> EPS
        case 28:
            return NULL;

        // <statement> -> <ioStmt>
        case 29:
        // <statement> -> <simpleStmt>
        case 30:
        // <statement> -> <declareStmt>
        case 31:
        // <statement> -> <condionalStmt>
        case 32:
        // <statement> -> <iterativeStmt>
        case 33:
            return buildASTTree(parseNodeRoot->child);

        // For design uniformity in <statement>, create <ioStmt> node.
        // <ioStmt> -> GET_VALUE BO ID BC SEMICOL
        case 34:
        {
            ASTNode *newNode = createASTNode(parseNodeRoot);

            parseNode *parse_child = parseNodeRoot->child;
            parse_child = parse_child->next->next;

            ASTNode *AST_child = buildASTTree(parse_child);
            AST_child->parent = newNode;
            newNode->child = AST_child;

            return newNode;
        }

        // <ioStmt> -> PRINT BO <var> BC SEMICOL
        case 35:
        {
            ASTNode *newNode = createASTNode(parseNodeRoot);

            parseNode *parse_child = parseNodeRoot->child;
            parse_child = parse_child->next->next;

            ASTNode *AST_child = buildASTTree(parse_child);
            AST_child->parent = newNode;
            newNode->child = AST_child;

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

// TODO : assignment statements
        // <assignmentStmt> -> ID <whichStmt>
        case 47:

        // <whichStmt> -> <lvalueIDStmt>
        case 48:
        // <whichStmt> -> <lvalueARRStmt>
        case 49: 
        
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

/* 60 .. 93 */
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

/*

<arithmeticOrBooleanExpr> -> <AnyTerm> <N7>
arithmeticOrBooleanExpr.addr = N7.syn
N7.inh = AnyTerm.addr

<N7> -> <logicalOp> <AnyTerm> <N71>
N71.inh = new Node(logicalOp.addr, N7.inh, AnyTerm.addr)
N7.syn = N71.syn

<N7> -> ε
N7.syn = N7.inh

<AnyTerm> -> <arithmeticExpr> <N8>
AnyTerm.addr = N8.syn
N8.inh = arithmeticExpr.addr

<AnyTerm> -> <boolConstt>
AnyTerm.addr = boolConstt.addr

<N8> -> <relationalOp> <arithmeticExpr>
N8.syn = new Node(relationalOp.addr, arithmeticExpr.addr)

<N8> -> ε
N8.syn = N8.inh

<arithmeticExpr> -> <term> <N4>
arithmeticExpr.addr = N4.syn
N4.inh = term.addr

<N4> -> <op1> <term> <N41>
N41.inh = new Node(op1.addr, N4.inh, term.addr)
N4.syn = N41.syn

<N4> -> ε
N4.addr = NULL

<term> -> <factor> <N5>
term.addr = N5.syn
N5.inh = factor.addr

<N5> -> <op2> <factor> <N51>
N51.inh = new Node(op2.addr, N5.inh, factor.addr)
N5.syn = N51.syn

<N5> -> ε
N5.syn = N5.inh

<factor> -> BO <arithmeticOrBooleanExpr> BC
factor.addr = arithmeticOrBooleanExpr.addr

<factor> -> <var_id_num>
factor.addr = var_id_num.addr

<op1> -> PLUS
op1.addr = new Leaf(PLUS.value)

<op1> -> MINUS
op1.addr = new Leaf(MINUS.value)

<op2> -> MUL
op2.addr = new Leaf(MUL.value)

<op2> -> DIV
op2.addr = new Leaf(DIV.value)

<logicalOp> -> AND
logicalOp.addr = new Leaf(AND.value)

<logicalOp> -> OR
logicalOp.addr = new Leaf(OR.value)

<relationalOp> -> LT
relationalOp.addr = new Leaf(LT.value)

<relationalOp> -> LE
relationalOp.addr = new Leaf(LE.value)

<relationalOp> -> GT
relationalOp.addr = new Leaf(GT.value)

<relationalOp> -> GE
relationalOp.addr = new Leaf(GE.value)

<relationalOp> -> EQ
relationalOp.addr = new Leaf(EQ.value)

<relationalOp> -> NE
relationalOp.addr = new Leaf(NE.value)

*/

        // <arithmeticExpr> -> <term> <N4>
        // <N4> -> <op1> <term> <N41>
        // <N4> -> ε
        // case 74:
        // {
        //     parseNode* parseChild = parseNodeRoot->child; // <term>
        //     ASTNode* ASTChild = buildASTTree(parseChild);

        //     parseChild = parseChild->next; // N4
        //     parseNode* parseGrandChild = parseChild->child; // Either <op1> or EPS

        //     ASTNode* newNode = buildASTTree(parseGrandChild);

        //     if(newNode == NULL) return ASTChild; // EPS

        //     // <op1>
        //     ASTChild->parent = newNode;
        //     newNode->child = ASTChild;

        //     parseGrandChild = parseGrandChild->next; // <term>
        //     ASTChild->next = buildASTTree(parseGrandChild);
        //     ASTChild = ASTChild->next;
        //     ASTChild->parent = newNode;

        //     ASTChild->next = buildASTTree(parseGrandChild->next); // <N41>


        // }

        case 74: // <arithmeticExpr> -> <term> <N4>
        {
            ASTNode* AST_N4 = buildASTTree((parseNodeRoot->child)->next);
            return AST_N4;
        }

        case 75: // <N4> -> <op1> <term> <N41>
        {
            ASTNode* AST_N41 = buildASTTree(parseNodeRoot->child->next->next); // <N41>

            ASTNode* ASTOp1 = buildASTTree(parseNodeRoot->child);
            ASTNode* ASTterm1 = parseNodeRoot->parent->child == g_term ? buildASTTree(parseNodeRoot->parent->child) : NULL;
            ASTNode* ASTterm2 = buildASTTree(parseNodeRoot->child->next);

            if(ASTterm1) {
                ASTOp1->child = ASTterm1;
                ASTterm1->parent = ASTOp1;
                ASTterm1->next = ASTterm2;
                ASTterm2->parent = ASTOp1;
            }
            else {
                ASTOp1->child = ASTterm2;
                ASTterm2->parent = ASTOp1;
            }

            if(AST_N41 == NULL)
                return ASTOp1;

            ASTNode* cur = AST_N41;

            while(cur->child == g_op1)
                cur = cur->child;

            ASTOp1->next = cur->child;
            ASTOp1->parent = cur;
            cur->child = ASTOp1;

            return AST_N41;
        }

        case 76:
        {
            return NULL;
        }

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

        // <condionalStmt> -> SWITCH BO ID BC START <caseStmts> <default> END
        case 95:
        {
            ASTNode *newNode = createASTNode(parseNodeRoot);

            parseNode *parse_child = parseNodeRoot->child;
            parse_child = parse_child->next->next; // ID

            ASTNode *AST_child = buildASTTree(parse_child);
            AST_child->parent = newNode;
            newNode->child = AST_child;

            parse_child = parse_child->next->next->next; // <caseStmts>
            ASTNode *AST_sibling = buildASTTree(parse_child);

            AST_child->next = AST_sibling;
            AST_sibling->parent = newNode;

            AST_child = AST_sibling;
            parse_child = parse_child->next; // <default>
            AST_sibling = buildASTTree(parse_child);

            if (AST_sibling) {
                AST_child->next = AST_sibling;
                AST_sibling->parent = newNode;
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
            ASTNode *newNode = createASTNode(parseNodeRoot);

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

/* 104 105 */

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

/* TODO:
idlist
*/
        default:
        {
            printf("%d entered here\n", parseNodeRoot->gRuleIndex + 2);
            return NULL;
        }
    }
}

void print_ASTTree(ASTNode *ASTNodeRoot) {

    if(ASTNodeRoot == NULL)
    {
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

    if(parseNodeRoot == NULL)
    {
        printf("NULL\n");
        return;
    }

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