#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "codeGen.h"
#include "symbolTable.h"
#include "typeCheck.h"
#include "lexerDef.h"


void printLeaf(ASTNode* leaf, FILE* fp) {

}

char *expreg[3] = {"AX","BX","CX"};
char *expscale = "word";
void genExpr(ASTNode *astNode, FILE *fp, bool firstCall, int lr, gSymbol expType){
    if(firstCall){
        if(astNode == NULL)
            return;
        else if(astNode->gs != g_assignmentStmt){
//            printf("%d\n",astNode->gs);
            genExpr(astNode->next,fp,true,lr,expType);
            genExpr(astNode->child,fp,true,lr,expType);
            return;
        }
        //assignmentStatement Node will be passed
        ASTNode *idNode = astNode->child->child->child;
        printf("%s\n",idNode->tkinfo->lexeme);
        if(idNode->next->next != NULL && idNode->next->gs == g_NUM){
            //array element and static index
            expType = idNode->stNode->info.var.vtype.baseType;
            //it can be boolean/int/real
            genExpr(idNode->next->next,fp,false,1,expType);
            if(expType == g_REAL){

            }
            else{
                fprintf(fp,"\tMOV %s[x_%x+%d], %s\n",expscale,idNode->stNode,idNode->next->tkinfo->value.num,expreg[1]);
                return;
            }
        }
        else if(idNode->next->next != NULL && idNode->next->gs == g_ID){
            //array element and dynamic index
        }
        else{
            //array or variable
            if(idNode->stNode->info.var.vtype.vaType == VARIABLE){
                //variable
                genExpr(idNode->next,fp,false,1,expType);
                fprintf(fp,"\tMOV [x_%x], %s\n",idNode->stNode,expreg[1]);
                return;
            }
            else{

            }
        }
    }
    else{
        if(astNode->gs == g_var_id_num){
            astNode = astNode->child;
            switch(astNode->gs){
                case g_NUM:
                    fprintf(fp,"\tMOV %s, %d\n",expreg[lr],astNode->tkinfo->value.num);
                    break;
                case g_RNUM:
                    break;
                case g_ID:
                {
                    if(astNode->stNode->info.var.vtype.baseType == g_REAL){

                    }
                    else{
                        if(astNode->stNode->info.var.vtype.vaType != VARIABLE){
                            if(astNode->next->gs == g_ID){
                                fprintf(fp,"\tMOV %s, x_%x\n",expreg[3],astNode->next->stNode);
                            }
                            else{
                                //g_num
                                fprintf(fp,"\tMOV %s, %d\n",expreg[2],astNode->next->tkinfo->value.num);
                            }
                        }
                        else
                            fprintf(fp,"\tMOV %s,0\n",expreg[2]);
                        fprintf(fp,"\tMOV %s, %s[x_%x + %s]\n",expreg[lr],expscale,astNode->stNode,expreg[2]);
                    }
                }
                break;
            }
        }
        else{
            //astnode is an operator
            genExpr(astNode->child,fp,false,0,expType);
            genExpr(astNode->child->next,fp,false,1,expType);
            switch(astNode->gs){
                case g_PLUS:
                    fprintf(fp,"\tADD %s, %s\n",expreg[lr],expreg[lr^1]);
                    break;
            }
            return;
        }
    }
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