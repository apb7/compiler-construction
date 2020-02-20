
#ifndef CCGIT_PARSER_H
#define CCGIT_PARSER_H

#include "../set.h"

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

#endif //CCGIT_PARSER_H
