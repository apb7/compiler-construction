#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "hash.h"
#include "util.h"


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
    fprintf(stderr,"getEnumValue: Key not found in HashTable.\n");
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

void fillHashTable(char *tokenList[], hashTable *mt){
    int i = 0;
    while(!equals(tokenList[i],"#")){
        addSymbol(tokenList[i], i, mt);
        i++;
    }
}