#include "treeNodePtr_stack.h"
#include "treeNodePtr_stack_config.h"
#include "parser/parserDef.h"


#define TYPE treeNode*
#define NAME_TYPE(a) treeNodePtr_##a

#include "generic_stack.c"

#undef TYPE
#undef NAME_TYPE

