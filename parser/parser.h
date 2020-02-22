
#ifndef CCGIT_PARSER_H
#define CCGIT_PARSER_H

#include "../utils/set.h"
#include "parserDef.h"

void populateGrammarStruct(char *grFile);
void printGrammar();
void populateFirstSet();
void populateFollowSet();
void initParseTable();
intSet predictSet(grammarNode* g);
int populateParseTable();
void printParseTable();
void printFirst();
void printFollow();
void printPredictSets();
treeNode *parseInputSourceCode(char *src);
grammarNode createRuleNode(char *rule);
void printTreeOld(treeNode *root);
void printTree(treeNode* root,  char* fname);

treeNode *newTreeNode(gSymbol sym, treeNode *parent);

#endif //CCGIT_PARSER_H
