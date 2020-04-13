#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbolHash.h"
#include "symbolTableDef.h"
#include "symbolTable.h"

#define FUNC_ALIGN 3

void printSymbolTable2(symbolTable* st, FILE *fp){
    // prints the whole SymbolTable Structure by calling printCurrSymTable
    // TODO: add a call to printCurrSymTable(..) appropriately
    if(st == NULL)
        return;
    symTableNode *currST = NULL;
    fprintf(fp,"\n\n####################################################################### SYMBOL TABLE #######################################################################\n\n");
    for(int i=0; i<SYMBOL_TABLE_SIZE; i++){
        currST = (st->tb)[i];
        while(currST != NULL){
            fprintf(fp,"Module '%s', slot %d\n",currST->lexeme,i);
            printCurrentSymTable(currST, fp);
            fprintf(fp,"\n\n");
            currST = currST->next;
        }
    }
    fprintf(fp,"########################################################################## ~ ** ~ ##########################################################################\n\n");
}

void printAtAlignment(char *toPrint, int align, FILE *fp){
    // prints 'toPrint', aligning it 'align' spaces from left
    fprintf(fp,"%*s",align,"");
    fprintf(fp,"%s", toPrint);
}

void setSymNodeTypeStr(varType vt, char *ts, char *boundInfo){
    int skip = 0;
    switch(vt.baseType){
        case g_INTEGER:
            skip += sprintf(ts + skip,"baseType: %s","integer");
            break;
        case g_REAL:
            skip += sprintf(ts + skip,"baseType: %s","real");
            break;
        case g_BOOLEAN:
            skip += sprintf(ts + skip,"baseType: %s","boolean");
            break;
        default: skip += sprintf(ts + skip,"baseType: %s","error_type");
    }

    switch(vt.vaType){
        case VARIABLE :
            skip += sprintf(ts + skip,", vaType: %s, width: %d","VARIABLE", vt.width);
            strcpy(boundInfo,"");
            break;
        case STAT_ARR :
            skip += sprintf(ts + skip,", vaType: %s, bounds: [%u..%u], width: %d", "STAT_ARR", vt.si.vt_num, vt.ei.vt_num, vt.width);
            strcpy(boundInfo,"");
            break;
        case DYN_L_ARR: {
            char pstr[150];
            skip += sprintf(ts + skip, ", vaType: %s, bounds: [%s..%u], width: %d", "DYN_L_ARR", vt.si.vt_id->lexeme, vt.ei.vt_num, vt.width);
            getSymNode(vt.si.vt_id, pstr);
            sprintf(boundInfo,", boundInfo: [ Left Bound: %s ]", pstr);
            break;
        }
        case DYN_R_ARR: {
            char pstr[150];
            skip += sprintf(ts + skip, ", vaType: %s, bounds: [%u..%s], width: %d", "DYN_R_ARR", vt.si.vt_num, vt.ei.vt_id->lexeme, vt.width);
            getSymNode(vt.ei.vt_id, pstr);
            sprintf(boundInfo,", boundInfo: [ Right Bound: %s ]", pstr);
            break;
        }
        case DYN_ARR: {
            char pstr1[150];
            char pstr2[150];
            skip += sprintf(ts + skip, ", vaType: %s, bounds: [%s..%s], width: %d", "DYN_ARR", vt.si.vt_id->lexeme, vt.ei.vt_id->lexeme, vt.width);
            getSymNode(vt.si.vt_id, pstr1);
            getSymNode(vt.ei.vt_id, pstr2);
            sprintf(boundInfo,", boundInfo: [ Left Bound: %s, Right Bound: %s ]", pstr1, pstr2);
            break;
        }
        default:
            skip += sprintf(ts + skip,", vaType: %s","ERROR_TYPE");
    }
}

void getSymNode(symTableNode *node, char *pstr){
    char typeStr[100];
    char boundStr[350];
    setSymNodeTypeStr(node->info.var.vtype, typeStr, boundStr);
    if(node->info.var.vtype.vaType == VARIABLE || node->info.var.vtype.vaType == STAT_ARR) {
        sprintf(pstr, "[ Name: '%s', Line No.: %d, Type: [ %s ], offset: %d, isAssigned: %s, isLoopVar: %s ]",
                node->lexeme, node->info.var.lno, typeStr, node->info.var.offset,
                node->info.var.isAssigned ? "true" : "false", node->info.var.isLoopVar ? "true" : "false");
    }
    else{
        sprintf(pstr, "[ Name: '%s', Line No.: %d, Type: [ %s ], offset: %d, isAssigned: %s, isLoopVar: %s%s ]",
                node->lexeme, node->info.var.lno, typeStr, node->info.var.offset,
                node->info.var.isAssigned ? "true" : "false", node->info.var.isLoopVar ? "true" : "false", boundStr);
    }
}

void printParamList(symTableNode *head, int baseAlign, FILE *fp){
    char pstr[500];
    int alignHence = baseAlign;
    printAtAlignment("[\n",alignHence, fp);// line 1

    alignHence += 3;
    // line 2 and henceforth
    while(head!=NULL){
        getSymNode(head, pstr);
        printAtAlignment(pstr, alignHence, fp);
        head = head->next;
        head == NULL ? fprintf(fp,"\n") : fprintf(fp,",\n");
    }
    printAtAlignment("]\n",baseAlign, fp);
}

void printSymNode(symTableNode *stn, int align, FILE *fp){
    char pstr[500];
    getSymNode(stn, pstr);
    printAtAlignment(pstr, align, fp);
    fprintf(fp,"\n");
}

void printThisST(symTableNode **stp, int align, FILE *fp){
    for(int i = 0; i<SYMBOL_TABLE_SIZE; i++){
        symTableNode *currVar = stp[i];
        while(currVar != NULL) {
            printSymNode(currVar, align, fp);
            currVar = currVar->next;
        }
    }
}

void printScopeDFS(symbolTable *st, int baseAlign, int level, FILE* fp){
    char lstr[20];
    sprintf(lstr, "--- LEVEL %d ---\n",level);
    printAtAlignment(lstr,baseAlign, fp);
    printThisST(st->tb, baseAlign, fp);

    if(st->headChild != NULL) {
        printScopeDFS(st->headChild, baseAlign + 5, level+1, fp);
    }
    fprintf(fp,"\n");
    if(st->next != NULL){
        printScopeDFS(st->next, baseAlign, level, fp);
    }

}

void printFuncVar(union funcVar *fvinfo, int baseAlign, FILE *fp){
    int alignHence = baseAlign;
    printAtAlignment("[\n",baseAlign, fp);// line 1
    alignHence+=3;
    printAtAlignment(" Module status: ",alignHence, fp);

    switch(fvinfo->func.status){
        case F_DECLARED:
            fprintf(fp,"DECLARED, line %d\n",fvinfo->func.lno);// line 1
            break;
        case F_DECLARATION_VALID:
            fprintf(fp,"DECLARATION VALID, line %d\n",fvinfo->func.lno);// line 1
            break;
        case F_DEFINED:
            fprintf(fp,"DEFINED, line %d\n",fvinfo->func.lno);// line 1
            break;
        default: break;
    }

    alignHence += 1; // align after beginning "[ "
    if(fvinfo->func.inpPListHead != NULL) {
        printAtAlignment("Input Parameters List:\n", alignHence, fp);
        printParamList(fvinfo->func.inpPListHead, alignHence, fp);
    }
    else{
        printAtAlignment("Input Parameters List Empty.\n",alignHence, fp);
    }

    fprintf(fp,"\n");

    if(fvinfo->func.outPListHead != NULL) {
        printAtAlignment("Output Parameters List:\n", alignHence, fp);
        printParamList(fvinfo->func.outPListHead, alignHence, fp);
    }
    else{
        printAtAlignment("Output Parameters List Empty.\n",alignHence, fp);
    }
    printAtAlignment("]\n",baseAlign, fp);// line 1

    printAtAlignment("Scope Hierarchy:\n",baseAlign, fp);
    printScopeDFS(fvinfo->func.st, baseAlign+3, 1, fp);

}
void printCurrentSymTable(symTableNode *stn, FILE *fp){    // prints just one symbol table and its hierarchies corresponding to a function scope
    // i.e. prints the whole scope structure of a function
    printFuncVar(&(stn->info), FUNC_ALIGN, fp);
}
