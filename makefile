all:    driver.c parserDef.h parser.h parser.c treeNodePtr_stack.c treeNodePtr_stack.h set.c set.h lexerDef.h lexer.c lexer.h hash.c hash.h util.c util.h astDef.h ast.h ast.c error.c error.h errorPtr_stack.h errorPtr_stack.c symbolTableDef.h symbolHash.h symbolHash.c symbolTable.h archive.h symbolTable.c typeCheck.c typeCheck.h printSymTable.c archive.c
	gcc driver.c parser.c treeNodePtr_stack.c set.c lexer.c  hash.c util.c ast.c error.c errorPtr_stack.c  symbolHash.c symbolTable.c typeCheck.c printSymTable.c archive.c -o test

