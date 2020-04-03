// Group Number: 31
// MADHUR PANWAR   2016B4A70933P
// TUSSANK GUPTA   2016B3A70528P
// SALMAAN SHAHID  2016B4A70580P
// APURV BAJAJ     2016B3A70549P
// HASAN NAQVI     2016B5A70452P

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "hash.h"
#include "parserDef.h"
#include "parser.h"
#include "set.h"
#include "config.h"
#include "lexer.h"
#include "errorPtr_stack.h"

#include "astDef.h"
#include "ast.h"
#include "symbolHash.h"

// Variables defined in lexer.c
extern unsigned int fp;
extern unsigned int bp;
extern unsigned int line_number;
extern int status;
extern int count;
extern hashTable *keyword_ht;

// Variables defined in parser.c
extern grammarNode *grammarArr;
extern struct hashTable *mt;
extern intSet* firstSet;
extern intSet* followSet;
extern char *inverseMappingTable[];
extern int **parseTable;
errorPtr_stack *errorStack;


int main(int argc, char *argv[]) {
//    printf("%d",sizeof(symbolTable));
    if(argc != 3) {
        printf("Usage: %s <source code file> <parse tree output file>\n", argv[0]);
        exit(1);
    }

    printf("\t FIRST and FOLLOW set automated. \n");
    printf("\t Both lexical and syntax analysis modules implemented. \n");
    printf("\t Modules work with all testcases. \n");
    printf("\t Source code is parsed successfully. \n");
    printf("\t Parse tree is printed in the output file. \n");
    printf("\t Error detection and recovery done. \n\n");

    mt = createHashTable(SYMBOL_HT_SIZE); // 131 is the nearest prime > 114 (# of symbols (NT + T))
    fillHashTable(inverseMappingTable,mt);

    // Create and populate hash table for keywords
    keyword_ht = createHashTable(KEYWORD_HT_SIZE);

    char* keywords[] = {
        #define K(a,b,c) c,
        #include "keywords.txt"
        #undef K
        "#"
    };

    fillHashTable(keywords,keyword_ht);

    populateGrammarStruct("grammar.txt");

//     printGrammar();

    populateFirstSet();
    populateFollowSet();
    populateParseTable();
    modifyParseTable_Err_Recovery();

//    printParseTable();

    char userInput;

    while(1) {
        printf("\n\t Press 0 to exit.\n\t Press 1 to remove comments.\n\t Press 2 to print all tokens.\n\t Press 3 to parse source code. \n\t Press 4 to print time taken.\n\t Press 5 to build AST tree\n");
        scanf(" %c", &userInput);
        switch(userInput) {

            case '0':
                return 0;
                break;

            case '1':
                removeComments(argv[1], NULL);
                break;

            case '2':
            {
                FILE *fp_arg = fopen(argv[1], "r");
                tokenInfo *tk;

                printf("%12s %20s %20s \n", "Line_number", "lexeme", "Token_name");

                // Initialise lexer every time.
                fp = 0; bp = 0; line_number = 1; status = 1; count = 0;
                while((tk = getNextToken(fp_arg)) != NULL) {
                    printf("%12d %20s %20s\n", tk->lno, tk->lexeme, inverseMappingTable[tk->type]);
                    free(tk);
                }
                fclose(fp_arg);
            } break;

            case '3':
            {
                // Initialise lexer every time.
                fp = 0; bp = 0; line_number = 1; status = 1; count = 0;

                treeNode *root = parseInputSourceCode(argv[1]); //this also frees the error stack
                printTree(root, argv[2]);   //printTree also frees the tree after printing it

            }
            break;

            case '4':
            {
                clock_t start_time, end_time;
                double total_CPU_time, total_CPU_time_in_seconds;

                start_time = clock();

                // Initialise lexer every time.
                fp = 0; bp = 0; line_number = 1; status = 1; count = 0;

                treeNode *root = parseInputSourceCode(argv[1]);

                end_time = clock();

                total_CPU_time  =  (double) (end_time - start_time);
                total_CPU_time_in_seconds =   total_CPU_time / CLOCKS_PER_SEC;

                printf("Total CPU time = %lf \nTotal CPU time in secs = %lf \n", total_CPU_time, total_CPU_time_in_seconds);
                destroyTree(root);
            }
            break;

            case '5':
            {
                // Initialise lexer every time.
                fp = 0; bp = 0; line_number = 1; status = 1; count = 0;

                treeNode *root = parseInputSourceCode(argv[1]); //this also frees the error stack
                ASTNode *ASTroot =buildASTTree(root);
                print_ASTTree(ASTroot);

                printf("No of nodes in AST Tree : %d \n", count_nodes_ASTTree(ASTroot));
                printf("No of nodes in parse Tree : %d \n", count_nodes_parseTree(root));
            }
            break;

            default:
                printf("Invalid Choice. Please try again! \n");
        }

    }

    return 0;
}
