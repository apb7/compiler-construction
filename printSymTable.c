#include <stdio.h>
#include <stdlib.h>
#include "symbolHash.h"
#include "symbolTableDef.h"
#include "symbolTable.h"
#include "util.h"
#include <string.h>

extern char *inverseMappingTable[];
int sno;

void printVarEntry(symTableNode *stNode, char *funcName, ASTNode *startNode, int level, FILE *fp) {
    fprintf(fp, "%-6d%-20s%-25s", sno, stNode->lexeme,funcName);
    char scl[30];
    sprintf(scl,"[%d,%d]",startNode->start_line_no,startNode->end_line_no);
    // earlier width was printed only for VARIABLE and STAT_ARR, now we print for all
    fprintf(fp,"%-15s%-8d",scl,stNode->info.var.vtype.width);

    char *va, *sd, *ty;
    if(stNode->info.var.vtype.vaType == VARIABLE){
        va = "No";
        sd = "---";
    }
    else{
        va = "Yes";
        if(stNode->info.var.vtype.vaType == STAT_ARR)
            sd = "Static";
        else
            sd = "Dynamic";
    }
    fprintf(fp,"%-10s%-11s",va,sd);
    ty = inverseMappingTable[stNode->info.var.vtype.baseType];
    varType vt =  stNode->info.var.vtype;
    char rng[26];
    switch(stNode->info.var.vtype.vaType){
        case VARIABLE:
            sprintf(rng,"%s","---");
            break;
        case STAT_ARR:
            sprintf(rng,"[%d,%d]",vt.si.vt_num,vt.ei.vt_num);
            break;
        case DYN_L_ARR:
            if(vt.si.vt_id != NULL)
                sprintf(rng,"[%s,%d]",vt.si.vt_id->lexeme,vt.ei.vt_num);
            else
                sprintf(rng,"[%s,%d]","NULL",vt.ei.vt_num);
            break;
        case DYN_R_ARR:
            if(vt.ei.vt_id != NULL)
                sprintf(rng,"[%d,%s]",vt.si.vt_num,vt.ei.vt_id->lexeme);
            else
                sprintf(rng,"[%d,%s]",vt.si.vt_num,"NULL");
            break;
        case DYN_ARR:
            if(vt.si.vt_id != NULL)
                sprintf(rng,"[%s,",vt.si.vt_id->lexeme);
            else
                sprintf(rng,"[%s,","NULL");
            if(vt.ei.vt_id != NULL)
                sprintf(rng+strlen(rng),"%s]",vt.ei.vt_id->lexeme);
            else
                sprintf(rng+strlen(rng),"%s]","NULL");
            break;
    }
    // earlier offset was printed only for VARIABLE and STAT_ARR, now we print for all
    fprintf(fp,"%-20s%-10s%-7d%-7d\n",rng,ty,stNode->info.var.offset,level);

}

void printSymbolTable(symbolTable* st, char *fname){
    if(st == NULL)
        return;
    FILE *fp;
    fname == NULL ? fp = stdout : (fp = fopen(fname,"w"));
    if(fp == NULL) {
        fprintf(stderr,"ERROR: Failed to open %s", fname);
        return;
    }
    symTableNode *currSTN = NULL;
    fprintf(fp,"\n\n####################################################################### SYMBOL TABLE #######################################################################\n\n");

    fprintf(fp,"%-6s%-20s%-25s%-15s%-8s%-10s%-11s%-20s%-10s%-7s%-7s\n","SNO","VAR-NAME","SCOPE: FUNC-NAME", "SCOPE: LINES","WIDTH","IS ARRAY?","STAT/DYN","RANGE", "TYPE", "OFFSET", "LEVEL");

    fprintf(fp,"\n");
    for(int i=0; i<SYMBOL_TABLE_SIZE; i++){
        currSTN = (st->tb)[i];
        while(currSTN != NULL){
            fprintf(fp,"[DEBUG] ACTIVATION RECORD SIZE = %d\n",currSTN->info.func.arSize);
            sno = 1;
            symTableNode *iohead = currSTN->info.func.inpPListHead;
            while(iohead != NULL){
                printVarEntry(iohead, currSTN->lexeme, currSTN->info.func.st->startNode, 0, fp);
                sno++;
                iohead = iohead->next;
            }
            iohead = currSTN->info.func.outPListHead;
            while(iohead != NULL){
                printVarEntry(iohead, currSTN->lexeme, currSTN->info.func.st->startNode, 0, fp);
                sno++;
                iohead = iohead->next;
            }
            printCurrSymTable(currSTN->info.func.st, 1, fp);
            currSTN = currSTN->next;
            fprintf(fp,"\n");
        }
    }
    fprintf(fp,"########################################################################## ~ ** ~ ##########################################################################\n\n");
    fprintf(fp,"NOTE: The variables which are undeclared and used as array indices are stated as NULL under RANGE header.\n");
    fcloseSafe(fp);
}

void printCurrSymTable(symbolTable *st, int level, FILE *fp){
    for(int i=0; i<SYMBOL_TABLE_SIZE; i++){
        symTableNode *currSTN = (st->tb)[i];
        while(currSTN != NULL){
            printVarEntry(currSTN, st->funcName, st->startNode, level, fp);
            currSTN = currSTN->next;
            sno++;
        }
    }
    symbolTable *childst = st->headChild;
    while(childst != NULL){
        printCurrSymTable(childst,level+1,fp);
        childst = childst->next;
    }
}

void printARSizes(symbolTable *funcTable, char *fname){
    if(funcTable == NULL)
        return;
    FILE *fp;
    fname == NULL ? fp = stdout : (fp = fopen(fname,"w"));
    if(fp == NULL) {
        fprintf(stderr,"ERROR: Failed to open %s", fname);
        return;
    }

    // implement here
    // TODO: implement this method printARSizes(..) Only need to print the function name and scopeSize field of the symbolTable *st inside symFuncInfo of each function


    fprintf(fp, "\n NOTE: The Activation record size also includes width of input and output parameters of each function.\n");
    fcloseSafe(fp);
}

void printArrayInfo(symbolTable *funcTable, char *fname){
    if(funcTable == NULL)
        return;
    FILE *fp;
    fname == NULL ? fp = stdout : (fp = fopen(fname,"w"));
    if(fp == NULL) {
        fprintf(stderr,"ERROR: Failed to open %s", fname);
        return;
    }
    // implement here
    //TODO: same as printSymbolTable, just print the arrays encountered in same order as they arrive

    fcloseSafe(fp);
}