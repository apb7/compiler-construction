#include "utils/hash.h"
#include "parser/parserDef.h"
#include "parser/parser.h"
#include <stdio.h>
#include "utils/set.h"
#include "config.h"
#include "lexer/lexer.h"
#include "utils/errorPtr_stack.h"


extern grammarNode *grammarArr;
extern struct hashTable *mt;
extern intSet* firstSet;
extern intSet* followSet;
hashTable *keyword_ht;
extern char *inverseMappingTable[];
extern int **parseTable;
errorPtr_stack *errorStack;


int main(int argc, char *argv[]){

    mt = createHashTable(SYMBOL_HT_SIZE); // 131 is the nearest prime > 114 (# of symbols (NT + T))
    fillHashTable(inverseMappingTable,mt);

    // Create and populate hash table for keywords
    keyword_ht = createHashTable(KEYWORD_HT_SIZE);

    char* keywords[] = {
        #define K(a,b,c) c,
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
//    printRuleRange();
//    printParseTable();
//    printf("\n");
    modifyParseTable_Err_Recovery();

//    printParseTable();

//    if(argc == 2){
//        removeComments(argv[1],TMP_SRC_FILE_PATH);
//    }
//    else
//        removeComments("../t6(with_syntax_errors).txt", TMP_SRC_FILE_PATH);

    treeNode *root = parseInputSourceCode("../test1.erp");

    printTreeOld(root);
    printTree(root, TREE_PRINT_FILE_PATH);

    destroyTree(root);



//    int arr[] = {2, 3, 5, 7, 11};
//    int a = sizeArr(arr);
//    printf("sizeof(arr): %d\n",sizeof(arr));
//    printf("sizeof(arr[0]): %d\n",sizeof(arr[0]));
//    printf("sizeof(arr)/sizeof(arr[0]): %d\n",sizeof(arr)/sizeof(arr[0]));
//    printf("ye lo sirji: %d",a);

}
