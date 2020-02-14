#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "hash.h"
#include "lexer.h"

struct hashTable *ht;

int main(){
    ht= createHashTable(37);
    
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

    printHashTable();
    
    removeComments("abc.txt", "abc1.txt");
    
    fp = fopen("abc2.txt", "r");

    tokenInfo *tk = getNextToken(fp);

    while(tk!=NULL){
        printf("%u %s\n", tk, tk->value.lexeme);
        tk = getNextToken(fp);
    }
    
}
