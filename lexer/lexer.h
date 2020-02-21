// struct tokenNode* pop_front();

// void push_back(struct token *tk);

// void add_num(int n);
// void add_rnum(float f);
// void add_lexeme(char *lex);

// char* get_block(FILE *fp);

// void DFA(char *buffer); //static line no

// void fill_keywords();
#include "lexerDef.h"
#include <stdbool.h>
#include <stdio.h>
bool checkPos(unsigned int pos);
void getStream(FILE *fp);
tokenInfo* getNextToken(FILE *fp);
char* getBlock(FILE *fp);
void removeComments(char *testcaseFile, char *cleanFile);
