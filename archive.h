// Group Number: 31
// MADHUR PANWAR   2016B4A70933P
// TUSSANK GUPTA   2016B3A70528P
// SALMAAN SHAHID  2016B4A70580P
// APURV BAJAJ     2016B3A70549P
// HASAN NAQVI     2016B5A70452P

#ifndef COMPILER_CONSTRUCTION_ARCHIVE_H
#define COMPILER_CONSTRUCTION_ARCHIVE_H


// functions for printSymTable1
void printSymbolTable1(symbolTable* st, char *fname);
void printCurrSymTable1(symbolTable *st,int level, FILE *fp);

// functions for printSymTable2
void printSymbolTable2(symbolTable *st, char *fname);
void getSymNode(symTableNode *node, char *pstr);
void printCurrSymTable2(symTableNode *stn, FILE *fp); // gets the symTableNode entry corresponding to a function and prints scope hierarchy of that function

#endif //COMPILER_CONSTRUCTION_ARCHIVE_H
