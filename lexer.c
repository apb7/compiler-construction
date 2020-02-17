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
uint line_number = 1;
uint pos_in_buffer = 0;
char *buffer_for_tokenization = NULL;

bool checkPos(uint pos) {
    if(pos < BUFFER_SIZE-1)
        return true;
    return false;
}

char* getStream(FILE *fp) {
    if(feof(fp))
        return NULL;

    // An extra char for delimitor '\0'
    char *buffer_from_file = (char*) calloc(BUFFER_SIZE+1, sizeof(char));
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

tokenInfo* getNextToken(FILE *fp) {

    if(buffer_for_tokenization == NULL) {
        buffer_for_tokenization = getStream(fp);
    }
    else if(pos_in_buffer >= BUFFER_SIZE) {
        free(buffer_for_tokenization);
        buffer_for_tokenization = getStream(fp);
        pos_in_buffer = 0;
    }
    else if(buffer_for_tokenization[pos_in_buffer] == '\0') {
        return NULL;
    }

    tokenInfo* tkin = (tokenInfo*) malloc(sizeof(tokenInfo));
    char lookahead;

    while(pos_in_buffer < BUFFER_SIZE) {
        lookahead = buffer_for_tokenization[pos_in_buffer];
    //    printf("'%d'", lookahead);

        // TODO: Check line number mechanism

        // Non-lookhead tokens: + - / [ ] ( ) , ;
        if(lookahead == '+') {
            tkin->type = PLUS;
            tkin->lno = line_number;
            strcpy(tkin->value.lexeme, "+");
            tkin->value.lexeme[0] = '+';
            tkin->value.lexeme[1] = '\0';
            pos_in_buffer++;
            return tkin;
        }

        if(lookahead == '-') {
            tkin->type = MINUS;
            tkin->lno = line_number;
            tkin->value.lexeme[0] = '-';
            tkin->value.lexeme[1] = '\0';
            pos_in_buffer++;
            return tkin;
        }

        if(lookahead == '/') {
            tkin->type = DIV;
            tkin->lno = line_number;
            tkin->value.lexeme[0] = '/';
            tkin->value.lexeme[1] = '\0';
            pos_in_buffer++;
            return tkin;
        }

        if(lookahead == '[') {
            tkin->type = SQBO;
            tkin->lno = line_number;
            tkin->value.lexeme[0] = '[';
            tkin->value.lexeme[1] = '\0';
            pos_in_buffer++;
            return tkin;
        }

        if(lookahead == ']') {
            tkin->type = SQBC;
            tkin->lno = line_number;
            tkin->value.lexeme[0] = ']';
            tkin->value.lexeme[1] = '\0';
            pos_in_buffer++;
            return tkin;
        }

        if(lookahead == '(') {
            tkin->type = BO;
            tkin->lno = line_number;
            tkin->value.lexeme[0] = '(';
            tkin->value.lexeme[1] = '\0';
            pos_in_buffer++;
            return tkin;
        }

        if(lookahead == ')') {
            tkin->type = BC;
            tkin->lno = line_number;
            tkin->value.lexeme[0] = ')';
            tkin->value.lexeme[1] = '\0';
            pos_in_buffer++;
            return tkin;
        }

        if(lookahead == ',') {
            tkin->type = COMMA;
            tkin->lno = line_number;
            tkin->value.lexeme[0] = ',';
            tkin->value.lexeme[1] = '\0';
            pos_in_buffer++;
            return tkin;
        }

        if(lookahead == ';') {
            tkin->type = SEMICOL;
            tkin->lno = line_number;
            tkin->value.lexeme[0] = ';';
            tkin->value.lexeme[1] = '\0';
            pos_in_buffer++;
            return tkin;
        }

        if(lookahead == '!' && checkPos(pos_in_buffer) &&  buffer_for_tokenization[pos_in_buffer+1] == '=') {
            tkin->type = NE;
            tkin->lno = line_number;
            tkin->value.lexeme[0] = '!';
            tkin->value.lexeme[1] = '=';
            tkin->value.lexeme[2] = '\0';
            pos_in_buffer += 2;
            return tkin;
        }

        if(lookahead == '=' && checkPos(pos_in_buffer) &&  buffer_for_tokenization[pos_in_buffer+1] == '=') {
            tkin->type = EQ;
            tkin->lno = line_number;
            tkin->value.lexeme[0] = '=';
            tkin->value.lexeme[1] = '=';
            tkin->value.lexeme[2] = '\0';
            pos_in_buffer += 2;
            return tkin;
        }

        if(lookahead == ':') {
            if(checkPos(pos_in_buffer) &&  buffer_for_tokenization[pos_in_buffer+1] == '=') {
                tkin->type = ASSIGNOP;
                tkin->lno = line_number;
                tkin->value.lexeme[0] = ':';
                tkin->value.lexeme[1] = '=';
                tkin->value.lexeme[2] = '\0';
                pos_in_buffer += 2;
                return tkin;
            }
            else {
                tkin->type = COLON;
                tkin->lno = line_number;
                tkin->value.lexeme[0] = ':';
                tkin->value.lexeme[1] = '\0';
                pos_in_buffer += 1;
                return tkin;
            }
        }

        if(lookahead == '.' && checkPos(pos_in_buffer) &&  buffer_for_tokenization[pos_in_buffer+1] == '.') {
            tkin->type = RANGEOP;
            tkin->lno = line_number;
            tkin->value.lexeme[0] = '.';
            tkin->value.lexeme[1] = '.';
            tkin->value.lexeme[2] = '\0';
            pos_in_buffer += 2;
            return tkin;
        }

        if(lookahead == '<') {
            if(checkPos(pos_in_buffer)) {
                if(buffer_for_tokenization[pos_in_buffer+1] == '=') {
                    tkin->type = LE;
                    tkin->lno = line_number;
                    tkin->value.lexeme[0] = '<';
                    tkin->value.lexeme[1] = '=';
                    tkin->value.lexeme[2] = '\0';
                    pos_in_buffer += 2;
                    return tkin;
                }

                if(buffer_for_tokenization[pos_in_buffer+1] == '<') {
                    tkin->type = DEF;
                    tkin->lno = line_number;
                    tkin->value.lexeme[0] = '<';
                    tkin->value.lexeme[1] = '<';
                    tkin->value.lexeme[2] = '\0';
                    pos_in_buffer += 2;
                    return tkin;
                }
            }
            else {
                tkin->type = LT;
                tkin->lno = line_number;
                tkin->value.lexeme[0] = '<';
                tkin->value.lexeme[1] = '\0';
                pos_in_buffer += 1;
                return tkin;
            }
        }

        if(lookahead == '>') {
            if(checkPos(pos_in_buffer)) {
                if(buffer_for_tokenization[pos_in_buffer+1] == '=') {
                    tkin->type = GE;
                    tkin->lno = line_number;
                    tkin->value.lexeme[0] = '>';
                    tkin->value.lexeme[1] = '=';
                    tkin->value.lexeme[2] = '\0';
                    pos_in_buffer += 2;
                    return tkin;
                }

                if(buffer_for_tokenization[pos_in_buffer+1] == '>') {
                    tkin->type = ENDDEF;
                    tkin->lno = line_number;
                    tkin->value.lexeme[0] = '>';
                    tkin->value.lexeme[1] = '>';
                    tkin->value.lexeme[2] = '\0';
                    pos_in_buffer += 2;
                    return tkin;
                }
            }
            else {
                tkin->type = GT;
                tkin->lno = line_number;
                tkin->value.lexeme[0] = '>';
                tkin->value.lexeme[1] = '\0';
                pos_in_buffer += 1;
                return tkin;
            }
        }

        if(lookahead == '*') {
            if(checkPos(pos_in_buffer) && buffer_for_tokenization[pos_in_buffer+1] == '*') {
                // TODO: Ignore comments
            }
            else {
                tkin->type = MUL;
                tkin->lno = line_number;
                tkin->value.lexeme[0] = '*';
                tkin->value.lexeme[1] = '\0';
                pos_in_buffer += 1;
                return tkin;
            }
        }

        // TODO: NUM, RNUM and ID/Keywords
        if(isdigit(lookahead)) {
            int i = pos_in_buffer;
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
                            
                            char *str_rnum = malloc(sizeof(char) * (i - pos_in_buffer + 1));
                            strncpy(str_rnum, buffer_for_tokenization + pos_in_buffer, i - pos_in_buffer);
                            str_rnum[i - pos_in_buffer] = '\0';

                            tkin->type = RNUM;
                            tkin->lno = line_number;
                            tkin->value.rnum = atof(str_rnum);
                            pos_in_buffer = i;
                            free(str_rnum);

                            return tkin;
                        }
                        else {
                            //TODO: throw error "123.45e(+/-) but no number"
                        }
                    }
                    else {
                        char *str_rnum = malloc(sizeof(char) * (i - pos_in_buffer + 1));
                        strncpy(str_rnum, buffer_for_tokenization + pos_in_buffer, i - pos_in_buffer);
                        str_rnum[i - pos_in_buffer] = '\0';

                        tkin->type = RNUM;
                        tkin->lno = line_number;
                        tkin->value.rnum = atof(str_rnum);
                        pos_in_buffer = i;
                        free(str_rnum);

                        return tkin;
                    }

                }
                else if(buffer_for_tokenization[i+1] == '.') {
                    char *str_num = malloc(sizeof(char) * (i - pos_in_buffer + 1));
                    strncpy(str_num, buffer_for_tokenization + pos_in_buffer, i - pos_in_buffer);
                    str_num[i - pos_in_buffer] = '\0';

                    tkin->type = NUM;
                    tkin->lno = line_number;
                    tkin->value.num = atoi(str_num);
                    pos_in_buffer = i - 1;
                    free(str_num);

                    return tkin;
                }
                else {
                    //TODO : throw error "1234.rubbish"
                }
            }
            else {
                char *str_num = malloc(sizeof(char) * (i - pos_in_buffer + 1));
                strncpy(str_num, buffer_for_tokenization + pos_in_buffer, i - pos_in_buffer);
                str_num[i - pos_in_buffer] = '\0';

                tkin->type = NUM;
                tkin->lno = line_number;
                tkin->value.num = atoi(str_num);
                pos_in_buffer = i;
                free(str_num);

                return tkin;
            }

        }

        if(isalpha(lookahead)) {
            int i = pos_in_buffer;

            while(isalnum(buffer_for_tokenization[i]) || buffer_for_tokenization[i] == '_')
                i++;

            if( i - pos_in_buffer > 20) {
                //TODO: throw error
            }
            else{
                strncpy(tkin->value.lexeme, buffer_for_tokenization + pos_in_buffer, i - pos_in_buffer);
                tkin->value.lexeme[i - pos_in_buffer] = '\0';

                //TODO: check for keywords!!!
                tkin->type = ID;
                tkin->lno = line_number;
                pos_in_buffer = i;

                return tkin;
            }
        }

        if(lookahead == '\n') {
            line_number += 1;
            pos_in_buffer += 1;
        }

        else if(lookahead == '\t' || lookahead == ' ') {
            pos_in_buffer++;
        }
        else
            pos_in_buffer++;

    }
    return NULL;
}

// int main() {
//     removeComments("abc.txt", "abc1.txt");

//     return 0;
// }
