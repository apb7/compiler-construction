// nasm -felf64 code.asm && gcc code.o && ./a.out

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
            fprintf(fp, "section .bss\n");
            fprintf(fp, "\tinta: resb 4\n");
            fprintf(fp, "\tfloatb: resb 8\n");

            fprintf(fp, "section .data\n");

            // To be removed
            fprintf(fp, "\tsampleInt: db 5,0\n");
            fprintf(fp, "\tsampleFloat: db -5.2,0\n");


            fprintf(fp,"\tmsgBoolean: db \"Input: Enter a boolean value:\", 10, 0\n");
            fprintf(fp,"\tinputBoolean: db \"%%d\", 0\n");

            fprintf(fp,"\tmsgInt: db \"Input: Enter an integer value:\", 10, 0\n");
            fprintf(fp,"\tinputInt: db \"%%d\", 0\n");

            fprintf(fp,"\tmsgFloat: db \"Input: Enter a float value:\", 10, 0\n");
            fprintf(fp,"\tinputFloat: db \"%%lf\",0\n");

            fprintf(fp,"\toutputBooleanTrue: db \"Output: true\", 10, 0,\n");
            fprintf(fp,"\toutputBooleanFalse: db \"Output: false\", 10, 0,\n");

            fprintf(fp,"\toutputInt: db \"Output: %%d\", 10, 0,\n");

            fprintf(fp,"\toutputFloat: db \"Output: %%lf\", 10, 0,\n");

            fprintf(fp, "\nsection .text\n");
            fprintf(fp, "\tglobal main\n");
            fprintf(fp, "\textern scanf\n");
            fprintf(fp, "\textern printf\n");

            ASTNode* ASTChild = root->child;

            // Might need to change its position.
            fprintf(fp, "\nmain:\n");
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

            // <ioStmt> -> GET_VALUE BO ID BC SEMICOL

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
                else if(idVarType.baseType == g_INTEGER) {
                    fprintf(fp, "\tmov rdi, msgInt\n");
                    fprintf(fp, "\tcall printf\n");
                    fprintf(fp, "\tmov rdi, inputInt\n");
                    fprintf(fp, "\tmov rsi, inta\n");
                    fprintf(fp, "\tcall scanf\n");
/*
                    // Check the value being scanned
                    fprintf(fp, "\tmov rdi, outputInt\n");
                    fprintf(fp, "\tmov rsi, [inta]\n");
                    fprintf(fp, "\tcall printf\n");
*/

                }
                else if(idVarType.baseType == g_REAL) {
                    fprintf(fp, "\tmov rdi, msgFloat\n");
                    fprintf(fp, "\tcall printf\n");

                    fprintf(fp, "\tmov rdi, inputFloat\n");
                    fprintf(fp, "\tmov rsi, floatb\n");
                    fprintf(fp, "\tcall scanf\n");
/*
                    // Check the value being scanned
                    fprintf(fp, "\tmov rdi, outputFloat\n");
                    fprintf(fp, "\tmov xmm0, [floatb]\n");
                    fprintf(fp, "\tcall printf\n");
*/
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

            // <ioStmt> -> PRINT BO <var> BC SEMICOL
            // <boolConstt> -> TRUE | FALSE
            // <var_id_num> -> ID <whichId> | NUM | RNUM
            // <var> -> <var_id_num> | <boolConstt>
            // <whichId> -> SQBO <index> SQBC | ε
            // <index> -> NUM | ID



            if(sibling->gs == g_TRUE) {
                fprintf(fp, "\tpush rbp\n");
                fprintf(fp, "\tmov rdi, outputBooleanTrue\n");
                fprintf(fp, "\tcall printf\n");
                fprintf(fp, "\tpop rbp\n");
                return;
            }

            if(sibling->gs == g_FALSE) {
                fprintf(fp, "\tpush rbp\n");
                fprintf(fp, "\tmov rdi, outputBooleanFalse\n");
                fprintf(fp, "\tcall printf\n");
                fprintf(fp, "\tpop rbp\n");
                return;
            }

            ASTNode *siblingId = sibling->child;

            if(siblingId->gs == g_NUM) {
                fprintf(fp, "\tpush rbp\n");
                fprintf(fp, "\tmov rdi, outputInt\n");
                fprintf(fp, "\tmov rsi, %d\n", siblingId->tkinfo->value.num);
                fprintf(fp, "\tcall printf\n");
                fprintf(fp, "\tpop rbp\n");
                return;
            }

            // TODO: see how floating pt values can be assigned!
            /*
            if(siblingId->gs == g_RNUM) {
                fprintf(fp, "\tpush rbp\n");
                fprintf(fp, "\tmov rdi, outputFloat\n");
                fprintf(fp, "\tmov rsi, %f\n", siblingId->tkinfo->value.rnum);
                fprintf(fp, "\tcall printf\n");
                fprintf(fp, "\tpop rbp\n");
                return;
            }

            */

            varType idVarType = siblingId->stNode->info.var.vtype;

            if(idVarType.vaType == VARIABLE) {
                // More registers need to me pushed to preserve
                // their values.
                // BEWARE: Number of pushes here should be odd.
                // push rbx

                fprintf(fp, "\tpush rbp\n");

                if(idVarType.baseType == g_BOOLEAN) {
                    if(1) // Check value of id here.(code needs to be generated to check at runtime!)
                        fprintf(fp, "\tmov rdi, outputBooleanTrue\n");
                    else
                        fprintf(fp, "\tmov rdi, outputBooleanFalse\n");

                    fprintf(fp, "\tcall printf\n");
                }
                else if(idVarType.baseType == g_INTEGER) {

                    fprintf(fp, "\tmov rdi, outputInt\n");
                    fprintf(fp, "\tmov rsi, [inta]\n");
                    fprintf(fp, "\tcall printf\n");
                }
                else if(idVarType.baseType == g_REAL) {
                    fprintf(fp, "\tmov rdi, outputFloat\n");
                    fprintf(fp, "\tmov rsi, [floatb]\n");
                    fprintf(fp, "\tcall printf\n");
                }

                fprintf(fp, "\tpop rbp\n");
            }
            else /* Arrays */ {
                // Use whichId AST Node here.
            }

            return;
        }

        default:
            printf("Default : %s \n", inverseMappingTable[gs]);
    }

}