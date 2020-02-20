#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "lexer/lexer.h"
#include "hash.h"
#include "set.h"
#include "utils.h"
#include "config.h"

hashTable *keyword_ht;
grammarNode *G;
struct hashTable *mt;
intSet* firstSet;
intSet* followSet;
hashTable *keyword_ht;


int main(int argc, char *argv[]) {

    // Create and poplate hash table for keywords
    keyword_ht = createHashTable(KEYWORD_HT_SIZE);

    char* keywords[] = {
        #define K(a,b,c) b,
        #include "data/keywords.txt"
        #undef K
        "#"
    };
    //# indicates end of string array

    fillHashTable(keywords,keyword_ht);

    // Populate token name
    char *tokenName[] = {
        #define K(a,b,c) c,
        #define X(a,b) b,
        #include "data/keywords.txt"
        #include "data/tokens.txt"
        "#"
        #undef X
        #undef K
    };


    //printHashTable(keyword_ht);

    //removeComments("abc.txt", "abc1.txt");
    FILE *fp;
    if (argc == 2)
        fp = fopen(argv[1], "r");
    else
        fp = fopen("../test_cases.txt", "r");
    //fp = fopen("abc2.txt", "r");
    int i;
    // Print all tokens.
    tokenInfo *tk = getNextToken(fp);
    while(tk!=NULL) {

        printf("LNo %6d ", tk->lno);
        printf("%20s ", tokenName[tk->type]);
        printf("%20s ", tk->lexeme);

        if(tk->type==NUM)
            printf("%20d\n", tk->value.num);

        else if(tk->type ==RNUM)
            printf("%20f\n", tk->value.rnum);

        else
            printf("%20s\n", "");

        free(tk);
        tk = getNextToken(fp);
        i++;

        if(i == 125000){
            printf("Interrupted");
            break;
        }
    }

}
