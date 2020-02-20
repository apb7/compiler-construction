#include "integer_stack.h"
#include "integer_stack_config.h"

#define TYPE int
#define NAME_TYPE(a) int_##a

#include "generic_stack.c"

#undef TYPE
#undef NAME_TYPE

