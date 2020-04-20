// Group Number: 31
// MADHUR PANWAR   2016B4A70933P
// TUSSANK GUPTA   2016B3A70528P
// SALMAAN SHAHID  2016B4A70580P
// APURV BAJAJ     2016B3A70549P
// HASAN NAQVI     2016B5A70452P

// nasm -felf64 code.asm && gcc code.o && ./a.out
// nasm -felf64 code.asm && gcc -no-pie code.o && ./a.out

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "codeGen.h"
#include "symbolTableDef.h"
#include "symbolHash.h"
#include "symbolTable.h"
#include "typeCheck.h"
#include "lexerDef.h"

//prereq: something related to stack alignment (odd number of pushes)
void RUNTIME_EXIT_WITH_ERROR(FILE *fp, char *e) {
    fprintf(fp, "\t mov rdi, %s \n", e); 
    fprintf(fp, "\t call printf \n"); 
    fprintf(fp, "\t mov rax, 60 \n"); 
    fprintf(fp, "\t xor rdi, rdi \n"); 
    fprintf(fp, "\t syscall \n");
}
extern char *inverseMappingTable[];


char *baseRegister[2] = {"RBP", "RBX"};

char *expreg[4] = {"r8","r9","r10","r11"};
int arrBaseSize = 1;    //in words
int scale = 2;

int getActivationRecordSize(char *functionName, symbolTable* symT) {
    symFuncInfo *finfo = stGetFuncInfo(functionName, symT);
    
    int size = scale * (finfo->st->scopeSize);

    if (size % 16)
        size += (16 - (size % 16));

    return size;
}

int getIOlistSize(char *functionName, symbolTable* symT) {
    symFuncInfo *finfo = stGetFuncInfo(functionName, symT);
    
    int size = (finfo->arSize) - (finfo->st->scopeSize);
    size = scale * size;

    if (size % 16)
        size += (16 - (size % 16));

    return size;
}

void setExpSize(gSymbol etype, char **expSizeStr, char **expSizeRegSuffix){
    switch(etype){
        case g_INTEGER:
            *expSizeStr = "dword";
            *expSizeRegSuffix = "d";
            break;
        case g_BOOLEAN:
            *expSizeStr = "word";
            *expSizeRegSuffix = "w";
            break;
        case g_REAL:
            *expSizeStr = "qword";
            *expSizeRegSuffix = "";
            break;
        default:
            *expSizeStr = "word";
            *expSizeRegSuffix = "w";
            break;
    }
}

//outcome: lower bound in expreg[0], upper bound in expreg[1]
//affects: No other register affected
void getArrBoundsInExpReg(ASTNode *arrNode, FILE *fp){
    char *expSizeStr, *expSizeRegSuffix;
    setExpSize(g_INTEGER,&expSizeStr,&expSizeRegSuffix);
    varType arrVtype = arrNode->stNode->info.var.vtype;
    if(arrVtype.vaType == DYN_L_ARR || arrVtype.vaType == DYN_ARR){
        //get the left bound to expreg[0]
        symTableNode *leftStn = arrVtype.si.vt_id;
        bool isIOlistVar = arrNode->stNode->info.var.isIOlistVar;
        int toSub;
        if(isIOlistVar){
            toSub = scale *(arrNode->stNode->info.var.offset + arrBaseSize + getSizeByType(g_INTEGER));
        }
        else{
            toSub = scale * (leftStn->info.var.offset + leftStn->info.var.vtype.width);
        }
        fprintf(fp,"\t xor %s, %s \n",expreg[0],expreg[0]);
        fprintf(fp, "\t mov %s%s, %s[%s-%d] \n", expreg[0],expSizeRegSuffix, expSizeStr, baseRegister[isIOlistVar||leftStn->info.var.isIOlistVar], toSub);
    }
    if(arrVtype.vaType == DYN_R_ARR || arrVtype.vaType == DYN_ARR){
        //get the right bound to expreg[1]
        symTableNode *rightStn = arrVtype.ei.vt_id;
        bool isIOlistVar = arrNode->stNode->info.var.isIOlistVar;
        int toSub;
        if(isIOlistVar){
            toSub = scale *(arrNode->stNode->info.var.offset + arrBaseSize + (2*getSizeByType(g_INTEGER)));
        }
        else{
            toSub = scale * (rightStn->info.var.offset + rightStn->info.var.vtype.width);
        }
        fprintf(fp,"\t xor %s, %s \n",expreg[1],expreg[1]);
        fprintf(fp, "\t mov %s%s, %s[%s-%d] \n", expreg[1],expSizeRegSuffix, expSizeStr, baseRegister[isIOlistVar||rightStn->info.var.isIOlistVar], toSub);
    }
    
    if(arrVtype.vaType == DYN_R_ARR || arrVtype.vaType == STAT_ARR){
        //get the left bound to expreg[0]
        fprintf(fp,"\t mov %s, %d \n",expreg[0],arrVtype.si.vt_num);
    }
    if(arrVtype.vaType == DYN_L_ARR || arrVtype.vaType == STAT_ARR){
        //get the right bound to expreg[1]
        fprintf(fp,"\t mov %s, %d \n",expreg[1],arrVtype.ei.vt_num);
    }
}

//prereq: lower bound in expreg[0], index in expreg[2]
//outcome: address of array element at idx in expreg[1]
//affects: expreg[2] value destroyed
void getArrAddrAtIdx(ASTNode *arrNode, FILE *fp){
    char *expSizeStr, *expSizeRegSuffix;
    fprintf(fp,"\t sub %s, %s \n",expreg[2],expreg[0]);
    //    fprintf(fp,"\t add %s, 1 \n",expreg[2]);
    fprintf(fp,"\t add %s, %s \n",expreg[2],expreg[2]);
    setExpSize(arrNode->stNode->info.var.vtype.baseType, &expSizeStr, &expSizeRegSuffix);
    if(arrNode->stNode->info.var.vtype.baseType == g_INTEGER){
        //size of int is 2words (4 bytes)
        fprintf(fp,"\t add %s, %s \n",expreg[2],expreg[2]);
    }
    else if(arrNode->stNode->info.var.vtype.baseType == g_REAL){
        fprintf(fp,"\t add %s, %s \n",expreg[2],expreg[2]);
        fprintf(fp,"\t add %s, %s \n",expreg[2],expreg[2]);
    }
    fprintf(fp,"\t xor %s, %s \n",expreg[1],expreg[1]);
    //br[isiolvar] - 2*(offset + arrBaseSize)
    int toSub = scale * (arrNode->stNode->info.var.offset + arrBaseSize);
    bool isIOlistVar = arrNode->stNode->info.var.isIOlistVar;
    fprintf(fp,"\t movsx %s, word[%s-%d] \n",expreg[1],baseRegister[isIOlistVar],toSub);
    fprintf(fp,"\t add %s, [stack_top] \n",expreg[1]);
    fprintf(fp,"\t sub %s, %s \n",expreg[1],expreg[2]);
}

//prereq: lower bound in expreg[0], index in expreg[2]
//outcome: array value at idx in expreg[0]
//affects: expreg[2] value destroyed
void getArrValueAtIdxInReg(ASTNode *arrNode, FILE *fp){
    char *expSizeStr, *expSizeRegSuffix;
    getArrAddrAtIdx(arrNode,fp);
    setExpSize(arrNode->stNode->info.var.vtype.baseType, &expSizeStr, &expSizeRegSuffix);
    fprintf(fp,"\t xor %s, %s \n",expreg[0],expreg[0]);
    fprintf(fp,"\t mov %s%s, %s[%s] \n",expreg[0],expSizeRegSuffix,expSizeStr,expreg[1]);
}

//prereq: lower bound in expreg[0], upper bound in expreg[1], index in expreg[2]
void boundCheckArrAndExit(void *someRefPtr, FILE *fp) {
    //someRefPtr is any unique address
    fprintf(fp,"\t cmp %s, %s \n",expreg[2],expreg[0]);
    fprintf(fp,"\t jge lb_ok_%p \n",someRefPtr);
    fprintf(fp,"\t mov rsp, [preExpRSP] \n\t push r8 ;just for stack alignment\n");
    RUNTIME_EXIT_WITH_ERROR(fp,"OUT_OF_BOUNDS");
    fprintf(fp,"lb_ok_%p: \n",someRefPtr);
    fprintf(fp,"\t cmp %s, %s \n",expreg[2],expreg[1]);
    fprintf(fp,"\t jle rb_ok_%p \n",someRefPtr);
    fprintf(fp,"\t mov rsp, [preExpRSP] \n\t push r8 ;just for stack alignment\n");
    RUNTIME_EXIT_WITH_ERROR(fp,"OUT_OF_BOUNDS");
    fprintf(fp,"rb_ok_%p: \n",someRefPtr);
}

void genExpr(ASTNode *astNode, FILE *fp, bool firstCall, gSymbol expType){
    char *expSizeStr = "word";
    char *expSizeRegSuffix = "";    //"d","w",""
    setExpSize(expType,&expSizeStr,&expSizeRegSuffix);
    if(firstCall){
        if(astNode == NULL)
            return;
        else if(astNode->gs != g_assignmentStmt){
            //            printf("%d \n",astNode->gs);
            genExpr(astNode->next,fp,true,expType);
            genExpr(astNode->child,fp,true,expType);
            return;
        }
        //assignmentStatement Node will be passed
        ASTNode *idNode = astNode->child->child->child;
        expType = idNode->stNode->info.var.vtype.baseType;
        setExpSize(idNode->stNode->info.var.vtype.baseType,&expSizeStr,&expSizeRegSuffix);
        // printf("%s \n",idNode->tkinfo->lexeme);
        if(idNode->next->next != NULL){
            genExpr(idNode->next->next,fp,false,expType);
            getArrBoundsInExpReg(idNode,fp);
            if(idNode->next->gs == g_ID){
                ASTNode *idxIdNode = idNode->next;
                bool isIOlistVar = idxIdNode->stNode->info.var.isIOlistVar;
                int toSub = scale * (idxIdNode->stNode->info.var.offset + idxIdNode->stNode->info.var.vtype.width);
                setExpSize(g_INTEGER,&expSizeStr,&expSizeRegSuffix);
                fprintf(fp,"\t xor %s,%s \n",expreg[2],expreg[2]);
                fprintf(fp,"\t mov %s%s, %s[%s-%d] \n",expreg[2],expSizeRegSuffix,expSizeStr,baseRegister[isIOlistVar],toSub);
            }
            else{
                fprintf(fp,"\t mov %s, %d \n",expreg[2],idNode->next->tkinfo->value.num);
            }
            //now we have left bound in expreg[0], right bound in expreg[1] and index in expreg[2]
            boundCheckArrAndExit(idNode->next, fp);
            getArrAddrAtIdx(idNode,fp);
            setExpSize(idNode->stNode->info.var.vtype.baseType,&expSizeStr,&expSizeRegSuffix);
            fprintf(fp,"\t pop %s \n",expreg[0]);
            fprintf(fp, "\t mov %s[%s], %s%s \n", expSizeStr, expreg[1],expreg[0], expSizeRegSuffix);
            return;
        }
        else{
            //array or variable
            if(idNode->stNode->info.var.vtype.vaType == VARIABLE){
                //variable
                genExpr(idNode->next,fp,false,expType);
                bool isIOlistVar = idNode->stNode->info.var.isIOlistVar;
                int toSub = scale * (idNode->stNode->info.var.offset + idNode->stNode->info.var.vtype.width);
                fprintf(fp,"\t xor %s, %s \n",expreg[1],expreg[1]);
                fprintf(fp,"\t pop %s \n",expreg[1]);
                fprintf(fp, "\t mov %s[%s-%d], %s%s \n", expSizeStr, baseRegister[isIOlistVar], toSub, expreg[1],expSizeRegSuffix);
                return;
            }
            else{
                //array
                ASTNode *arr1Node = idNode;
                ASTNode *arr2Node = idNode->next->child;
                getArrBoundsInExpReg(arr1Node,fp);
                fprintf(fp,"\t mov %s, %s \n",expreg[2],expreg[0]);
                fprintf(fp,"\t mov %s, %s \n",expreg[3],expreg[1]);
                getArrBoundsInExpReg(arr2Node,fp);
                fprintf(fp,"\t cmp %s, %s \n",expreg[0],expreg[2]);
                fprintf(fp,"\t je lb_match_%p_%p \n",arr1Node,arr2Node);
                fprintf(fp,"\t mov rsp, [preExpRSP] \n\t push r8 ;just for stack alignment\n");
                RUNTIME_EXIT_WITH_ERROR(fp,"ARR_TYPE_MISMATCH");
                fprintf(fp,"lb_match_%p_%p:\n",arr1Node,arr2Node);
                fprintf(fp,"\t cmp %s, %s \n",expreg[1],expreg[3]);
                fprintf(fp,"\t je rb_match_%p_%p \n",arr1Node,arr2Node);
                fprintf(fp,"\t mov rsp, [preExpRSP] \n\t push r8 ;just for stack alignment\n");
                RUNTIME_EXIT_WITH_ERROR(fp,"ARR_TYPE_MISMATCH");
                fprintf(fp,"rb_match_%p_%p:\n",arr1Node,arr2Node);
                //match successful, now copy

                //get base address of arr2
                int toSub2 = scale * (arr2Node->stNode->info.var.offset + arrBaseSize);
                bool isIOlistVar2 = arr2Node->stNode->info.var.isIOlistVar;
                fprintf(fp,"\t mov %sw, word[%s-%d] \n",expreg[0],baseRegister[isIOlistVar2],toSub2);

                //copy base address to arr1
                int toSub1 = scale * (arr1Node->stNode->info.var.offset + arrBaseSize);
                bool isIOlistVar1 = arr1Node->stNode->info.var.isIOlistVar;
                fprintf(fp,"\t mov word[%s-%d], %sw \n",baseRegister[isIOlistVar1],toSub1,expreg[0]);

                /* //ARRAY COPY IMPLEMENTATION
                fprintf(fp,"\t mov [asgnLB], %s \n",expreg[0]);
                fprintf(fp,"\t mov [asgnRB], %s \n",expreg[1]);
                //prereq: lower bound in expreg[0], index in expreg[2]
                //outcome: address of array element at idx in expreg[1]
                //affects: expreg[2] value destroyed
                getArrAddrAtIdx(arr2Node,fp);
                fprintf(fp,"\t mov %s, %s \n",expreg[3],expreg[1]);
                fprintf(fp,"\t mov %s, [asgnLB] \n",expreg[0]); //left bound
                fprintf(fp,"\t mov %s, [asgnRB] \n",expreg[1]); //right bound
                fprintf(fp,"\t mov %s, [asgnLB] \n",expreg[2]); //first index
                //prereq: lower bound in expreg[0], index in expreg[2]
                //outcome: address of array element at idx in expreg[1]
                //affects: expreg[2] value destroyed
                getArrAddrAtIdx(arr1Node,fp);
                fprintf(fp,"\t mov %s, %s \n",expreg[2],expreg[1]);
                //NOW eR[2] contains addr of first element of arr1 & eR[3] contains addr of first element of arr2
                fprintf(fp,"\t mov %s, [asgnLB] \n",expreg[0]); //current index in er0
                setExpSize(arr1Node->stNode->info.var.vtype.baseType,&expSizeStr,&expSizeRegSuffix);
                int toSub = scale * getSizeByType(arr1Node->stNode->info.var.vtype.baseType);
                fprintf(fp,"arr_asgn_%p_%p: \n",arr1Node,arr2Node);
                fprintf(fp,"\t xor %s, %s \n",expreg[1],expreg[1]);
                //move data from arr2[i] to arr1[i]
                fprintf(fp,"\t mov %s%s, %s[%s] \n",expreg[1],expSizeRegSuffix,expSizeStr,expreg[3]);
                fprintf(fp,"\t mov %s[%s], %s%s \n",expSizeStr,expreg[2],expreg[1],expSizeRegSuffix);
                //move index & offsets accordingly
                fprintf(fp,"\t sub %s, %d \n",expreg[3],toSub);
                fprintf(fp,"\t sub %s, %d \n",expreg[2],toSub);
                fprintf(fp,"\t inc %s \n",expreg[0]);
                fprintf(fp,"\t cmp %s, [asgnRB] \n",expreg[0]);
                fprintf(fp,"\t jle arr_asgn_%p_%p \n",arr1Node,arr2Node);
                 */
            }
        }
    }
    else{
        if(astNode->gs == g_u){
            ASTNode *uOp = astNode->child;
            genExpr(uOp->next,fp,false,expType);
            if(uOp->gs == g_MINUS){
                fprintf(fp,"\t pop %s \n",expreg[1]);
                fprintf(fp,"\t xor %s, %s \n",expreg[0],expreg[0]);
                fprintf(fp,"\t sub %s,%s \n",expreg[0],expreg[1]);
                fprintf(fp,"\t push %s \n", expreg[0]);
            }
        }
        else if(astNode->gs == g_var_id_num){
            astNode = astNode->child;
            switch(astNode->gs){
                case g_NUM:
                    fprintf(fp,"\t push %d \n",astNode->tkinfo->value.num);
                    break;
                case g_RNUM:
                    //TODO: Mov real constant to stack
                    break;
                case g_ID:
                {
                    setExpSize(astNode->stNode->info.var.vtype.baseType,&expSizeStr,&expSizeRegSuffix);
                    if(astNode->stNode->info.var.vtype.vaType == VARIABLE){
                        bool isIOlistVar = astNode->stNode->info.var.isIOlistVar;
                        int toSub = scale * (astNode->stNode->info.var.offset + astNode->stNode->info.var.vtype.width);
                        switch(astNode->stNode->info.var.vtype.baseType){
                            case g_REAL:
                                //TODO: Handle real variables
                                break;
                            case g_INTEGER:
                            case g_BOOLEAN:
                                fprintf(fp,"\t xor %s,%s \n",expreg[0],expreg[0]);
                                fprintf(fp,"\t mov %s%s, %s[%s-%d] \n",expreg[0],expSizeRegSuffix,expSizeStr,baseRegister[isIOlistVar],toSub);
                                fprintf(fp,"\t push %s \n",expreg[0]);
                                break;
                        }
                    }
                    else{
                        //for bounds
                        getArrBoundsInExpReg(astNode,fp);
                        if(astNode->next->gs == g_ID){
                            ASTNode *idxIdNode = astNode->next;
                            bool isIOlistVar = idxIdNode->stNode->info.var.isIOlistVar;
                            int toSub = scale * (idxIdNode->stNode->info.var.offset + idxIdNode->stNode->info.var.vtype.width);
                            setExpSize(g_INTEGER,&expSizeStr,&expSizeRegSuffix);
                            fprintf(fp,"\t xor %s,%s \n",expreg[2],expreg[2]);
                            fprintf(fp,"\t mov %s%s, %s[%s-%d] \n",expreg[2],expSizeRegSuffix,expSizeStr,baseRegister[isIOlistVar],toSub);
                        }
                        else{
                            fprintf(fp,"\t mov %s, %d \n",expreg[2],astNode->next->tkinfo->value.num);
                        }
                        //now we have left bound in expreg[0], right bound in expreg[1] and index in expreg[2]
                        boundCheckArrAndExit(astNode->next, fp);
                        //toSub from array base
                        getArrValueAtIdxInReg(astNode,fp);
                        fprintf(fp,"\t push %s \n",expreg[0]);
                    }
                }
                break;
            }
        }
        else if(astNode->gs == g_TRUE){
            fprintf(fp,"\t push 1 \n");
        }
        else if(astNode->gs == g_FALSE){
            fprintf(fp,"\t push 0 \n");
        }
        else{
            //astnode is an operator
            genExpr(astNode->child,fp,false,expType);
            genExpr(astNode->child->next,fp,false,expType);
            if(expType == g_REAL){
                //handle real operations
            }
            else{
                fprintf(fp,"\t xor %s,%s \n",expreg[1],expreg[1]);
                fprintf(fp,"\t pop %s \n",expreg[1]);
                fprintf(fp,"\t xor %s,%s \n",expreg[0],expreg[0]);
                fprintf(fp,"\t pop %s \n",expreg[0]);
                char *jCmd = NULL;
                switch(astNode->gs){
                    case g_PLUS:
                        fprintf(fp,"\t add %s, %s \n",expreg[0],expreg[1]);
                        break;
                    case g_MINUS:
                        fprintf(fp,"\t sub %s, %s \n",expreg[0],expreg[1]);
                        break;
                    case g_MUL:
                        setExpSize(g_INTEGER,&expSizeStr,&expSizeRegSuffix);
                        fprintf(fp,"\t push rax \n\t push rdx \n\t xor rdx,rdx \n");   //to save the prev value
                        fprintf(fp,"\t mov rax, %s \n",expreg[0]);
                        fprintf(fp,"\t imul %s%s \n",expreg[1],expSizeRegSuffix);
                        fprintf(fp,"\t mov %s, rax \n",expreg[0]);
                        fprintf(fp,"\t pop rdx \n\t pop rax \n");    //to restore the prev value
                        break;
                    case g_DIV:
                        setExpSize(g_INTEGER,&expSizeStr,&expSizeRegSuffix);
                        fprintf(fp,"\t push rax \n\t push rdx \n\t xor rdx,rdx \n");   //to save the prev value
                        fprintf(fp,"\t mov rax, %s \n",expreg[0]);
                        fprintf(fp,"\t cmp %s%s, 0 \n",expreg[1],expSizeRegSuffix);
                        fprintf(fp,"\t jne div0_safe_%p \n",astNode);
                        fprintf(fp,"\t mov rsp, [preExpRSP] \n\t push r8 ;just for stack alignment\n");
                        RUNTIME_EXIT_WITH_ERROR(fp,"DIV_BY_ZERO");
                        fprintf(fp,"div0_safe_%p: \n",astNode);
                        fprintf(fp,"\t idiv %s%s \n",expreg[1],expSizeRegSuffix);
                        fprintf(fp,"\t mov %s, rax \n",expreg[0]);
                        fprintf(fp,"\t pop rdx \n\t pop rax \n");    //to restore the prev value
                        break;
                    case g_AND:
                        fprintf(fp,"\t and %s, %s \n",expreg[0],expreg[1]);
                        break;
                    case g_OR:
                        fprintf(fp,"\t or %s, %s \n",expreg[0],expreg[1]);
                        break;
                    case g_GT:
                        jCmd = "jg";
                        break;
                    case g_LT:
                        jCmd = "jl";
                        break;
                    case g_GE:
                        jCmd = "jge";
                        break;
                    case g_LE:
                        jCmd = "jle";
                        break;
                    case g_EQ:
                        jCmd = "je";
                        break;
                    case g_NE:
                        jCmd = "jne";
                        break;
                }
                if(jCmd != NULL){
                    setExpSize(g_INTEGER,&expSizeStr,&expSizeRegSuffix);
                    fprintf(fp,"\t cmp %s%s, %s%s \n",expreg[0],expSizeRegSuffix,expreg[1],expSizeRegSuffix);
                    fprintf(fp,"\t %s exp_t_%p \n",jCmd,(void *)astNode);
                    fprintf(fp,"\t mov %s, 0 \n",expreg[0]);
                    fprintf(fp,"\t jmp exp_f_%p \n",(void *)astNode);
                    fprintf(fp," exp_t_%p:\n",(void*)astNode);
                    fprintf(fp,"\t mov %s, 1 \n",expreg[0]);
                    fprintf(fp," exp_f_%p:\n",(void*)astNode);
                }
                fprintf(fp,"\t push %s \n",expreg[0]);
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
            fprintf(fp, "section .bss \n");
            fprintf(fp, "\t stack_top: resb 8 \n");
            fprintf(fp, "\t inta: resb 4 \n");
            fprintf(fp, "\t floatb: resb 8 \n");
            fprintf(fp, "\t boolc: resb 2 \n");
            fprintf(fp,"\t asgnLB: resb 8 \n");
            fprintf(fp,"\t asgnRB: resb 8 \n");
            fprintf(fp,"\t preExpRSP: resb 8 \n");

            fprintf(fp, "section .data \n");

            fprintf(fp,"\t msgBoolean: db \"Input: Enter a boolean value\", 10, 0 \n");
            fprintf(fp,"\t inputBoolean: db \"%%hd\", 0 \n");
            fprintf(fp,"\t msgBooleanArr: db \"Input: Enter %%d array elements of boolean type for range %%d to %%d\", 10, 0 \n");

            fprintf(fp,"\t msgInt: db \"Input: Enter an integer value\", 10, 0 \n");
            fprintf(fp,"\t inputInt: db \"%%d\", 0 \n");
            fprintf(fp,"\t msgIntArr: db \"Input: Enter %%d array elements of integer type for range %%d to %%d\", 10, 0 \n");

            fprintf(fp,"\t msgFloat: db \"Input: Enter a float value\", 10, 0 \n");
            fprintf(fp,"\t inputFloat: db \"%%lf\",0 \n");

            fprintf(fp,"\t outputBooleanTrue: db \"Output: true\", 10, 0, \n");
            fprintf(fp,"\t outputBooleanFalse: db \"Output: false\", 10, 0, \n");

            fprintf(fp,"\t outputInt: db \"Output: %%d\", 10, 0, \n");
            fprintf(fp,"\t outputFloat: db \"Output: %%lf\", 10, 0, \n");

            fprintf(fp,"\t output: db \"Output: \", 0 \n");
            fprintf(fp,"\t intHolder: db \"%%d \", 0 \n");
            fprintf(fp,"\t booleanTrue: db \"true \", 0 \n");
            fprintf(fp,"\t booleanFalse: db \"false \", 0 \n");
            fprintf(fp,"\t newLine: db \" \", 10, 0 \n");

            fprintf(fp, "\t OUT_OF_BOUNDS: db \"RUN TIME ERROR:  Array index out of bound\", 10, 0 \n");
            fprintf(fp, "\t ARR_TYPE_MISMATCH: db \"RUN TIME ERROR:  Bounds do not match for LHS Array and RHS Array\", 10, 0 \n");
            fprintf(fp, "\t ARR_TYPE_MISMATCH2: db \"RUN TIME ERROR:  Bounds do not match for formal and actual arrays\", 10, 0 \n");
            fprintf(fp, "\t UPPER_BOUND_SMALL: db \"RUN TIME ERROR:  Upper bound of dynamic array is smaller than lower bound\", 10, 0 \n");
            fprintf(fp, "\t DIV_BY_ZERO: db \"RUN TIME ERROR:  Division by Zero.\", 10, 0 \n");



            fprintf(fp, "\n section .text \n");
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
        {
            // moduleDeclarations -> ID moduleDeclarations

            ASTNode* ASTChild = root->child;

            while(ASTChild) {
                generateCode(ASTChild, symT, fp);
                ASTChild = ASTChild->next;
            }

            return;
        }

        case g_module:
        {
            //<module> -> DEF MODULE ID ENDDEF TAKES INPUT SQBO <input_plist> SQBC SEMICOL <ret> <moduleDef>
            ASTNode *functionID = root->child;
            ASTNode *inputList = functionID->next;
            ASTNode *outputList = inputList->next;
            ASTNode *moduleDef = outputList->next;

            if(outputList->gs == g_moduleDef)
                moduleDef = outputList;

            fprintf(fp, "\n %s: \n", functionID->tkinfo->lexeme);
            fprintf(fp, "\t ; stack init starts \n");
            fprintf(fp, "\t mov rbp, rsp \n");
            fprintf(fp, "\t sub rsp, %d \n", getActivationRecordSize(functionID->tkinfo->lexeme, symT)); 
            fprintf(fp, "\t ; stack init done. \n\n");

            generateCode(moduleDef, symT, fp);

            fprintf(fp, "\t mov rsp, rbp \n");
            fprintf(fp, "\t ret \n");

            return;
        }


        case g_DRIVER:
        {
            fprintf(fp, "\n main: \n");
            fprintf(fp, "\t ; stack init starts \n");
            fprintf(fp, "\t mov rbp, rsp \n");
            fprintf(fp, "\t mov QWORD[stack_top], rsp \n");

            fprintf(fp, "\t sub rsp, %d \n", getActivationRecordSize("@driver", symT));
            fprintf(fp, "\t ; stack init done. \n\n");

            generateCode(root->child, symT, fp);

            fprintf(fp, "\t mov rsp, rbp \n");
            fprintf(fp, "\t push rsp \n");
            fprintf(fp, "\t mov rax, 60 \n"); 
            fprintf(fp, "\t xor rdi, rdi \n"); 
            fprintf(fp, "\t syscall \n");
            fprintf(fp, "\t ; driver ends \n\n");

            return;
        }
        case g_moduleDef:
        case g_simpleStmt:
        case g_START:
        {
            generateCode(root->child, symT, fp);
            return;
        }

        case g_ID:
        {
            return;
        }

        case g_iterativeStmt:
        {
            generateCode(root->child, symT, fp);
            return;
        }

        case g_FOR:
        {
            ASTNode* idNode = root->next; // ID
            ASTNode* rangeNode = idNode->next;
            ASTNode* startNode = rangeNode->child;
            ASTNode* endNode = startNode->next;
            ASTNode* statementsNode = rangeNode->next->child;

            symVarInfo idNodeVar = idNode->stNode->info.var;
            // Experimental
            fprintf(fp, "\t mov dword[%s - %d], %s \n", baseRegister[idNodeVar.isIOlistVar], 2*(idNodeVar.vtype.width + idNodeVar.offset), startNode->tkinfo->lexeme);
            fprintf(fp, "\t cmp dword[%s - %d], %s \n", baseRegister[idNodeVar.isIOlistVar], 2*(idNodeVar.vtype.width + idNodeVar.offset), endNode->tkinfo->lexeme);
            fprintf(fp, "\t ja forLoopEnd_%p \n", endNode);
            fprintf(fp, "forLoopStart_%p: \n", startNode);
            generateCode(statementsNode, symT, fp);
            fprintf(fp, "\t inc dword[%s - %d] \n", baseRegister[idNodeVar.isIOlistVar], 2*(idNodeVar.vtype.width + idNodeVar.offset));
            fprintf(fp, "\t cmp dword[%s - %d], %s \n", baseRegister[idNodeVar.isIOlistVar], 2*(idNodeVar.vtype.width + idNodeVar.offset), endNode->tkinfo->lexeme);
            fprintf(fp, "\t jna forLoopStart_%p \n", startNode);
            fprintf(fp, "forLoopEnd_%p: \n", endNode);

            return;
        }

        case g_WHILE:
        {
            fprintf(fp,"\t ; WHILE starts \n");

            fprintf(fp," WHILE_loop_%p: \n", root->next->next); //pointer to WHILE's START ASTNode (uniqueness assured across entire code)
            fprintf(fp,"\t ; evaluating while condition \n");
            fprintf(fp,"\t mov [preExpRSP], rsp \n");   //store the pre expression rsp into preExpRSP
            genExpr(root->next, fp, false, g_BOOLEAN); // puts 8 bytes containing result of condition evaluation on stack
            fprintf(fp,"\t ; while condition evaluated \n");


            fprintf(fp,"\t pop r8 \n"); // pop in r8
            fprintf(fp,"\t cmp r8, 0 \n"); // cmp r8, 0

            fprintf(fp,"\t ; exit the loop if condition was false \n");
            fprintf(fp,"\t jz EXIT_WHILE_loop_%p \n", root);

            fprintf(fp,"\t ; execute following statements if condition was true \n");
            fprintf(fp,"\t ; while loop statements start \n");
            generateCode(root->next->next->child, symT, fp); // recurse on statements
            fprintf(fp,"\t ; while loop statements end \n");

            fprintf(fp,"\t jmp WHILE_loop_%p \n", root->next->next);
            fprintf(fp," EXIT_WHILE_loop_%p: \n", root); //pointer to WHILE's ASTNode (uniqueness assured across entire code)

            fprintf(fp,"\t ; WHILE ends \n");

        }
        return;

        case g_moduleReuseStmt:
        {
            ASTNode *idOrAssignop = root->child;

            ASTNode *outputList = NULL, *functionID, *inputList;

            if(idOrAssignop->gs == g_ASSIGNOP) {
                outputList = idOrAssignop->child; // If present, return values
                functionID = outputList->next;
                inputList = functionID->next;
            }
            else {
                functionID = idOrAssignop;
                inputList = functionID->next;
            }

            symFuncInfo *finfo = stGetFuncInfo(functionID->tkinfo->lexeme, symT);
            symTableNode *inputParam = finfo->inpPListHead;
            symTableNode *outputParam = finfo->outPListHead;

            ASTNode *idNode = inputList->child;

            char *sizeStr, *regSuffix;

            while(idNode != NULL) {

                varType actualVarType = idNode->stNode->info.var.vtype;
                symVarInfo actualVar = idNode->stNode->info.var;

                varType formalVarType = inputParam->info.var.vtype;
                symVarInfo formalVar = inputParam->info.var;

                if (actualVarType.vaType == VARIABLE) {
                    setExpSize(actualVarType.baseType, &sizeStr, &regSuffix);
                    fprintf(fp, "\t mov r12%s, %s [%s - %d] \n", regSuffix, sizeStr, baseRegister[actualVar.isIOlistVar], scale * (actualVarType.width + actualVar.offset)); 
                    fprintf(fp, "\t mov %s [rsp - %d], r12%s \n", sizeStr, scale * (formalVarType.width + formalVar.offset), regSuffix);
                }
                else {
                    getArrBoundsInExpReg(idNode, fp);

                    fprintf(fp, "\t mov dword [rsp - %d], %sd \n",  scale * (arrBaseSize + formalVar.offset + getSizeByType(g_INTEGER)), expreg[0]); // lb
                    fprintf(fp, "\t mov dword [rsp - %d], %sd \n",  scale * (arrBaseSize + formalVar.offset + 2*getSizeByType(g_INTEGER)), expreg[1]); // ub

                    fprintf(fp, "\t mov r12, %s \n", expreg[0]);
                    fprintf(fp, "\t mov r13, %s \n", expreg[1]);

                    fprintf(fp, "\t mov rsi, %s \n", baseRegister[actualVar.isIOlistVar]); 
                    fprintf(fp, "\t sub rsi, %d \n", scale * (arrBaseSize + actualVar.offset));
                    fprintf(fp, "\t movsx rsi, word[rsi] \n"); // move base val
                    fprintf(fp, "\t mov word [rsp - %d], si \n",  scale * (arrBaseSize + formalVar.offset)); // base address

                    varType arrVtype = inputParam->info.var.vtype;

                    if(arrVtype.vaType == DYN_R_ARR || arrVtype.vaType == STAT_ARR){
                        //get the left bound to expreg[0]
                        fprintf(fp,"\t mov %s, %d \n",expreg[0],arrVtype.si.vt_num);
                        fprintf(fp, "\t cmp %s, r12 \n", expreg[0]);
                        fprintf(fp, "\t je lbound_match_%p \n", idNode);

                        fprintf(fp, "\t push rbx \n");
                        RUNTIME_EXIT_WITH_ERROR(fp, "ARR_TYPE_MISMATCH2");
                        fprintf(fp, "\t pop rbx \n");
                    }

                    fprintf(fp, "lbound_match_%p: \n", idNode);

                    if(arrVtype.vaType == DYN_L_ARR || arrVtype.vaType == STAT_ARR){
                        //get the right bound to expreg[1]
                        fprintf(fp,"\t mov %s, %d \n",expreg[1],arrVtype.ei.vt_num);
                        fprintf(fp, "\t cmp %s, r13 \n", expreg[1]);
                        fprintf(fp, "\t je ubound_match_%p \n", idNode);

                        fprintf(fp, "\t push rbx \n");
                        RUNTIME_EXIT_WITH_ERROR(fp, "ARR_TYPE_MISMATCH2");
                        fprintf(fp, "\t pop rbx \n");
                    }                  

                    fprintf(fp, "ubound_match_%p: \n", idNode);
                }
                inputParam = inputParam->next;
                idNode = idNode->next;
            }

            fprintf(fp, "\t sub rsp, %d \n", getIOlistSize(functionID->tkinfo->lexeme, symT)); 
            fprintf(fp, "\t push rbp \n");
            fprintf(fp, "\t push rbx \n");
            fprintf(fp, "\t mov rbx, rsp \n");
            fprintf(fp, "\t add rbx, %d \n", getIOlistSize(functionID->tkinfo->lexeme, symT) + 16); 

            fprintf(fp, "\t push rsi \n"); // Odd no of register
            fprintf(fp, "\t call %s \n", functionID->tkinfo->lexeme);
            fprintf(fp, "\t pop rsi \n");

            fprintf(fp, "\t pop rbx \n");
            fprintf(fp, "\t pop rbp \n");
            fprintf(fp, "\t add rsp, %d \n", getIOlistSize(functionID->tkinfo->lexeme, symT)); 

            if(outputList) {
                idNode = outputList->child;

                while(idNode != NULL) {

                    varType actualVarType = idNode->stNode->info.var.vtype;
                    symVarInfo actualVar = idNode->stNode->info.var;

                    varType formalVarType = outputParam->info.var.vtype;
                    symVarInfo formalVar = outputParam->info.var;

                    fprintf(fp, "\t mov rsi, %s \n", baseRegister[actualVar.isIOlistVar]);
                    fprintf(fp, "\t sub rsi, %d \n", scale * (actualVarType.width + actualVar.offset));
                    
                    setExpSize(actualVarType.baseType, &sizeStr, &regSuffix);
                    fprintf(fp, "\t mov r12%s, %s [rsp - %d] \n", regSuffix, sizeStr, scale * (formalVarType.width + formalVar.offset)); 
                    fprintf(fp, "\t mov %s [rsi], r12%s \n", sizeStr, regSuffix);
       
                    outputParam = outputParam->next;
                    idNode = idNode->next;
                }
            }

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
            fprintf(fp,"\t ; GET_VALUE(%s: %s) starts\n", siblingId->tkinfo->lexeme, inverseMappingTable[siblingId->stNode->info.var.vtype.baseType]);

            // <ioStmt> -> GET_VALUE BO ID BC SEMICOL

            if(! siblingId->stNode) {
                // Undeclared variable. Must have been already handled.
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
                }

                fprintf(fp, "\t pop rbp \n");
            }

            // Scan whole array
            else {

                fprintf(fp, "\t push rbp \n");

                if(idVarType.baseType == g_INTEGER) 
                    fprintf(fp, "\t mov rdi, msgIntArr \n");
                else if(idVarType.baseType == g_BOOLEAN) 
                    fprintf(fp, "\t mov rdi, msgBooleanArr \n");
                else{
                    printf("Real array!\n");
                    return;
                }

                getArrBoundsInExpReg(siblingId, fp);

                fprintf(fp, "\t mov r12, %s \n", expreg[0]); // lb
                fprintf(fp, "\t mov r13, %s \n", expreg[1]); // ub

                fprintf(fp, "\t sub %s, %s \n", expreg[1], expreg[0]);
                fprintf(fp, "\t inc %s \n", expreg[1]);

                fprintf(fp, "\t mov rsi, %s \n", expreg[1]); // no of elems
                fprintf(fp, "\t mov rdx, r12 \n"); // lb
                fprintf(fp, "\t mov rcx, r13 \n"); // ub

                fprintf(fp, "\t call printf \n");

                fprintf(fp, "\t mov rsi, %s \n", baseRegister[idVar.isIOlistVar]); // isIOlistVar may be 0 or 1
                fprintf(fp, "\t sub rsi, %d \n", scale * (arrBaseSize + idVar.offset));
                fprintf(fp, "\t movsx rsi, word[rsi] \n"); // move base val
                fprintf(fp, "\t add rsi, [stack_top] \n"); // address of first elem!

                if(idVarType.baseType == g_INTEGER)
                    fprintf(fp, "\t mov rdi, inputInt \n");
                else if(idVarType.baseType == g_BOOLEAN)
                    fprintf(fp, "\t mov rdi, inputBoolean \n");

                fprintf(fp, "scan_arr_%p: \n", siblingId);

                fprintf(fp, "\t push rsi \n");
                fprintf(fp, "\t push rdi \n");
                fprintf(fp, "\t call scanf \n");
                fprintf(fp, "\t pop rdi \n");
                fprintf(fp, "\t pop rsi \n");

                fprintf(fp, "\t cmp r12, r13 \n"); // ub
                fprintf(fp, "\t jz scan_arr_exit_%p \n", siblingId);

                fprintf(fp, "\t inc r12 \n");
                fprintf(fp, "\t sub rsi, %d \n", scale * getSizeByType(idVarType.baseType)); // address of n-th elem
                fprintf(fp, "\t jmp scan_arr_%p \n", siblingId);

                fprintf(fp, "scan_arr_exit_%p: \n", siblingId);

                fprintf(fp, "\t pop rbp \n");
            }

            fprintf(fp,"\t ; GET_VALUE(%s: %s) ends \n\n", siblingId->tkinfo->lexeme, inverseMappingTable[siblingId->stNode->info.var.vtype.baseType]);
            return;
        }

        case g_PRINT:
        {
            ASTNode* sibling = root->next;

            // <ioStmt> -> PRINT BO <var> BC SEMICOL
            // <boolConstt> -> TRUE | FALSE
            // <var_id_num> -> ID <whichId> | NUM | RNUM
            // <var> -> <var_id_num> | <boolConstt>
            // <whichId> -> SQBO <index> SQBC | ε
            // <index> -> NUM | ID


            if(sibling->gs == g_TRUE) {
                fprintf(fp,"\t ; PRINT(true) starts\n");

                fprintf(fp, "\t push rbp \n");
                fprintf(fp, "\t mov rdi, outputBooleanTrue \n");
                fprintf(fp, "\t call printf \n");
                fprintf(fp, "\t pop rbp \n");

                fprintf(fp,"\t ; PRINT(true) ends \n\n");
                return;
            }

            if(sibling->gs == g_FALSE) {
                fprintf(fp,"\t ; PRINT(false) starts \n");

                fprintf(fp, "\t push rbp \n");
                fprintf(fp, "\t mov rdi, outputBooleanFalse \n");
                fprintf(fp, "\t call printf \n");
                fprintf(fp, "\t pop rbp \n");

                fprintf(fp,"\t ; PRINT(false) ends \n\n");
                return;
            }

            ASTNode *siblingId = sibling->child;

            if(siblingId->gs == g_NUM) {
                fprintf(fp,"\t ; PRINT(%d) starts \n",siblingId->tkinfo->value.num);

                fprintf(fp, "\t push rbp \n");
                fprintf(fp, "\t mov rdi, outputInt \n");
                fprintf(fp, "\t mov rsi, %d \n", siblingId->tkinfo->value.num);
                fprintf(fp, "\t call printf \n");
                fprintf(fp, "\t pop rbp \n");

                fprintf(fp,"\t ; PRINT(%d) ends \n\n", siblingId->tkinfo->value.num);
                return;
            }

            if(siblingId->gs == g_RNUM) {
                fprintf(fp,"\t ; PRINT(%lf) starts \n",siblingId->tkinfo->value.rnum);

                fprintf(fp, "\t push rbp \n");
                fprintf(fp, "\t mov rdi, outputFloat \n");
                fprintf(fp, "\t mov rsi, __float64__(%s) \n", siblingId->tkinfo->lexeme);
                fprintf(fp, "\t movq xmm0, rsi \n");
                fprintf(fp, "\t mov rax, 1 \n");
                fprintf(fp, "\t call printf \n");
                fprintf(fp, "\t pop rbp \n");

                fprintf(fp,"\t ; PRINT(%lf) ends \n\n",siblingId->tkinfo->value.rnum);
                return;
            }

            varType idVarType = siblingId->stNode->info.var.vtype;
            symVarInfo idVar = siblingId->stNode->info.var;

            if(siblingId->next != NULL) {
                // Individual element of array is being accessed!

                ASTNode *idOrNum = siblingId->next;

                fprintf(fp, "\t push rbp \n");

                // The only registers that the called function is required to preserve (the calle-save registers) are:
                // rbp, rbx, r12, r13, r14, r15. All others are free to be changed by the called function.
                if(idVarType.baseType == g_BOOLEAN) {
                    getArrBoundsInExpReg(siblingId, fp);

                    if(idOrNum->gs == g_NUM) {
                        fprintf(fp, "\t mov %s, %d \n", expreg[2], idOrNum->tkinfo->value.num);
                    }
                    else {
                        varType idVarType = idOrNum->stNode->info.var.vtype;
                        symVarInfo idVar = idOrNum->stNode->info.var;

                        // DWORD because ID will (and should) always be of type INTEGER.
                        fprintf(fp, "\t movsx %s, DWORD [%s - %d] \n", expreg[2], baseRegister[idVar.isIOlistVar], scale * (idVarType.width + idVar.offset));
                    }

                    fprintf(fp, "\t cmp %s, %s \n", expreg[2], expreg[1]);
                    fprintf(fp, "\t jbe stat_valid_ub_%p \n", idOrNum); // UB satisfied
                    RUNTIME_EXIT_WITH_ERROR (fp, "OUT_OF_BOUNDS");

                    fprintf(fp, "stat_valid_ub_%p: \n", idOrNum);
                    fprintf(fp, "\t cmp %s, %s \n", expreg[2], expreg[0]); 
                    fprintf(fp, "\t jae stat_valid_lb_%p \n", idOrNum); // LB satisfied
                    RUNTIME_EXIT_WITH_ERROR (fp, "OUT_OF_BOUNDS");

                    fprintf(fp, "stat_valid_lb_%p: \n", idOrNum);

                    //prereq: lower bound in expreg[0], index in expreg[2]
                    //outcome: address of array element at idx in expreg[1]
                    getArrAddrAtIdx(siblingId, fp);

                    fprintf(fp, "\t cmp word[%s], 0 \n", expreg[1]);
                    fprintf(fp, "\t jz boolPrintFalse_%p \n", idOrNum);

                    fprintf(fp, " boolPrintTrue_%p: \n", idOrNum);
                    fprintf(fp, "\t mov rdi, outputBooleanTrue \n");
                    fprintf(fp, "\t jmp boolPrintEnd_%p \n", idOrNum);

                    fprintf(fp, " boolPrintFalse_%p: \n", idOrNum);
                    fprintf(fp, "\t mov rdi, outputBooleanFalse \n");

                    fprintf(fp, " boolPrintEnd_%p: \n", idOrNum);
                    fprintf(fp, "\t call printf \n");

                }
                else if(idVarType.baseType == g_INTEGER) {

                    fprintf(fp, "\t mov rdi, outputInt \n");

                    getArrBoundsInExpReg(siblingId, fp);

                    // Bound check done at compile time, but no harm!
                    if (idOrNum->gs == g_NUM)
                        fprintf(fp, "\t mov %s, %d \n", expreg[2], idOrNum->tkinfo->value.num);

                    // ID, we need to do bounds check!
                    else {
                        // Create scope for array index!
                        varType idVarType = idOrNum->stNode->info.var.vtype;
                        symVarInfo idVar = idOrNum->stNode->info.var;

                        fprintf(fp, "\t movsx %s, DWORD [%s - %d] \n", expreg[2], baseRegister[idVar.isIOlistVar], scale * (idVarType.width + idVar.offset));
                    }

                    fprintf(fp, "\t cmp %s, %s \n", expreg[2], expreg[1]);
                    fprintf(fp, "\t jbe stat_valid_ub_%p \n", idOrNum); // UB satisfied
                    RUNTIME_EXIT_WITH_ERROR (fp, "OUT_OF_BOUNDS");

                    fprintf(fp, "stat_valid_ub_%p: \n", idOrNum);
                    fprintf(fp, "\t cmp %s, %s \n", expreg[2], expreg[0]); 
                    fprintf(fp, "\t jae stat_valid_lb_%p \n", idOrNum); // LB satisfied
                    RUNTIME_EXIT_WITH_ERROR (fp, "OUT_OF_BOUNDS");

                    fprintf(fp, "stat_valid_lb_%p: \n", idOrNum);

                    //prereq: lower bound in expreg[0], index in expreg[2]
                    //outcome: address of array element at idx in expreg[1]
                    getArrAddrAtIdx(siblingId, fp);

                    fprintf(fp, "\t mov rsi, %s \n", expreg[1]);
                    fprintf(fp, "\t movsx rsi, DWORD[rsi] \n");
                    fprintf(fp, "\t call printf \n");
                }

                fprintf(fp, "\t pop rbp \n");
                return;
            }

            if(idVarType.vaType == VARIABLE) {
                fprintf(fp,"\t ; PRINT(%s: %s) starts\n",siblingId->tkinfo->lexeme, inverseMappingTable[siblingId->stNode->info.var.vtype.baseType]);

                // More registers need to me pushed to preserve
                // their values.
                // BEWARE: Number of pushes here should be odd.
                // push rbx

                fprintf(fp, "\t push rbp \n");

                if(idVarType.baseType == g_BOOLEAN) {
                    fprintf(fp, "\t cmp word[%s - %d], 0 \n", baseRegister[idVar.isIOlistVar], 2 * (idVarType.width + idVar.offset));
                    fprintf(fp, "\t jz boolPrintFalse_%p \n", siblingId);

                    fprintf(fp, " boolPrintTrue_%p: \n", siblingId);
                    fprintf(fp, "\t mov rdi, outputBooleanTrue \n");
                    fprintf(fp, "\t jmp boolPrintEnd_%p \n", siblingId);

                    fprintf(fp, " boolPrintFalse_%p: \n", siblingId);
                    fprintf(fp, "\t mov rdi, outputBooleanFalse \n");

                    fprintf(fp, " boolPrintEnd_%p: \n", siblingId);
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
                fprintf(fp,"\t ; PRINT(%s: %s) ends \n\n",siblingId->tkinfo->lexeme, inverseMappingTable[siblingId->stNode->info.var.vtype.baseType]);
                return;
            }

            // Print whole array!
            else  {

                fprintf(fp,"\t ; PRINT(array %s: %s) starts \n",siblingId->tkinfo->lexeme, inverseMappingTable[siblingId->stNode->info.var.vtype.baseType]);

                fprintf(fp, "\t push rbp \n");
    
                // The only registers that the called function is required to preserve (the calle-save registers) are:
                // rbp, rbx, r12, r13, r14, r15. All others are free to be changed by the called function.
                if(idVarType.baseType == g_INTEGER) {

                    fprintf(fp, "\t mov rdi, output \n");
                    fprintf(fp, "\t call printf \n");

                    fprintf(fp, "\t mov rsi, %s \n", baseRegister[idVar.isIOlistVar]); // isIOlistVar may be 0 or 1
                    fprintf(fp, "\t sub rsi, %d \n", scale * (1 + idVar.offset));
                    fprintf(fp, "\t movsx rsi, word[rsi] \n"); // move base val
                    fprintf(fp, "\t add rsi, [stack_top] \n"); // address of first elem!

                    getArrBoundsInExpReg(siblingId, fp);

                    fprintf(fp, "\t mov r12, %s \n", expreg[0]); // lb
                    fprintf(fp, "\t mov r13, %s \n", expreg[1]); // ub

                    fprintf(fp, "\t mov rdi, intHolder \n");

                    fprintf(fp, "print_arr_%p: \n", siblingId);
                    
                    fprintf(fp, "\t push rsi \n");
                    fprintf(fp, "\t push rdi \n");
                    fprintf(fp, "\t movsx rsi, DWORD[rsi] \n");
                    fprintf(fp, "\t call printf \n");
                    fprintf(fp, "\t pop rdi \n");
                    fprintf(fp, "\t pop rsi \n");

                    fprintf(fp, "\t cmp r12, r13 \n");
                    fprintf(fp, "\t jz print_arr_exit_%p \n", siblingId);

                    fprintf(fp, "\t inc r12 \n");
                    fprintf(fp, "\t sub rsi, 4 \n"); // address of n-th elem
                    fprintf(fp, "\t jmp print_arr_%p \n", siblingId);

                    fprintf(fp, " print_arr_exit_%p: \n", siblingId);
                    fprintf(fp, "\t mov rdi, newLine \n");
                    fprintf(fp, "\t call printf \n");
                }
                else if(idVarType.baseType == g_BOOLEAN) {

                    fprintf(fp, "\t mov rdi, output \n");
                    fprintf(fp, "\t call printf \n");

                    fprintf(fp, "\t mov rsi, %s \n", baseRegister[idVar.isIOlistVar]); // isIOlistVar may be 0 or 1
                    fprintf(fp, "\t sub rsi, %d \n", 2 * (1 + idVar.offset));
                    fprintf(fp, "\t movsx rsi, word[rsi] \n"); // move base val
                    fprintf(fp, "\t add rsi, [stack_top] \n"); // address of first elem!

                    getArrBoundsInExpReg(siblingId, fp);

                    fprintf(fp, "\t mov r12, %s \n", expreg[0]); // lb
                    fprintf(fp, "\t mov r13, %s \n", expreg[1]); // ub

                    fprintf(fp, " print_b_arr_%p: \n", siblingId);
                    fprintf(fp, "\t cmp r12, r13 \n");
                    fprintf(fp, "\t ja print_b_array_exit_%p \n", siblingId);

                    fprintf(fp, "\t cmp word[rsi], 0 \n");
                    fprintf(fp, "\t je print_b_arr_f_%p \n", siblingId);

                    fprintf(fp, "\t mov rdi, booleanTrue \n");
                    fprintf(fp, "\t jmp print_bool_%p \n", siblingId);

                    fprintf(fp, " print_b_arr_f_%p: \n", siblingId);
                    fprintf(fp, "\t mov rdi, booleanFalse \n");

                    fprintf(fp, " print_bool_%p: \n", siblingId);
                    // call to printf can modify rdi and rsi. Therefore, save them.
                    fprintf(fp, "\t push rsi \n");
                    fprintf(fp, "\t push rdi \n");
                    fprintf(fp, "\t call printf \n");
                    fprintf(fp, "\t pop rdi \n");
                    fprintf(fp, "\t pop rsi \n");

                    fprintf(fp, "\t inc r12 \n");
                    fprintf(fp, "\t sub rsi, 2 \n"); // 2*width of boolean datatype
                    fprintf(fp, "\t jmp print_b_arr_%p \n", siblingId);

                    fprintf(fp, " print_b_array_exit_%p: \n", siblingId);
                    fprintf(fp, "\t mov rdi, newLine \n");
                    fprintf(fp, "\t call printf \n");
                }

                fprintf(fp, "\t pop rbp \n");

                fprintf(fp,"\t ; PRINT(array %s: %s) ends \n\n",siblingId->tkinfo->lexeme, inverseMappingTable[siblingId->stNode->info.var.vtype.baseType]);

            }

            return;
        }

        case g_declareStmt:
        {
            ASTNode *idList = root->child;

            ASTNode *id = idList->child;

            while(id != NULL) {
                symVarInfo idVar = id->stNode->info.var;

                if(idVar.vtype.vaType == VARIABLE)
                    return;
                
                else if(idVar.vtype.vaType == STAT_ARR) {
                    fprintf(fp, "\t mov rsi, %s \n", baseRegister[idVar.isIOlistVar]); // isIOlistVar must be 0!
                    fprintf(fp, "\t mov rdi, rsi \n");
                    fprintf(fp, "\t sub rdi, %d \n", scale * (idVar.offset + 1)); // Base address
                    fprintf(fp, "\t sub rsi, %d \n", scale * (idVar.offset + 1 + getSizeByType(idVar.vtype.baseType))); // First elem
                    fprintf(fp, "\t sub rsi, [stack_top] \n"); // Find location relative to top of stack!
                    fprintf(fp, "\t mov word[rdi], si \n"); // only 1 location = 2B available!
                }

                else {
                    fprintf(fp, "\t mov rdi, %s \n", baseRegister[idVar.isIOlistVar]); // isIOlistVar must be 0!
                    fprintf(fp, "\t sub rdi, %d \n", scale * (idVar.offset + 1)); // Location for Base address
                    fprintf(fp, "\t mov rsi, rsp \n");
                    fprintf(fp, "\t sub rsi, %d \n", scale * getSizeByType(idVar.vtype.baseType)); // First elem
                    fprintf(fp, "\t sub rsi, [stack_top] \n"); // Find location relative to top of stack!
                    fprintf(fp, "\t mov word[rdi], si \n"); // stack position where dynamic array begins!

                    getArrBoundsInExpReg(id, fp);
                    fprintf(fp, "\t sub %s, %s \n", expreg[1], expreg[0]); // UB - LB 
                    fprintf(fp, "\t jae dyn_arr_valid_%p \n", id);

                    fprintf(fp, "\t push rbx \n");
                    RUNTIME_EXIT_WITH_ERROR(fp, "UPPER_BOUND_SMALL");
                    fprintf(fp, "\t pop rbx \n");

                    fprintf(fp, " dyn_arr_valid_%p: \n", id);
                    fprintf(fp, "\t inc %s \n", expreg[1]);

                    if(idVar.vtype.baseType == g_BOOLEAN)
                        fprintf(fp, "\t shl %s, %d \n", expreg[1], 1);
                    else if(idVar.vtype.baseType == g_INTEGER)
                        fprintf(fp, "\t shl %s, %d \n", expreg[1], 2);
                    else
                        fprintf(fp, "\t shl %s, %d \n", expreg[1], 3);

                    fprintf(fp, "\t mov r12, %s \n", expreg[1]);
                    fprintf(fp, "\t and r12, 15 \n");
                    fprintf(fp, "\t cmp r12, 0 \n");
                    fprintf(fp, "\t je dyn_arr_aligned_%p \n", id);
                    
                    fprintf(fp, "\t mov r13, 16 \n");
                    fprintf(fp, "\t sub r13, r12 \n");
                    fprintf(fp, "\t add %s, r13 \n", expreg[1]);

                    fprintf(fp, " dyn_arr_aligned_%p: \n", id);
                    fprintf(fp, "\t sub rsp, %s \n", expreg[1]);
                }


                id = id->next;
                fprintf(fp, "\t ;array declaration done \n\n");

            }

            return;
        }

        case g_assignmentStmt:
            fprintf(fp,"\t ; Expression generation starts\n");
            fprintf(fp,"\t mov [preExpRSP], rsp \n");   //store the pre expression rsp into preExpRSP
            genExpr(root,fp,true,0);
            fprintf(fp,"\t ; Expression generation ends \n\n");
            return;

        case g_conditionalStmt:{
            ASTNode *idNode = root->child; // on ID
            symVarInfo vi = idNode->stNode->info.var;
            fprintf(fp,"\t ; switch(%s: %s) starts\n", idNode->tkinfo->lexeme, inverseMappingTable[vi.vtype.baseType]);

            fprintf(fp,"\t mov rsi, %s \n", baseRegister[vi.isIOlistVar]);
            fprintf(fp,"\t sub rsi, %d \n", 2*(vi.offset + vi.vtype.width));
            // rsi now points to where the data will be extracted from
            char memToRegStr[2][6] = {'\0', '\0'};
            getAptReg(memToRegStr, 8, vi.vtype.width);
            fprintf(fp, "\t mov %s, %s[ rsi ] \n", memToRegStr[0], memToRegStr[1]);

            switch(idNode->stNode->info.var.vtype.baseType){
                case g_INTEGER: {
                    ASTNode *valList = idNode->next->child->child; // On NUM
                    ASTNode *ptr = valList;
                    fprintf(fp, "\t ; comparisons start for cases \n");
                    while(ptr!=NULL){
                        fprintf(fp, "\t cmp %s, %d \n", memToRegStr[0], ptr->tkinfo->value.num);
                        fprintf(fp, "\t jz case_NUM_ptr_%p \n",ptr);
                        ptr = ptr->next;
                    }
                    fprintf(fp, "\t ; comparisons end for cases \n");
                    fprintf(fp, "\t ; jump to default \n");

                    fprintf(fp, "\t jmp DEFAULT_ptr_%p \n",valList->parent->next); // default always there in int switch

                    fprintf(fp, "\t ; case definitions begin \n");
                    ptr = valList;
                    while(ptr != NULL){
                        fprintf(fp, " case_NUM_ptr_%p: \n",ptr);
                        generateCode(ptr->child, symT, fp);
                        fprintf(fp, "\t jmp BREAK_ptr_%p \n",root); // BREAK out of switch pointer is the conditionalStmt pointer
                        ptr = ptr->next;
                    }
                    fprintf(fp, "\t ; case definitions end \n");
                    fprintf(fp, "\t ; default begin \n");

                    fprintf(fp, " DEFAULT_ptr_%p: \n",valList->parent->next);
                    generateCode(valList->parent->next->child, symT, fp);

                    fprintf(fp, "\t ; default ends \n\n");
                    fprintf(fp, " BREAK_ptr_%p: \n",root); // BREAK out of switch pointer is the conditionalStmt pointer
                }
                break;
                case g_BOOLEAN:{
                    ASTNode *valList = idNode->next->child->child; // On TRUE or FALSE
                    ASTNode *ptr = valList;
                    fprintf(fp, "\t ; comparisons start for cases \n");
                    while(ptr!=NULL){
                        // this loop runs only two times: TRUE and FALSE
                        fprintf(fp, "\t cmp %s, %d \n", memToRegStr[0], ptr->gs == FALSE ? 0 : 1);
                        fprintf(fp, "\t jz case_%s_ptr_%p \n",inverseMappingTable[ptr->gs], ptr);
                        ptr = ptr->next;
                    }
                    fprintf(fp, "\t ; comparisons end for cases \n");
                    // In a semantically correct code, there won't be a default and exactly one case for TRUE and FALSE.
                    // Thus we only need exactly two comparisons made above (in loop). One of them is sure to match.

                    fprintf(fp, "\t ; case definitions begin \n");
                    ptr = valList;
                    while(ptr != NULL){
                        // this loop runs two times
                        fprintf(fp, " case_%s_ptr_%p: \n",inverseMappingTable[ptr->gs], ptr);
                        generateCode(ptr->child, symT, fp);
                        fprintf(fp, "\t jmp BREAK_ptr_%p \n",root); // BREAK out of switch pointer is the conditionalStmt pointer
                        ptr = ptr->next;
                    }
                    fprintf(fp, "\t ; case definitions end \n");

                    fprintf(fp, " BREAK_ptr_%p: \n",root); // BREAK out of switch pointer is the conditionalStmt pointer
                }
                    break;
                default:
                    printf("generateCode: Mistake in semantic analyser. Got invalid switch var data type.\n");
            }
            fprintf(fp,"\t ; switch(%s: %s) ends \n\n", idNode->tkinfo->lexeme, inverseMappingTable[vi.vtype.baseType]);

        }
        return;

        default:
            printf("Default : %s \n", inverseMappingTable[gs]);
    }

}

void getAptReg(char memToRegStr[][6], int regno, int width){
//R0  R1  R2  R3  R4  R5  R6  R7  R8  R9  R10  R11  R12  R13  R14  R15
//RAX RCX RDX RBX RSP RBP RSI RDI
    switch(width){
        case 1:
            sprintf(memToRegStr[0], "r%dw", regno); // 2 bytes
            sprintf(memToRegStr[1],"word");
            break;
        case 2:
            sprintf(memToRegStr[0],"r%dd", regno); // 4 bytes
            sprintf(memToRegStr[1],"dword");
            break;
        case 4:
            sprintf(memToRegStr[0], "r%d", regno); // 8 bytes
            sprintf(memToRegStr[1],"qword");
            break;
        default:
            printf("getAptReg: Got invalid width.\n");
    }
}