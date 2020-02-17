#include "set.h"
#include <stdbool.h>

//set elt bit
intSet add_elt(intSet s, unsigned int elt) {return s|(1ULL<<elt);}

//unset elt bit
intSet remove_elt(intSet s, unsigned int elt) {return s&(~(1ULL<<elt));}

//returns 0 if elt bit is 0
bool isPresent(intSet s, unsigned int elt) {return s&(1ULL<<elt);}

// union of 2 sets
intSet union_set(intSet s1, intSet s2) {return (s1|s2);}

//intersection of 2 sets
intSet intersect_set(intSet s1, intSet s2) {return (s1&s2);}