//#include "lexer.h"
#include "parserDef.h"
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include "../utils/set.h"
#include "../utils/hash.h"
#include "../utils/util.h"
#include "../config.h"
#include "../lexer/lexer.h"
#include "../utils/treeNodePtr_stack.h" // TYPE_stack.h
#include "../utils/treeNodePtr_stack_config.h" // TYPE_stack_config.h
#include "../lexer/lexerDef.h"


grammarNode *grammarArr;    //This array stores all the grammar rules each in a grammarNode
struct hashTable *mt;       //It is a mapping table used to map Symbols to their Enum Values
int numRules;               //Number of elements in the grammarArr
ruleRange ruleRangeArr[NUM_NON_TERMINALS];  //Number of rules for each non terminal
intSet* firstSet;   //This array stores first sets of all the non terminals
intSet* followSet;  //This array stores follow sets of all the non terminals
int parseTable[NUM_NON_TERMINALS][NUM_TERMINALS + 1];   //This table is used for predictive parsing

//This table is used to convert Enums to their corresponding strings
char *inverseMappingTable[] = {
#define X(a,b) b,
#define K(a,b,c) c,
#include "../data/keywords.txt"
#include "../data/tokens.txt"
        "EPS",
        "$",
#include "../data/nonTerminals.txt"
        "#"
#undef K
#undef X
};

//ntx can be used to map NonTerminal Enums to 0 based indexing
int ntx(int nonTerminalId){
    int adjustedValue = nonTerminalId - g_EOS - 1;
    if(adjustedValue < 0){
        fprintf(stderr,"ntx: Invalid nonTerminal, %d\n",nonTerminalId);
        return 0;
    }
    return adjustedValue;
}

//rntx is the inverse of ntx
int rntx(int nonTerminalIdx){ return nonTerminalIdx + g_EOS + 1; }

/*------- BOOLEAN CHECK FUNCTIONS -------*/
int isEpsilon(gSymbol symbol) {
    return (symbol==g_EPS?1:0);
}

int isTerminal(gSymbol symbol) {
    return (symbol<g_EOS?1:0);
}

int isNonTerminal(gSymbol symbol) {
    return ((symbol>g_EOS && symbol<g_numSymbols)?1:0);
}

bool isLeafNode(treeNode *ptr){
    if(ptr == NULL)
        return false;
    if(ptr->child == NULL)
        return true;
    else
        return false;
}

/*------- \BOOLEAN CHECK FUNCTIONS -------*/


/* ------------------ GRAMMAR REPRESENTATION STARTS ------------------*/

/*
 * This function is used to create and return a RhsNode for a rule of the grammar
 */
rhsNode *createRhsNode(char *rhsTk){
    if(!rhsTk){
        fprintf(stderr,"createRhsNode: NULL Token\n");
        return NULL;
    }
    rhsNode *rhs = (rhsNode *) malloc(sizeof(rhsNode));
    rhs->s = getEnumValue(rhsTk,mt);
    if(rhs->s == -1){
        fprintf(stderr,"createRhsNode: Invalid token %s\n",rhsTk);
        return NULL;
    }
    rhs->next = NULL;
    return rhs;
}

/*
 * This functions assumes that grammar rules are properly defined
 */
grammarNode createRuleNode(char *rule){
    //rule has the format "A,B,c,D,a" for a rule of type A -> BcDa
    char **ruleArr = strSplit(rule,',');
    grammarNode gnode;
    gnode.lhs = getEnumValue(ruleArr[0],mt);
    gnode.head = createRhsNode(ruleArr[1]);
    int i = 2;
    rhsNode *tmp = gnode.head;
    while(ruleArr[i] != NULL){
        tmp->next = createRhsNode(ruleArr[i]);
        tmp = tmp->next;
        i++;
    }
    //free up memory
    free(ruleArr[0]);   //frees up all the strings in ruleArr
    free(ruleArr);  //frees up all char pointers in ruleArr
    return gnode;
}

/*
 * Example for grammar file
 * 3
 * program,moduleDeclarations,otherModules,driverModule,otherModules
 * moduleDeclarations,moduleDeclaration,moduleDeclarations
 * moduleDeclarations,EPS
 */
void populateGrammarStruct(char *grFile){
    grammarArr = NULL;
    if(!grFile)
        return;
    FILE *fp = fopen(grFile,"r");
    if(!fp){
        fprintf(stderr,"populateGrammarStruct: ERROR, Unable to open file %s\n",grFile);
        return;
    }
    int i;
    char buf[MAX_LINE_LEN] = {0};
    fgets(buf,MAX_LINE_LEN,fp);
    sscanf(buf,"%d",&numRules);
    grammarArr = (grammarNode *) calloc(numRules, sizeof(grammarNode));
    for(i=0;i<numRules;i++){
        fgets(buf,MAX_LINE_LEN,fp);
        char *tRule = trim(buf);
        grammarArr[i] = createRuleNode(tRule);
        if(i>0 && grammarArr[i - 1].lhs != grammarArr[i].lhs){
            ruleRangeArr[ntx(grammarArr[i - 1].lhs)].end = i - 1;
            ruleRangeArr[ntx(grammarArr[i].lhs)].start = i;
        }
        else if(i == 0){
            ruleRangeArr[ntx(grammarArr[i].lhs)].start = i;
        }
    }
    ruleRangeArr[ntx(grammarArr[numRules - 1].lhs)].end = numRules - 1;

}

/* ------------------ GRAMMAR REPRESENTATION ENDS ------------------*/



/*------------FIRST AND FOLLOW STARTS-------------*/
/*
 * This function fills the entries of the global First array
 */
void populateFirstSet() {
    int n=numRules;
    int isChanged=1;
    firstSet = (intSet*)calloc(NUM_NON_TERMINALS, sizeof(intSet));
    memset(firstSet,0,sizeof(intSet));
    while(isChanged) {
        isChanged=0;
        for(int i = 0; i < n; i++) {
            gSymbol left_val=grammarArr[i].lhs;
            rhsNode* first = grammarArr[i].head;
            while(first!=NULL) {
                gSymbol ff_val=first->s;
                if(isTerminal(ff_val)) {
                    intSet prev=firstSet[ntx(left_val)];
                    firstSet[ntx(left_val)]=add_elt(firstSet[ntx(left_val)],ff_val);
                    if(prev != firstSet[ntx(left_val)]) 
                        isChanged=1;
                    break;
                } else if(isEpsilon(ff_val)) {
                    first=first->next;
                } else {
                    intSet prev=firstSet[ntx(left_val)];
                    firstSet[ntx(left_val)]=union_set(firstSet[ntx(left_val)],remove_elt(firstSet[ntx(ff_val)],g_EPS));
                    if(prev != firstSet[ntx(left_val)])
                        isChanged=1;
                    if(isPresent(firstSet[ntx(ff_val)],g_EPS)) {
                        first=first->next;
                    }
                    else break;
                }
            }
            if(first==NULL) {
                intSet prev=firstSet[ntx(left_val)];
                firstSet[ntx(left_val)]=add_elt(firstSet[ntx(left_val)],g_EPS);
                if(prev != firstSet[ntx(left_val)])
                    isChanged=1;
            }
        }
    }
}


/*
 * This function fills the entries of the global Follow array
 */
void populateFollowSet() {
    int n=numRules;
    int isChanged=1;
    followSet = (intSet*)calloc(NUM_NON_TERMINALS, sizeof(intSet));
    memset(followSet,0,sizeof(intSet));
    //follow of topmost NT is $;
    followSet[0]=add_elt(followSet[0],g_EOS);
    while(isChanged) {

        isChanged=0;
        
        for(int i = 0; i < n; i++) {
            gSymbol left_val=grammarArr[i].lhs;
            rhsNode* first = grammarArr[i].head;
            while(first!=NULL) {
                gSymbol ff_val=first->s;
                if(isTerminal(ff_val) || isEpsilon(ff_val)) {
                    first=first->next;
                    continue;
                }
                rhsNode* second = first->next;

                while(1) {
                    if(second==NULL) {
                        intSet prev=followSet[ntx(ff_val)];
                        followSet[ntx(ff_val)]=union_set(followSet[ntx(ff_val)],followSet[ntx(left_val)]);
                        if(prev!=followSet[ntx(ff_val)])
                            isChanged=1;
                        break;
                    }
                    gSymbol ss_val=second->s;
                    if(isEpsilon(ss_val)) {
                        second=second->next;
                    } else if(isTerminal(ss_val)) {
                        intSet prev=followSet[ntx(ff_val)];
                        followSet[ntx(ff_val)]=add_elt(followSet[ntx(ff_val)],ss_val);
                        if(prev!=followSet[ntx(ff_val)])
                            isChanged=1;
                        break;
                    } else {
                        intSet prev=followSet[ntx(ff_val)];
                        followSet[ntx(ff_val)]=union_set(followSet[ntx(ff_val)],firstSet[ntx(ss_val)]);
                        if(prev!=followSet[ntx(ff_val)])
                            isChanged=1;
                        if(isPresent(firstSet[ntx(ss_val)],g_EPS)) second=second->next; 
                        else break;
                    }
                }

                first=first->next;
            }
        }
    }
    for(int i = 0; i < NUM_NON_TERMINALS; i++) {
        if(isPresent(followSet[i],g_EPS))
            followSet[i] = remove_elt(followSet[i],g_EPS);
    }
}
/*------------FIRST AND FOLLOW ENDS-------------*/



/*------------PARSE TABLE STARTS-------------*/

//To initialize the parse Table
void initParseTable() {
    for(int i = 0; i < NUM_NON_TERMINALS; i++) {
        for(int j = 0; j <= NUM_TERMINALS; j++)
            parseTable[i][j]=-1;
    }
}

//To compute the predictSet used for populating the parse table
intSet predictSet(grammarNode* g) {
    intSet mask=0;
    gSymbol lval=g->lhs;
    rhsNode* current=g->head;
    while(current != NULL) {
        int rval=current->s;
        if(isEpsilon(rval)) {
            current=current->next;
        } else if(isTerminal(rval)) {
            mask=add_elt(mask,rval);
            break;
        } else {
            mask=union_set(mask,remove_elt(firstSet[ntx(rval)],g_EPS));
            if(isPresent(firstSet[ntx(rval)],g_EPS))
                current=current->next;
            else break;
        }
    }
    if(current==NULL) {
        mask=union_set(mask,followSet[ntx(lval)]);
    }
    return mask;
}

// returns 0 if grammar is not LL(1);
int populateParseTable() {
    initParseTable();
    int br=0;
    for(int i = 0; i < numRules; i++) {
        intSet mask=predictSet(&grammarArr[i]);
        for(unsigned int bit = 0; bit < 64; ++bit) {
            if(isPresent(mask,bit)) {
                if(parseTable[ntx(grammarArr[i].lhs)][bit] != -1) {
                    fprintf(stderr, "populateParseTable: ERROR, Grammar is not LL(1). See line %d, %d of Grammar\n", i+1, parseTable[ntx(grammarArr[i].lhs)][bit] + 1);
                    br=1;
                    break;
                }
                else
                    parseTable[ntx(grammarArr[i].lhs)][bit]=i;
            }
        }
        if(br) break;
    }
    if(br) return 0;
    return 1;
}

/*------------PARSE TABLE ENDS-------------*/


/* ------------------ PARSE TREE HELPER FUNCTIONS START ------------------*/

treeNode *newTreeNode(gSymbol sym, treeNode *parent){
    treeNode *tmp = (treeNode *) malloc(sizeof(treeNode));
    tmp->next = NULL;
    tmp->child = NULL;
    tmp->tkinfo = NULL;
    tmp->tk = sym;
    tmp->parent = parent;
    return tmp;
}

/* ------------------ PARSE TREE HELPER FUNCTIONS END ------------------*/


/*------------PARSE I/P SOURCECODE STARTS-------------*/
treeNode *parseInputSourceCode(char *src){
    bool errorFree = true;
    if(!src) {
        fprintf(stderr,"parseInputSourceCode: ERROR, Invalid source file\n");
        return NULL;
    }
    //open the source file
    FILE *srcFilePtr = fopen(src,"r");
    if(!srcFilePtr){
        fprintf(stderr,"parseInputSourceCode: ERROR, Cannot open file, %s\n",src);
        return NULL;
    }

    //Init two stacks
    treeNodePtr_stack *parseStack = treeNodePtr_stack_create();
    treeNodePtr_stack *tmpStack = treeNodePtr_stack_create();

    //Push $ symbol to stack
    treeNode *dummyNode = newTreeNode(g_EOS,NULL);
    treeNodePtr_stack_push(parseStack,dummyNode);

    //Push Start Symbol (program) to stack
    treeNode *parseTreeRoot = newTreeNode(g_program,NULL);
    treeNodePtr_stack_push(parseStack,parseTreeRoot);

    tokenInfo *tkinfo = getNextToken(srcFilePtr);
    if(tkinfo == NULL){
        fprintf(stderr,"parseInputSourceCode: ERROR, Source file empty.\n");
        return NULL;
    }
    //loop until there are no more tokens
    bool eosEncountered = false;
    while(1){
        gSymbol sym = eosEncountered ? g_EOS : (tkinfo->type);
        treeNode *topNode = treeNodePtr_stack_top(parseStack);
        if(topNode->tk == g_EPS){
            topNode->tkinfo = NULL;
            topNode->child = NULL;
            treeNodePtr_stack_pop(parseStack);
        }
        else if(topNode->tk == sym){
            if(eosEncountered)
                break;
            topNode->tkinfo = tkinfo;
            topNode->child = NULL;
            treeNodePtr_stack_pop(parseStack);
            tkinfo = getNextToken(srcFilePtr);
            if(tkinfo == NULL){
                eosEncountered = true;
            }
        }
        else if(topNode->tk <= g_EOS){
            //topNode is not a non Terminal
            errorFree = false;
            fprintf(stderr,"parseInputSourceCode: ERROR, Stack Top & Lexer token mismatch. Found %s and %s respectively.\n",inverseMappingTable[topNode->tk],inverseMappingTable[sym]);
            return NULL;
        }
        else{
            //topNode is a non Terminal
            int ruleId = parseTable[ntx(topNode->tk)][sym];
            if(ruleId == -1){
                //Invalid Combination
                errorFree = false;
                //TODO: Error Reporting
                printf("Error due to no grammar rule for %d , %d\n",ntx(topNode->tk),sym);
                return NULL;
            }
            else{
                grammarNode gNode = grammarArr[ruleId];
                if(gNode.lhs != topNode->tk){
                    //Report Unexpected Error
                }
                treeNodePtr_stack_pop(parseStack);
                rhsNode *rhsPtr = gNode.head;
                treeNode *currChild = NULL;
                while(rhsPtr != NULL){
                    treeNode *tmpChild = newTreeNode(rhsPtr->s,topNode);
                    treeNodePtr_stack_push(tmpStack, tmpChild);
                    if(currChild == NULL){
                        topNode->child = tmpChild;
                        currChild = tmpChild;
                    }
                    else{
                        currChild->next = tmpChild;
                        currChild = tmpChild;
                    }
                    rhsPtr = rhsPtr->next;
                }
                while(!treeNodePtr_stack_isEmpty(tmpStack)){
                    treeNode *tmpTop = treeNodePtr_stack_top(tmpStack);
                    treeNodePtr_stack_push(parseStack,tmpTop);
                    treeNodePtr_stack_pop(tmpStack);
                }
            }
        }
    }
    if(errorFree){
        printf("Input source code is syntactically correct...........\n");
    }
    return parseTreeRoot;
}

/*------------PARSE I/P SOURCECODE ENDS-------------*/

/*------------PRINTING STARTS-------------*/

void printParseTable() {
    for(int i = 0; i < NUM_NON_TERMINALS; i++) {
        for(int j = 0; j <= NUM_TERMINALS; j++)
            printf("%d\t", parseTable[i][j]);
        printf("\n");
    }
}
void printGrammar() {
    int n=numRules;
    printf("Number of rules = %d\n",n);
    for(int i = 0; i < n; i++) {
        printf("%s -> ",inverseMappingTable[grammarArr[i].lhs]);
        rhsNode* start=grammarArr[i].head;
        while(start!=NULL) {
            printf("%s ", inverseMappingTable[start->s]);
            start=start->next;
        }
        printf("\n");
    }
}
void printFirst() {
    for(int i = 0; i < NUM_NON_TERMINALS; i++) {
        unsigned long long num=firstSet[i];
        printf("%s -> {", inverseMappingTable[rntx(i)]);
        for(int j =0; j < 64; j++) {
            if(isPresent(num,j))
                printf("%s ", inverseMappingTable[j]);
        }
        printf("}\n");
    }
}
void printFollow() {
    for(int i = 0; i < NUM_NON_TERMINALS; i++) {
        unsigned long long num=followSet[i];
        printf("%s -> {", inverseMappingTable[rntx(i)]);
        for(int j =0; j < 64; j++) {
            if(isPresent(num,j))
                printf("%s ", inverseMappingTable[j]);
        }
        printf("}\n");
    }
}
void printPredictSets() {
    for(int i = 0; i < numRules; i++) {
        intSet mask=predictSet(&grammarArr[i]);
        for(unsigned int bit=0; bit<64; ++bit) {
         if(isPresent(mask,bit))
             printf("%s ", inverseMappingTable[bit]);
        }
        printf("\n");
    }
}

/*------------PRINTING ENDS-------------*/


/*------------TREE PRINTING STARTS-------------*/

/*
*   Prints the node information to the file pointed by fp
 */
void printTreeNode(treeNode *ptr, FILE *fp){
    const char blank[] = "----";
    bool isLeaf = isLeafNode(ptr);
    if(fp == NULL){
        fprintf(stderr,"printTreeNode: Invalid file pointer.\n");
        return;
    }
    if(!ptr)
        return;
    if(isLeaf && ptr->tk != g_EPS){
        fprintf(fp,"%-21s",ptr->tkinfo->lexeme);
        fprintf(fp,"%-15u",ptr->tkinfo->lno);
    }
    else
        fprintf(fp,"%-21s%-15s",blank,blank);

    fprintf(fp,"%-25s",inverseMappingTable[ptr->tk]);

    if(ptr->tk == g_NUM){
        fprintf(fp,"%-15d",(ptr->tkinfo->value).num);
    }
    else if(ptr->tk == g_RNUM){
        fprintf(fp,"%-15f",(ptr->tkinfo->value).rnum);
    }
    else{
        fprintf(fp,"%-15s",blank);
    }

    if(ptr->parent == NULL)
        fprintf(fp,"%-25s","ROOT");
    else
        fprintf(fp,"%-25s",inverseMappingTable[ptr->parent->tk]);

    if(isLeaf){
        fprintf(fp,"%-10s","yes");
        fprintf(fp,"%s\n",blank);
    }
    else{
        fprintf(fp,"%-10s","no");
        fprintf(fp,"%s\n",inverseMappingTable[ptr->tk]);
    }


}

void printTreeUtil(treeNode* cur, FILE* fpt) {
    if(cur==NULL) return;
    treeNode* lchild=cur->child;
    if(lchild==NULL) {
        printTreeNode(cur, fpt);
        return ;
    }
    printTreeUtil(lchild, fpt);
    printTreeNode(cur, fpt);
    treeNode* rchild=lchild->next;
    while(rchild != NULL) {
        printTreeUtil(rchild, fpt);
        rchild=rchild->next;
    }
}

void printTree(treeNode* root,  char* fname) {
    if(!root)
        return;
    FILE *fpt = fopen(fname,"w");
    fprintf(fpt,"%-21s%-15s%-25s%-15s%-25s%-10s%s\n\n","[LEXEME]","[LINE_NO]","[TOKEN_NAME]","[VALUE]","[PARENT_NODE]","[IS_LEAF]","[NODE_SYMBOL]");
    printTreeUtil(root, fpt);
    fclose(fpt);
}

/*------------TREE PRINTING ENDS-------------*/


/*----------- TEST TREE PRINTING STARTS -----------*/
void putAllChildrenInSt(treeNode *child, treeNodePtr_stack *s1){
    while(child != NULL){
        treeNodePtr_stack_push(s1,child);
        child = child->next;
    }
}

void printInfoTreeNode(treeNode *ptr){
    if(ptr->parent == NULL){
        printf("(NULL->%s)\t",inverseMappingTable[ptr->tk]);
    }
    else
        printf("(%s->%s)\t",inverseMappingTable[ptr->parent->tk],inverseMappingTable[ptr->tk]);
}

void printTreeOld(treeNode *root){
    if(!root)
        return;
    treeNodePtr_stack *s1 = treeNodePtr_stack_create();
    treeNodePtr_stack *s2 = treeNodePtr_stack_create();

    treeNode *child = root->child;

    FILE *fpt = fopen("outputPrint.txt","w");

    fprintf(fpt,"%-21s%-15s%-25s%-15s%-25s%-10s%s\n\n","[LEXEME]","[LINE_NO]","[TOKEN_NAME]","[VALUE]","[PARENT_NODE]","[IS_LEAF]","[NODE_SYMBOL]");

    printInfoTreeNode(root);
    printTreeNode(root,fpt);

    putAllChildrenInSt(child,s1);

    while(!treeNodePtr_stack_isEmpty(s1)){
        while(!treeNodePtr_stack_isEmpty(s1)){
            treeNodePtr_stack_push(s2,treeNodePtr_stack_pop(s1));
        }

        while(!treeNodePtr_stack_isEmpty(s2)){
            treeNode *curr = treeNodePtr_stack_top(s2);
            putAllChildrenInSt(curr->child,s1);
            printTreeNode(curr,fpt);
            printInfoTreeNode(curr);
//            printf("%d\t",curr->tk);
            treeNodePtr_stack_pop(s2);
        }
        printf("\n\n");
    }
}

/*----------- TEST TREE PRINTING ENDS -----------*/

