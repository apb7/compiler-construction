#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "hash.h"


extern struct hashTable *ht;

void addKeyword(char *keyword){
    if(ht == NULL) {
        printf("HashTable doesn't exist\n");
        return;
    }

    int h = hashFunction(keyword);
    
    if(h < 0 || h > ht->size) {
        printf("HashTable not initialised properly\n");
        return;
    }

    struct hashNode *new = malloc(sizeof(struct hashNode));

    new->keyword = malloc(sizeof(char) * (strlen(keyword) + 1));
    strcpy(new->keyword, keyword);

    new->next = ht->entries[h].first;

    ht->entries[h].size += 1;
    ht->entries[h].first = new;
}

bool searchKeyword(char *keyword){
    if(ht == NULL) {
        printf("HashTable doesn't exist\n");
        return false;
    }

    int h = hashFunction(keyword);

    if(h < 0 || h > ht->size) {
        printf("HashTable not initialised properly\n");
        return false;
    }

    struct hashNode *ptr = ht->entries[h].first;

    while(ptr != NULL) {
        if(strcmp(keyword, ptr->keyword) == 0)
            return true;
        ptr = ptr->next;
    }

    return false;
}

int hashFunction(char *keyword){
    if (ht == NULL)
        return -1;

    int m = ht->size;

    unsigned long hash = 5381;
    char c;

    while(c = *keyword++)
        hash = hash * 33 + c; //djb2 hash function

    return hash % m;
}

struct hashTable* createHashTable(int size){
    struct hashTable *ht = malloc(sizeof(struct hashTable));
    ht->size = size;
    ht->entries = malloc(sizeof(struct hashEntry) * size);

    int i;

    for(i=0; i<size; i++){
        ht->entries[i].size = 0;
        ht->entries[i].first = NULL;
    }

    return ht;
}

void printHashTable(){
    if (ht == NULL){
        printf("HashTable doesn't exist\n");
        return;
    }

    int i;

    for(i = 0; i < ht->size; i++){
        struct hashNode *ptr = ht->entries[i].first;
        printf("%d: %d ", i, ht->entries[i].size);
        
        while(ptr!=NULL){
            printf("%s ", ptr->keyword);
            ptr = ptr->next;
        }

        printf("\n");
    }
}
