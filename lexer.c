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
    char *buffer = (char*) calloc(BUFFER_SIZE, sizeof(char));
    uint status = fread(buffer, BUFFER_SIZE, sizeof(char), fp);

    if(status == 0) {
        free(buffer);
        return NULL;
    }

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

    while(buffer) {
        uint i,j;

        for(i=0,j=0; i < BUFFER_SIZE && buffer[i] != '\0';) {
            if(buffer[i] == '*' && (i < BUFFER_SIZE - 1 && buffer[i+1] == '*')) {
                i = i + 2;
                while(i < BUFFER_SIZE-1 && buffer[i] != '*' && buffer[i+1] != '*') {
                    buffer_to_write[j] = buffer[i];
                    ++i;
                    ++j;
                }
                i = i + 2;
            }
            else {
                ++i;
            }
        }

        fwrite(buffer_to_write, sizeof(char), j-1, fp_cleanFile);

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

