#include "parserHash.h"
#include "parserDef.h"
#include "parser.h"
#include <stdio.h>
#include "set.h"

struct mappingTable *mt, *imt;
grammarNode *G;
struct hashTable *ht;
intSet* firstSet;
intSet* followSet;

int main(void){
    // TESTING MAPPING TABLE
    // test createMappingTable, fillMappingTable, printMappingTable, addSymbol, mappingHashFunction, ripOffX
    mt = createMappingTable(131); // 131 is the nearest prime > 114 (# of symbols (NT + T))
    fillMappingTable("../data/tokens.txt","../data/nonTerminals.txt");
//    printMappingTable();
//    printf("\n\n Total values hashed : %d\n",mt->hashed);
//
//    // test searchSymbol()
//    printf("\nIs 'DECLARE' present in mt : %d",searchSymbol("DECLARE")); //should be 1
//    printf("\nIs 'condionalStmt' present in mt : %d\n",searchSymbol("condionalStmt")); //should be 1
//
//    // test getEnumValue()
//    printf("\nThe Enum index of 'SQBC' is : %d",getEnumValue("SQBC")); //should be 14
//    printf("\nThe Enum index of 'range_arrays' is : %d\n",getEnumValue("range_arrays")); //should be 69

    // TESTING INVERSE MAPPING TABLE
    imt = createInverseMappingTable(131); // 131 is the nearest prime > 114 (# of symbols (NT + T))
    fillInverseMappingTable("../data/tokens.txt","../data/nonTerminals.txt");
    printInverseMappingTable();
    printf("\n\n Total values hashed : %d\n",imt->hashed);

    // test searchEnum()
    printf("\nIs DECLARE present in imt : %d",searchEnum(g_DECLARE)); // should be 1
    printf("\nIs condionalStmt present in imt : %d\n",searchEnum(g_condionalStmt)); //should be 1
    printf("\nIs -4 present in imt : %d\n",searchEnum(-4)); //should print "Hash slot index out of bounds" and 0
    printf("\nIs 150 present in imt : %d\n",searchEnum(150)); //should print "Hash slot index out of bounds" and 0
    printf("\nIs 130 present in imt : %d\n",searchEnum(130)); // should be 0

    // test getSymbol()
    printf("\nThe Symbol corresponding to SQBC is : %s",getSymbol(g_SQBC)); //should be "SQBC"
    printf("\nThe Symbol corresponding to range_arrays is : %s\n",getSymbol(g_range_arrays)); //should be "range_arrays"

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