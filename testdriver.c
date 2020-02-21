#include "utils/hash.h"
#include "parser/parserDef.h"
#include "parser/parser.h"
#include <stdio.h>
#include "utils/set.h"
#include "config.h"
#include "lexer/lexer.h"

grammarNode *G;
struct hashTable *mt;
intSet* firstSet;
intSet* followSet;
hashTable *keyword_ht;
extern char *inverseMappingTable[];
extern int **pTb;



int main(int argc, char *argv[]){

    mt = createHashTable(SYMBOL_HT_SIZE); // 131 is the nearest prime > 114 (# of symbols (NT + T))
    fillHashTable(inverseMappingTable,mt);

    // Create and populate hash table for keywords
    keyword_ht = createHashTable(KEYWORD_HT_SIZE);

    char* keywords[] = {
        #define K(a,b,c) b,
        #include "data/keywords.txt"
        #undef K
        "#"
    };

    fillHashTable(keywords,keyword_ht);

    populateGrammarStruct("../data/grammar.txt");

//     printGrammar();

    populateFirstSet();
    populateFollowSet();
    populateParseTable();

//    printParseTable();

    if(argc == 2){
        removeComments(argv[1],"source.tmp");
    }
    else
        removeComments("../test1.erp",TMP_SRC_FILE_NAME);

    treeNode *root = parseInputSourceCode(TMP_SRC_FILE_NAME);

    printTree(root);



}
