all:    driver.c lexer/lexer.c hash.c utils.c
	gcc driver.c lexer/lexer.c hash.c utils.c -o lexer/lexer.out