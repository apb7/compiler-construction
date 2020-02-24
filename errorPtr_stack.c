// Group Number: 31
// MADHUR PANWAR   2016B4A70933P
// TUSSANK GUPTA   2016B3A70528P
// SALMAAN SHAHID  2016B4A70580P
// APURV BAJAJ     2016B3A70549P
// HASAN NAQVI     2016B5A70452P

#include "errorPtr_stack.h"
#include "errorPtr_stack_config.h"
#include "error.h"

#define TYPE error*
#define NAME_TYPE(a) errorPtr_##a

#include "generic_stack.c"

#undef TYPE
#undef NAME_TYPE

