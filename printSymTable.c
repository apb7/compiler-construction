#include <stdio.h>
#include <stdlib.h>
#include "symbolHash.h"
#include "symbolTableDef.h"
#include "symbolTable.h"
#include <string.h>

extern char *inverseMappingTable[];

void printVarEntry(symTableNode *stNode, int sno, char *funcName, ASTNode *startNode, int level, FILE *fp) {
//    "SNO","VAR-NAME","SCOPE: FUNC-NAME", "SCOPE: LINES","WIDTH","IS ARRAY?","STAT/DYN","RANGE", "TYPE", "OFFSET", "LEVEL"
    //fprintf(fp,"%-4s%-25s%-25s%-11s%-6s%-10s%-9s%-25s%-10s%-7s%-7s","SNO","VAR-NAME","SCOPE: FUNC-NAME", "SCOPE: LINES","WIDTH","IS ARRAY?","STAT/DYN","RANGE", "TYPE", "OFFSET", "LEVEL");
    fprintf(fp, "%-4d%-20s%-25s", sno, stNode->lexeme,funcName);
    char scl[30];
    sprintf(scl,"[%d,%d]",startNode->start_line_no,startNode->end_line_no);
    if(stNode->info.var.vtype.vaType == VARIABLE || stNode->info.var.vtype.vaType == STAT_ARR)
        fprintf(fp,"%-15s%-8d",scl,stNode->info.var.vtype.width);
    else
        fprintf(fp,"%-15s%-8s",scl,"Runtime");
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
    fprintf(fp,"%-20s%-10s%-7d%-7d\n",rng,ty,stNode->info.var.offset,level);
}

void printSymbolTable(symbolTable* st, FILE *fp){
    // prints the whole SymbolTable Structure by calling printCurrSymTable
    // TODO: add a call to printCurrSymTable(..) appropriately
    if(st == NULL)
        return;
    symTableNode *currSTN = NULL;
    fprintf(fp,"################################# SYMBOL TABLE #################################\n\n");
    fprintf(fp,"################################# FUNCTION TABLE #################################\n\n");
    fprintf(fp,"%-4s%-20s%-6s\n", "SNO", "FUNCTION-NAME", "LINE");
    int sno = 1;
    for(int i=0; i<SYMBOL_TABLE_SIZE; i++){
        currSTN = (st->tb)[i];
        while(currSTN != NULL){
            fprintf(fp, "%-4d%-20s%-6d\n", sno, currSTN->lexeme, currSTN->info.func.lno);
            currSTN = currSTN->next;
            sno++;
        }
    }
    fprintf(fp,"\n");
    for(int i=0; i<SYMBOL_TABLE_SIZE; i++){
        currSTN = (st->tb)[i];
        while(currSTN != NULL){
            fprintf(fp, "################################# MODULE '%s' #################################\n\n", currSTN->lexeme);
            int sno = 1;
            fprintf(fp,"############################ I/O VARS (Level 0) ############################\n\n");
            fprintf(fp,"%-4s%-20s%-25s%-15s%-8s%-10s%-11s%-20s%-10s%-7s%-7s\n","SNO","VAR-NAME","SCOPE: FUNC-NAME", "SCOPE: LINES","WIDTH","IS ARRAY?","STAT/DYN","RANGE", "TYPE", "OFFSET", "LEVEL");
//            fprintf(fp,"%-4s %-20s %-10s %-10s %-25s %-25s %-5s %-6s\n","SNO","VAR-NAME","VAR/ARR","TYPE","LB","UB","LINE","OFFSET");
            symTableNode *iohead = currSTN->info.func.inpPListHead;
            while(iohead != NULL){
                printVarEntry(iohead, sno, currSTN->lexeme, currSTN->info.func.st->startNode, 0, fp);
                sno++;
                iohead = iohead->next;
            }
            iohead = currSTN->info.func.outPListHead;
            while(iohead != NULL){
                printVarEntry(iohead, sno, currSTN->lexeme, currSTN->info.func.st->startNode, 0, fp);
                sno++;
                iohead = iohead->next;
            }
            fprintf(fp,"\n");
            printCurrSymTable(currSTN->info.func.st, 1, fp);
            currSTN = currSTN->next;
        }
    }
    fprintf(fp,"##################################### ~ ** ~ #####################################\n\n");
}

void printCurrSymTable(symbolTable *st,int level, FILE *fp){
    fprintf(fp,"################################# Level %d #################################\n\n",level);
    fprintf(fp,"%-4s%-20s%-25s%-15s%-8s%-10s%-11s%-20s%-10s%-7s%-7s\n","SNO","VAR-NAME","SCOPE:FUNC-NAME", "SCOPE:LINES","WIDTH","IS ARRAY?","STAT/DYN","RANGE", "TYPE", "OFFSET", "LEVEL");
//    fprintf(fp,"%-4s %-20s %-10s %-10s %-25s %-25s %-5s %-6s\n","SNO","LEXEME","VAR/ARR","TYPE","LB","UB","LINE","OFFSET");
    int sno = 1;
    for(int i=0; i<SYMBOL_TABLE_SIZE; i++){
        symTableNode *currSTN = (st->tb)[i];
        while(currSTN != NULL){
            printVarEntry(currSTN, sno, st->funcName, st->startNode, level, fp);
            currSTN = currSTN->next;
            sno++;
        }
    }
    fprintf(fp,"\n");
    symbolTable *childst = st->headChild;
    while(childst != NULL){
        printCurrSymTable(childst,level+1,fp);
        childst = childst->next;
    }
}
