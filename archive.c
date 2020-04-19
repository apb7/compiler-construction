#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbolHash.h"
#include "symbolTableDef.h"
#include "archive.h"
#include "util.h"

// ################################################################# printSymTable1 starts ########################################################

extern char *inverseMappingTable[];

void printVarEntry1(symTableNode *stNode, int sno, FILE *fp){
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

void printSymbolTable1(symbolTable* st, char *fname){
    if(st == NULL)
        return;
    FILE *fp;
    fname == NULL ? fp = stdout : (fp = fopen(fname,"w"));
    if(fp == NULL) {
        fprintf(stderr,"ERROR: Failed to open %s", fname);
        return;
    }

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
                printVarEntry1(iohead,sno,fp);
                sno++;
                iohead = iohead->next;
            }
            iohead = currSTN->info.func.outPListHead;
            while(iohead != NULL){
                printVarEntry1(iohead,sno,fp);
                sno++;
                iohead = iohead->next;
            }
            fprintf(fp,"\n");
            printCurrSymTable1(currSTN->info.func.st, 0, fp);
            currSTN = currSTN->next;
        }
    }
    fprintf(fp,"##################################### ~ ** ~ #####################################\n\n");
    fcloseSafe(fp);
}

void printCurrSymTable1(symbolTable *st,int level, FILE *fp){
    fprintf(fp,"################################# Level %d #################################\n\n",level);
    fprintf(fp,"%-4s %-20s %-10s %-10s %-25s %-25s %-5s %-6s\n","SNO","LEXEME","VAR/ARR","TYPE","LB","UB","LINE","OFFSET");
    int sno = 1;
    for(int i=0; i<SYMBOL_TABLE_SIZE; i++){
        symTableNode *currSTN = (st->tb)[i];
        while(currSTN != NULL){
            printVarEntry1(currSTN,sno,fp);
            currSTN = currSTN->next;
            sno++;
        }
    }
    fprintf(fp,"\n");
    symbolTable *childst = st->headChild;
    while(childst != NULL){
        printCurrSymTable1(childst,level+1,fp);
        childst = childst->next;
    }
}

// ################################################################# printSymTable1 ends ########################################################




// ################################################################# printSymTable2 starts ########################################################
#define FUNC_ALIGN 3 // how farther from the left to align the [ of the function.
#define LEVEL_STRING 20 // just to print "--- LEVEL %d ---"
#define TYPE_STRING_WO_BOUND_INFO 150 // type string without bound info
#define SYM_NODE_FOR_ONE_BOUND 250 // for the symbol table node of one bound (this ID will be INTEGER VARIABLE type) of dynamic array
#define BOUND_STRING SYM_NODE_FOR_ONE_BOUND*2 + 50 // for dynamic array's bounds' info
#define GENERAL_SYM_NODE TYPE_STRING_WO_BOUND_INFO +  BOUND_STRING + 120 // for any general symbol table node (possibly a dynamic array requiring both bounds as VARIABLE INTEGER symbol table nodes i.e. the largest possible)

void printSymbolTable2(symbolTable* st, char *fname){
    // prints the whole SymbolTable Structure by calling printCurrSymTable2
    if(st == NULL)
        return;
    FILE *fp;
    fname == NULL ? fp = stdout : (fp = fopen(fname,"w"));
    if(fp == NULL) {
        fprintf(stderr,"ERROR: Failed to open %s", fname);
        return;
    }

    symTableNode *currST = NULL;
    fprintf(fp,"\n\n####################################################################### SYMBOL TABLE #######################################################################\n\n");
    for(int i=0; i<SYMBOL_TABLE_SIZE; i++){
        currST = (st->tb)[i];
        while(currST != NULL){
            fprintf(fp,"Module '%s', slot %d\n",currST->lexeme,i);
            printCurrSymTable2(currST, fp);
            fprintf(fp,"\n\n");
            currST = currST->next;
        }
    }
    fprintf(fp,"########################################################################## ~ ** ~ ##########################################################################\n\n");
    fcloseSafe(fp);
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
            char pstr[SYM_NODE_FOR_ONE_BOUND];
            if(vt.si.vt_id == NULL){
                skip += sprintf(ts + skip, ", vaType: %s, bounds: [%s..%u], width: %d", "DYN_L_ARR", "NULL", vt.ei.vt_num, vt.width);
                strcpy(pstr,"NULL");
            }
            else{
                skip += sprintf(ts + skip, ", vaType: %s, bounds: [%s..%u], width: %d", "DYN_L_ARR", vt.si.vt_id->lexeme, vt.ei.vt_num, vt.width);
                getSymNode(vt.si.vt_id, pstr);
            }
            sprintf(boundInfo,", boundInfo: [ Left Bound: %s ]", pstr);
            break;
        }
        case DYN_R_ARR: {
            char pstr[SYM_NODE_FOR_ONE_BOUND];
            if(vt.ei.vt_id == NULL){
                skip += sprintf(ts + skip, ", vaType: %s, bounds: [%u..%s], width: %d", "DYN_R_ARR", vt.si.vt_num, "NULL", vt.width);
                strcpy(pstr,"NULL");
            }
            else{
                skip += sprintf(ts + skip, ", vaType: %s, bounds: [%u..%s], width: %d", "DYN_R_ARR", vt.si.vt_num, vt.ei.vt_id->lexeme, vt.width);
                getSymNode(vt.ei.vt_id, pstr);
            }
            sprintf(boundInfo,", boundInfo: [ Right Bound: %s ]", pstr);
            break;
        }
        case DYN_ARR: {
            char pstr1[SYM_NODE_FOR_ONE_BOUND];
            char pstr2[SYM_NODE_FOR_ONE_BOUND];
            skip += sprintf(ts + skip, ", vaType: %s, bounds: [%s..%s], width: %d", "DYN_ARR", vt.si.vt_id == NULL ? "NULL" : vt.si.vt_id->lexeme, vt.ei.vt_id == NULL ? "NULL" : vt.ei.vt_id->lexeme, vt.width);
            vt.si.vt_id == NULL ? strcpy(pstr1,"NULL") : getSymNode(vt.si.vt_id, pstr1);
            vt.ei.vt_id == NULL ? strcpy(pstr2,"NULL") : getSymNode(vt.ei.vt_id, pstr2);
            sprintf(boundInfo,", boundInfo: [ Left Bound: %s, Right Bound: %s ]", pstr1, pstr2);
            break;
        }
        default:
            skip += sprintf(ts + skip,", vaType: %s","ERROR_TYPE");
    }
}

void getSymNode(symTableNode *node, char *pstr){
    char typeStr[TYPE_STRING_WO_BOUND_INFO];
    char boundStr[BOUND_STRING];
    setSymNodeTypeStr(node->info.var.vtype, typeStr, boundStr);
    if(node->info.var.vtype.vaType == VARIABLE || node->info.var.vtype.vaType == STAT_ARR) {
        sprintf(pstr, "[ Name: '%s', Line No.: %d, Type: [ %s ], offset: %d, isAssigned: %s, forLoop: %s, whileLevel: %d, isIOlistVar: %s ]",
                node->lexeme, node->info.var.lno, typeStr, node->info.var.offset,
                node->info.var.isAssigned ? "true" : "false", node->info.var.forLoop ? ((node->info.var.forLoop == 1) ? "FOR_IN" : "FOR_OUT")  : "NOT_FOR",
                node->info.var.whileLevel, node->info.var.isIOlistVar ? "true" : "false");
    }
    else{
        sprintf(pstr, "[ Name: '%s', Line No.: %d, Type: [ %s ], offset: %d, isAssigned: %s, forLoop: %s, whileLevel: %d, isIOlistVar: %s%s ]",
                node->lexeme, node->info.var.lno, typeStr, node->info.var.offset,
                node->info.var.isAssigned ? "true" : "false", node->info.var.forLoop ? ((node->info.var.forLoop == 1) ? "FOR_IN" : "FOR_OUT")  : "NOT_FOR",
                node->info.var.whileLevel, node->info.var.isIOlistVar ? "true" : "false", boundStr);
    }
}

void printParamList(symTableNode *head, int baseAlign, FILE *fp){
    char pstr[GENERAL_SYM_NODE];
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
    char pstr[GENERAL_SYM_NODE];
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
    char lstr[LEVEL_STRING];
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
            fprintf(fp,"DECLARED, line %d. AR Size: %d\n",fvinfo->func.lno, fvinfo->func.arSize);// line 1
            break;
        case F_DECLARATION_VALID:
            fprintf(fp,"DECLARATION VALID, line %d. AR Size: %d\n",fvinfo->func.lno, fvinfo->func.arSize);// line 1
            break;
        case F_DEFINED:
            fprintf(fp,"DEFINED, line %d. AR Size: %d\n",fvinfo->func.lno, fvinfo->func.arSize);// line 1
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
void printCurrSymTable2(symTableNode *stn, FILE *fp){    // prints just one symbol table and its hierarchies corresponding to a function scope
    // i.e. prints the whole scope structure of a function
    printFuncVar(&(stn->info), FUNC_ALIGN, fp);
}

// ################################################################# printSymTable2 ends ########################################################
