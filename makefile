all:    driver.c lexer_alt.c hash.c
	gcc driver.c lexer_alt.c hash.c -o alt
