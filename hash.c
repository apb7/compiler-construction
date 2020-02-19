#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "hash.h"


#define MAX_SYMBOL_LENGTH 30

// Mapping Table Functions
void addSymbol(char *symbol, int index, hashTable *mt){
    if (mt == NULL){
        printf("Mapping Table doesn't exist\n");
        return;
    }

    int h = symbolHashFunction(symbol, mt);

    if (h < 0 || h >= mt->size){
        printf("Hash slot index out of bounds\n");
        return;
    }

    mt->hashed++;
    struct hashTableNode *new = malloc(sizeof(struct hashTableNode));

    new->symEnum = index;
    new->symbol = malloc(sizeof(char) * (strlen(symbol) + 1));
    strcpy(new->symbol, symbol);

    new->next = mt->entries[h].first;

    mt->entries[h].size += 1;
    mt->entries[h].first = new;
}

bool searchSymbol(char *symbol, hashTable *mt){
    if (mt == NULL){
        printf("Mapping Table doesn't exist\n");
        return false;
    }

    int h = symbolHashFunction(symbol, mt);

    if (h < 0 || h >= mt->size){
        printf("Hash slot index out of bounds\n");
        return false;
    }

    struct hashTableNode *ptr = mt->entries[h].first;

    while(ptr!=NULL){
        if(strcmp(symbol, ptr->symbol) == 0)
            return true;
        ptr = ptr->next;
    }

    return false;
}

gSymbol getEnumValue(char *symbol, hashTable *mt){
    if (mt == NULL){
        printf("Mapping Table doesn't exist\n");
        return -1;
    }

    int h = symbolHashFunction(symbol, mt);

    if (h < 0 || h >= mt->size){
        printf("Hash slot index out of bounds\n");
        return -1;
    }

    struct hashTableNode *ptr = mt->entries[h].first;

    while(ptr!=NULL){
        if(strcmp(symbol, ptr->symbol) == 0)
            return ptr->symEnum;
        ptr = ptr->next;
    }

    return -1;
}

int symbolHashFunction(char *symbol, hashTable *mt){
    if (mt == NULL)
        return -1;

    int m = mt->size;

    unsigned long hash = 5381;
    char c;

    while( c = *symbol++)
        hash = hash * 33 + c; //djb2 hash function

    return hash % m;
}

struct hashTable* createHashTable(int size){ // creates and returns an empty mapping table
    struct hashTable *mt = malloc(sizeof(struct hashTable));
    mt->size = size;
    mt->hashed = 0;
    mt->entries = malloc(sizeof(struct hashTableEntry) * size);

    int i;

    for(i=0; i<size; i++){
        mt->entries[i].size = 0;
        mt->entries[i].first = NULL;
    }

    return mt;
}

void printHashTable(hashTable *mt){
    if (mt == NULL){
        printf("Mapping Table doesn't exist\n");
        return;
    }

    int i;

    for(i = 0; i < mt->size; i++){
        struct hashTableNode *ptr = mt->entries[i].first;
        printf("Slot %d has %d entries:\n", i, mt->entries[i].size);

        while(ptr!=NULL){
            printf("\t%s: %d\n", ptr->symbol, ptr->symEnum);
            ptr = ptr->next;
        }

        printf("\n\n");
    }
}

void fillHashTable(char *tokenPath, char *nonTerminalsPath, hashTable *mt){
// fills the empty mapping table after reading strings from the files specified in arguments:
//    i = 0
//    for each entry k in tokenPath
//      s = string b/w "X(" and ")" in k
//      addSymbol(s,i++)
//    for each entry k in nonTerminalsPath
//      s = string b/w "X(" and ")" in k
//      addSymbol(s,i++)

// read tokenPath
    int indexEnum=0; //this is the Enum mapped integer value of symbol to be stored in every hashTableNode
    FILE *fp = fopen(tokenPath, "r");

    if(fp == NULL){
        printf("Can't open %s file",tokenPath);
        exit(1);
    }

    char symbol[MAX_SYMBOL_LENGTH], *rippedSymbol; //the size here must be > the longest string in files tokenPath and nonTerminalsPath
    while(!feof(fp)){
        fscanf(fp, " %s", symbol);
        rippedSymbol = ripOffX(symbol);
        if(rippedSymbol) {
            addSymbol(rippedSymbol, indexEnum++, mt);
            free(rippedSymbol-2);
        }
    }
    fclose(fp);

    if(nonTerminalsPath){
        addSymbol("$", indexEnum++,mt); // adding ("$", g_EOS)

// read nonTerminalsPath
        indexEnum++; // skip g_EOS
        fp = fopen(nonTerminalsPath, "r");

        if(fp == NULL){
            printf("Can't open %s file",nonTerminalsPath);
            exit(1);
        }

        while(!feof(fp)){
            fscanf(fp, " %s", symbol);
            rippedSymbol = ripOffX(symbol);
            if(rippedSymbol) {
                addSymbol(rippedSymbol, indexEnum++, mt);
                free(rippedSymbol-2);
            }
        }
        fclose(fp);
    }
}

char* ripOffX(char* symbol){
    // takes s = "X(...)" returns a string "..."

    int l = strlen(symbol);
    if(l<4)
        return NULL;
    char* ripped = (char *) malloc(sizeof(char)*(l+1));
    strcpy(ripped,symbol);
    ripped[l-1]='\0';
    ripped+=2;
    return ripped;
}

/*
// Inverse Mapping Table Functions
struct hashTable* createInverseMappingTable(int size){ // creates and returns an empty mapping table
    struct hashTable *imt = malloc(sizeof(struct hashTable));
    imt->size = size;
    imt->hashed = 0;
    imt->entries = malloc(sizeof(struct hashTableEntry) * size);

    int i;

    for(i=0; i<size; i++){
        imt->entries[i].size = 0;
        imt->entries[i].first = NULL;
    }

    return imt;
}

void addEnum(gSymbol enum_val, char *symbol){
    if (imt == NULL){
        printf("Inverse Mapping Table doesn't exist\n");
        return;
    }

    int h = enumHashFunction(enum_val);

    if (h < 0 || h >= imt->size){
        printf("Hash slot index out of bounds\n");
        return;
    }

    imt->hashed++;
    struct hashTableNode *new = malloc(sizeof(struct hashTableNode));

    new->symEnum = enum_val;
    new->symbol = malloc(sizeof(char) * (strlen(symbol) + 1));
    strcpy(new->symbol, symbol);

    new->next = imt->entries[h].first;

    imt->entries[h].size += 1;
    imt->entries[h].first = new;
}

bool searchEnum(gSymbol enum_val){
    if (imt == NULL){
        printf("Inverse Mapping Table doesn't exist\n");
        return false;
    }

    int h = enumHashFunction(enum_val);

    if (h < 0 || h >= imt->size){
        printf("Hash slot index out of bounds\n");
        return false;
    }

    struct hashTableNode *ptr = imt->entries[h].first;

    while(ptr!=NULL){
        if(ptr->symEnum == enum_val)
            return true;
        ptr = ptr->next;
    }

    return false;
}

char* getSymbol(gSymbol enum_val){
    if (imt == NULL){
        printf("Inverse Mapping Table doesn't exist\n");
        return NULL;
    }

    int h = enumHashFunction(enum_val);

    if (h < 0 || h >= imt->size){
        printf("Hash slot index out of bounds\n");
        return NULL;
    }

    struct hashTableNode *ptr = imt->entries[h].first;
    char* sym;
    while(ptr!=NULL){
        if(ptr->symEnum == enum_val) {
            sym = (char *)malloc(sizeof(char)*strlen(ptr->symbol)+1);
            strcpy(sym, ptr->symbol);
            return sym;
        }
        ptr = ptr->next;
    }

    return NULL;
}

int enumHashFunction(gSymbol enum_val){
    if (imt == NULL)
        return -1;

    return (int)enum_val;
}

void printInverseMappingTable(){
    if (imt == NULL){
        printf("Inverse Mapping Table doesn't exist\n");
        return;
    }

    int i;

    for(i = 0; i < imt->size; i++){
        struct hashTableNode *ptr = imt->entries[i].first;
        printf("Slot %d has %d entries:\n", i, imt->entries[i].size);

        while(ptr!=NULL){
            printf("\t%d: %s\n", ptr->symEnum, ptr->symbol);
            ptr = ptr->next;
        }

        printf("\n\n");
    }
}

void fillInverseMappingTable(char *tokenPath, char *nonTerminalsPath){
// read tokenPath
    int indexEnum=0; //this is the Enum mapped integer value of symbol that will serve as the key
    FILE *fp = fopen(tokenPath, "r");

    if(fp == NULL){
        printf("Can't open %s file",tokenPath);
        exit(1);
    }

    char symbol[28], *rippedSymbol; //the size here must be > the longest string in files tokenPath and nonTerminalsPath
    while(!feof(fp)){
        fscanf(fp, " %s", symbol);
        rippedSymbol = ripOffX(symbol);
        addEnum(indexEnum++,rippedSymbol);
    }
    fclose(fp);

    addEnum(indexEnum++,"$"); // adding (g_EOS, "$")

// read nonTerminalsPath
    fp = fopen(nonTerminalsPath, "r");

    if(fp == NULL){
        printf("Can't open %s file",nonTerminalsPath);
        exit(1);
    }

    while(!feof(fp)){
        fscanf(fp, " %s", symbol);
        rippedSymbol = ripOffX(symbol);
        addEnum(indexEnum++, rippedSymbol);
    }
    fclose(fp);
}

*/