#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "codeGen.h"
#include "symbolTable.h"
#include "typeCheck.h"


void printLeaf(ASTNode* leaf, FILE* fp) {

}

void generateCode(ASTNode* root, symbolTable* symT, FILE* fp) {
    if(root == NULL) return;

    gSymbol gs = root->gs;

    switch(gs) {
        case g_PROGRAM:
        {
            fprintf(fp, "section .data\n");
            fprintf(fp,"\tinputLabel: db \"%%hd\",0\n");
            fprintf(fp,"\toutputLabel: db \"%%hd\",10,0,\n");

            fprintf(fp, "section .text\n");
            fprintf(fp, "\tglobal main\n");
            fprintf(fp, "\textern scanf\n");
            fprintf(fp, "\textern printf\n");

            generateCode(root->child, symT, fp);
            return;
        }
    }

}