//#include "lexer.h"
#include "parserDef.h"
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

#define MAX_LINE_LEN 150
#define rrx(y) y - g_EPS - 1
extern grammarNode *G;
int numRules;
ruleRange rule_range[nt_numNonTerminals];

/*
 * Some helper functions
 * TODO: Later, we can move them to a separate file
 */


char *allocString(int size){
    char *cstr = (char *)(calloc(size+1,sizeof(char)));
    return cstr;
}

/*
 * Tests whether strings s1 and s2 are equal
 */
int equals(char *s1, char *s2){
    if(s1 == NULL && s2 == NULL)
        return true;
    else if(s1 == NULL || s2 == NULL)
        return false;
    else
        return (strcmp(s1,s2) == 0);
}

char * trim (char *str) { // remove leading and trailing spaces
    str = strdup(str);
    if(str == NULL)
        return NULL;
    int begin = 0, end = strlen(str) -1, i;
    while (isspace (str[begin])){
        begin++;
    }

    if (str[begin] != '\0'){
        while (isspace (str[end]) || str[end] == '\n'){
            end--;
        }
    }
    str[end + 1] = '\0';

    return str+begin;
}

/*
Count the number of occurrences of tk in str
*/
int numTk(char *str,char tk){
    int i;
    int len = strlen(str);
    int cnt = 0;
    for(i=0;i<len;i++)
        if(str[i] == tk)
            cnt++;
    return cnt;
}

/*
 * Split a string and put into an array based on the given delimiter
 */
char **strSplit(char *str, char tk){
    if(equals(str,"") || equals(str,NULL))
        return NULL;
    char tkn[] = {tk};
    int sz = numTk(str,tk) + 2;
    char **arr = (char **)(calloc(sz,sizeof(char *)));
    char *tmpstr = strdup(str);
    int i = 0;
    char *token;
    while((token = strsep(&tmpstr,tkn)))
        arr[i++] = token;
    arr[i] = NULL;
    return arr;
}

/*
 * End of Helper functions
 */

rhsNode *createRhsNode(char *rhsTk){
    rhsNode *rhs = (rhsNode *) malloc(sizeof(rhsNode));
    rhs->s = 0; //change this to mapping_func(rhsTk)
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
    gnode.lhs = 0;  //change this to mapping_func(ruleArr[0])
    gnode.head = createRhsNode(ruleArr[1]);
    int i = 2;
    rhsNode *tmp = gnode.head;
    while(ruleArr[i] != NULL){
        tmp->next = createRhsNode(ruleArr[i]);
        tmp = tmp->next;
        i++;
    }
    return gnode;
}

/*
 * Example for grammar file
 * 3
 * program,moduleDeclarations,otherModules,driverModule,otherModules
 * moduleDeclarations,moduleDeclaration,moduleDeclarations
 * moduleDeclarations,EPSILON
 */
void populateGrammarStruct(char *grFile){
    G = NULL;
    if(!grFile)
        return;
    FILE *fp = fopen(grFile,"r");

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
            rule_range[rrx(G[i-1].lhs)].end = i-1;
            rule_range[rrx(G[i].lhs)].start = i;
        }
        else if(i == 0){
            rule_range[rrx(G[i].lhs)].start = i;
        }
        free(tRule);
    }
    rule_range[rrx(G[numRules-1].lhs)].end = numRules - 1;

}




//set elt bit
unsigned long long add_elt(unsigned long long s, unsigned long long elt) {return s|(1UL<<elt);}

//unset elt bit
unsigned long long remove_elt(unsigned long long s, unsigned long long elt) {return s&(~(1UL<<elt));}

//returns 0 if elt bit is 0
int isPresent(unsigned long long s, unsigned long long elt) {return s&(1UL<<elt);}

// union of 2 sets
unsigned long long union_set(unsigned long long s1, unsigned long long s2) {return (s1|s2);}

//intersection of 2 sets
unsigned long long intersect_set(unsigned long long s1, unsigned long long s2) {return (s1&s2);}


unsigned long long* firstSet;
unsigned long long* followSet;
int size() {
    if(sizeof(G)==0) return 0;
    return sizeof(G)/sizeof(G[0]);
}

int isEpsilon(gSymbol symbol) {
    return (symbol==g_EPS?1:0);
}

int isTerminal(gSymbol symbol) {
    return (symbol<g_EPS-1?1:0);
}

int isNonTerminal(gSymbol symbol) {
    return ((symbol>g_EPS && symbol<g_numSymbols)?1:0);
}

void first_set() {
    int n=size();
    int isChanged=0;
    int nonTerminal_count=g_numSymbols-g_EPS-1;
    firstSet = (unsigned long long*)malloc(nonTerminal_count * sizeof(unsigned long long)); 
    // unsigned long long firstSet[nonTerminal_count];
    memset(firstSet,0,sizeof(firstSet));
    //follow of topmost NT is $;
    // firstSet[0]=add_elt(firstSet[0],g_EOS);
    while(isChanged) {
        isChanged=0;
        for(int i = 0; i < n; i++) {
            gSymbol left_val=G[i].lhs;
            rhsNode* first = G[i].head;
            while(first!=NULL) {
                gSymbol ff_val=first->s;
                if(isTerminal(ff_val)) {
                    int prev=firstSet[left_val];
                    firstSet[left_val]=add_elt(firstSet[left_val],ff_val);
                    if(prev != firstSet[left_val])
                        isChanged=1;
                    break;
                } else if(isEpsilon(ff_val)) {
                    first=first->next;
                } else {
                    int prev=firstSet[left_val];
                    firstSet[left_val]=union_set(firstSet[left_val],firstSet[ff_val]);
                    if(prev != firstSet[left_val])
                        isChanged=1;
                    if(isPresent(firstSet[ff_val],g_EPS)) first=first->next;
                    else break;
                }
            }
            if(first==NULL) {
                int prev=firstSet[left_val];
                firstSet[left_val]=add_elt(firstSet[left_val],g_EPS);
                if(prev != firstSet[left_val])
                    isChanged=1;
            }
        }
    }
    for(int i = 0; i < nonTerminal_count; i++) {
        if(isPresent(firstSet[i],g_EPS))
            firstSet[i] = remove_elt(firstSet[i],g_EPS);
    }
}


//Need first set from mdrp aka proof aka refurbished
void follow_set() {
    int n=size();
    int isChanged=0;
    int nonTerminal_count=g_numSymbols-g_EPS-1;
    followSet = (unsigned long long*)malloc(nonTerminal_count * sizeof(unsigned long long)); 
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
                        int prev=followSet[ff_val];
                        followSet[ff_val]=union_set(followSet[ff_val],followSet[left_val]);
                        if(prev!=followSet[ff_val])
                            isChanged=1;
                        break;
                    }
                    gSymbol ss_val=second->s;
                    //this case should not occur
                    if(isEpsilon(ss_val)) {
                        second=second->next;
                    } else if(isTerminal(ss_val)) {
                        int prev=followSet[ff_val];
                        followSet[ff_val]=add_elt(followSet[ff_val],ss_val);
                        if(prev!=followSet[ff_val])
                            isChanged=1;
                        break;
                    } else {
                        int prev=followSet[ff_val];
                        followSet[ff_val]=union_set(followSet[ff_val],firstSet[ss_val]);
                        if(prev!=followSet[ff_val])
                            isChanged=1;
                        if(isPresent(firstSet[ss_val],g_EPS)) second=second->next; 
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

void printGrammar() {
    int n=size();
    printf("%d\n",n);
    for(int i = 0; i < n; i++) {
        printf("%d -> { ",G[i].lhs);
        // cout<<G[i].lhs<<" -> { ";
        rhsNode* start=G[i].head;
        while(start!=NULL) {
            printf("%d,", start->s);
            // cout<<start->s<<" "
            start=start->next;
        }
        printf("}\n");
    }
}


