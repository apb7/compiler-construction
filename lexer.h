struct token {
    int type;
    union {
        int num;
        float rnum;
        char *lexeme;
    } value;
};

struct tokenNode {
    struct token *tk;
    struct tokenNode *next;
};

struct tokenQueue {
    int size;
    struct tokenNode *first;
    struct tokenNOde *last;
};

struct tokenNode* pop_front();

void push_back(struct token *tk);

void add_num(int n);
void add_rnum(float f);
void add_lexeme(char *lex);

char* get_block(FILE *fp);

void DFA(char *buffer); //static line no

void fill_keywords();

typedef enum {

} tokenType;
