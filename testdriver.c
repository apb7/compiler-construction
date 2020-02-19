#include "hash.h"
#include "parserDef.h"
#include "parser.h"
#include <stdio.h>
#include "set.h"

grammarNode *G;
struct hashTable *mt;
intSet* firstSet;
intSet* followSet;
hashTable *keyword_ht;

#define SYMBOL_HT_SIZE 131

int main(void){
    // TESTING Hash TABLE
    // test createHashTable, fillHashTable, printHashTable, addSymbol, HashHashFunction, ripOffX
    mt = createHashTable(SYMBOL_HT_SIZE); // 131 is the nearest prime > 114 (# of symbols (NT + T))
    fillHashTable("../data/tokens.txt","../data/nonTerminals.txt",mt);
    printHashTable(mt);
    printf("\n\n Total values hashed : %d\n",mt->hashed);
//
//    // test searchSymbol(mt)
//    printf("\nIs 'DECLARE' present in mt : %d",searchSymbol("DECLARE",mt)); //should be 1
//    printf("\nIs 'condionalStmt' present in mt : %d\n",searchSymbol("condionalStmt",mt)); //should be 1
//
//    // test getEnumValue()
//    printf("\nThe Enum index of 'SQBC' is : %d",getEnumValue("SQBC",mt)); //should be 14
//    printf("\nThe Enum index of 'range_arrays' is : %d\n",getEnumValue("range_arrays",mt)); //should be 69

    // TESTING FIRST AND FOLLOW
    populateGrammarStruct("../data/grammar.txt");

    // printGrammar();

    populateFirstSet();
        populateFollowSet();
    int nonTerminal_count=g_numSymbols-g_EOS-1;

    for(int i = 0; i  <nonTerminal_count; i++) {
        intSet num = followSet[i];
        printf("%d -> ", i+g_EOS+1);
        for(intSet j = 0; j < 64; j++) {
            if(isPresent(num,j))
                printf("%llu ", j);
        }
        printf("\n");
    }
    
}
