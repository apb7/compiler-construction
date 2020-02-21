all:    driver.c lexer/lexer.c utils/hash.c utils/util.c
	gcc driver.c lexer/lexer.c utils/hash.c utils/util.c -o lexer/lexer.out
