// Group Number: 31
// MADHUR PANWAR   2016B4A70933P
// TUSSANK GUPTA   2016B3A70528P
// SALMAAN SHAHID  2016B4A70580P
// APURV BAJAJ     2016B3A70549P
// HASAN NAQVI     2016B5A70452P

//#include "lexer.h"
#include "parserDef.h"
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include "set.h"
#include "hash.h"
#include "util.h"
#include "config.h"
#include "lexer.h"
#include "treeNodePtr_stack.h" // TYPE_stack.h
#include "lexerDef.h"
#include "error.h"
#include "errorPtr_stack.h"


grammarNode *grammarArr;    //This array stores all the grammar rules each in a grammarNode
struct hashTable *mt;       //It is a mapping table used to map Symbols to their Enum Values
int numRules;               //Number of elements in the grammarArr
ruleRange ruleRangeArr[NUM_NON_TERMINALS];  //Number of rules for each non terminal
intSet* firstSet;   //This array stores first sets of all the non terminals
intSet* followSet;  //This array stores follow sets of all the non terminals
int parseTable[NUM_NON_TERMINALS][NUM_TERMINALS + 1];   //This table is used for predictive parsing
intSet defaultSyn;

//This table is used to convert Enums to their corresponding strings
char *inverseMappingTable[] = {
#define X(a,b) b,
#define K(a,b,c) b,
#include "keywords.txt"
#include "tokens.txt"
        "EPS",
        "$",
#include "nonTerminals.txt"
        "#"
#undef K
#undef X
};

//used mainly for error printing
char *enum2LexemeTable[] = {
#define K(a,b,c) c,
#include "keywords.txt"
#include "tokens.txt"
"EPS", "$",
"#"
#undef K
};

#define len(arr) (sizeof(arr) ? (sizeof(arr)/sizeof((arr)[0])) : 0);
#define ERROR_RECOVERY_POP -(numRules+1)
#define ERROR_RECOVERY_SKIP -numRules

//ntx can be used to map NonTerminal Enums to 0 based indexing
int ntx(int nonTerminalId) {
    int adjustedValue = nonTerminalId - g_EOS - 1;
    if(adjustedValue < 0){
        fprintf(stderr,"ntx: Invalid nonTerminal, %d\n",nonTerminalId);
        return 0;
    }
    return adjustedValue;
}

//rntx is the inverse of ntx
int rntx(int nonTerminalIdx) {
    return nonTerminalIdx + g_EOS + 1;
}

/*------- BOOLEAN CHECK FUNCTIONS -------*/
int isEpsilon(gSymbol symbol) {
    return symbol == g_EPS;
}

int isTerminal(gSymbol symbol) {
    return symbol < g_EOS;
}

int isNonTerminal(gSymbol symbol) {
    return (symbol > g_EOS && symbol < g_numSymbols);
}

bool isLeafNode(treeNode *ptr) {
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
rhsNode *createRhsNode(char *rhsTk) {
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
grammarNode createRuleNode(char *rule) {
    //rule has the format "A,B,c,D,a" for a rule of type A -> BcDa
    char **ruleArr = strSplit(rule, ',');

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
void printRuleRange() {
    for(int i = 0; i < NUM_NON_TERMINALS; i++){
        printf("start: %d, end: %d\n", ruleRangeArr[i].start, ruleRangeArr[i].end);
    }
}

void populateGrammarStruct(char *grammarFile) {
    grammarArr = NULL;

    if(!grammarFile)
        return;

    FILE *fp = fopen(grammarFile, "r");
    if(!fp) {
        fprintf(stderr,"populateGrammarStruct: ERROR, Unable to open file %s\n", grammarFile);
        return;
    }

    int i;
    char buf[MAX_LINE_LEN] = {0};
    fgets(buf, MAX_LINE_LEN, fp);

    sscanf(buf, "%d", &numRules);
    grammarArr = (grammarNode *) calloc(numRules, sizeof(grammarNode));

    for(i = 0; i < numRules; i++) {
        fgets(buf,MAX_LINE_LEN,fp);
        char *tRule = trim(buf);
        grammarArr[i] = createRuleNode(tRule);

        if(i>0 && grammarArr[i - 1].lhs != grammarArr[i].lhs) {
            ruleRangeArr[ntx(grammarArr[i - 1].lhs)].end = i - 1;
            ruleRangeArr[ntx(grammarArr[i].lhs)].start = i;
        }
    }
    ruleRangeArr[ntx(grammarArr[0].lhs)].start = 0;
    ruleRangeArr[ntx(grammarArr[numRules - 1].lhs)].end = numRules - 1;
}

/* ------------------ GRAMMAR REPRESENTATION ENDS ------------------*/



/*------------FIRST AND FOLLOW STARTS-------------*/
/*
 * This function fills the entries of the global First array
 */
void populateFirstSet() {
    int isChanged=1;

    firstSet = (intSet*)calloc(NUM_NON_TERMINALS, sizeof(intSet));
    memset(firstSet, 0, sizeof(intSet));

    while(isChanged) {
        isChanged=0;
        for(int i = 0; i < numRules; i++) {
            gSymbol left_val = grammarArr[i].lhs;
            rhsNode* first = grammarArr[i].head;

            while(first != NULL) {
                gSymbol ff_val = first->s;
                if(isTerminal(ff_val)) {
                    intSet prev=firstSet[ntx(left_val)];
                    firstSet[ntx(left_val)] = add_element(firstSet[ntx(left_val)],ff_val);
                    if(prev != firstSet[ntx(left_val)]) 
                        isChanged = 1;
                    break;
                }
                else if(isEpsilon(ff_val)) {
                    first = first->next;
                }
                else {
                    intSet prev = firstSet[ntx(left_val)];
                    firstSet[ntx(left_val)] = union_set(firstSet[ntx(left_val)], remove_element(firstSet[ntx(ff_val)], g_EPS));

                    if(prev != firstSet[ntx(left_val)])
                        isChanged = 1;

                    if(isPresent(firstSet[ntx(ff_val)], g_EPS))
                        first = first->next;
                    else
                        break;
                }
            }
            if(first == NULL) {
                intSet prev = firstSet[ntx(left_val)];
                firstSet[ntx(left_val)] = add_element(firstSet[ntx(left_val)], g_EPS);
                if(prev != firstSet[ntx(left_val)])
                    isChanged = 1;
            }
        }
    }
}


/*
 * This function fills the entries of the global Follow array
 */
void populateFollowSet() {
    int isChanged = 1;
    followSet = (intSet*)calloc(NUM_NON_TERMINALS, sizeof(intSet));
    memset(followSet,0,sizeof(intSet));
    //follow of topmost NT is $;
    followSet[0] = add_element(followSet[0], g_EOS);
    while(isChanged) {

        isChanged=0;

        for(int i = 0; i < numRules; i++) {
            gSymbol left_val = grammarArr[i].lhs;
            rhsNode* first = grammarArr[i].head;

            while(first != NULL) {
                gSymbol ff_val = first->s;
                if(isTerminal(ff_val) || isEpsilon(ff_val)) {
                    first = first->next;
                    continue;
                }
                rhsNode* second = first->next;

                while(1) {
                    if(second == NULL) {
                        intSet prev = followSet[ntx(ff_val)];
                        followSet[ntx(ff_val)] = union_set(followSet[ntx(ff_val)], followSet[ntx(left_val)]);
                        if(prev != followSet[ntx(ff_val)])
                            isChanged = 1;
                        break;
                    }
                    gSymbol ss_val = second->s;
                    if(isEpsilon(ss_val)) {
                        second = second->next;
                    }
                    else if(isTerminal(ss_val)) {
                        intSet prev = followSet[ntx(ff_val)];
                        followSet[ntx(ff_val)] = add_element(followSet[ntx(ff_val)], ss_val);
                        if(prev != followSet[ntx(ff_val)])
                            isChanged = 1;
                        break;
                    }
                    else {
                        intSet prev = followSet[ntx(ff_val)];
                        followSet[ntx(ff_val)] = union_set(followSet[ntx(ff_val)], firstSet[ntx(ss_val)]);
                        if(prev != followSet[ntx(ff_val)])
                            isChanged = 1;

                        if(isPresent(firstSet[ntx(ss_val)], g_EPS))
                            second = second->next;
                        else
                            break;
                    }
                }
                first = first->next;
            }
        }
    }

    for(int i = 0; i < NUM_NON_TERMINALS; i++) {
        if(isPresent(followSet[i], g_EPS))
            followSet[i] = remove_element(followSet[i],g_EPS);
    }
}
/*------------FIRST AND FOLLOW ENDS-------------*/



/*------------PARSE TABLE STARTS-------------*/

//To initialize the parse Table
void initParseTable() {
    for(int i = 0; i < NUM_NON_TERMINALS; i++) {
        for(int j = 0; j <= NUM_TERMINALS; j++)
            parseTable[i][j]= -1 * (numRules);
    }
}

//To compute the predictSet used for populating the parse table
intSet predictSet(grammarNode* g) {
    intSet mask = 0;
    gSymbol lval = g->lhs;
    rhsNode* current = g->head;
    while(current != NULL) {
        int rval = current->s;
        if(isEpsilon(rval)) {
            current = current->next;
        }
        else if(isTerminal(rval)) {
            mask = add_element(mask, rval);
            break;
        }
        else {
            mask = union_set(mask,remove_element(firstSet[ntx(rval)],g_EPS));
            if(isPresent(firstSet[ntx(rval)],g_EPS))
                current = current->next;
            else
                break;
        }
    }
    if(current == NULL) {
        mask = union_set(mask,followSet[ntx(lval)]);
    }
    return mask;
}

// returns 0 if grammar is not LL(1);
int populateParseTable() {
    initParseTable();
    for(int i = 0; i < numRules; i++) {
        intSet mask=predictSet(&grammarArr[i]);
        for(unsigned int bit = 0; bit < 64; ++bit) {
            if(isPresent(mask,bit)) {
                if(parseTable[ntx(grammarArr[i].lhs)][bit] != -numRules) {
                    fprintf(stderr, "populateParseTable: ERROR, Grammar is not LL(1). See line %d, %d of Grammar\n", i+1, parseTable[ntx(grammarArr[i].lhs)][bit] + 1);
                    return 0;
                }
                else
                    parseTable[ntx(grammarArr[i].lhs)][bit] = i;
            }
        }
    }
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


/*------------ERROR RECOVERY STARTS-----------------*/


intSet makeDefaultSynSet(){
    intSet defaultSyn = 0;
    gSymbol default_terminal[] = {g_DECLARE, g_START};
    int length = len(default_terminal);
    for(int i = 0; i < length; i++) {
        defaultSyn = add_element(defaultSyn, default_terminal[i]);
    }

    return defaultSyn;
}


void modifyParseTable_Err_Recovery(){
/*
 * in the places with -1 currently
 * [M,a] = ruleId for M->EPSILON if that is a rule
 * [M,a] = -2 if 'a' belongs to synset(M)
 *
 * Construction of synset(M):
 * union of
 * default synset: {terminals that begin higher level constructs}
 * and follow(M)
 */

    defaultSyn = 0;
    intSet currSyn = 0;
    defaultSyn = makeDefaultSynSet();
    for(gSymbol i = g_EOS + 1 ; i < g_numSymbols; i++){
        currSyn = union_set(defaultSyn, followSet[ntx(i)]);
        for(int j = 0; j < NUM_TERMINALS + 1; j++) {
            if(isPresent(currSyn, j) && parseTable[ntx(i)][j] == ERROR_RECOVERY_SKIP){
                parseTable[ntx(i)][j] = ERROR_RECOVERY_POP;
            }
        }
    }
}

/* Assumptions:
 * parse table has the last column corresponding to g_EOS
 * even on multiple calls, getNextToken gives NULL when input has reached end
 * whenever I am in else of recoverNonTerminal_Terminal, tkinfo is not NULL (i.e. sym == tkinfo->type)
 */
void popSafe(treeNodePtr_stack **parseStack){
    /*
     * pops the top of stack after setting the fields in topNode struct so that the topNode becomes a leaf
     * in parse tree regardless of it being a terminal or a non-terminal
     */
    treeNode *topNode = treeNodePtr_stack_top(*parseStack);
    topNode->tkinfo = NULL;
    topNode->child = NULL;
    if(ERROR_RECOVERY_VERBOSE)
        fprintf(stderr,"Safely popping '%s'\n",inverseMappingTable[topNode->tk]);
    treeNodePtr_stack_pop(*parseStack);
}
void recoverNonTerminal_Terminal(treeNodePtr_stack **parseStack, FILE **srcFilePtr, tokenInfo **tkinfo, bool *eosEncountered){
    treeNode *topNode = treeNodePtr_stack_top(*parseStack);

    if (*tkinfo == NULL){// can't skip tokens in the input -- need to keep popping the parseStack
        /* if we have a non-terminal on top of stack and we have reached the end of input string
         * then we simply pop the stack after setting the values in the topNode struct for correct
         * termination and printing of parse tree and set 'eosEncountered' to 'true'
         */

        error tmp;
        tmp.errType = STACK_NON_EMPTY;
        foundNewError(tmp);
        while(topNode->tk != g_EOS){
            popSafe(parseStack);
            topNode = treeNodePtr_stack_top(*parseStack);
        }
        *eosEncountered = true;
    }
    else {
        error e = {SYNTAX_NTT,(*tkinfo)->lno};
        e.edata.se.tkinfo = *tkinfo;
        e.edata.se.stackTopSymbol = topNode->tk;
        foundNewError(e);
        // can skip tokens in the input
        // skipping tokens code goes here
        /*
         *
         * if the topNode generates EPSILON (=> put relevant rule in parseTable), apply the topNode -> EPSILON rule and resume parsing
         * construct the syn set of topNode as default syn set and followSet of topNode
         * getNextToken (skip one token initially) and keep getting next token until:
         * you get a token in first(topNode) (=> fill -1 in parseTable i.e. no modification needed): resume parsing (don't pop) i.e. return
         * you get a token in the syn set of topNode (=> fill -2 in parseTable): pop and resume parsing i.e. return
         *
         * The above is achieved by using the modified parse table:
         * if pTb[M,a] is -1, skip 'a' from input (i.e. getNextToken)
         * if pTb[M,a] is -2, popSafe from the stack (don't get next token)
         */
        if(parseTable[ntx(topNode->tk)][(*tkinfo)->type] == ERROR_RECOVERY_SKIP){
            // the following wrapping if might be unnecessary since whenever we are here this if would always result in 'true'
            if(ERROR_RECOVERY_VERBOSE)
                fprintf(stderr,"Skipping token '%s'.\n",(*tkinfo)->lexeme);
            free(*tkinfo);
            if(!(*eosEncountered)) {
                *tkinfo = getNextToken(*srcFilePtr);
                if (*tkinfo == NULL) {
                    *eosEncountered = true;
                }
            }
        }
        else{
            popSafe(parseStack);
        }

    }
}
void recoverTerminal_Terminal(treeNodePtr_stack **parseStack, FILE **srcFilePtr, tokenInfo **tkinfo, bool *eosEncountered){
/* Assumption: The syn set of terminals is taken as all the other tokens.
 * So we simply pop this terminal from stack and continue parsing from that point.
 * This makes sense since there's nothing we can do if a terminal on top of stack doesn't match the one in the input.
 * All we can do is pop it and hope that we can resume parsing normally henceforth.
 */
    treeNode *topNode = treeNodePtr_stack_top(*parseStack);
    if(*tkinfo == NULL){
        //input is over
        error tmp;
        tmp.errType = STACK_NON_EMPTY;
        foundNewError(tmp);
        while(topNode->tk != g_EOS){
            popSafe(parseStack);
            topNode = treeNodePtr_stack_top(*parseStack);
        }
        *eosEncountered = true;
        return;
    }

    error e = {SYNTAX_TT,(*tkinfo)->lno};
    e.edata.se.tkinfo = *tkinfo;
    e.edata.se.stackTopSymbol = topNode->tk;
    foundNewError(e);
    if(topNode->tk == g_EOS){
        /* If the terminal on top of stck is $, we must end parsing
         * Making 'eosEncountered' as 'true' and returning takes care of terminating the parsing
         * We don't need to pop from parseStack in this case
         */
        *eosEncountered = true;
        return;
        // after we return, this will result in a case corresponding to terminal-terminal match
        // (although it may not be a match in actual) followed by termination of parsing
    }
    /* following includes the case when *tkinfo == NULL
     * i.e. when input string has been read till end
     */
    else { // top of stack is not '$'
        popSafe(parseStack);
    }
}



/*------------ERROR RECOVERY ENDS-------------------*/


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
    initErrorStack();

    while(1){
        /* ensure that whenever you land here, tkinfo is never NULL
         * This is ensured by checking if tkinfo = getNextToken is NULL whenever we call it
         * and if that is NULL making eosEncountered = true so that the following ternary statement does not break
         */
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
            recoverTerminal_Terminal(&parseStack, &srcFilePtr, &tkinfo, &eosEncountered);
        }
        else{
            //topNode is a non Terminal
            int ruleId = parseTable[ntx(topNode->tk)][sym];
            if(ruleId == ERROR_RECOVERY_SKIP || ruleId == ERROR_RECOVERY_POP){
                //Invalid Combination
                errorFree = false;
                recoverNonTerminal_Terminal(&parseStack, &srcFilePtr, &tkinfo, &eosEncountered);
            }
            else{
                grammarNode gNode = grammarArr[ruleId];
                if(gNode.lhs != topNode->tk){
                    //Report Unexpected Error
                    fprintf(stderr,"SYNTAX ANALYSER, Unexpected Error: Either the grammar was incorrectly built or the parse table was wrongly computed.\n");
                }
                treeNodePtr_stack_pop(parseStack);
                rhsNode *rhsPtr = gNode.head;
                treeNode *currChild = NULL;
                while(rhsPtr != NULL){
                    /* since in the newTreeNode(...) we set the 'next' pointer to NULL,
                     * we need not check here to find when have we encountered the last rhsNode
                     * and we can simply terminate as soon as rhsPtr becomes NULL.
                     * Had we not set 'next' to NULL in the newTreeNode(...) function, we would need to find
                     * when we encounter the last rhsNode and set its 'next' to NULL before pushing it on the tmpStack
                     * or we would have done so after this loop had terminated (since at that time the last rhsNode would be on
                     * top of tmpStack)
                     */
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
    else{
        printAllErrors();
    }
    destroyErrorStack();
    treeNodePtr_stack_del_head(parseStack);
    treeNodePtr_stack_del_head(tmpStack);
    return parseTreeRoot;
}

void destroyTree(treeNode *root){
    if(root == NULL)
        return;
    treeNode* child=root->child;
    while(child != NULL) {
        treeNode *tmp = child->next;
        destroyTree(child);
        child=tmp;
    }
    if(root->tkinfo)
        free(root->tkinfo);
    free(root);
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
    if(isLeaf && ptr->tk != g_EPS && ptr->tkinfo != NULL){
        fprintf(fp,"%-21s",ptr->tkinfo->lexeme);
        fprintf(fp,"%-15u",ptr->tkinfo->lno);
    }
    else
        fprintf(fp,"%-21s%-15s",blank,blank);

    if(isTerminal(ptr->tk))
        fprintf(fp,"%-25s",inverseMappingTable[ptr->tk]);
    else
        fprintf(fp,"%-25s",blank);

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
    destroyTree(root);
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

