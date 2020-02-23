
#ifndef PARSER_H
#define PARSER_H

#include "../utils/set.h"
#include "parserDef.h"

void printRuleRange();
void populateGrammarStruct(char *grFile);
void printGrammar();
void populateFirstSet();
void populateFollowSet();
void initParseTable();
intSet predictSet(grammarNode* g);
int populateParseTable();
void modifyParseTable_Err_Recovery();
void printParseTable();
void printFirst();
void printFollow();
void printPredictSets();
treeNode *parseInputSourceCode(char *src);
grammarNode createRuleNode(char *rule);
void printTreeOld(treeNode *root);
void printTree(treeNode* root,  char* fname);
void destroyTree(treeNode *root);

treeNode *newTreeNode(gSymbol sym, treeNode *parent);

#endif //PARSER_H
