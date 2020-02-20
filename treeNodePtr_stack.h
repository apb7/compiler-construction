#ifndef COMPILER_CONSTRUCTION_TREENODEPTR_STACK_H
#define COMPILER_CONSTRUCTION_TREENODEPTR_STACK_H

#include "treeNodePtr_Def.h"

#define TYPE treeNode*
#define NAME_TYPE(a) treeNodePtr_##a

#include "generic_stack.h"

#undef TYPE
#undef NAME_TYPE

#endif //COMPILER_CONSTRUCTION_TREENODEPTR_STACK_H
