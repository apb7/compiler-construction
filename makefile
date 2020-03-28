all:    lexer.c lexer.h lexerDef.h parser.c parser.h parserDef.h set.c set.h hash.h hash.c driver.c util.c util.h config.h treeNodePtr_stack.c treeNodePtr_stack.h error.c error.h errorPtr_stack.c errorPtr_stack.h ast.c ast.h astDef.h
	gcc lexer.c parser.c set.c hash.c driver.c util.c treeNodePtr_stack.c error.c errorPtr_stack.c ast.c -o test
