#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "hash.h"

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
        printf("%s\n", keyword);
        addKeyword(keyword);
    }
    
    printHashTable();
}
