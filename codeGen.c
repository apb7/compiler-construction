#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "codeGen.h"
#include "symbolTable.h"
#include "typeCheck.h"

extern char *inverseMappingTable[];


void printLeaf(ASTNode* leaf, FILE* fp) {

}

void generateCode(ASTNode* root, symbolTable* symT, FILE* fp) {
    if(root == NULL) return;

    gSymbol gs = root->gs;
    printf("%s \n", inverseMappingTable[gs]);
    switch(gs) {
        case g_program:
        {
            fprintf(fp, "section .data\n");
            fprintf(fp,"\tmsgBoolean: db \"Input: Enter a boolean value:\", 10, 0\n");
            fprintf(fp,"\tinputBoolean: db \"%d\", 0\n");

            fprintf(fp,"\tmsgInt: db \"Input: Enter an integer value:\", 10, 0\n");
            fprintf(fp,"\tinputInt: db \"%d\", 0\n");

            fprintf(fp,"\tmsgFloat: db \"Input: Enter a float value:\", 10, 0\n");
            fprintf(fp,"\tinputFloat: db \"%f\",0\n");

            fprintf(fp,"\toutputBooleanTrue: db \"Output: true\", 10, 0,\n");
            fprintf(fp,"\toutputBooleanFalse: db \"Output: false\", 10, 0,\n");

            fprintf(fp,"\toutputInt: db \"Output: %d\", 10, 0,\n");

            fprintf(fp,"\toutputFloat: db \"Output: %f\", 10, 0,\n");

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
                // More registers need to me pushed to preserve
                // their values.
                // BEWARE: Number of pushes here should be odd.
                // push rbx
                fprintf(fp, "\tpush rbp\n");

                if(idVarType.baseType == g_BOOLEAN) {
                    fprintf(fp, "\tmov rdi, msgBoolean\n");
                    fprintf(fp, "\tcall printf\n");
                    fprintf(fp, "\tmov rdi, inputBoolean\n");
                    fprintf(fp, "\tcall scanf\n");
                    // Scanned int goes to rax or rdx:rax.
                    // Scanned float goes to xmm0 or xmm1:xmm0.
                }
                else if(idVarType.baseType == g_NUM) {
                    fprintf(fp, "\tmov rdi, msgInt\n");
                    fprintf(fp, "\tcall printf\n");
                    fprintf(fp, "\tmov rdi, inputInt\n");
                    fprintf(fp, "\tcall scanf\n");
                }
                else /* RNUM */{
                    fprintf(fp, "\tmov rdi, msgFloat\n");
                    fprintf(fp, "\tcall printf\n");
                    fprintf(fp, "\tmov rdi, inputFloat\n");
                    fprintf(fp, "\tcall scanf\n");
                }

                fprintf(fp, "\tpop rbp\n");
            }
            else /* Arrays */ {

            }

            return;
        }

        case g_PRINT:
        {   
            // Need changes here!
            ASTNode* sibling = root->next;
            
            if(sibling->gs != g_var_id_num) {
                // TODO(apb7): Handle BOOL, NUM, RNUM constant
                return;
            }

            ASTNode *siblingId = sibling->child;
            varType idVarType = siblingId->stNode->info.var.vtype;

            if(idVarType.vaType == VARIABLE) {
                unsigned int varValue = idVarType.si.vt_num;
                // More registers need to me pushed to preserve
                // their values.
                // BEWARE: Number of pushes here should be odd.
                // push rbx

                fprintf(fp, "\tpush rbp\n");

                if(idVarType.baseType == g_BOOLEAN) {
                    if(varValue)
                        fprintf(fp, "\tmov rdi, outputBooleanTrue\n");
                    else
                        fprintf(fp, "\tmov rdi, outputBooleanFalse\n");

                    fprintf(fp, "\tcall printf\n");
                }
                else if(idVarType.baseType == g_INTEGER) {

                    fprintf(fp, "\tmov rdi, outputInt\n");
                    fprintf(fp, "\tmov rsi, %d\n", varValue);
                    fprintf(fp, "\tcall printf\n");
                }
                else /* RNUM */{
                    fprintf(fp, "\tmov rdi, outputFloat\n");
                    fprintf(fp, "\tmov rsi, %f\n", varValue);
                    fprintf(fp, "\tcall printf\n");
                }

                fprintf(fp, "\tpop rbp\n");
            }
            else /* Arrays */ {

            }

            return;
        }
        case g_conditionalStmt:{

        }

            return;
        default:
            printf("Default : %s \n", inverseMappingTable[gs]);
    }

}