//#include "lexer.h"
#include "parserDef.h"
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include "../set.h"
#include "../hash.h"
#include "../utils.h"
#include "../config.h"
#include "../lexer/lexer.h"
#include "../treeNodePtr_stack.h" // TYPE_stack.h
#include "../treeNodePtr_stack_config.h" // TYPE_stack_config.h


extern grammarNode *G;
extern struct hashTable *mt;
int numRules;
ruleRange rule_range[nt_numNonTerminals];
extern intSet* firstSet;
extern intSet* followSet;
int pTb[nt_numNonTerminals][t_numTerminals+1];

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


int isEpsilon(gSymbol symbol) {
    return (symbol==g_EPS?1:0);
}

int isTerminal(gSymbol symbol) {
    return (symbol<g_EOS?1:0);
}

int isNonTerminal(gSymbol symbol) {
    return ((symbol>g_EOS && symbol<g_numSymbols)?1:0);
}

rhsNode *createRhsNode(char *rhsTk){
    if(!rhsTk)
        return NULL;
    rhsNode *rhs = (rhsNode *) malloc(sizeof(rhsNode));
    rhs->s = getEnumValue(rhsTk,mt);
    if(rhs->s == -1)
        return NULL;
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
    G = NULL;
    if(!grFile)
        return;
    FILE *fp = fopen(grFile,"r");
    if(!fp)
        return;
    int i;
    char buf[MAX_LINE_LEN] = {0};
    fgets(buf,MAX_LINE_LEN,fp);
    sscanf(buf,"%d",&numRules);
    G = (grammarNode *) calloc(numRules,sizeof(grammarNode));
    for(i=0;i<numRules;i++){
        fgets(buf,MAX_LINE_LEN,fp);
        char *tRule = trim(buf);
        G[i] = createRuleNode(tRule);
        if(i>0 && G[i-1].lhs != G[i].lhs){
            rule_range[ntx(G[i-1].lhs)].end = i-1;
            rule_range[ntx(G[i].lhs)].start = i;
        }
        else if(i == 0){
            rule_range[ntx(G[i].lhs)].start = i;
        }
    }
    rule_range[ntx(G[numRules-1].lhs)].end = numRules - 1;

}


/*------------FIRST AND FOLLOW STARTS-------------*/
void populateFirstSet() {
    int n=numRules;
    int isChanged=1;
    int nonTerminal_count=g_numSymbols-g_EOS-1;
    firstSet = (intSet*)calloc(nonTerminal_count,sizeof(intSet));
    memset(firstSet,0,sizeof(firstSet));
    while(isChanged) {
        isChanged=0;
        for(int i = 0; i < n; i++) {
            gSymbol left_val=G[i].lhs;
            rhsNode* first = G[i].head;
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


void populateFollowSet() {
    int n=numRules;
    int isChanged=1;
    int nonTerminal_count=g_numSymbols-g_EOS-1;
    followSet = (intSet*)calloc(nonTerminal_count,sizeof(intSet));
    memset(followSet,0,sizeof(followSet));
    //follow of topmost NT is $;
    followSet[0]=add_elt(followSet[0],g_EOS);
    while(isChanged) {

        isChanged=0;
        
        for(int i = 0; i < n; i++) {
            gSymbol left_val=G[i].lhs;
            rhsNode* first = G[i].head;
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
    for(int i = 0; i < nonTerminal_count; i++) {
        if(isPresent(followSet[i],g_EPS))
            followSet[i] = remove_elt(followSet[i],g_EPS);
    }
}
/*------------FIRST AND FOLLOW ENDS-------------*/



/*------------PARSE TABLE STARTS-------------*/
void initParseTable() {
    for(int i = 0; i < nt_numNonTerminals; i++) {
        for(int j = 0; j <= t_numTerminals; j++)
            pTb[i][j]=-1;
    }
}

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
        intSet mask=predictSet(&G[i]);
        for(unsigned int bit = 0; bit < 64; ++bit) {
            if(isPresent(mask,bit)) {
                if(pTb[ntx(G[i].lhs)][bit]!=-1) {
                    printf("Grammar is not LL(1). See line %d, %d of Grammar\n", i+1, pTb[ntx(G[i].lhs)][bit]+1);
                    br=1;
                    break;
                }
                else
                    pTb[ntx(G[i].lhs)][bit]=i;
            }
        }
        if(br) break;
    }
    if(br) return 0;
    return 1;
}
/*------------PARSE TABLE ENDS-------------*/



treeNode *newTreeNode(gSymbol sym, treeNode *parent){
    treeNode *tmp = (treeNode *) malloc(sizeof(treeNode));
    tmp->next = NULL;
    tmp->child = NULL;
    tmp->tkinfo = NULL;
    tmp->tk = sym;
    tmp->parent = parent;
    return tmp;
}

/*------------PARSE I/P SOURCECODE STARTS-------------*/
treeNode *parseInputSourceCode(char *src){
    bool errorFree = true;
    if(!src) {
        //TODO: Error Reporting
        return NULL;
    }
    //open the source file
    FILE *srcFilePtr = fopen(src,"r");
    if(!srcFilePtr){
        //TODO: Error Reporting
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
            printf("Error due to topNode not a non Terminal\n");
            //TODO: Error Reporting
            return NULL;
        }
        else{
            //topNode is a non Terminal
            int ruleId = pTb[ntx(topNode->tk)][sym];
            if(ruleId == -1){
                //Invalid Combination
                errorFree = false;
                //TODO: Error Reporting
                printf("Error due to no grammar rule for %d , %d\n",ntx(topNode->tk),sym);
                return NULL;
            }
            else{
                grammarNode gNode = G[ruleId];
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

/*----------- INFORMAL TREE PRINTING STARTS -----------*/
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

void printTree(treeNode *root){
    treeNodePtr_stack *s1 = treeNodePtr_stack_create();
    treeNodePtr_stack *s2 = treeNodePtr_stack_create();

    treeNode *child = root->child;

    printInfoTreeNode(root);

    putAllChildrenInSt(child,s1);

    while(!treeNodePtr_stack_isEmpty(s1)){
        while(!treeNodePtr_stack_isEmpty(s1)){
            treeNodePtr_stack_push(s2,treeNodePtr_stack_pop(s1));
        }

        while(!treeNodePtr_stack_isEmpty(s2)){
            treeNode *curr = treeNodePtr_stack_top(s2);
            putAllChildrenInSt(curr->child,s1);
            printInfoTreeNode(curr);
//            printf("%d\t",curr->tk);
            treeNodePtr_stack_pop(s2);
        }
        printf("\n\n");
    }
}

/*----------- INFORMAL TREE PRINTING ENDS -----------*/

void printParseTable() {
    for(int i = 0; i < nt_numNonTerminals; i++) {
        for(int j = 0; j <= t_numTerminals; j++)
            printf("%d\t",pTb[i][j]);
        printf("\n");
    }
}
void printGrammar() {
    int n=numRules;
    printf("Number of rules = %d\n",n);
    for(int i = 0; i < n; i++) {
        printf("%s -> ",inverseMappingTable[G[i].lhs]);
        rhsNode* start=G[i].head;
        while(start!=NULL) {
            printf("%s ", inverseMappingTable[start->s]);
            start=start->next;
        }
        printf("\n");
    }
}
void printFirst() {
    for(int i = 0; i < nt_numNonTerminals; i++) {
        unsigned long long num=firstSet[i];
        printf("%s -> {", inverseMappingTable[i+g_EOS+1]);
        for(int j =0; j < 64; j++) {
            if(isPresent(num,j))
                printf("%s ", inverseMappingTable[j]);
        }
        printf("}\n");
    }
}
void printFollow() {
    for(int i = 0; i < nt_numNonTerminals; i++) {
        unsigned long long num=followSet[i];
        printf("%s -> {", inverseMappingTable[i+g_EOS+1]);
        for(int j =0; j < 64; j++) {
            if(isPresent(num,j))
                printf("%s ", inverseMappingTable[j]);
        }
        printf("}\n");
    }
}
void printPredictSets() {
    for(int i = 0; i < numRules; i++) {
        intSet mask=predictSet(&G[i]);
        for(unsigned int bit=0; bit<64; ++bit) {
         if(isPresent(mask,bit))
             printf("%s ", inverseMappingTable[bit]);
        }
        printf("\n");
    }
}
/*------------PRINTING ENDS-------------*/



