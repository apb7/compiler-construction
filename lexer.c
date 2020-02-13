#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef unsigned int uint;

uint BUFFER_SIZE = 512;
uint line_number = 0;

char* getStream(FILE *fp) {
    if(feof(fp))
        return NULL;

    char *buffer = (char*) calloc(BUFFER_SIZE, sizeof(char));
    fread(buffer, BUFFER_SIZE, sizeof(char), fp);
    return buffer;
}

void removeComments(char *testcaseFile, char *cleanFile) {
    FILE *fp_testcaseFile = fopen(testcaseFile, "r");

    if(fp_testcaseFile == NULL) {
        printf("ERROR: Failed to open %s", testcaseFile);
        return;
    }

    FILE *fp_cleanFile = fopen(cleanFile, "w");

    if(fp_cleanFile == NULL) {
        printf("ERROR: Failed to open %s", cleanFile);
        return;
    }

    char *buffer = getStream(fp_testcaseFile);
    char *buffer_to_write = (char*) malloc(BUFFER_SIZE * sizeof(char));

    int isComment = 0, wasAsterisk = 0;

    while(buffer) {
        uint i = 0, j = 0;

        if(wasAsterisk && buffer[0] == '*'){
            isComment ^= 1;
            i++;
        }

        for(; i < BUFFER_SIZE && buffer[i] != '\0'; i++){

            if(buffer[i] == '*' && (i < BUFFER_SIZE - 1 && buffer[i+1] == '*')){
                isComment ^= 1;
                i += 2;
            }

            if(!isComment)
                buffer_to_write[j++] = buffer[i];
        }

        fwrite(buffer_to_write, sizeof(char), j, fp_cleanFile);

        if(buffer[BUFFER_SIZE - 1] == '*')
            wasAsterisk = 1;
        else
            wasAsterisk = 0;

        free(buffer);
        buffer = getStream(fp_testcaseFile);
    }
}

struct tokenInfo* getNextToken(FILE *fp) {
    char *buffer = getStream(fp);

    while(buffer) {
        uint i;

        for(i=0; i<BUFFER_SIZE && buffer[i] != '\0';) {
            
        }
    }
}

int main() {
    removeComments("abc.txt", "abc1.txt");

    return 0;
}