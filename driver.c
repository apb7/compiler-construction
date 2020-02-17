#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
//#include "hash.h"
//#include "lexer.h"

struct hashTable *ht;


#include "parserDef.h"
#include "parser.h"
#include <string.h>

grammarNode *G;
int main(){
//    ht= createHashTable(31);
    
//    FILE *fp = fopen("../keywords.txt", "r");
//
//    if(fp == NULL){
//        printf("Can't open file");
//        exit(1);
//    }
//
//    char keyword[21];
//    while(!feof(fp)){
//        fscanf(fp, " %s", keyword);
//        addKeyword(keyword);
//    }
//    fclose(fp);

//    printHashTable();
    
    //removeComments("abc.txt", "abc1.txt");
    
//    fp = fopen("abc1.txt", "r");
//
//    tokenInfo *tk = getNextToken(fp);
//
//    while(tk!=NULL){
//
//        if(tk->type==NUM)
//            printf("%d\n", tk->value.num);
//
//        else if(tk->type ==RNUM)
//            printf("%f\n", tk->value.rnum);
//
//        else
//            printf("%s\n", tk->value.lexeme);
//
//        tk = getNextToken(fp);
//    }
//    mappingTable mt;
//    mt[0].tag = S_T;
//    strcpy(mt[0].str,"PROGRAM");
//    mt[0].sym.t =  PROGRAM;
//    printf("%d %s %d",mt[0].tag,mt[0].str,mt[0].sym.t);


    populateGrammarStruct("../data/grammar.txt");
    printf("%d %d\n",g_EPS,G[0].lhs);
}


