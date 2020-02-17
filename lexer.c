#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "lexerDef.h"

typedef unsigned int uint;

uint BUFFER_SIZE = 512;
uint TWIN_BUFFER_SIZE = 1024;
uint line_number = 1;
uint bp = 0; // begin ptr
uint fp = 0; // forward ptr
char *buffer_for_tokenization[1024]; // a global buffer of size 2 * BUFFER_SIZE

bool checkPos(uint pos) {
    if(pos < BUFFER_SIZE-1)
        return true;
    return false;
}

bool getStream(FILE *file_ptr) {
    static int status = 1;

    if(feof(file_ptr))
        return false;

    // TODO: Fill the global buffer with zeros.

    // Selecting a half from the global buffer, alternatively.
    status ^= 1;

    fread(buffer_for_tokenization + (status * BUFFER_SIZE), BUFFER_SIZE, sizeof(char), file_ptr);
    return true;
}

// TODO: Correct getStream functionality.
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

    char *buffer_to_read = getStream(fp_testcaseFile);
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
        buffer_to_read = getStream(fp_testcaseFile);
    }

    free(buffer_to_write);
}

// Prints the string in global buffer from start to end, including end.
// [start, end]
void print_lexical_error(uint start, uint end) {
    printf("LEXICAL ERROR: No such token ");

    // Do we need these?
    start = start % TWIN_BUFFER_SIZE;
    end = end % TWIN_BUFFER_SIZE;

    do {
        print("%c", buffer_for_tokenization[start]);
        start = (start + 1) % TWIN_BUFFER_SIZE;
    }
    while(start != end);

    print(" found on line number %ui.\n", line_number);
}

tokenInfo* getNextToken(FILE *file_ptr) {

    if(fp == 0 || fp == BUFFER_SIZE) {
        getStream(file_ptr);
    }

    // TODO: Clarify allocation.
    tokenInfo* tkin = (tokenInfo*) malloc(sizeof(tokenInfo));
    char lookahead;

    while(fp < TWIN_BUFFER_SIZE) {
        lookahead = buffer_for_tokenization[fp];
    //    printf("'%d'", lookahead);

        // TODO: Check line number mechanism
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
                tkin->value.lexeme[0] = '-';
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
                        print_lexical_error(bp, (fp+TWIN_BUFFER_SIZE-1)%TWIN_BUFFER_SIZE);
                        // Since fp has already been moved forward, bring bp to fp.
                        bp = fp;
                        // Recursive call not required. We are in a loop.
                        // return getNextToken(file_ptr);
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
                        print_lexical_error(bp, (fp+TWIN_BUFFER_SIZE-1)%TWIN_BUFFER_SIZE);
                        bp = fp;
                        // return getNextToken(file_ptr);
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
                        // Since fp has already been incremented, move bp to fp. (TODO: Check!)
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
                        print_lexical_error(bp, (fp+TWIN_BUFFER_SIZE-1)%TWIN_BUFFER_SIZE);
                        bp = fp;
                        // return getNextToken(file_ptr);
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
                        // TODO: Ignore comments.
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

            // TODO: Modify NUM, RNUM and ID/Keywords code for case.
/*
        if(isdigit(lookahead)) {
            int i = fp;
            while(isdigit(buffer_for_tokenization[i]))
                i++;

            if(buffer_for_tokenization[i] == '.') {
                if(isdigit(buffer_for_tokenization[i+1])) {
                    i++;
                    while(isdigit(buffer_for_tokenization[i]))
                        i++;

                    if(buffer_for_tokenization[i] == 'e') {
                        i++;
                        if(buffer_for_tokenization[i] == '+' || buffer_for_tokenization[i] == '-') {
                            i++;
                        }
                        if(isdigit(buffer_for_tokenization[i])) {
                            while(isdigit(buffer_for_tokenization[i]))
                                i++;
                            
                            char *str_rnum = malloc(sizeof(char) * (i - fp + 1));
                            strncpy(str_rnum, buffer_for_tokenization + fp, i - fp);
                            str_rnum[i - fp] = '\0';

                            tkin->type = RNUM;
                            tkin->lno = line_number;
                            tkin->value.rnum = atof(str_rnum);
                            fp = i;
                            free(str_rnum);

                            return tkin;
                        }
                        else {
                            //TODO: throw error "123.45e(+/-) but no number"
                        }
                    }
                    else {
                        char *str_rnum = malloc(sizeof(char) * (i - fp + 1));
                        strncpy(str_rnum, buffer_for_tokenization + fp, i - fp);
                        str_rnum[i - fp] = '\0';

                        tkin->type = RNUM;
                        tkin->lno = line_number;
                        tkin->value.rnum = atof(str_rnum);
                        fp = i;
                        free(str_rnum);

                        return tkin;
                    }

                }
                else if(buffer_for_tokenization[i+1] == '.') {
                    char *str_num = malloc(sizeof(char) * (i - fp + 1));
                    strncpy(str_num, buffer_for_tokenization + fp, i - fp);
                    str_num[i - fp] = '\0';

                    tkin->type = NUM;
                    tkin->lno = line_number;
                    tkin->value.num = atoi(str_num);
                    fp = i - 1;
                    free(str_num);

                    return tkin;
                }
                else {
                    //TODO : throw error "1234.rubbish"
                }
            }
            else {
                char *str_num = malloc(sizeof(char) * (i - fp + 1));
                strncpy(str_num, buffer_for_tokenization + fp, i - fp);
                str_num[i - fp] = '\0';

                tkin->type = NUM;
                tkin->lno = line_number;
                tkin->value.num = atoi(str_num);
                fp = i;
                free(str_num);

                return tkin;
            }

        }

        if(isalpha(lookahead)) {
            int i = fp;

            while(isalnum(buffer_for_tokenization[i]) || buffer_for_tokenization[i] == '_')
                i++;

            if( i - fp > 20) {
                //TODO: throw error
            }
            else{
                strncpy(tkin->value.lexeme, buffer_for_tokenization + fp, i - fp);
                tkin->value.lexeme[i - fp] = '\0';

                //TODO: check for keywords!!!
                tkin->type = ID;
                tkin->lno = line_number;
                fp = i;

                return tkin;
            }
        }
*/
            // Run these cases together.
            case '\n': line_number += 1;
            case '\t':
            case ' ':
            default: fp = (fp + 1) % TWIN_BUFFER_SIZE;
        }
    }

    return NULL;
}

// int main() {
//     removeComments("abc.txt", "abc1.txt");

//     return 0;
// }
