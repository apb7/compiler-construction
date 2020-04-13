#include <stdio.h>
#include <stdlib.h>
#include "symbolHash.h"
#include "symbolTableDef.h"
#include "symbolTable.h"

extern char *inverseMappingTable[];

void printVarEntry(symTableNode *stNode, int sno, FILE *fp){
    fprintf(fp, "%-4d %-20s", sno, stNode->lexeme);
    char *va, *ty;
    if(stNode->info.var.vtype.vaType == VARIABLE)
        va = "Variable";
    else
        va = "Array";
    ty = inverseMappingTable[stNode->info.var.vtype.baseType];
    fprintf(fp," %-10s %-10s",va,ty);
    varType vt =  stNode->info.var.vtype;
    switch(stNode->info.var.vtype.vaType){
        case VARIABLE:
            fprintf(fp," %-25s %-25s","----","----");
            break;
        case STAT_ARR:
            fprintf(fp," %-25d %-25d",vt.si.vt_num,vt.ei.vt_num);
            break;
        case DYN_L_ARR:
            if(vt.si.vt_id != NULL)
                fprintf(fp," %-25s",vt.si.vt_id->lexeme);
            else
                fprintf(fp," %-25s","NULL");
            fprintf(fp," %-25d",vt.ei.vt_num);
            break;
        case DYN_R_ARR:
            fprintf(fp," %-25d",vt.si.vt_num);
            if(vt.ei.vt_id != NULL)
                fprintf(fp," %-25s",vt.ei.vt_id->lexeme);
            else
                fprintf(fp," %-25s","NULL");
            break;
        case DYN_ARR:
            if(vt.si.vt_id != NULL)
                fprintf(fp," %-25s",vt.si.vt_id->lexeme);
            else
                fprintf(fp," %-25s","NULL");
            if(vt.ei.vt_id != NULL)
                fprintf(fp," %-25s",vt.ei.vt_id->lexeme);
            else
                fprintf(fp," %-25s","NULL");
            break;
    }
    fprintf(fp, " %-5d %-6d\n", stNode->info.var.lno, stNode->info.var.offset);
}

void printSymbolTable(symbolTable* st, FILE *fp){
    // prints the whole SymbolTable Structure by calling printCurrSymTable
    // TODO: add a call to printCurrSymTable(..) appropriately
    if(st == NULL)
        return;
    symTableNode *currSTN = NULL;
    fprintf(fp,"################################# SYMBOL TABLE #################################\n\n");
    fprintf(fp,"################################# FUNCTION TABLE #################################\n\n");
    fprintf(fp,"%-4s%-20s%-7s%-6s\n", "SNO", "Func-Name", "Status", "Line");
    int sno = 1;
    for(int i=0; i<SYMBOL_TABLE_SIZE; i++){
        currSTN = (st->tb)[i];
        while(currSTN != NULL){
            fprintf(fp, "%-4d%-20s%-7d%-6d\n", sno, currSTN->lexeme, currSTN->info.func.status, currSTN->info.func.lno);
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
            fprintf(fp,"################################# I/O VARS #################################\n\n");
            fprintf(fp,"%-4s %-20s %-10s %-10s %-25s %-25s %-5s %-6s\n","SNO","LEXEME","VAR/ARR","TYPE","LB","UB","LINE","OFFSET");
            symTableNode *iohead = currSTN->info.func.inpPListHead;
            while(iohead != NULL){
                printVarEntry(iohead,sno,fp);
                sno++;
                iohead = iohead->next;
            }
            iohead = currSTN->info.func.outPListHead;
            while(iohead != NULL){
                printVarEntry(iohead,sno,fp);
                sno++;
                iohead = iohead->next;
            }
            fprintf(fp,"\n");
            printCurrSymTable(currSTN->info.func.st, 0, fp);
            currSTN = currSTN->next;
        }
    }
    fprintf(fp,"##################################### ~ ** ~ #####################################\n\n");
}

void printCurrSymTable(symbolTable *st,int level, FILE *fp){
    fprintf(fp,"################################# Level %d #################################\n\n",level);
    fprintf(fp,"%-4s %-20s %-10s %-10s %-25s %-25s %-5s %-6s\n","SNO","LEXEME","VAR/ARR","TYPE","LB","UB","LINE","OFFSET");
    int sno = 1;
    for(int i=0; i<SYMBOL_TABLE_SIZE; i++){
        symTableNode *currSTN = (st->tb)[i];
        while(currSTN != NULL){
            printVarEntry(currSTN,sno,fp);
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
