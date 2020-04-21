// Group Number: 31
// MADHUR PANWAR   2016B4A70933P
// TUSSANK GUPTA   2016B3A70528P
// SALMAAN SHAHID  2016B4A70580P
// APURV BAJAJ     2016B3A70549P
// HASAN NAQVI     2016B5A70452P

#ifndef AST_H
#define AST_H

#include "astDef.h"
#include <stdbool.h>

bool isLeafNodeAST(ASTNode *ptr);
ASTNode* createASTNode(parseNode *parseNode);

ASTNode* create_self_node_with_single_child(parseNode *parseNodeRoot);
ASTNode* createASTNodeForRightRecursiveRule(parseNode *parseNodeRoot, gSymbol g_op1, gSymbol g_op2);
ASTNode* simple_recursive_list_non_empty(parseNode *parseNodeRoot, int stop_condition);

ASTNode* buildASTTree(parseNode *parseNodeRoot);

void print_Inorder_ASTTree(ASTNode *ASTNodeRoot, char *fname);
void print_ASTTree_LevelDepth(ASTNode *ASTNodeRoot, FILE *fp);
void print_ASTTree(ASTNode *ASTNoderoot, char *fname);
int count_nodes_ASTTree(ASTNode *root);
int count_nodes_parseTree(parseNode *root);
void destroyAST(ASTNode *root);
#endif
