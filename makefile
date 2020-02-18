all:    driver.c lexer_alt.c hash.c
	gcc driver.c lexer_alt.c hash.c -o alt
	gcc driver.c lexer.c hash.c -o new
	./new 1> new.txt
	./alt 1> alt.txt
	diff alt.txt new.txt

