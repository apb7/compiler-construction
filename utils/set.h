// Group Number: 31
// MADHUR PANWAR   2016B4A70933P
// TUSSANK GUPTA   2016B3A70528P
// SALMAAN SHAHID  2016B4A70580P
// APURV BAJAJ     2016B3A70549P
// HASAN NAQVI     2016B5A70452P

#ifndef CCGIT_SET_H
#define CCGIT_SET_H
#include <stdbool.h>
typedef unsigned long long intSet;

intSet add_element(intSet s, unsigned int elt);
intSet remove_element(intSet s, unsigned int elt);
bool isPresent(intSet s, unsigned int elt);
intSet union_set(intSet s1, intSet s2);
intSet intersect_set(intSet s1, intSet s2);

#endif //CCGIT_SET_H
