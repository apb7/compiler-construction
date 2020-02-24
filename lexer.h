// Group Number: 31
// MADHUR PANWAR   2016B4A70933P
// TUSSANK GUPTA   2016B3A70528P
// SALMAAN SHAHID  2016B4A70580P
// APURV BAJAJ     2016B3A70549P
// HASAN NAQVI     2016B5A70452P

#ifndef LEXER_H
#define LEXER_H

#include "lexerDef.h"
#include <stdbool.h>
#include <stdio.h>


bool checkPos(unsigned int pos);
void getStream(FILE *fp);
tokenInfo* getNextToken(FILE *fp);
char* getBlock(FILE *fp);
void removeComments(char *testcaseFile, char *cleanFile);


#endif