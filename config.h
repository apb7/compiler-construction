#ifndef CCGIT_CONFIG_H
#define CCGIT_CONFIG_H

//This file will contain all the defined constant values, so that it is easy to change them from here whenever required

#include "parser/parserDef.h"

/*
 * This defines the maximum length of a line in grammar.txt file
 */
#define MAX_LINE_LEN 150

/*
 * These define the number of non-terminals and number of terminals
 */
#define nt_numNonTerminals g_numSymbols - g_EOS - 1
#define t_numTerminals g_EOS //excluding $ (g_EOS)
//ntx can be used to map NonTerminal Enums to 0 based indexing
#define ntx(y) y - g_EOS - 1

/*
 * This defines the size of the hash table for storing all symbols (tokens, NTs, EPS, $)
 */
#define SYMBOL_HT_SIZE 131

/*
 * This defines the size of the hash table for storing all Keywords
 */
#define KEYWORD_HT_SIZE 31

/*
 * This defines the buffer size used in lexer.c
 */
#define BUFFER_SIZE 512

#endif //CCGIT_CONFIG_H
