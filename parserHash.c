#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "parserHash.h"


extern struct mappingTable *mt;

void addSymbol(char *symbol, int index){
    if (mt == NULL){
        printf("Mapping Table doesn't exist\n");
        return;
    }

    int h = mappingHashFunction(symbol);

    if (h < 0 || h > mt->size){
        printf("Mapping Table not initialised properly\n");
        return;
    }

    mt->hashed++;
    struct mappingTableNode *new = malloc(sizeof(struct mappingTableNode));

    new->symEnum = index;
    new->symbol = malloc(sizeof(char) * (strlen(symbol) + 1));
    strcpy(new->symbol, symbol);

    new->next = mt->entries[h].first;

    mt->entries[h].size += 1;
    mt->entries[h].first = new;
}

bool searchSymbol(char *symbol){
    if (mt == NULL){
        printf("Mapping Table doesn't exist\n");
        return false;
    }

    int h = mappingHashFunction(symbol);

    if (h < 0 || h > mt->size){
        printf("Mapping Table not initialised properly\n");
        return false;
    }

    struct mappingTableNode *ptr = mt->entries[h].first;

    while(ptr!=NULL){
        if(strcmp(symbol, ptr->symbol) == 0)
            return true;
        ptr = ptr->next;
    }

    return false;
}

gSymbol getEnumValue(char *symbol){
    if (mt == NULL){
        printf("Mapping Table doesn't exist\n");
        return -1;
    }

    int h = mappingHashFunction(symbol);

    if (h < 0 || h > mt->size){
        printf("Mapping Table not initialised properly\n");
        return -1;
    }

    struct mappingTableNode *ptr = mt->entries[h].first;

    while(ptr!=NULL){
        if(strcmp(symbol, ptr->symbol) == 0)
            return ptr->symEnum;
        ptr = ptr->next;
    }

    return -1;
}

int mappingHashFunction(char *symbol){
    if (mt == NULL)
        return -1;

    int m = mt->size;

    unsigned long hash = 5381;
    char c;

    while( c = *symbol++)
        hash = hash * 33 + c; //djb2 hash function

    return hash % m;
}

struct mappingTable* createMappingTable(int size){ // creates and returns an empty mapping table
    struct mappingTable *mt = malloc(sizeof(struct mappingTable));
    mt->size = size;
    mt->hashed = 0;
    mt->entries = malloc(sizeof(struct mappingTableEntry) * size);

    int i;

    for(i=0; i<size; i++){
        mt->entries[i].size = 0;
        mt->entries[i].first = NULL;
    }

    return mt;
}

void printMappingTable(){
    if (mt == NULL){
        printf("Mapping Table doesn't exist\n");
        return;
    }

    int i;

    for(i = 0; i < mt->size; i++){
        struct mappingTableNode *ptr = mt->entries[i].first;
        printf("Slot %d has %d entries:\n", i, mt->entries[i].size);

        while(ptr!=NULL){
            printf("\t%s: %d\n", ptr->symbol, ptr->symEnum);
            ptr = ptr->next;
        }

        printf("\n\n");
    }
}

void fillMappingTable(char *tokenPath, char *nonTerminalsPath){
// fills the empty mapping table after reading strings from the files specified in arguments:
//    i = 0
//    for each entry k in tokenPath
//      s = string b/w "X(" and ")" in k
//      addSymbol(s,i++)
//    for each entry k in nonTerminalsPath
//      s = string b/w "X(" and ")" in k
//      addSymbol(s,i++)

// read tokenPath
    int indexEnum=0; //this is the Enum mapped integer value of symbol to be stored in every mappingTableNode
    FILE *fp = fopen(tokenPath, "r");

    if(fp == NULL){
        printf("Can't open %s file",tokenPath);
        exit(1);
    }

    char symbol[28], *rippedSymbol; //the size here must be > the longest string in files tokenPath and nonTerminalsPath
    while(!feof(fp)){
        fscanf(fp, " %s", symbol);
        rippedSymbol = ripOffX(symbol);
        addSymbol(rippedSymbol,indexEnum++);
    }
    fclose(fp);

// read nonTerminalsPath
    fp = fopen(nonTerminalsPath, "r");

    if(fp == NULL){
        printf("Can't open %s file",nonTerminalsPath);
        exit(1);
    }

    while(!feof(fp)){
        fscanf(fp, " %s", symbol);
        rippedSymbol = ripOffX(symbol);
        addSymbol(rippedSymbol,indexEnum++);
    }
    fclose(fp);
}

char* ripOffX(char* symbol){
    // takes s = "X(...)" returns a string "..."
    int l = strlen(symbol);
    char* ripped = (char *) malloc(sizeof(char)*l);
    strcpy(ripped,symbol);
    ripped[l-1]='\0';
    ripped+=2;
    return ripped;
}