// Group Number: 31
// MADHUR PANWAR   2016B4A70933P
// TUSSANK GUPTA   2016B3A70528P
// SALMAAN SHAHID  2016B4A70580P
// APURV BAJAJ     2016B3A70549P
// HASAN NAQVI     2016B5A70452P

#include "set.h"

//set element bit
intSet add_element(intSet s, unsigned int elt) {return s|(1ULL<<elt);}

//unset element bit
intSet remove_element(intSet s, unsigned int elt) {return s&(~(1ULL<<elt));}

//returns 0 if elt bit is 0
bool isPresent(intSet s, unsigned int elt) {return s&(1ULL<<elt);}

// union of 2 sets
intSet union_set(intSet s1, intSet s2) {return (s1|s2);}

//intersection of 2 sets
intSet intersect_set(intSet s1, intSet s2) {return (s1&s2);}