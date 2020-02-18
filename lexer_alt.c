#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "hash.h"
#include "lexerDef.h"

typedef unsigned int uint;

uint BUFFER_SIZE = 512;
uint TWIN_BUFFER_SIZE = 1024;
uint line_number = 1;
uint bp = 0; // begin ptr
uint fp = 0; // forward ptr
char buffer_for_tokenization[1024]; // a global buffer of size 2 * BUFFER_SIZE


void getStream(FILE *file_ptr) {
    static int status = 1;
    static int count = 0;

    count++;

    // Selecting a half from the global buffer, alternatively.
    status ^= 1;

    // Invalid call, discard all changes
    if((fp == 0 && status != 0) || (fp == BUFFER_SIZE && status != 1)) {
        status ^= 1;
        count--;
        return;
    }

    // Fill one half of the global buffer with zeros.
    uint i;
    for(i=0; i<BUFFER_SIZE; i++)
        buffer_for_tokenization[(status * BUFFER_SIZE) + i] = 0;

    if(feof(file_ptr))
        fprintf(stderr, "EOF REACHED : %d buffer reads\n", count-1);

    fread(buffer_for_tokenization + (status * BUFFER_SIZE), BUFFER_SIZE, sizeof(char), file_ptr);

    if(feof(file_ptr))
        fprintf(stderr, "EOF REACHED : %d buffer reads\n", count);

    return;
}

// only for removeComments, don't need twin buffers there
char* getBlock(FILE *fp) {
    if(feof(fp))
        return NULL;

    char *buffer_from_file = (char*) calloc(BUFFER_SIZE, sizeof(char));
    fread(buffer_from_file, BUFFER_SIZE, sizeof(char), fp);
    return buffer_from_file;
}

void removeComments(char *testcaseFile, char *cleanFile) {
    FILE *fp_testcaseFile = fopen(testcaseFile, "r");

    if(fp_testcaseFile == NULL) {
        printf("ERROR: Failed to open %s", testcaseFile);
        return;
    }

    FILE *fp_cleanFile = fopen(cleanFile, "w");

    if(fp_cleanFile == NULL) {
        printf("ERROR: Failed to open %s", cleanFile);
        return;
    }

    char *buffer_to_read = getBlock(fp_testcaseFile);
    char *buffer_to_write = (char*) malloc(BUFFER_SIZE * sizeof(char));

    int isComment = 0, wasAsterisk = 0;

    while(buffer_to_read) {
        uint i = 0, j = 0;

        if(wasAsterisk && buffer_to_read[0] == '*'){
            isComment ^= 1;
            i++;
        }

        for(; i < BUFFER_SIZE && buffer_to_read[i] != '\0'; i++){

            if(buffer_to_read[i] == '*' && (i < BUFFER_SIZE - 1 && buffer_to_read[i+1] == '*')){
                isComment ^= 1;
                i += 2;
            }

            if(!isComment || buffer_to_read[i] == '\n')
                buffer_to_write[j++] = buffer_to_read[i];
        }

        fwrite(buffer_to_write, sizeof(char), j, fp_cleanFile);

        if(buffer_to_read[BUFFER_SIZE - 1] == '*')
            wasAsterisk = 1;
        else
            wasAsterisk = 0;

        free(buffer_to_read);
        buffer_to_read = getBlock(fp_testcaseFile);
    }

    free(buffer_to_write);
}

// Prints the string in global buffer from start to end, not including end.
// [start, end)
void print_lexical_error(uint start, uint end) {
    fprintf(stderr, "LEXICAL ERROR: invalid token on line number %u.\n \t", line_number);

    start = start % TWIN_BUFFER_SIZE;
    end = end % TWIN_BUFFER_SIZE;

    do {
        // Put the error in the error stream, stderr.
        fprintf(stderr, "%c", buffer_for_tokenization[start]);
        start = (start + 1) % TWIN_BUFFER_SIZE;
    }
    while(start != end);

    fprintf(stderr, "\n");
}

tokenInfo* getNextToken(FILE *file_ptr) {

    int state = 0;

    if(fp == 0 || fp == BUFFER_SIZE) {
        getStream(file_ptr);
    }

    // TODO: Clarify allocation.
    tokenInfo* tkin = (tokenInfo*) malloc(sizeof(tokenInfo));
    char lookahead;

    while(1) {
        lookahead = buffer_for_tokenization[fp];
        fp = (fp + 1) % TWIN_BUFFER_SIZE;
        
        if(fp == 0 || fp == BUFFER_SIZE)
            getStream(file_ptr);

        switch(state) {

            case 0:
            {
                switch (lookahead)
                {
                    case '+':
                    {
                        state = 17;
                        tkin->type = PLUS;
                        tkin->lno = line_number;
                        tkin->value.lexeme[0] = '+';
                        tkin->value.lexeme[1] = '\0';
                        bp = fp;
                        return tkin;
                    }
                    break;

                    case '-':
                    {
                        state = 18;
                        tkin->type = MINUS;
                        tkin->lno = line_number;
                        tkin->value.lexeme[0] = '-';
                        tkin->value.lexeme[1] = '\0';
                        bp = fp;
                        return tkin;
                    }
                    break;

                    case '/':
                    {
                        state = 19;
                        tkin->type = DIV;
                        tkin->lno = line_number;
                        tkin->value.lexeme[0] = '/';
                        tkin->value.lexeme[1] = '\0';
                        bp = fp;
                        return tkin;
                    }
                    break;

                    case '[':
                    {
                        state = 28;
                        tkin->type = SQBO;
                        tkin->lno = line_number;
                        tkin->value.lexeme[0] = '[';
                        tkin->value.lexeme[1] = '\0';
                        bp = fp;
                        return tkin;
                    }
                    break;

                    case ']':
                    {
                        state = 29;
                        tkin->type = SQBC;
                        tkin->lno = line_number;
                        tkin->value.lexeme[0] = ']';
                        tkin->value.lexeme[1] = '\0';
                        bp = fp;
                        return tkin;
                    }
                    break;

                    case '(':
                    {
                        state = 30;
                        tkin->type = BO;
                        tkin->lno = line_number;
                        tkin->value.lexeme[0] = '(';
                        tkin->value.lexeme[1] = '\0';
                        bp = fp;
                        return tkin;
                    }
                    break;

                    case ')':
                    {
                        state = 31;
                        tkin->type = BC;
                        tkin->lno = line_number;
                        tkin->value.lexeme[0] = ')';
                        tkin->value.lexeme[1] = '\0';
                        bp = fp;
                        return tkin;
                    }
                    break;

                    case ',':
                    {
                        state = 32;
                        tkin->type = COMMA;
                        tkin->lno = line_number;
                        tkin->value.lexeme[0] = ',';
                        tkin->value.lexeme[1] = '\0';
                        bp = fp;
                        return tkin;
                    }
                    break;

                    case ';':
                    {
                        state = 33;
                        tkin->type = SEMICOL;
                        tkin->lno = line_number;
                        tkin->value.lexeme[0] = ';';
                        tkin->value.lexeme[1] = '\0';
                        bp = fp;
                        return tkin;
                    }
                    break;

                    case '!':
                        state = 34;
                    break;

                    case '=':
                        state = 36;
                    break;

                    case ':':
                        state = 38;
                    break;

                    case '.':
                        state = 41;
                    break;

                    case '<':
                        state = 20;
                    break;

                    case '>':
                        state = 24;
                    break;

                    case '*':
                        state = 12;
                    break;

                    // Run these cases together. No change in state.
                    case '\n': line_number += 1;
                    case '\t':
                    case ' ':  
                        bp = fp;
                    break;

                    // Check for NUM, RNUM, ID/Keywords and Invalid characters.
                    default: 
                    {
                        // Check for ID/Keywords.
                        if(isalpha(lookahead)) 
                            state = 10;

                        else if(isdigit(lookahead)) 
                            state = 1;

                        else if(lookahead == '\0')
                            return NULL;

                        else {   
                            state = 0;
                            print_lexical_error(bp, fp);
                            bp = fp;
                        }
                    }
                }
            }
            break;

            case 1:
            {
                if (isdigit(lookahead)){
                    // Do nothing. State remains 1
                }
                
                else if(lookahead == '.')  {
                    state = 3;
                }

                else {
                    state = 2;
                    fp = (fp + TWIN_BUFFER_SIZE - 1) % TWIN_BUFFER_SIZE; // Retract 1

                    char *str_num = malloc(sizeof(char) * ((fp - bp + TWIN_BUFFER_SIZE + 1) % TWIN_BUFFER_SIZE));
                                
                    if(fp > bp)
                        strncpy(str_num, buffer_for_tokenization + bp, fp - bp);
                    else {
                        strncpy(str_num, buffer_for_tokenization + bp, TWIN_BUFFER_SIZE - bp);
                        strncpy(str_num + TWIN_BUFFER_SIZE - bp, buffer_for_tokenization, fp);
                    }

                    str_num[(fp - bp + TWIN_BUFFER_SIZE) % TWIN_BUFFER_SIZE] = '\0';

                    tkin->type = NUM;
                    tkin->lno = line_number;
                    tkin->value.num = atoi(str_num);
                    free(str_num);
                    bp = fp;
                    return tkin;
                }
            }
            break;

            case 3:
            {
                if(lookahead == '.') {
                    state = 4;
                    fp = (fp + TWIN_BUFFER_SIZE - 2) % TWIN_BUFFER_SIZE; // Retract 2

                    char *str_num = malloc(sizeof(char) * ((fp - bp + TWIN_BUFFER_SIZE + 1) % TWIN_BUFFER_SIZE));

                    if(fp > bp)
                            strncpy(str_num, buffer_for_tokenization + bp, fp - bp);
                    else {
                        strncpy(str_num, buffer_for_tokenization + bp, TWIN_BUFFER_SIZE - bp);
                        strncpy(str_num + TWIN_BUFFER_SIZE - bp, buffer_for_tokenization, fp);
                    }

                    str_num[(fp - bp + TWIN_BUFFER_SIZE) % TWIN_BUFFER_SIZE] = '\0';

                    tkin->type = NUM;
                    tkin->lno = line_number;
                    tkin->value.num = atoi(str_num);
                    free(str_num);
                    bp = fp;
                    return tkin;
                }
                else if(lookahead >= '0' && lookahead <= '9') {
                    state = 5;
                }
                else {   
                    state = 0;
                    fp = (fp + TWIN_BUFFER_SIZE - 1) % TWIN_BUFFER_SIZE; // Retract 1
                    print_lexical_error(bp, fp);
                    bp = fp;
                }
            }
            break;

            case 5:
            {
                if(lookahead == 'e' || lookahead == 'E')
                    state = 6;
                
                else if(lookahead >= '0' && lookahead <= '9') {
                    // Do nothing. State remains 5.
                }

                else{
                    state = 9;
                    fp = (fp + TWIN_BUFFER_SIZE - 1) % TWIN_BUFFER_SIZE; //Retract 1

                    char *str_rnum = malloc(sizeof(char) * ((fp - bp + 1 + TWIN_BUFFER_SIZE) % TWIN_BUFFER_SIZE));
                                        
                    if(fp > bp)
                        strncpy(str_rnum, buffer_for_tokenization + bp, fp - bp);
                    else {
                        strncpy(str_rnum, buffer_for_tokenization + bp, TWIN_BUFFER_SIZE - bp);
                        strncpy(str_rnum + TWIN_BUFFER_SIZE - bp, buffer_for_tokenization, fp);
                    }

                    str_rnum[(fp - bp + TWIN_BUFFER_SIZE) % TWIN_BUFFER_SIZE] = '\0';

                    tkin->type = RNUM;
                    tkin->lno = line_number;
                    tkin->value.rnum = atof(str_rnum);
                    free(str_rnum);
                    bp = fp;
                    return tkin;
                }
            }
            break;

            case 6:
            {
                if(lookahead == '+' || lookahead == '-') {
                    state = 7;
                }

                else if(lookahead >= '0' && lookahead <= '9') {
                    state = 8;
                }

                else {   
                    state = 0;
                    fp = (fp + TWIN_BUFFER_SIZE - 1) % TWIN_BUFFER_SIZE; // Retract 1
                    print_lexical_error(bp, fp);
                    bp = fp;
                }
            }
            break;

            case 7:
            {
                if(lookahead >= '0' && lookahead <= '9') {
                    state = 8;
                }

                else {   
                    state = 0;
                    fp = (fp + TWIN_BUFFER_SIZE - 1) % TWIN_BUFFER_SIZE; // Retract 1
                    print_lexical_error(bp, fp);
                    bp = fp;
                }
            }
            break;

            case 8:
            {
                if(lookahead >= '0' && lookahead <= '9') {
                    // Do nothing. State remains 8.
                }

                else {
                    state = 9;
                    fp = (fp + TWIN_BUFFER_SIZE - 1) % TWIN_BUFFER_SIZE; // Retract 1

                    char *str_rnum = malloc(sizeof(char) * ((fp - bp + 1 + TWIN_BUFFER_SIZE) % TWIN_BUFFER_SIZE));
                                        
                    if(fp > bp)
                        strncpy(str_rnum, buffer_for_tokenization + bp, fp - bp);
                    else {
                        strncpy(str_rnum, buffer_for_tokenization + bp, TWIN_BUFFER_SIZE - bp);
                        strncpy(str_rnum + TWIN_BUFFER_SIZE - bp, buffer_for_tokenization, fp);
                    }

                    str_rnum[(fp - bp + TWIN_BUFFER_SIZE) % TWIN_BUFFER_SIZE] = '\0';

                    tkin->type = RNUM;
                    tkin->lno = line_number;
                    tkin->value.rnum = atof(str_rnum);
                    free(str_rnum);
                    bp = fp;

                    return tkin;
                }
            }
            break;

            case 10:
            {
                if(isalnum(lookahead) || lookahead == '_') {
                    // Do nothing. Stay at state 10;
                }

                else {
                    state = 11;
                    fp = (fp + TWIN_BUFFER_SIZE - 1) % TWIN_BUFFER_SIZE; //Retract 1

                    if(fp > bp)
                        strncpy(tkin->value.lexeme, buffer_for_tokenization + bp, fp - bp);
                    else {
                        strncpy(tkin->value.lexeme, buffer_for_tokenization + bp, TWIN_BUFFER_SIZE - bp);
                        strncpy(tkin->value.lexeme + TWIN_BUFFER_SIZE - bp, buffer_for_tokenization, fp);
                    }

                    tkin->value.lexeme[(fp - bp + TWIN_BUFFER_SIZE) % TWIN_BUFFER_SIZE] = '\0';

                    int keywordType = searchKeyword(tkin->value.lexeme);
                    if(keywordType == -1)
                        tkin->type = ID;
                    else
                        tkin->type = keywordType;

                    tkin->lno = line_number;
                    bp = fp;
                    return tkin;
                }
            }
            break;

            case 12:
            {
                if(lookahead == '*')
                    state = 14;
                
                else {
                    state = 13;
                    fp = (fp + TWIN_BUFFER_SIZE - 1) % TWIN_BUFFER_SIZE; // Retract 1
                    bp = fp;

                    tkin->type = MUL;
                    tkin->lno = line_number;
                    tkin->value.lexeme[0] = '*';
                    tkin->value.lexeme[1] = '\0';
                    return tkin;
                }
            }
            break;

            case 14:
            {
                if(lookahead == '*')
                    state = 15;
                
                else if(lookahead == '\n')
                    line_number++;

                else {
                    // Do nothing
                }
            }
            break;

            case 15:
            {
                // Commentmark is no longer lexeme, we need to restart now, instead of returning.
                if(lookahead == '*') {
                    state = 0;
                    bp = fp;
                }  
                
                else if(lookahead == '\n') {
                    line_number++;
                    state = 14;
                }

                else {
                    state = 14;
                }
            }
            break;
            
            case 20:
            {
                if(lookahead == '=') {
                    state = 21;

                    tkin->type = LE;
                    tkin->lno = line_number;
                    tkin->value.lexeme[0] = '<';
                    tkin->value.lexeme[1] = '=';
                    tkin->value.lexeme[2] = '\0';
                    bp = fp;
                    return tkin;
                }

                else if(lookahead == '<') {
                    state = 22; // No longer final; 
                }

                else {
                    state = 23;
                    fp = (fp + TWIN_BUFFER_SIZE - 1) % TWIN_BUFFER_SIZE; // Retract 1
                    bp = fp;

                    tkin->type = LT;
                    tkin->lno = line_number;
                    tkin->value.lexeme[0] = '<';
                    tkin->value.lexeme[1] = '\0';
                    return tkin;
                }
            }
            break;

            case 24:
            {
                if(lookahead == '=') {
                    state = 25;

                    tkin->type = GE;
                    tkin->lno = line_number;
                    tkin->value.lexeme[0] = '>';
                    tkin->value.lexeme[1] = '=';
                    tkin->value.lexeme[2] = '\0';
                    bp = fp;
                    return tkin;
                }

                else if(lookahead == '>') {
                    state = 26; // No longer final; 
                }

                else {
                    state = 27;
                    fp = (fp + TWIN_BUFFER_SIZE - 1) % TWIN_BUFFER_SIZE; // Retract 1
                    bp = fp;

                    tkin->type = GT;
                    tkin->lno = line_number;
                    tkin->value.lexeme[0] = '>';
                    tkin->value.lexeme[1] = '\0';
                    return tkin;
                }
            }
            break;

            case 34:
            {
                if(lookahead == '=') {
                    state = 35;

                    tkin->type = NE;
                    tkin->lno = line_number;
                    tkin->value.lexeme[0] = '!';
                    tkin->value.lexeme[1] = '=';
                    tkin->value.lexeme[2] = '\0';
                    bp = fp;
                    return tkin;
                }
                
                else {   
                    state = 0;
                    fp = (fp + TWIN_BUFFER_SIZE - 1) % TWIN_BUFFER_SIZE; // Retract 1
                    print_lexical_error(bp, fp);
                    bp = fp;
                }
            }
            break;

            case 36:
            {
                if(lookahead == '=') {
                    state = 37;

                    tkin->type = EQ;
                    tkin->lno = line_number;
                    tkin->value.lexeme[0] = '=';
                    tkin->value.lexeme[1] = '=';
                    tkin->value.lexeme[2] = '\0';
                    bp = fp;
                    return tkin;
                }
                else {   
                    state = 0;
                    fp = (fp + TWIN_BUFFER_SIZE - 1) % TWIN_BUFFER_SIZE; // Retract 1
                    print_lexical_error(bp, fp);
                    bp = fp;
                }
            }
            break;

            case 38:
            {
                if(lookahead == '=') {
                    state = 39;

                    tkin->type = ASSIGNOP;
                    tkin->lno = line_number;
                    tkin->value.lexeme[0] = ':';
                    tkin->value.lexeme[1] = '=';
                    tkin->value.lexeme[2] = '\0';
                    bp = fp;
                    return tkin;
                }

                else {
                    state = 40;
                    fp = (fp + TWIN_BUFFER_SIZE - 1) % TWIN_BUFFER_SIZE; // Retract 1
                    
                    tkin->type = COLON;
                    tkin->lno = line_number;
                    tkin->value.lexeme[0] = ':';
                    tkin->value.lexeme[1] = '\0';
                    bp = fp;
                    return tkin;
                }
            }
            break;

            case 41:
            {
                if(lookahead == '.') {
                    state = 42;

                    tkin->type = RANGEOP;
                    tkin->lno = line_number;
                    tkin->value.lexeme[0] = '.';
                    tkin->value.lexeme[1] = '.';
                    tkin->value.lexeme[2] = '\0';
                    bp = fp;
                    return tkin;
                }
                else {   
                    state = 0;
                    fp = (fp + TWIN_BUFFER_SIZE - 1) % TWIN_BUFFER_SIZE; // Retract 1
                    print_lexical_error(bp, fp);
                    bp = fp;
                }    
            }
            break;

            case 22:
            {
                if(lookahead == '<') {
                    state = 43;

                    tkin->type = DRIVERDEF;
                    tkin->lno = line_number;
                    tkin->value.lexeme[0] = '<';
                    tkin->value.lexeme[1] = '<';
                    tkin->value.lexeme[2] = '<';
                    tkin->value.lexeme[3] = '\0';
                    bp = fp;
                    return tkin;
                }

                else {
                    state = 44;
                    fp = (fp + TWIN_BUFFER_SIZE - 1) % TWIN_BUFFER_SIZE; // Retract 1
                    bp = fp;
                    
                    tkin->type = DEF;
                    tkin->lno = line_number;
                    tkin->value.lexeme[0] = '<';
                    tkin->value.lexeme[1] = '<';
                    tkin->value.lexeme[2] = '\0';
                    return tkin;
                }
            }
            break;

            case 26:
            {
                if(lookahead == '>') {
                    state = 45;

                    tkin->type = DRIVERENDDEF;
                    tkin->lno = line_number;
                    tkin->value.lexeme[0] = '>';
                    tkin->value.lexeme[1] = '>';
                    tkin->value.lexeme[2] = '>';
                    tkin->value.lexeme[3] = '\0';
                    bp = fp;
                    return tkin;
                }

                else {
                    state = 46;
                    fp = (fp + TWIN_BUFFER_SIZE - 1) % TWIN_BUFFER_SIZE; // Retract 1
                    bp = fp;
                    
                    tkin->type = ENDDEF;
                    tkin->lno = line_number;
                    tkin->value.lexeme[0] = '>';
                    tkin->value.lexeme[1] = '>';
                    tkin->value.lexeme[2] = '\0';
                    return tkin;
                }
            }
            break;
        }       
    }
}
