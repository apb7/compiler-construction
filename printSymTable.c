#include <stdio.h>
#include <stdlib.h>
#include "symbolHash.h"
#include "symbolTableDef.h"
#include "symbolTable.h"

#define FUNC_ALIGN 3
int curr_align;

void printSymbolTable(symbolTable* st){
    // prints the whole SymbolTable Structure by calling printCurrSymTable
    // TODO: add a call to printCurrSymTable(..) appropriately
    if(st == NULL)
        return;
    symTableNode *currST = NULL;
    printf("\n\n################################# SYMBOL TABLE #################################\n\n");
    for(int i=0; i<SYMBOL_TABLE_SIZE; i++){
        currST = (st->tb)[i];
        while(currST != NULL){
            printf("Module '%s', slot %d\n",currST->lexeme,i);
            printCurrSymTable(currST);
            printf("\n\n");
            currST = currST->next;
        }
    }
    printf("##################################### ~ ** ~ #####################################\n\n");
}

void printAtAlignment(char *toPrint, int align){
    // prints 'toPrint', aligning it 'align' spaces from left
    printf("%*s",align,"");
    printf("%s", toPrint);
}

void setSymNodeTypeStr(varType vt, char *ts){
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
        default:skip += sprintf(ts + skip,"baseType: %s","error_type");
    }

    switch(vt.vaType){
        case VARIABLE :
            skip += sprintf(ts + skip,", varOrArr: %s","VARIABLE");
            break;
        case STAT_ARR :
            skip += sprintf(ts + skip,", varOrArr: %s","STAT_ARR");
            skip += sprintf(ts + skip,", bounds: [%u..%d]",vt.si.vt_num);
            break;
        case DYN_L_ARR:
            skip += sprintf(ts + skip,", varOrArr: %s","DYN_L_ARR");
            break;
        case DYN_R_ARR:
            skip += sprintf(ts + skip,", varOrArr: %s","DYN_R_ARR");
            break;
        case DYN_ARR  :
            skip += sprintf(ts + skip,", varOrArr: %s","DYN_ARR");
            break;
        default:
            skip += sprintf(ts + skip,", varOrArr: %s","ERROR_TYPE");
    }
}

void printSymNode(symTableNode *node, int align){
    char pstr[400];
    char typeStr[250];
    setSymNodeTypeStr(node->info.var.vtype, typeStr);
    sprintf(pstr,"[ Name: %s, Line No.: %d, Type: [ %s ], Offset: %d, isAssigned: %s, isLoopVar: %s ]",node->lexeme, node->info.var.lno, typeStr, node->info.var.offset, node->info.var.isAssigned ? "true" : "false", node->info.var.isLoopVar ? "true" : "false");
    printAtAlignment(pstr, align);
}

void printParamList(symTableNode *head, int baseAlign, pListType pt){
    char pstr[100];
    int alignHence = baseAlign;
    printAtAlignment("[\n",curr_align);// line 1

    alignHence += 3;
    // line 2 and henceforth
    while(head!=NULL){
        printSymNode(head, alignHence);
        head = head->next;
        head == NULL ? printf("\n") : printf(",\n");
    }
    printAtAlignment("]\n",curr_align);
}

void printFuncVar(union funcVar *fvinfo, int baseAlign){
    char pstr[100];
    int alignHence = baseAlign;
    printAtAlignment("[ Module status: ",curr_align);// line 1

    switch(fvinfo->func.status){
        case F_DECLARED:
            printf("DECLARED, line %d\n",fvinfo->func.lno);// line 1
            break;
        case F_DECLARATION_VALID:
            printf("DECLARATION VALID, line %d\n",fvinfo->func.lno);// line 1
            break;
        case F_DEFINED:
            printf("DEFINED, line %d\n",fvinfo->func.lno);// line 1
            break;
        default: printf("");
    }

    alignHence += 2; // align after beginning "[ "
    printAtAlignment("Input Parameters List:\n",alignHence);
    printParamList(fvinfo->func.inpPListHead, alignHence, INP_PLIST);

}
void printCurrSymTable(symTableNode *stn){
    // prints just one symbol table and its hierarchies corresponding to a function scope
    // i.e. prints the whole scope structure of a function
    printFuncVar(&(stn->info), FUNC_ALIGN);



}


void makeSampleSymTableForTest(symbolTable* funcTable){

}