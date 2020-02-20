#ifndef lexerDef
#define lexerDef

// Sync with terminals.txt
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

// struct tokenQueue {
//     int size;
//     struct tokenNode *first;
//     struct tokenNOde *last;
// };

#endif
