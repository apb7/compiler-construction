// Group Number: 31
// MADHUR PANWAR   2016B4A70933P
// TUSSANK GUPTA   2016B3A70528P
// SALMAAN SHAHID  2016B4A70580P
// APURV BAJAJ     2016B3A70549P
// HASAN NAQVI     2016B5A70452P
#include <stdio.h>

#ifndef UTILS_H
#define UTILS_H

char *allocString(int size);
int equals(char *s1, char *s2);
char * trim (char *str);
int numTk(char *str,char tk);
char **strSplit(char *str, char tk);
void fcloseSafe(FILE *fp);

#endif //UTILS_H
