all:    driver.c lexer/lexer.c hash.c
	gcc driver.c lexer/lexer.c hash.c -o lexer/lexer.out
