#ifndef ERROR_STACK_H
#define ERROR_STACK_H

#include "../error.h"

#define TYPE error*
#define NAME_TYPE(a) errorPtr_##a

#include "generic_stack.h"

#undef TYPE
#undef NAME_TYPE

#endif //ERROR_STACK_H
