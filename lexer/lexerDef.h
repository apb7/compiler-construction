// Group Number: 31
// MADHUR PANWAR   2016B4A70933P
// TUSSANK GUPTA   2016B3A70528P
// SALMAAN SHAHID  2016B4A70580P
// APURV BAJAJ     2016B3A70549P
// HASAN NAQVI     2016B5A70452P

#ifndef lexerDef
#define lexerDef

typedef enum {
    #define X(a,b) a,
    #define K(a,b,c) a,
    #include "../data/keywords.txt"
    #include "../data/tokens.txt"
    COMMENTMARK
    #undef K
    #undef X
} tokenType;

struct tokenInfo {
    tokenType type;
    unsigned int lno;
    char lexeme[101];
    union {
        int num;
        float rnum;
    } value;
};

struct tokenNode {
    struct tokenInfo *tk;
    struct tokenNode *next;
};

typedef struct tokenInfo tokenInfo;
typedef struct tokenNode tokenNode;

#endif
