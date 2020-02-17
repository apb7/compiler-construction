#include "parserHash.h"
#include "parserDef.h"
#include "parser.h"
#include <stdio.h>
#include "set.h"

struct mappingTable *mt;
grammarNode *G;
struct hashTable *ht;
intSet* firstSet;
intSet* followSet;

int main(void){
    // test createMappingTable, fillMappingTable, printMappingTable, addSymbol, mappingHashFunction, ripOffX
    mt = createMappingTable(131); // 131 is the nearest prime > 114 (# of symbols (NT + T))
    fillMappingTable("../data/tokens.txt","../data/nonTerminals.txt");
//    printMappingTable();
    printf("\n\n Total values hashed : %d\n",mt->hashed);

    populateGrammarStruct("../data/grammar.txt");

//    printGrammar();

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




//    // test searchSymbol()
//    printf("\nIs 'DECLARE' present in mt : %d",searchSymbol("DECLARE")); //should be 1
//    printf("\nIs 'condionalStmt' present in mt : %d\n",searchSymbol("condionalStmt")); //should be 1
//
//    // test getEnumValue()
//    printf("\nThe Enum index of 'SQBC' is : %d",getEnumValue("SQBC")); //should be 14
//    printf("\nThe Enum index of 'range_arrays' is : %d\n",getEnumValue("range_arrays")); //should be 69

}