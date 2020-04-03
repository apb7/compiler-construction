// Group Number: 31
// MADHUR PANWAR   2016B4A70933P
// TUSSANK GUPTA   2016B3A70528P
// SALMAAN SHAHID  2016B4A70580P
// APURV BAJAJ     2016B3A70549P
// HASAN NAQVI     2016B5A70452P

#ifndef PARSER_H
#define PARSER_H

#include "set.h"
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
grammarNode createRuleNode(char *rule, int ruleIndex);
void printTreeOld(treeNode *root);
void printTree(treeNode* root,  char* fname);
void destroyTree(treeNode *root);

treeNode *newTreeNode(gSymbol sym, treeNode *parent, int gRuleIndex);

#endif //PARSER_H
