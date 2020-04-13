// Group Number: 31
// MADHUR PANWAR   2016B4A70933P
// TUSSANK GUPTA   2016B3A70528P
// SALMAAN SHAHID  2016B4A70580P
// APURV BAJAJ     2016B3A70549P
// HASAN NAQVI     2016B5A70452P

#ifndef CONFIG_H
#define CONFIG_H

//This file will contain all the defined constant values, so that it is easy to change them from here whenever required

#include "parserDef.h"

#define ERROR_RECOVERY_VERBOSE 0

/*
 * This defines the maximum length of a line in grammar.txt file
 */
#define MAX_LINE_LEN 150

/*
 * These define the number of non-terminals and number of terminals
 */
#define NUM_NON_TERMINALS g_numSymbols - g_EOS - 1
#define NUM_TERMINALS g_EOS //excluding $ (g_EOS)

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

/*
 * This defines the size of error string used in error.c
 */
#define ERROR_STRING_LENGTH 30

#define TMP_SRC_FILE_PATH "source.tmp"

#define SYMBOL_TABLE_SIZE 101
#define SIZE_INTEGER 2
#define SIZE_REAL 4
#define SIZE_BOOLEAN 1

#endif //CONFIG_H
