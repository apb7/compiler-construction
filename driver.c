#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "hash.h"
#include "lexer.h"

struct hashTable *ht;

int main() {

    //  Create and poplate hash table for keywords
    ht= createHashTable(31);

    FILE *fp = fopen("keywords.txt", "r");

    if(fp == NULL){
        printf("Can't open file");
        exit(1);
    }

    char keyword[21];
    int i = 0;
    while(!feof(fp)){
        fscanf(fp, " %s", keyword);
        addKeyword(keyword, i++);
    }
    fclose(fp);

    // Populate token name
    char tokenName[57][21]; // Sync with terminals.txt

    fp = fopen("terminals.txt", "r");

    if(fp == NULL){
        printf("Can't open file");
        exit(1);
    }

    i = 0;
    while(!feof(fp)){
        fscanf(fp, " %s", tokenName[i++]);
    }
    fclose(fp);

    //printHashTable();

    //removeComments("abc.txt", "abc1.txt");

    fp = fopen("test_cases.txt", "r");

    tokenInfo *tk = getNextToken(fp);

    while(tk!=NULL){

        printf("LNo %d ", tk->lno);
        printf("%d %s ", tk->type, tokenName[tk->type]);

        if(tk->type==NUM)
            printf("%d\n", tk->value.num);

        else if(tk->type ==RNUM)
            printf("%f\n", tk->value.rnum);

        else
            printf("%s\n", tk->value.lexeme);

        tk = getNextToken(fp);
        i++;

        if(i== 125000)
            break;
    }

}
