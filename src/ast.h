#ifndef AST_H
#define AST_H

/* Arvore sintatica abstrata (AST) construida pelo parser e percorrida pelo
   interpretador. */

typedef enum {
    N_PROGRAM, N_BLOCK,
    N_INT, N_FLOAT, N_STR, N_BOOL, N_NONE, N_VAR,
    N_BINOP, N_UNOP, N_BOOLOP, N_COMPARE, N_CMPITEM, N_NOT,
    N_ASSIGN,
    N_IF, N_WHILE,
    N_CALL,
    N_PASS, N_BREAK, N_CONTINUE,
    N_EXPRSTMT
} NodeType;

typedef struct Node {
    NodeType type;
    long ival;
    double dval;
    char *sval;
    struct Node *a, *b, *c;
    struct Node *next; /* encadeamento: statements e argumentos */
} Node;

/* Cria um no vazio do tipo t. */
Node *mknode(NodeType t);

/* Acrescenta item ao fim da lista encadeada por next e devolve o inicio. */
Node *append_list(Node *list, Node *item);

#endif
