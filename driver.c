#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "hash.h"
#include "lexer.h"

struct hashTable *ht;

int main(){
    ht= createHashTable(31);

    FILE *fp = fopen("keywords.txt", "r");

    if(fp == NULL){
        printf("Can't open file");
        exit(1);
    }

    char keyword[21];
    while(!feof(fp)){
        fscanf(fp, " %s", keyword);
        addKeyword(keyword);
    }
    fclose(fp);

    //printHashTable();

    //removeComments("abc.txt", "abc1.txt");

    fp = fopen("abc.txt", "r");

    tokenInfo *tk = getNextToken(fp);

    while(tk!=NULL){

        if(tk->type==NUM)
            printf("%d\n", tk->value.num);

        else if(tk->type ==RNUM)
            printf("%f\n", tk->value.rnum);

        else
            printf("%s\n", tk->value.lexeme);

        tk = getNextToken(fp);
    }

}
