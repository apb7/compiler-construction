#ifndef CCGIT_SET_H
#define CCGIT_SET_H
#include <stdbool.h>
typedef unsigned long long intSet;

intSet add_elt(intSet s, unsigned int elt);
intSet remove_elt(intSet s, unsigned int elt);
bool isPresent(intSet s, unsigned int elt);
intSet union_set(intSet s1, intSet s2);
intSet intersect_set(intSet s1, intSet s2);

#endif //CCGIT_SET_H
