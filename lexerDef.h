#ifndef lexerDef
#define lexerDef

typedef unsigned int uint;
typedef enum {
    INTEGER, REAL, BOOLEAN, OF, ARRAY, START, END, DECLARE, MODULE, DRIVER,
    PROGRAM, DRIVERDEF, DRIVERENDDEF, GET_VALUE, PRINT, USE, WITH, PARAMETERS,
    TRUE, FALSE, TAKES, INPUT, RETURNS, AND, OR, FOR, IN, SWITCH, CASE, BREAK,
    DEFAULT, WHILE, PLUS, MINUS, MUL, DIV, LT, LE, GE, GT, EQ, NE, DEF, ENDDEF,
    COLON, RANGEOP, SEMICOL, COMMA, ASSIGNOP, SQBO, SQBC, BO, BC, COMMENTMARK,
    ID, NUM, RNUM
} tokenType;

struct tokenInfo {
    tokenType type;
    int lno;
    union {
        int num;
        float rnum;
        char lexeme[21];
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
