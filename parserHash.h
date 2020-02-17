// since the structure of the nodes in the hash table here and the one used in lexer is different,
// I created this another set of hash definitions
// May think of abstracting out the minimum functionality of just the hash part so that redefinition isn't required.
// For now, this should do.
#ifndef PARSERHASH_H
#define PARSERHASH_H
#include <stdbool.h>
#include "parserDef.h"

// struct definitions

struct mappingTableNode{
    gSymbol symEnum; // to store the Enum index value
    char *symbol; // stores the symbol string
    struct mappingTableNode *next; // points to next node in the linked list (Separate chaining is used for handling collisions)
}; //114 symbols (57 each of terminals and non terminals)
typedef struct mappingTableNode mappingTableNode;

struct mappingTableEntry {
    int size; // size of the linked list pointed by 'first'
    struct mappingTableNode *first; // points to the first node in the linked list
};

struct mappingTable {
    int size; // no. of slots in the mapping table
    int hashed; // no. of symbols hashed into the mapping table (this is incremented when we add a symbol to the mapping table)
    struct mappingTableEntry *entries; // points to the array of elements of type mappingTableEntry
};

void addSymbol(char *symbol, int index); // add 'symbol' to mapping table with index as 'index'
bool searchSymbol(char *symbol); // check if 'symbol' is present in mapping table or not
gSymbol getEnumValue(char *symbol); // get the Enum value (viz. stored index) of the 'symbol'
int mappingHashFunction(char *symbol); // returns the slot value in the mapping table where 'symbol' is to be hashed
struct mappingTable* createMappingTable(int size); // creates an empty mapping table and returns a pointer to the structure
void printMappingTable(); // prints the mapping table
void fillMappingTable(char *tokenPath, char *nonTerminalsPath); // fills the mapping table with the symbols read from tokenPath and nonTerminalsPath
char *ripOffX(char *symbol); // takes string "X(...)", returns a string "..."

#endif
