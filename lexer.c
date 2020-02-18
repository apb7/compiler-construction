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

    if(fp == 0 || fp == BUFFER_SIZE) {
        getStream(file_ptr);
    }

    // TODO: Clarify allocation.
    tokenInfo* tkin = (tokenInfo*) malloc(sizeof(tokenInfo));
    char lookahead;

    while(buffer_for_tokenization[fp] != '\0') {
        lookahead = buffer_for_tokenization[fp];

        // TODO: Maintain state variable (why?)

        // Non-lookhead tokens: + - / [ ] ( ) , ;
        switch (lookahead)
        {
            case '+':
            {
                tkin->type = PLUS;
                tkin->lno = line_number;
                tkin->value.lexeme[0] = '+';
                tkin->value.lexeme[1] = '\0';
                fp = (fp + 1) % TWIN_BUFFER_SIZE;
                bp = fp;
                return tkin;
            }
            break;

            case '-':
            {
                tkin->type = MINUS;
                tkin->lno = line_number;
                tkin->value.lexeme[0] = '-';
                tkin->value.lexeme[1] = '\0';
                fp = (fp + 1) % TWIN_BUFFER_SIZE;
                bp = fp;
                return tkin;
            }
            break;

            case '/':
            {
                tkin->type = DIV;
                tkin->lno = line_number;
                tkin->value.lexeme[0] = '/';
                tkin->value.lexeme[1] = '\0';
                fp = (fp + 1) % TWIN_BUFFER_SIZE;
                bp = fp;
                return tkin;
            }
            break;

            case '[':
            {
                tkin->type = SQBO;
                tkin->lno = line_number;
                tkin->value.lexeme[0] = '[';
                tkin->value.lexeme[1] = '\0';
                fp = (fp + 1) % TWIN_BUFFER_SIZE;
                bp = fp;
                return tkin;
            }
            break;

            case ']':
            {
                tkin->type = SQBC;
                tkin->lno = line_number;
                tkin->value.lexeme[0] = ']';
                tkin->value.lexeme[1] = '\0';
                fp = (fp + 1) % TWIN_BUFFER_SIZE;
                bp = fp;
                return tkin;
            }
            break;

            case '(':
            {
                tkin->type = BO;
                tkin->lno = line_number;
                tkin->value.lexeme[0] = '(';
                tkin->value.lexeme[1] = '\0';
                fp = (fp + 1) % TWIN_BUFFER_SIZE;
                bp = fp;
                return tkin;
            }
            break;

            case ')':
            {
                tkin->type = BC;
                tkin->lno = line_number;
                tkin->value.lexeme[0] = ')';
                tkin->value.lexeme[1] = '\0';
                fp = (fp + 1) % TWIN_BUFFER_SIZE;
                bp = fp;
                return tkin;
            }
            break;

            case ',':
            {
                tkin->type = COMMA;
                tkin->lno = line_number;
                tkin->value.lexeme[0] = ',';
                tkin->value.lexeme[1] = '\0';
                fp = (fp + 1) % TWIN_BUFFER_SIZE;
                bp = fp;
                return tkin;
            }
            break;

            case ';':
            {
                tkin->type = SEMICOL;
                tkin->lno = line_number;
                tkin->value.lexeme[0] = ';';
                tkin->value.lexeme[1] = '\0';
                fp = (fp + 1) % TWIN_BUFFER_SIZE;
                bp = fp;
                return tkin;
            }
            break;

            case '!':
            {
                fp = (fp + 1) % TWIN_BUFFER_SIZE;

                if(fp == 0 || fp == BUFFER_SIZE)
                    getStream(file_ptr);

                char lookahead_one = buffer_for_tokenization[fp];

                switch(lookahead_one)
                {
                    case '=':
                    {
                        tkin->type = NE;
                        tkin->lno = line_number;
                        tkin->value.lexeme[0] = '!';
                        tkin->value.lexeme[1] = '=';
                        tkin->value.lexeme[2] = '\0';
                        fp = (fp + 1) % TWIN_BUFFER_SIZE;
                        bp = fp;
                        return tkin;
                    }
                    break;

                    // Trap state for !=
                    default:
                    {
                        // No retract required since lexical error.
                        // Lexical Error for !
                        print_lexical_error(bp, fp);
                        // Since fp has already been moved forward, bring bp to fp.
                        bp = fp;
                    }
                    break;
                }

            }
            break;

            case '=':
            {
                fp = (fp + 1) % TWIN_BUFFER_SIZE;

                if(fp == 0 || fp == BUFFER_SIZE)
                    getStream(file_ptr);

                char lookahead_one = buffer_for_tokenization[fp];

                switch(lookahead_one)
                {
                    case '=':
                    {
                        tkin->type = EQ;
                        tkin->lno = line_number;
                        tkin->value.lexeme[0] = '=';
                        tkin->value.lexeme[1] = '=';
                        tkin->value.lexeme[2] = '\0';
                        fp = (fp + 1) % TWIN_BUFFER_SIZE;
                        bp = fp;
                        return tkin;
                    }
                    break;

                    // Trap state for ==
                    default:
                    {
                        // Lexical Error for =
                        print_lexical_error(bp, fp);
                        bp = fp;
                    }
                    break;
                }

            }
            break;

            case ':':
            {
                fp = (fp + 1) % TWIN_BUFFER_SIZE;

                if(fp == 0 || fp == BUFFER_SIZE)
                    getStream(file_ptr);

                char lookahead_one = buffer_for_tokenization[fp];

                switch(lookahead_one)
                {
                    case '=':
                    {
                        tkin->type = ASSIGNOP;
                        tkin->lno = line_number;
                        tkin->value.lexeme[0] = ':';
                        tkin->value.lexeme[1] = '=';
                        tkin->value.lexeme[2] = '\0';
                        fp = (fp + 1) % TWIN_BUFFER_SIZE;
                        bp = fp;
                        return tkin;
                    }
                    break;

                    // Other state for :
                    default:
                    {
                        // Retract by 1.
                        // Since fp has already been incremented, move bp to fp.
                        tkin->type = COLON;
                        tkin->lno = line_number;
                        tkin->value.lexeme[0] = ':';
                        tkin->value.lexeme[1] = '\0';
                        bp = fp;
                        return tkin;
                    }
                    break;
                }

            }
            break;

            case '.':
            {
                fp = (fp + 1) % TWIN_BUFFER_SIZE;

                if(fp == 0 || fp == BUFFER_SIZE)
                    getStream(file_ptr);

                char lookahead_one = buffer_for_tokenization[fp];

                switch(lookahead_one)
                {
                    case '.':
                    {
                        tkin->type = RANGEOP;
                        tkin->lno = line_number;
                        tkin->value.lexeme[0] = '.';
                        tkin->value.lexeme[1] = '.';
                        tkin->value.lexeme[2] = '\0';
                        fp = (fp + 1) % TWIN_BUFFER_SIZE;
                        bp = fp;
                        return tkin;
                    }
                    break;

                    // Trap state for ..
                    default:
                    {
                        // Lexical Error for .
                        print_lexical_error(bp, fp);
                        bp = fp;
                    }
                    break;
                }

            }
            break;

            case '<':
            {
                fp = (fp + 1) % TWIN_BUFFER_SIZE;

                if(fp == 0 || fp == BUFFER_SIZE)
                    getStream(file_ptr);

                char lookahead_one = buffer_for_tokenization[fp];

                switch(lookahead_one)
                {
                    case '=':
                    {
                        tkin->type = LE;
                        tkin->lno = line_number;
                        tkin->value.lexeme[0] = '<';
                        tkin->value.lexeme[1] = '=';
                        tkin->value.lexeme[2] = '\0';
                        fp = (fp + 1) % TWIN_BUFFER_SIZE;
                        bp = fp;
                        return tkin;
                    }
                    break;

                    case '<':
                    {
                        fp = (fp + 1) % TWIN_BUFFER_SIZE;

                        if(fp == 0 || fp == BUFFER_SIZE)
                            getStream(file_ptr);

                        char lookahead_two = buffer_for_tokenization[fp];

                        switch(lookahead_two)
                        {
                            case '<':
                            {
                                tkin->type = DRIVERDEF;
                                tkin->lno = line_number;
                                tkin->value.lexeme[0] = '<';
                                tkin->value.lexeme[1] = '<';
                                tkin->value.lexeme[2] = '<';
                                tkin->value.lexeme[3] = '\0';
                                fp = (fp + 1) % TWIN_BUFFER_SIZE;
                                bp = fp;
                                return tkin;
                            }
                            break;

                            default:
                            {
                                // For <<
                                tkin->type = DEF;
                                tkin->lno = line_number;
                                tkin->value.lexeme[0] = '<';
                                tkin->value.lexeme[1] = '<';
                                tkin->value.lexeme[2] = '\0';
                                bp = fp;
                                return tkin;
                            }
                            break;
                        }
                    }
                    break;

                    // Other state for <
                    default:
                    {
                        // Retract by 1.
                        // Since fp has already been incremented, move bp to fp. (TODO: Check!)
                        tkin->type = LT;
                        tkin->lno = line_number;
                        tkin->value.lexeme[0] = '<';
                        tkin->value.lexeme[1] = '\0';
                        bp = fp;
                        return tkin;
                    }
                    break;
                }

            }
            break;

            case '>':
            {
                fp = (fp + 1) % TWIN_BUFFER_SIZE;

                if(fp == 0 || fp == BUFFER_SIZE)
                    getStream(file_ptr);

                char lookahead_one = buffer_for_tokenization[fp];

                switch(lookahead_one)
                {
                    case '=':
                    {
                        tkin->type = GE;
                        tkin->lno = line_number;
                        tkin->value.lexeme[0] = '>';
                        tkin->value.lexeme[1] = '=';
                        tkin->value.lexeme[2] = '\0';
                        fp = (fp + 1) % TWIN_BUFFER_SIZE;
                        bp = fp;
                        return tkin;
                    }
                    break;

                    case '>':
                    {
                        fp = (fp + 1) % TWIN_BUFFER_SIZE;

                        if(fp == 0 || fp == BUFFER_SIZE)
                            getStream(file_ptr);

                        char lookahead_two = buffer_for_tokenization[fp];

                        switch(lookahead_two)
                        {
                            case '>':
                            {
                                tkin->type = DRIVERENDDEF;
                                tkin->lno = line_number;
                                tkin->value.lexeme[0] = '>';
                                tkin->value.lexeme[1] = '>';
                                tkin->value.lexeme[2] = '>';
                                tkin->value.lexeme[3] = '\0';
                                fp = (fp + 1) % TWIN_BUFFER_SIZE;
                                bp = fp;
                                return tkin;
                            }
                            break;

                            default:
                            {
                                // For >>
                                tkin->type = ENDDEF;
                                tkin->lno = line_number;
                                tkin->value.lexeme[0] = '>';
                                tkin->value.lexeme[1] = '>';
                                tkin->value.lexeme[2] = '\0';
                                bp = fp;
                                return tkin;
                            }
                            break;
                        }
                    }
                    break;

                    // Other state for >
                    default:
                    {
                        // Retract by 1.
                        // Since fp has already been incremented, move bp to fp. (TODO: Check!)
                        tkin->type = GT;
                        tkin->lno = line_number;
                        tkin->value.lexeme[0] = '>';
                        tkin->value.lexeme[1] = '\0';
                        bp = fp;
                        return tkin;
                    }
                    break;
                }

            }
            break;

            case '*':
            {
                fp = (fp + 1) % TWIN_BUFFER_SIZE;

                if(fp == 0 || fp == BUFFER_SIZE)
                    getStream(file_ptr);

                char lookahead_one = buffer_for_tokenization[fp];

                switch(lookahead_one)
                {
                    case '*':
                    {
                        // Ignore comments.
                        char lookahead_i;

                        do {
                            fp = (fp + 1) % TWIN_BUFFER_SIZE;

                            if(fp == 0 || fp == BUFFER_SIZE)
                                getStream(file_ptr);

                            lookahead_i = buffer_for_tokenization[fp];

                            if(lookahead_i == '\n')
                                line_number++;

                            // first '*' encountered
                            if(lookahead_i == '*') {
                                fp = (fp + 1) % TWIN_BUFFER_SIZE;

                                if(fp == 0 || fp == BUFFER_SIZE)
                                    getStream(file_ptr);

                                lookahead_i = buffer_for_tokenization[fp]; // maybe second '*'

                                if(lookahead_i == '\n')
                                    line_number++;
                            }
                        } while(lookahead_i != '*'); // will always check for second consecutive '*'
                        
                        fp = (fp + 1) % TWIN_BUFFER_SIZE;
                        bp = fp; // move 1 ahead of second consecutive '*'
                    }
                    break;

                    // Other state for *
                    default:
                    {
                        // Retract by 1
                        tkin->type = MUL;
                        tkin->lno = line_number;
                        tkin->value.lexeme[0] = '*';
                        tkin->value.lexeme[1] = '\0';
                        bp = fp;
                        return tkin;
                    }
                    break;
                }

            }
            break;

            // Run these cases together.
            case '\n': line_number += 1;
            case '\t':
            case ' ': 
                fp = (fp + 1) % TWIN_BUFFER_SIZE; 
                if(fp == 0 || fp == BUFFER_SIZE)
                    getStream(file_ptr);
                bp = fp;
            break;

            // Check for NUM, RNUM, ID/Keywords and Invalid characters.
            default: 
            {
                // Check for ID/Keywords.
                if(isalpha(lookahead)) {
                    char lookahead_i;

                    do {
                        fp = (fp + 1) % TWIN_BUFFER_SIZE;

                        if(fp == 0 || fp == BUFFER_SIZE)
                            getStream(file_ptr);

                        lookahead_i = buffer_for_tokenization[fp];

                    } while(isalnum(lookahead_i) || lookahead_i == '_');

                    if((fp - bp + TWIN_BUFFER_SIZE) % TWIN_BUFFER_SIZE > 20) {
                        print_lexical_error(bp, fp);
                        bp = fp;
                    }
                    else {
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

                else if(isdigit(lookahead)) {

                    char lookahead_i;

                    do{
                        fp = (fp + 1) % TWIN_BUFFER_SIZE;

                        if(fp == 0 || fp == BUFFER_SIZE)
                            getStream(file_ptr);

                        lookahead_i = buffer_for_tokenization[fp];

                    } while(isdigit(lookahead_i));

                    switch(lookahead_i){
                        case '.':
                        {
                            fp = (fp + 1) % TWIN_BUFFER_SIZE;

                            if(fp == 0 || fp == BUFFER_SIZE)
                                getStream(file_ptr);

                            char lookahead_i_one = buffer_for_tokenization[fp];

                            if(isdigit(lookahead_i_one)) {

                                do{
                                    fp = (fp + 1) % TWIN_BUFFER_SIZE;

                                    if(fp == 0 || fp == BUFFER_SIZE)
                                        getStream(file_ptr);

                                    lookahead_i = buffer_for_tokenization[fp];

                                } while(isdigit(lookahead_i));

                                if(lookahead_i == 'e') {
                                    fp = (fp + 1) % TWIN_BUFFER_SIZE;

                                    if(fp == 0 || fp == BUFFER_SIZE)
                                        getStream(file_ptr);

                                    lookahead_i_one = buffer_for_tokenization[fp];
                                        
                                    if(lookahead_i_one == '+' || lookahead_i_one == '-') {
                                        fp = (fp + 1) % TWIN_BUFFER_SIZE;

                                        if(fp == 0 || fp == BUFFER_SIZE)
                                            getStream(file_ptr);

                                        lookahead_i_one = buffer_for_tokenization[fp];
                                    }

                                    if(isdigit(lookahead_i_one)) {
                                        do{
                                            fp = (fp + 1) % TWIN_BUFFER_SIZE;

                                            if(fp == 0 || fp == BUFFER_SIZE)
                                                getStream(file_ptr);

                                            lookahead_i = buffer_for_tokenization[fp];

                                        } while(isdigit(lookahead_i));
                                        
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
                                        bp = fp;
                                        free(str_rnum);

                                        return tkin;
                                    }
                                    else {
                                        //TODO: throw error "123.45e(+/-) but no number"
                                        print_lexical_error(bp, fp);
                                        bp = fp;
                                    }
                                }
                                else {
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
                                    bp = fp;
                                    free(str_rnum);

                                    return tkin;
                                }
                            }

                            else if(lookahead_i_one == '.') {
                                char *str_num = malloc(sizeof(char) * ((fp - bp + TWIN_BUFFER_SIZE) % TWIN_BUFFER_SIZE));

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
                                fp = (fp + TWIN_BUFFER_SIZE - 1) % TWIN_BUFFER_SIZE;
                                bp = fp;
                                free(str_num);

                                return tkin;
                            }

                            else {
                                //TODO : throw error "1234.rubbish"
                                print_lexical_error(bp, fp);
                                bp = fp;
                            }
                        }
                        break;

                        default:
                        {
                            char *str_num = malloc(sizeof(char) * ((fp - bp + 1 + TWIN_BUFFER_SIZE) % TWIN_BUFFER_SIZE));
                            
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
                            bp = fp;
                            free(str_num);

                            return tkin;
                        }
                    }
                }

                else
                {
                    print_lexical_error(bp, (fp+1)%TWIN_BUFFER_SIZE);

                    fp = (fp + 1) % TWIN_BUFFER_SIZE; 
                    if(fp == 0 || fp == BUFFER_SIZE)
                            getStream(file_ptr);
                    bp = fp;
                }
            }
        }
    }

    return NULL;
}
