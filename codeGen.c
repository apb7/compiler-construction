// nasm -felf64 code.asm && gcc code.o && ./a.out
// nasm -felf64 code.asm && gcc -no-pie code.o && ./a.out

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "codeGen.h"
#include "symbolTable.h"
#include "typeCheck.h"
#include "lexerDef.h"

extern char *inverseMappingTable[];


void printLeaf(ASTNode* leaf, FILE* fp) {

}

//EXPERIMENTAL CODE _ IGNORE FOR NOW _ STARTS

char *baseRegister[2] = {"RBP", "RSI"};

char *expreg[3] = {"AX","BX","CX"};
char *expscale = "word";

void genExpr(ASTNode *astNode, FILE *fp, bool firstCall, int lr, gSymbol expType){
    if(firstCall){
        if(astNode == NULL)
            return;
        else if(astNode->gs != g_assignmentStmt){
            //            printf("%d \n",astNode->gs);
            genExpr(astNode->next,fp,true,lr,expType);
            genExpr(astNode->child,fp,true,lr,expType);
            return;
        }
        //assignmentStatement Node will be passed
        ASTNode *idNode = astNode->child->child->child;
        printf("%s \n",idNode->tkinfo->lexeme);
        if(idNode->next->next != NULL && idNode->next->gs == g_NUM){
            //array element and static index
            expType = idNode->stNode->info.var.vtype.baseType;
            //it can be boolean/int/real
            genExpr(idNode->next->next,fp,false,1,expType);
            if(expType == g_REAL){

            }
            else{
                fprintf(fp,"\t MOV %s[x_%x+%d], %s \n",expscale,idNode->stNode,idNode->next->tkinfo->value.num,expreg[1]);
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
                fprintf(fp,"\t MOV [x_%x], %s \n",idNode->stNode,expreg[1]);
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
                    fprintf(fp,"\t MOV %s, %d \n",expreg[lr],astNode->tkinfo->value.num);
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
                                fprintf(fp,"\t MOV %s, x_%x \n",expreg[2],astNode->next->stNode);
                            }
                            else{
                                //g_num
                                fprintf(fp,"\t MOV %s, %d \n",expreg[2],astNode->next->tkinfo->value.num);
                            }
                        }
                        else
                            fprintf(fp,"\t MOV %s,0 \n",expreg[2]);
                        fprintf(fp,"\t MOV %s, %s[x_%x + %s] \n",expreg[lr],expscale,astNode->stNode,expreg[2]);
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
                    fprintf(fp,"\t ADD %s, %s \n",expreg[lr],expreg[lr^1]);
                    break;
            }
            return;
        }
    }
}

//EXPERIMENTAL CODE _ ENDS

void generateCode(ASTNode* root, symbolTable* symT, FILE* fp) {
    if(root == NULL) return;

    gSymbol gs = root->gs;
    printf("%s  \n", inverseMappingTable[gs]);
    switch(gs) {
        case g_program:
        {
            fprintf(fp, "section .bss \n");
            fprintf(fp, "\t inta: resb 4 \n");
            fprintf(fp, "\t floatb: resb 8 \n");
            fprintf(fp, "\t boolc: resb 2 \n");

            fprintf(fp, "section .data \n");

            // To be removed
            fprintf(fp, "\t sampleInt: db 5,0 \n");
            fprintf(fp, "\t sampleFloat: db -5.2,0 \n");


            fprintf(fp,"\t msgBoolean: db \"Input: Enter a boolean value:\", 10, 0 \n");
            fprintf(fp,"\t inputBoolean: db \"%%hd\", 0 \n");

            fprintf(fp,"\t msgInt: db \"Input: Enter an integer value:\", 10, 0 \n");
            fprintf(fp,"\t inputInt: db \"%%d\", 0 \n");

            fprintf(fp,"\t msgFloat: db \"Input: Enter a float value:\", 10, 0 \n");
            fprintf(fp,"\t inputFloat: db \"%%lf\",0 \n");

            fprintf(fp,"\t outputBooleanTrue: db \"Output: true\", 10, 0, \n");
            fprintf(fp,"\t outputBooleanFalse: db \"Output: false\", 10, 0, \n");

            fprintf(fp,"\t outputInt: db \"Output: %%d\", 10, 0, \n");

            fprintf(fp,"\t outputFloat: db \"Output: %%lf\", 10, 0, \n");

            fprintf(fp, " \nsection .text \n");
            fprintf(fp, "\t global main \n");
            fprintf(fp, "\t extern scanf \n");
            fprintf(fp, "\t extern printf \n");

            ASTNode* ASTChild = root->child;

            // Might need to change its position.
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
        {
            fprintf(fp, " \nmain: \n");
            fprintf(fp, "\t mov rbp, rsp \n");
            fprintf(fp, "\t mov rdx, rsp \n");
            fprintf(fp, "\t sub rsp, 192 \n"); // to fix this! AR space needed

            generateCode(root->child, symT, fp);

            fprintf(fp, "\t mov rsp, rbp \n");

            return;
        }
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
                // printf("ERROR: Undeclared variable \n");
                // Already being handled.
                return;
            }

            varType idVarType = siblingId->stNode->info.var.vtype;
            symVarInfo idVar = siblingId->stNode->info.var;

            if(idVarType.vaType == VARIABLE) {
                // More registers need to me pushed to preserve
                // their values.
                // BEWARE: Number of pushes here should be odd.
                // push rbx
                fprintf(fp, "\t push rbp \n");

                if(idVarType.baseType == g_BOOLEAN) {
                    fprintf(fp, "\t mov rdi, msgBoolean  \n ");
                    fprintf(fp, "\t call printf  \n ");
                    fprintf(fp, "\t mov rdi, inputBoolean \n");
                    fprintf(fp, "\t mov rsi, %s \n", baseRegister[idVar.isIOlistVar]);
                    fprintf(fp, "\t sub rsi, %d \n", 2 * (idVarType.width + idVar.offset));
                    fprintf(fp, "\t call scanf \n");
                    // Scanned int goes to rax or rdx:rax.
                    // Scanned float goes to xmm0 or xmm1:xmm0.
                    // Note by Hasan: doesn't work with regs
                }
                else if(idVarType.baseType == g_INTEGER) {
                    fprintf(fp, "\t mov rdi, msgInt \n");
                    fprintf(fp, "\t call printf \n");
                    fprintf(fp, "\t mov rdi, inputInt \n");
                    fprintf(fp, "\t mov rsi, %s \n", baseRegister[idVar.isIOlistVar]);
                    fprintf(fp, "\t sub rsi, %d \n", 2 * (idVarType.width + idVar.offset));
                    fprintf(fp, "\t call scanf \n");

                    // Check the value being scanned
                    // fprintf(fp, "\t mov rdi, outputInt \n");
                    // fprintf(fp, "\t mov rsi, [inta] \n");
                    // fprintf(fp, "\t call printf \n");


                }
                else if(idVarType.baseType == g_REAL) {
                    fprintf(fp, "\t mov rdi, msgFloat \n");
                    fprintf(fp, "\t call printf \n");

                    fprintf(fp, "\t mov rdi, inputFloat \n");
                    fprintf(fp, "\t mov rsi, %s \n", baseRegister[idVar.isIOlistVar]);
                    fprintf(fp, "\t sub rsi, %d \n", 2 * (idVarType.width + idVar.offset));
                    fprintf(fp, "\t call scanf \n");

                    // Check the value being scanned
                    // fprintf(fp, "\t mov rdi, outputFloat \n");
                    // fprintf(fp, "\t mov xmm0, [floatb] \n");
                    // fprintf(fp, "\t call printf \n");

                }

                fprintf(fp, "\t pop rbp \n");
            }
            else /* Arrays */ {

            }

            return;
        }

        case g_PRINT:
        {
            fprintf(fp,"pp\n");
            // Need changes here!
            ASTNode* sibling = root->next;

            // <ioStmt> -> PRINT BO <var> BC SEMICOL
            // <boolConstt> -> TRUE | FALSE
            // <var_id_num> -> ID <whichId> | NUM | RNUM
            // <var> -> <var_id_num> | <boolConstt>
            // <whichId> -> SQBO <index> SQBC | ε
            // <index> -> NUM | ID



            if(sibling->gs == g_TRUE) {
                fprintf(fp, "\t push rbp \n");
                fprintf(fp, "\t mov rdi, outputBooleanTrue \n");
                fprintf(fp, "\t call printf \n");
                fprintf(fp, "\t pop rbp \n");
                return;
            }

            if(sibling->gs == g_FALSE) {
                fprintf(fp, "\t push rbp \n");
                fprintf(fp, "\t mov rdi, outputBooleanFalse \n");
                fprintf(fp, "\t call printf \n");
                fprintf(fp, "\t pop rbp \n");
                return;
            }

            ASTNode *siblingId = sibling->child;

            if(siblingId->gs == g_NUM) {
                fprintf(fp, "\t push rbp \n");
                fprintf(fp, "\t mov rdi, outputInt \n");
                fprintf(fp, "\t mov rsi, %d \n", siblingId->tkinfo->value.num);
                fprintf(fp, "\t call printf \n");
                fprintf(fp, "\t pop rbp \n");
                return;
            }

            // TODO: see how floating pt values can be assigned!

            if(siblingId->gs == g_RNUM) {
                fprintf(fp, "\t push rbp \n");
                fprintf(fp, "\t mov rdi, outputFloat \n");
                fprintf(fp, "\t mov rsi, __float64__(%s) \n", siblingId->tkinfo->lexeme);
                fprintf(fp, "\t movq xmm0, rsi  \n");
                fprintf(fp, "\t mov rax, 1  \n");
                // printf expects double but rsi has float. Therefore, output is 0.000
                // Need to find a way around this using fld instr but be careful with stack.
                fprintf(fp, "\t call printf \n");
                fprintf(fp, "\t pop rbp \n");
                return;
            }

            varType idVarType = siblingId->stNode->info.var.vtype;
            symVarInfo idVar = siblingId->stNode->info.var;

            if(idVarType.vaType == VARIABLE) {
                // More registers need to me pushed to preserve
                // their values.
                // BEWARE: Number of pushes here should be odd.
                // push rbx

                fprintf(fp, "\t push rbp \n");

                if(idVarType.baseType == g_BOOLEAN) {
                    fprintf(fp, "\t cmp word[%s - %d], 0 \n", baseRegister[idVar.isIOlistVar], 2 * (idVarType.width + idVar.offset));
                    fprintf(fp, "\t jz boolPrintFalse%d \n", siblingId->tkinfo->lno);

                    fprintf(fp, "boolPrintTrue%d: \n", siblingId->tkinfo->lno);
                    fprintf(fp, "\t mov rdi, outputBooleanTrue \n");
                    fprintf(fp, "\t jmp boolPrintEnd%d \n", siblingId->tkinfo->lno);

                    fprintf(fp, "boolPrintFalse%d: \n", siblingId->tkinfo->lno);
                    fprintf(fp, "\t mov rdi, outputBooleanFalse \n");

                    fprintf(fp, "boolPrintEnd%d: \n", siblingId->tkinfo->lno);
                    fprintf(fp, "\t call printf \n");
                }
                else if(idVarType.baseType == g_INTEGER) {

                    fprintf(fp, "\t mov rdi, outputInt \n");
                    fprintf(fp, "\t mov rsi, [%s - %d] \n", baseRegister[idVar.isIOlistVar], 2 * (idVarType.width + idVar.offset));
                    fprintf(fp, "\t call printf \n");
                }
                else if(idVarType.baseType == g_REAL) {
                    fprintf(fp, "\t mov rdi, outputFloat \n");
                    fprintf(fp, "\t mov rsi, [%s - %d] \n", baseRegister[idVar.isIOlistVar], 2 * (idVarType.width + idVar.offset));
                    fprintf(fp, "\t call printf \n");
                }

                fprintf(fp, "\t pop rbp \n");
            }
            else /* Arrays */ {
                // Use whichId AST Node here.
            }

            return;
        }
        case g_conditionalStmt:{
            ASTNode *idNode = root->child;
            symVarInfo vi = idNode->stNode->info.var;
//            mov rsi, rbp
//            sub rsi, 4
            fprintf(fp,"\t mov rsi, %s \n",baseRegister[vi.isIOlistVar]);
            fprintf(fp,"\t sub rsi, [ %s - %d ] \n",baseRegister[vi.isIOlistVar], 2*(vi.offset + vi.vtype.width));
            // rsi now points to where the data will be extracted from
            char regStr[5];
            switch(idNode->stNode->info.var.vtype.baseType){
                case g_INTEGER: {
                    getAptReg(regStr, 8, vi.vtype.width);
                    fprintf(fp, "\t mov %s, [ rsi ] \n", regStr);
                    ASTNode *valList = idNode->next->child->child; // On NUM or TRUE or FALSE
                    ASTNode *ptr = valList;
                    fprintf(fp, "\t ; comparisons start for cases \n");
                    while(ptr!=NULL){
                        fprintf(fp, "\t cmp %s, [ rsi ] \n", regStr);
                    }
                }
                break;
                case g_BOOLEAN:
                    break;
                default:
                    printf("generateCode: Mistake in semantic analyser. Got invalid switch var data type.\n");
            }
        }

            return;
        default:
            printf("Default : %s \n", inverseMappingTable[gs]);
    }

}

void getAptReg(char *regStr, int regno, int width){
//R0  R1  R2  R3  R4  R5  R6  R7  R8  R9  R10  R11  R12  R13  R14  R15
//RAX RCX RDX RBX RSP RBP RSI RDI
    switch(width){
        case 1:
            sprintf(regStr,"r%dw",regno); // 2 bytes
            break;
        case 2:
            sprintf(regStr,"r%dd",regno); // 4 bytes
            break;
        case 4:
            sprintf(regStr,"r%d",regno); // 8 bytes
            break;
        default:
            printf("getAptReg: Got invalid width.\n");
    }
}