#include <stdio.h>
#include <stdlib.h>
#include "symbolHash.h"
#include "symbolTableDef.h"
#include "symbolTable.h"
#include "util.h"
#include <string.h>

extern char *inverseMappingTable[];
int sno;

void printVarEntry(symTableNode *stNode, char *funcName, ASTNode *startNode, bool onlyArrayInfo, int level, FILE *fp) {
    if(!onlyArrayInfo) {
        fprintf(fp, "%-6d%-20s%-25s", sno, stNode->lexeme, funcName);
        char scl[30];
        sprintf(scl, "[%d,%d]", startNode->start_line_no, startNode->end_line_no);
        // earlier width was printed only for VARIABLE and STAT_ARR, now we print for all
        fprintf(fp, "%-15s%-8d", scl, stNode->info.var.vtype.width);

        char *va, *sd, *ty;
        if (stNode->info.var.vtype.vaType == VARIABLE) {
            va = "No";
            sd = "---";
        } else {
            va = "Yes";
            if (stNode->info.var.vtype.vaType == STAT_ARR)
                sd = "Static";
            else
                sd = "Dynamic";
        }
        fprintf(fp, "%-10s%-18s", va, sd);
        ty = inverseMappingTable[stNode->info.var.vtype.baseType];
        varType vt = stNode->info.var.vtype;
        char rng[63];
        switch (stNode->info.var.vtype.vaType) {
            case VARIABLE:
                sprintf(rng, "%s", "---");
                break;
            case STAT_ARR:
                sprintf(rng, "[%d,%d]", vt.si.vt_num, vt.ei.vt_num);
                break;
            case DYN_L_ARR:
                if (vt.si.vt_id != NULL)
                    sprintf(rng, "[%s,%d]", vt.si.vt_id->lexeme, vt.ei.vt_num);
                else
                    sprintf(rng, "[%s,%d]", "NULL", vt.ei.vt_num);
                break;
            case DYN_R_ARR:
                if (vt.ei.vt_id != NULL)
                    sprintf(rng, "[%d,%s]", vt.si.vt_num, vt.ei.vt_id->lexeme);
                else
                    sprintf(rng, "[%d,%s]", vt.si.vt_num, "NULL");
                break;
            case DYN_ARR:
                if (vt.si.vt_id != NULL)
                    sprintf(rng, "[%s,", vt.si.vt_id->lexeme);
                else
                    sprintf(rng, "[%s,", "NULL");
                if (vt.ei.vt_id != NULL)
                    sprintf(rng + strlen(rng), "%s]", vt.ei.vt_id->lexeme);
                else
                    sprintf(rng + strlen(rng), "%s]", "NULL");
                break;
        }
        // earlier offset was printed only for VARIABLE and STAT_ARR, now we print for all
        fprintf(fp, "%-20s%-10s%-7d%-7d\n", rng, ty, stNode->info.var.offset, level);
    }
    else{
        if(stNode->info.var.vtype.vaType == VARIABLE){
            return;
        }
        char scl[30];
        sprintf(scl, "[%d,%d]", startNode->start_line_no, startNode->end_line_no);
        fprintf(fp, "%-25s%-15s%-20s",funcName, scl,stNode->lexeme);

        char *sd, *ty;
        if (stNode->info.var.vtype.vaType == STAT_ARR)
            sd = "Static";
        else
            sd = "Dynamic";

        ty = inverseMappingTable[stNode->info.var.vtype.baseType];
        varType vt = stNode->info.var.vtype;
        char rng[50];
        switch (stNode->info.var.vtype.vaType) {
            case VARIABLE:
                sprintf(rng, "%s", "---");
                break;
            case STAT_ARR:
                sprintf(rng, "[%d,%d]", vt.si.vt_num, vt.ei.vt_num);
                break;
            case DYN_L_ARR:
                if (vt.si.vt_id != NULL)
                    sprintf(rng, "[%s,%d]", vt.si.vt_id->lexeme, vt.ei.vt_num);
                else
                    sprintf(rng, "[%s,%d]", "NULL", vt.ei.vt_num);
                break;
            case DYN_R_ARR:
                if (vt.ei.vt_id != NULL)
                    sprintf(rng, "[%d,%s]", vt.si.vt_num, vt.ei.vt_id->lexeme);
                else
                    sprintf(rng, "[%d,%s]", vt.si.vt_num, "NULL");
                break;
            case DYN_ARR:
                if (vt.si.vt_id != NULL)
                    sprintf(rng, "[%s,", vt.si.vt_id->lexeme);
                else
                    sprintf(rng, "[%s,", "NULL");
                if (vt.ei.vt_id != NULL)
                    sprintf(rng + strlen(rng), "%s]", vt.ei.vt_id->lexeme);
                else
                    sprintf(rng + strlen(rng), "%s]", "NULL");
                break;
        }
        fprintf(fp, "%-18s%-20s%-10s%-8d\n", sd, rng, ty, stNode->info.var.vtype.width);
    }
}

void printSymbolTable(symbolTable* st, bool onlyArrayInfo, char *fname){
    if(st == NULL)
        return;
    FILE *fp;
    fname == NULL ? fp = stdout : (fp = fopen(fname,"w"));
    if(fp == NULL) {
        fprintf(stderr,"ERROR: Failed to open %s", fname);
        return;
    }
    symTableNode *currSTN = NULL;
    if(!onlyArrayInfo) {
        fprintf(fp,
                "\n\n####################################################################### SYMBOL TABLE "
                "#######################################################################\n\n");

        fprintf(fp, "%-6s%-20s%-25s%-15s%-8s%-10s%-18s%-20s%-10s%-7s%-7s\n", "S.NO.", "VARIABLE-NAME",
                "SCOPE: MODULE-NAME", "SCOPE: LINES", "WIDTH", "IS ARRAY?", "STATIC/DYNAMIC", "RANGE", "TYPE", "OFFSET",
                "LEVEL");
    }
    else {
    fprintf(fp,
            "\n\n######################################################################## ARRAY INFO "
            "########################################################################\n\n");

    fprintf(fp, "%-25s%-15s%-20s%-18s%-20s%-10s%-8s\n", "SCOPE: MODULE-NAME", "SCOPE: LINES", "ARRAY-NAME",
            "STATIC/DYNAMIC", "RANGE", "TYPE", "WIDTH");
    }

    for(int i=0; i<SYMBOL_TABLE_SIZE; i++){
        currSTN = (st->tb)[i];
        while(currSTN != NULL){
            sno = 1;
            symTableNode *iohead = currSTN->info.func.inpPListHead;
            while (iohead != NULL) {
                printVarEntry(iohead, currSTN->lexeme, currSTN->info.func.st->startNode, onlyArrayInfo,0, fp);
                sno++;
                iohead = iohead->next;
            }
            iohead = currSTN->info.func.outPListHead;
            while (iohead != NULL) {
                printVarEntry(iohead, currSTN->lexeme, currSTN->info.func.st->startNode, onlyArrayInfo,0, fp);
                sno++;
                iohead = iohead->next;
            }

            printCurrSymTable(currSTN->info.func.st, onlyArrayInfo, 1, fp);
            currSTN = currSTN->next;
            if(!onlyArrayInfo)
                fprintf(fp, "\n");
        }
    }
    if(onlyArrayInfo)
        fprintf(fp, "\n");
    fprintf(fp,"########################################################################## ~ ** ~ ##########################################################################\n\n");
    fprintf(fp,"NOTE: The variables which are undeclared and used as array ranges are stated as NULL under RANGE header.\n"
               "\t  The dynamic ranges of arrays in input list are printed as '----' because they are placeholders and not associated with any variable.\n");
    fcloseSafe(fp);
}

void printCurrSymTable(symbolTable *st, bool onlyArrayInfo, int level, FILE *fp){
    if(st == NULL)
        return;
    for(int i=0; i<SYMBOL_TABLE_SIZE; i++){
        symTableNode *currSTN = (st->tb)[i];
        while(currSTN != NULL){
            printVarEntry(currSTN, st->funcName, st->startNode,onlyArrayInfo, level, fp);
            currSTN = currSTN->next;
            sno++;
        }
    }
    symbolTable *childst = st->headChild;
    while(childst != NULL){
        printCurrSymTable(childst,onlyArrayInfo,level + 1,fp);
        childst = childst->next;
    }
}

/*
 * ----------------------------------------- Helper functions for driver ------------------------------------------------
 */
void printARSizes(symbolTable *funcTable, char *fname){
    if(funcTable == NULL)
        return;
    FILE *fp;
    fname == NULL ? fp = stdout : (fp = fopen(fname,"w"));
    if(fp == NULL) {
        fprintf(stderr,"ERROR: Failed to open %s", fname);
        return;
    }
    fprintf(fp,"\n%-30s %-12s\n", "FUNCTION NAME", "AR SIZE");
    for(int i = 0; i < SYMBOL_TABLE_SIZE; i++){
        symTableNode *currSTN = (funcTable->tb)[i];
        while(currSTN != NULL) {
            if (!currSTN->info.func.st) // function only declared and not defined
                fprintf(fp, "%-30s %-12s\n", currSTN->lexeme, "NOT DEFINED");
            else
                fprintf(fp, "%-30s %-4d\n", currSTN->lexeme, currSTN->info.func.st->scopeSize);
            currSTN = currSTN->next; // explore all functions hashed to current slot i
        }
    }

    fprintf(fp, "\nNOTE: The Activation Record (AR) size does not include width of input and output parameters of each function."
                "\n      Only the variables defined in local scope constitute the AR.\n");
    fcloseSafe(fp);
}
