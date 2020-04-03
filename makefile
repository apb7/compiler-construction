all:    lexer/lexer.c lexer/lexer.h lexer/lexerDef.h parser/parser.c parser/parser.h parser/parserDef.h utils/set.c utils/set.h utils/hash.h utils/hash.c driver.c utils/util.c utils/util.h config.h utils/treeNodePtr_stack.c utils/treeNodePtr_stack.h utils/treeNodePtr_stack_config.h utils/treeNodePtr_stack_config.c error.c error.h utils/errorPtr_stack.c utils/errorPtr_stack.h utils/errorPtr_stack_config.c utils/errorPtr_stack_config.h
	gcc lexer/lexer.c parser/parser.c utils/set.c utils/hash.c driver.c utils/util.c utils/treeNodePtr_stack.c utils/treeNodePtr_stack_config.c error.c utils/errorPtr_stack.c utils/errorPtr_stack_config.c -o stage1exe
