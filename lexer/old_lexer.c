// DO NOT USE THIS FILE. OUTDATED.

tokenInfo* getNextToken(FILE *file_ptr) {

    if(buffer_for_tokenization == NULL) {
        buffer_for_tokenization = getStream(file_ptr);
    }
    else if(fp >= BUFFER_SIZE) {
        free(buffer_for_tokenization);
        buffer_for_tokenization = getStream(file_ptr);
        fp = 0;
    }
    else if(buffer_for_tokenization[fp] == '\0') {
        return NULL;
    }

    tokenInfo* tkin = (tokenInfo*) malloc(sizeof(tokenInfo));
    char lookahead;

    while(fp < BUFFER_SIZE) {
        lookahead = buffer_for_tokenization[fp];
    //    printf("'%d'", lookahead);

        // TODO: Check line number mechanism

        // Non-lookhead tokens: + - / [ ] ( ) , ;
        if(lookahead == '+') {
            tkin->type = PLUS;
            tkin->lno = line_number;
            strcpy(tkin->value.lexeme, "+");
            fp++;
            return tkin;
        }

        if(lookahead == '-') {
            tkin->type = MINUS;
            tkin->lno = line_number;
            tkin->value.lexeme[0] = '-';
            tkin->value.lexeme[1] = '\0';
            fp++;
            return tkin;
        }

        if(lookahead == '/') {
            tkin->type = DIV;
            tkin->lno = line_number;
            tkin->value.lexeme[0] = '/';
            tkin->value.lexeme[1] = '\0';
            fp++;
            return tkin;
        }

        if(lookahead == '[') {
            tkin->type = SQBO;
            tkin->lno = line_number;
            tkin->value.lexeme[0] = '[';
            tkin->value.lexeme[1] = '\0';
            fp++;
            return tkin;
        }

        if(lookahead == ']') {
            tkin->type = SQBC;
            tkin->lno = line_number;
            tkin->value.lexeme[0] = ']';
            tkin->value.lexeme[1] = '\0';
            fp++;
            return tkin;
        }

        if(lookahead == '(') {
            tkin->type = BO;
            tkin->lno = line_number;
            tkin->value.lexeme[0] = '(';
            tkin->value.lexeme[1] = '\0';
            fp++;
            return tkin;
        }

        if(lookahead == ')') {
            tkin->type = BC;
            tkin->lno = line_number;
            tkin->value.lexeme[0] = ')';
            tkin->value.lexeme[1] = '\0';
            fp++;
            return tkin;
        }

        if(lookahead == ',') {
            tkin->type = COMMA;
            tkin->lno = line_number;
            tkin->value.lexeme[0] = ',';
            tkin->value.lexeme[1] = '\0';
            fp++;
            return tkin;
        }

        if(lookahead == ';') {
            tkin->type = SEMICOL;
            tkin->lno = line_number;
            tkin->value.lexeme[0] = ';';
            tkin->value.lexeme[1] = '\0';
            fp++;
            return tkin;
        }

        if(lookahead == '!' && checkPos(fp) &&  buffer_for_tokenization[fp+1] == '=') {
            tkin->type = NE;
            tkin->lno = line_number;
            tkin->value.lexeme[0] = '!';
            tkin->value.lexeme[1] = '=';
            tkin->value.lexeme[2] = '\0';
            fp += 2;
            return tkin;
        }

        if(lookahead == '=' && checkPos(fp) &&  buffer_for_tokenization[fp+1] == '=') {
            tkin->type = EQ;
            tkin->lno = line_number;
            tkin->value.lexeme[0] = '=';
            tkin->value.lexeme[1] = '=';
            tkin->value.lexeme[2] = '\0';
            fp += 2;
            return tkin;
        }

        if(lookahead == ':') {
            if(checkPos(fp) &&  buffer_for_tokenization[fp+1] == '=') {
                tkin->type = ASSIGNOP;
                tkin->lno = line_number;
                tkin->value.lexeme[0] = ':';
                tkin->value.lexeme[1] = '=';
                tkin->value.lexeme[2] = '\0';
                fp += 2;
                return tkin;
            }
            else {
                tkin->type = COLON;
                tkin->lno = line_number;
                tkin->value.lexeme[0] = ':';
                tkin->value.lexeme[1] = '\0';
                fp += 1;
                return tkin;
            }
        }

        if(lookahead == '.' && checkPos(fp) &&  buffer_for_tokenization[fp+1] == '.') {
            tkin->type = RANGEOP;
            tkin->lno = line_number;
            tkin->value.lexeme[0] = '.';
            tkin->value.lexeme[1] = '.';
            tkin->value.lexeme[2] = '\0';
            fp += 2;
            return tkin;
        }

        if(lookahead == '<') {
            if(checkPos(fp)) {
                if(buffer_for_tokenization[fp+1] == '=') {
                    tkin->type = LE;
                    tkin->lno = line_number;
                    tkin->value.lexeme[0] = '<';
                    tkin->value.lexeme[1] = '=';
                    tkin->value.lexeme[2] = '\0';
                    fp += 2;
                    return tkin;
                }

                if(buffer_for_tokenization[fp+1] == '<') {
                    tkin->type = DEF;
                    tkin->lno = line_number;
                    tkin->value.lexeme[0] = '<';
                    tkin->value.lexeme[1] = '<';
                    tkin->value.lexeme[2] = '\0';
                    fp += 2;
                    return tkin;
                }
            }
            else {
                tkin->type = LT;
                tkin->lno = line_number;
                tkin->value.lexeme[0] = '<';
                tkin->value.lexeme[1] = '\0';
                fp += 1;
                return tkin;
            }
        }

        if(lookahead == '>') {
            if(checkPos(fp)) {
                if(buffer_for_tokenization[fp+1] == '=') {
                    tkin->type = GE;
                    tkin->lno = line_number;
                    tkin->value.lexeme[0] = '>';
                    tkin->value.lexeme[1] = '=';
                    tkin->value.lexeme[2] = '\0';
                    fp += 2;
                    return tkin;
                }

                if(buffer_for_tokenization[fp+1] == '>') {
                    tkin->type = ENDDEF;
                    tkin->lno = line_number;
                    tkin->value.lexeme[0] = '>';
                    tkin->value.lexeme[1] = '>';
                    tkin->value.lexeme[2] = '\0';
                    fp += 2;
                    return tkin;
                }
            }
            else {
                tkin->type = GT;
                tkin->lno = line_number;
                tkin->value.lexeme[0] = '>';
                tkin->value.lexeme[1] = '\0';
                fp += 1;
                return tkin;
            }
        }

        if(lookahead == '*') {
            if(checkPos(fp) && buffer_for_tokenization[fp+1] == '*') {
                // TODO: Ignore comments
            }
            else {
                tkin->type = MUL;
                tkin->lno = line_number;
                tkin->value.lexeme[0] = '*';
                tkin->value.lexeme[1] = '\0';
                fp += 1;
                return tkin;
            }
        }

        // TODO: NUM, RNUM and ID/Keywords
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

        if(lookahead == '\n') {
            line_number += 1;
            fp += 1;
        }

        else if(lookahead == '\t' || lookahead == ' ') {
            fp++;
        }
        else
            fp++;

    }
    return NULL;
}
