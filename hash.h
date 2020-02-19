#include <stdbool.h>

struct hashNode {
    char* keyword;
    int val;
    struct hashNode *next;
};

struct hashEntry {
    int size;
    struct hashNode *first;
};

struct hashTable {
    int size;
    struct hashEntry *entries;
};

void addKeyword(char *keyword, int val);
int searchKeyword(char *keyword);
int hashFunction(char *keyword);
struct hashTable* createHashTable(int size);
void printHashTable();
