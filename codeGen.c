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
        case g_program:
        {
            fprintf(fp, "section .data\n");
            fprintf(fp,"\tinputFormat: db \"%%hd\",0\n");
            fprintf(fp,"\toutputFormat: db \"%%hd\",10,0,\n");

            fprintf(fp, "section .text\n");
            fprintf(fp, "\tglobal main\n");
            fprintf(fp, "\textern scanf\n");
            fprintf(fp, "\textern printf\n");

            ASTNode* ASTChild = root->child;

            while(ASTChild) {
                generateCode(ASTChild, symT, fp);
                ASTChild = ASTChild->next;
            }

            return;
        }

        // TODO(apb7): Pull common functionality out and separate these cases if needed.
        case g_moduleDeclarations:
        case g_otherModules:
        case g_statements:
        case g_module:
        {
            // moduleDeclarations -> ID moduleDeclarations

            ASTNode* ASTChild = root->child;

            while(ASTChild) {
                generateCode(ASTChild, symT, fp);
                ASTChild = ASTChild->next;
            }

            return;
        }

        case g_DRIVER:
        case g_moduleDef:
        case g_START:
        {
            generateCode(root->child, symT, fp);

            return;
        }

        case g_ID:
        {
            return;
        }

        case g_ioStmt:
        {
            generateCode(root->child, symT, fp);
            return;
        }

        case g_GET_VALUE:
        {

            ASTNode* siblingId = root->next;
            
            if(! siblingId->stNode) {
                // printf("ERROR: Undeclared variable\n");
                // Already being handled.
                return;
            }

            varType idVarType = siblingId->stNode->info.var.vtype;

            if(idVarType.vaType == VARIABLE) {
                if(idVarType.baseType == g_BOOLEAN) {

                }
                else if(idVarType.baseType == g_NUM) {

                }
                else /* RNUM */{

                }
            }
            else /* Arrays */ {

            }

            return;
        }

        case g_PRINT:
        {
            return;
        }
    }

}