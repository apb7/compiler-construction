#include "errorPtr_stack.h"
#include "errorPtr_stack_config.h"
#include "../error.h"

#define TYPE error*
#define NAME_TYPE(a) errorPtr_##a

#include "generic_stack.c"

#undef TYPE
#undef NAME_TYPE

