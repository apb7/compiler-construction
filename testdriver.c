#include "hash.h"
#include "parser/parserDef.h"
#include "parser/parser.h"
#include <stdio.h>
#include "set.h"
#include "config.h"

grammarNode *G;
struct hashTable *mt;
intSet* firstSet;
intSet* followSet;
hashTable *keyword_ht;
extern char *inverseMappingTable[];
extern int **pTb;



int main(void){

    // TESTING Hash TABLE
    // test createHashTable, fillHashTable, printHashTable, addSymbol, HashHashFunction, ripOffX
    mt = createHashTable(SYMBOL_HT_SIZE); // 131 is the nearest prime > 114 (# of symbols (NT + T))
    fillHashTable(inverseMappingTable,mt);
//    printHashTable(mt);
//    printf("\n\n Total values hashed : %d\n",mt->hashed);
//
//    // test searchSymbol(mt)
//    printf("\nIs 'DECLARE' present in mt : %d",searchSymbol("DECLARE",mt)); //should be 1
//    printf("\nIs 'condionalStmt' present in mt : %d\n",searchSymbol("condionalStmt",mt)); //should be 1
//
//    // test getEnumValue()
//    printf("\nThe Enum index of 'SQBC' is : %d",getEnumValue("$",mt)); //should be 14
//    printf("\nThe Enum index of 'range_arrays' is : %d\n",getEnumValue("program",mt)); //should be 69

//    printf("%s\n",ripOffX("X(tg)"));
//    printf("%s\n",ripOffX("gt"));

    // TESTING FIRST AND FOLLOW
    populateGrammarStruct("../data/grammar.txt");

//     printGrammar();

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

    populateParseTable();

    printParseTable();
    
}
