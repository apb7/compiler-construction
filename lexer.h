// struct tokenNode* pop_front();

// void push_back(struct token *tk);

// void add_num(int n);
// void add_rnum(float f);
// void add_lexeme(char *lex);

// char* get_block(FILE *fp);

// void DFA(char *buffer); //static line no

// void fill_keywords();

FILE* getStream(FILE *fp);
struct tokenInfo* getNextToken(char *buffer);
void removeComments(char *testcaseFile, char *cleanFile);
