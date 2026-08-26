%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int yylex(void);
void yyerror(const char *s);
extern int linha_atual;

/* ===================== AST ===================== */

typedef enum {
    N_PROGRAM, N_BLOCK,
    N_INT, N_FLOAT, N_STR, N_BOOL, N_NONE, N_VAR, N_LIST,
    N_BINOP, N_UNOP, N_BOOLOP, N_COMPARE, N_NOT,
    N_ASSIGN, N_AUGASSIGN,
    N_IF, N_WHILE, N_FOR,
    N_FUNCDEF, N_CALL, N_RETURN,
    N_PASS, N_BREAK, N_CONTINUE,
    N_INDEX, N_EXPRSTMT
} NodeType;

typedef struct Node {
    NodeType type;
    long ival;
    double dval;
    char *sval;
    struct Node *a, *b, *c;
    struct Node *next; /* encadeamento: statements, argumentos, parametros, itens de lista */
} Node;

static Node *mknode(NodeType t) {
    Node *n = calloc(1, sizeof(Node));
    n->type = t;
    return n;
}

static Node *append_list(Node *list, Node *item) {
    if (!list) return item;
    Node *p = list;
    while (p->next) p = p->next;
    p->next = item;
    return list;
}

/* ===================== Valores em tempo de execucao ===================== */

typedef enum { V_INT, V_FLOAT, V_STR, V_BOOL, V_NONE, V_LIST } VType;

typedef struct Value {
    VType type;
    long ival;
    double dval;
    char *sval;
    struct Value **items;
    int nitems;
} Value;

static Value *new_int(long v)   { Value *x = calloc(1,sizeof(Value)); x->type=V_INT; x->ival=v; return x; }
static Value *new_float(double v){ Value *x = calloc(1,sizeof(Value)); x->type=V_FLOAT; x->dval=v; return x; }
static Value *new_str(const char *s){ Value *x = calloc(1,sizeof(Value)); x->type=V_STR; x->sval=strdup(s); return x; }
static Value *new_bool(int b)   { Value *x = calloc(1,sizeof(Value)); x->type=V_BOOL; x->ival=b?1:0; return x; }
static Value *new_none(void)    { Value *x = calloc(1,sizeof(Value)); x->type=V_NONE; return x; }
static Value *new_list(void)    { Value *x = calloc(1,sizeof(Value)); x->type=V_LIST; return x; }

static double as_double(Value *v) {
    switch (v->type) {
        case V_INT: return (double)v->ival;
        case V_FLOAT: return v->dval;
        case V_BOOL: return (double)v->ival;
        default: return 0.0;
    }
}
static int is_floaty(Value *v) { return v->type == V_FLOAT; }
static int truthy(Value *v) {
    switch (v->type) {
        case V_INT: return v->ival != 0;
        case V_FLOAT: return v->dval != 0.0;
        case V_BOOL: return v->ival != 0;
        case V_STR: return v->sval && v->sval[0] != '\0';
        case V_NONE: return 0;
        case V_LIST: return v->nitems > 0;
    }
    return 0;
}

static void print_value(Value *v) {
    switch (v->type) {
        case V_INT: printf("%ld", v->ival); break;
        case V_FLOAT: printf("%g", v->dval); break;
        case V_STR: printf("%s", v->sval); break;
        case V_BOOL: printf("%s", v->ival ? "True" : "False"); break;
        case V_NONE: printf("None"); break;
        case V_LIST:
            printf("[");
            for (int i = 0; i < v->nitems; i++) {
                if (i) printf(", ");
                print_value(v->items[i]);
            }
            printf("]");
            break;
    }
}

/* ===================== Escopo / variaveis ===================== */

typedef struct VarEntry {
    char *name;
    Value *val;
    struct VarEntry *next;
} VarEntry;

typedef struct Scope {
    VarEntry *vars;
    struct Scope *parent;
} Scope;

static Scope *new_scope(Scope *parent) {
    Scope *s = calloc(1, sizeof(Scope));
    s->parent = parent;
    return s;
}

static Value *scope_get(Scope *s, const char *name) {
    for (Scope *cur = s; cur; cur = cur->parent) {
        for (VarEntry *e = cur->vars; e; e = e->next)
            if (strcmp(e->name, name) == 0) return e->val;
    }
    fprintf(stderr, "ERRO: variavel '%s' nao definida\n", name);
    return new_none();
}

static void scope_set(Scope *s, const char *name, Value *val) {
    for (VarEntry *e = s->vars; e; e = e->next) {
        if (strcmp(e->name, name) == 0) { e->val = val; return; }
    }
    VarEntry *e = calloc(1, sizeof(VarEntry));
    e->name = strdup(name);
    e->val = val;
    e->next = s->vars;
    s->vars = e;
}

Node *raiz = NULL;
%}

%union {
    long ival;
    double dval;
    char *str;
    struct Node *node;
}

%token <str> IDENTIFICADOR TEXTO
%token <ival> INTEIRO
%token <dval> FLUTUANTE

%token FALSE_ NONE_ TRUE_
%token AND AS ASSERT ASYNC AWAIT BREAK CLASS CONTINUE DEF DEL ELIF ELSE
%token EXCEPT FINALLY FOR FROM GLOBAL IF IMPORT IN IS LAMBDA NONLOCAL NOT
%token OR PASS RAISE RETURN TRY WHILE WITH YIELD

%token OP_MAIS OP_MENOS OP_POTENCIA OP_MULTIPLICACAO OP_DIVISAO_INTEIRA
%token OP_DIVISAO OP_MODULO OP_MATRIZ_MULT OP_DESLOC_ESQUERDA OP_DESLOC_DIREITA
%token OP_E_BIT OP_OU_BIT OP_XOU_BIT OP_NAO_BIT
%token OP_MENOR_IGUAL OP_MAIOR_IGUAL OP_IGUAL_IGUAL OP_DIFERENTE OP_MENOR OP_MAIOR
%token OP_ATRIBUICAO_MAIS OP_ATRIBUICAO_MENOS OP_ATRIBUICAO_POTENCIA
%token OP_ATRIBUICAO_MULTIPLICACAO OP_ATRIBUICAO_DIVISAO_INTEIRA OP_ATRIBUICAO_DIVISAO
%token OP_ATRIBUICAO_MODULO OP_ATRIBUICAO_E_BIT OP_ATRIBUICAO_OU_BIT OP_ATRIBUICAO_XOU_BIT
%token OP_ATRIBUICAO_DESLOC_ESQUERDA OP_ATRIBUICAO_DESLOC_DIREITA OP_ATRIBUICAO
%token PARENTESES_ESQ PARENTESES_DIR COLCHETE_ESQ COLCHETE_DIR CHAVE_ESQ CHAVE_DIR
%token VIRGULA DOIS_PONTOS PONTO PONTO_E_VIRGULA SETA
%token NOVA_LINHA INDENT DEDENT

%type <node> program stmts stmt simple_stmt compound_stmt block
%type <node> expr_stmt pass_stmt break_stmt continue_stmt return_stmt
%type <node> if_stmt elif_chain while_stmt for_stmt funcdef_stmt
%type <node> paramlist_opt paramlist arglist_opt arglist
%type <node> expr or_test and_test not_test comparison arith_expr term factor power atom_trailer atom
%type <ival> augassign_op compare_op


%{
/* ===================== Funcoes definidas pelo usuario ===================== */

typedef struct FuncEntry {
    char *name;
    Node *def; /* N_FUNCDEF */
    struct FuncEntry *next;
} FuncEntry;

static FuncEntry *funcs = NULL;

static void register_func(const char *name, Node *def) {
    FuncEntry *f = calloc(1, sizeof(FuncEntry));
    f->name = strdup(name);
    f->def = def;
    f->next = funcs;
    funcs = f;
}

static Node *find_func(const char *name) {
    for (FuncEntry *f = funcs; f; f = f->next)
        if (strcmp(f->name, name) == 0) return f->def;
    return NULL;
}

/* ===================== Sinalizacao de break/continue/return ===================== */

typedef enum { SIG_NONE, SIG_BREAK, SIG_CONTINUE, SIG_RETURN } Signal;
static Signal signal_flag = SIG_NONE;
static Value *return_value = NULL;

/* ===================== Interpretador ===================== */

static Value *eval_expr(Node *n, Scope *sc);
static void exec_stmt(Node *n, Scope *sc);
static void exec_block(Node *block, Scope *sc);

static Value *call_range(Node *args, Scope *sc) {
    long start = 0, stop = 0, step = 1;
    int nargs = 0;
    long vals[3];
    for (Node *a = args; a; a = a->next) {
        Value *v = eval_expr(a, sc);
        vals[nargs++] = (long) as_double(v);
        if (nargs >= 3) break;
    }
    if (nargs == 1) { stop = vals[0]; }
    else if (nargs == 2) { start = vals[0]; stop = vals[1]; }
    else if (nargs == 3) { start = vals[0]; stop = vals[1]; step = vals[2]; }

    Value *lst = new_list();
    int cap = 16;
    lst->items = malloc(sizeof(Value*) * cap);
    lst->nitems = 0;
    if (step > 0) {
        for (long i = start; i < stop; i += step) {
            if (lst->nitems >= cap) { cap *= 2; lst->items = realloc(lst->items, sizeof(Value*) * cap); }
            lst->items[lst->nitems++] = new_int(i);
        }
    } else if (step < 0) {
        for (long i = start; i > stop; i += step) {
            if (lst->nitems >= cap) { cap *= 2; lst->items = realloc(lst->items, sizeof(Value*) * cap); }
            lst->items[lst->nitems++] = new_int(i);
        }
    }
    return lst;
}

static Value *call_function(const char *name, Node *args, Scope *sc) {
    if (strcmp(name, "print") == 0) {
        int first = 1;
        for (Node *a = args; a; a = a->next) {
            if (!first) printf(" ");
            print_value(eval_expr(a, sc));
            first = 0;
        }
        printf("\n");
        return new_none();
    }
    if (strcmp(name, "range") == 0) {
        return call_range(args, sc);
    }
    if (strcmp(name, "len") == 0) {
        Value *v = eval_expr(args, sc);
        if (v->type == V_LIST) return new_int(v->nitems);
        if (v->type == V_STR) return new_int((long)strlen(v->sval));
        return new_int(0);
    }
    if (strcmp(name, "int") == 0) {
        Value *v = eval_expr(args, sc);
        return new_int((long) as_double(v));
    }
    if (strcmp(name, "float") == 0) {
        Value *v = eval_expr(args, sc);
        return new_float(as_double(v));
    }
    if (strcmp(name, "str") == 0) {
        Value *v = eval_expr(args, sc);
        char buf[128];
        switch (v->type) {
            case V_INT: snprintf(buf, sizeof(buf), "%ld", v->ival); return new_str(buf);
            case V_FLOAT: snprintf(buf, sizeof(buf), "%g", v->dval); return new_str(buf);
            case V_BOOL: return new_str(v->ival ? "True" : "False");
            case V_NONE: return new_str("None");
            case V_STR: return new_str(v->sval);
            default: return new_str("[list]");
        }
    }

    Node *def = find_func(name);
    if (!def) {
        fprintf(stderr, "ERRO: funcao '%s' nao definida\n", name);
        return new_none();
    }

    Scope *fscope = new_scope(NULL); /* escopo simples: apenas locais/parametros */
    Node *param = def->a;
    Node *arg = args;
    while (param && arg) {
        scope_set(fscope, param->sval, eval_expr(arg, sc));
        param = param->next;
        arg = arg->next;
    }

    return_value = new_none();
    exec_block(def->b, fscope);
    if (signal_flag == SIG_RETURN) {
        signal_flag = SIG_NONE;
        return return_value;
    }
    return new_none();
}

static Value *eval_expr(Node *n, Scope *sc) {
    if (!n) return new_none();
    switch (n->type) {
        case N_INT: return new_int(n->ival);
        case N_FLOAT: return new_float(n->dval);
        case N_STR: return new_str(n->sval);
        case N_BOOL: return new_bool(n->ival);
        case N_NONE: return new_none();
        case N_VAR: return scope_get(sc, n->sval);
        case N_LIST: {
            Value *lst = new_list();
            int cap = 8;
            lst->items = malloc(sizeof(Value*) * cap);
            lst->nitems = 0;
            for (Node *e = n->a; e; e = e->next) {
                if (lst->nitems >= cap) { cap *= 2; lst->items = realloc(lst->items, sizeof(Value*)*cap); }
                lst->items[lst->nitems++] = eval_expr(e, sc);
            }
            return lst;
        }
        case N_INDEX: {
            Value *base = eval_expr(n->a, sc);
            Value *idx = eval_expr(n->b, sc);
            long i = (long) as_double(idx);
            if (base->type == V_LIST) {
                if (i < 0) i += base->nitems;
                if (i < 0 || i >= base->nitems) { fprintf(stderr, "ERRO: indice fora do intervalo\n"); return new_none(); }
                return base->items[i];
            }
            if (base->type == V_STR) {
                int len = strlen(base->sval);
                if (i < 0) i += len;
                if (i < 0 || i >= len) { fprintf(stderr, "ERRO: indice fora do intervalo\n"); return new_none(); }
                char buf[2] = { base->sval[i], '\0' };
                return new_str(buf);
            }
            return new_none();
        }
        case N_CALL:
            return call_function(n->sval, n->a, sc);
        case N_NOT:
            return new_bool(!truthy(eval_expr(n->a, sc)));
        case N_UNOP: {
            Value *v = eval_expr(n->a, sc);
            if (n->ival == OP_MENOS) {
                if (is_floaty(v)) return new_float(-as_double(v));
                return new_int(-(long)as_double(v));
            }
            return v; /* unario + */
        }
        case N_BOOLOP: {
            Value *l = eval_expr(n->a, sc);
            if (n->ival == AND) {
                if (!truthy(l)) return l;
                return eval_expr(n->b, sc);
            } else { /* OR */
                if (truthy(l)) return l;
                return eval_expr(n->b, sc);
            }
        }
        case N_COMPARE: {
            Value *l = eval_expr(n->a, sc);
            Value *r = eval_expr(n->b, sc);
            int cmp;
            if (l->type == V_STR && r->type == V_STR) {
                cmp = strcmp(l->sval, r->sval);
                switch (n->ival) {
                    case OP_MENOR: return new_bool(cmp < 0);
                    case OP_MAIOR: return new_bool(cmp > 0);
                    case OP_MENOR_IGUAL: return new_bool(cmp <= 0);
                    case OP_MAIOR_IGUAL: return new_bool(cmp >= 0);
                    case OP_IGUAL_IGUAL: return new_bool(cmp == 0);
                    case OP_DIFERENTE: return new_bool(cmp != 0);
                }
            }
            double a = as_double(l), b = as_double(r);
            switch (n->ival) {
                case OP_MENOR: return new_bool(a < b);
                case OP_MAIOR: return new_bool(a > b);
                case OP_MENOR_IGUAL: return new_bool(a <= b);
                case OP_MAIOR_IGUAL: return new_bool(a >= b);
                case OP_IGUAL_IGUAL: return new_bool(a == b);
                case OP_DIFERENTE: return new_bool(a != b);
            }
            return new_bool(0);
        }
        case N_BINOP: {
            Value *l = eval_expr(n->a, sc);
            Value *r = eval_expr(n->b, sc);

            if (n->ival == OP_MAIS && (l->type == V_STR || r->type == V_STR)) {
                char buf[512];
                snprintf(buf, sizeof(buf), "%s%s", l->sval ? l->sval : "", r->sval ? r->sval : "");
                return new_str(buf);
            }

            int floaty = is_floaty(l) || is_floaty(r);
            double a = as_double(l), b = as_double(r);

            switch (n->ival) {
                case OP_MAIS: return floaty ? new_float(a+b) : new_int((long)a + (long)b);
                case OP_MENOS: return floaty ? new_float(a-b) : new_int((long)a - (long)b);
                case OP_MULTIPLICACAO: return floaty ? new_float(a*b) : new_int((long)a * (long)b);
                case OP_DIVISAO: return new_float(a / b);
                case OP_DIVISAO_INTEIRA: return new_int((long) floor(a / b));
                case OP_MODULO: return floaty ? new_float(fmod(a,b)) : new_int((long)a % (long)b);
                case OP_POTENCIA: {
                    double res = pow(a, b);
                    if (!floaty && b >= 0) return new_int((long) res);
                    return new_float(res);
                }
            }
            return new_none();
        }
        default:
            return new_none();
    }
}

static void exec_block(Node *block, Scope *sc) {
    for (Node *s = block; s; s = s->next) {
        exec_stmt(s, sc);
        if (signal_flag != SIG_NONE) return;
    }
}

static void exec_stmt(Node *n, Scope *sc) {
    if (!n) return;
    switch (n->type) {
        case N_EXPRSTMT:
            eval_expr(n->a, sc);
            break;
        case N_ASSIGN:
            scope_set(sc, n->sval, eval_expr(n->a, sc));
            break;
        case N_AUGASSIGN: {
            Value *cur = scope_get(sc, n->sval);
            Value *rhs = eval_expr(n->a, sc);
            int floaty = is_floaty(cur) || is_floaty(rhs);
            double x = as_double(cur), y = as_double(rhs);
            Value *res;
            switch (n->ival) {
                case OP_ATRIBUICAO_MAIS:
                    if (cur->type == V_STR) {
                        char buf[512];
                        snprintf(buf, sizeof(buf), "%s%s", cur->sval, rhs->sval ? rhs->sval : "");
                        res = new_str(buf);
                    } else res = floaty ? new_float(x+y) : new_int((long)x+(long)y);
                    break;
                case OP_ATRIBUICAO_MENOS: res = floaty ? new_float(x-y) : new_int((long)x-(long)y); break;
                case OP_ATRIBUICAO_MULTIPLICACAO: res = floaty ? new_float(x*y) : new_int((long)x*(long)y); break;
                case OP_ATRIBUICAO_DIVISAO: res = new_float(x/y); break;
                case OP_ATRIBUICAO_DIVISAO_INTEIRA: res = new_int((long)floor(x/y)); break;
                case OP_ATRIBUICAO_MODULO: res = floaty ? new_float(fmod(x,y)) : new_int((long)x%(long)y); break;
                default: res = cur; break;
            }
            scope_set(sc, n->sval, res);
            break;
        }
        case N_IF:
            if (truthy(eval_expr(n->a, sc))) {
                exec_block(n->b, sc);
            } else if (n->c) {
                if (n->c->type == N_IF) exec_stmt(n->c, sc);
                else exec_block(n->c, sc);
            }
            break;
        case N_WHILE:
            while (truthy(eval_expr(n->a, sc))) {
                exec_block(n->b, sc);
                if (signal_flag == SIG_BREAK) { signal_flag = SIG_NONE; break; }
                if (signal_flag == SIG_CONTINUE) { signal_flag = SIG_NONE; continue; }
                if (signal_flag == SIG_RETURN) return;
            }
            break;
        case N_FOR: {
            Value *iterable = eval_expr(n->a, sc);
            if (iterable->type == V_LIST) {
                for (int i = 0; i < iterable->nitems; i++) {
                    scope_set(sc, n->sval, iterable->items[i]);
                    exec_block(n->b, sc);
                    if (signal_flag == SIG_BREAK) { signal_flag = SIG_NONE; break; }
                    if (signal_flag == SIG_CONTINUE) { signal_flag = SIG_NONE; continue; }
                    if (signal_flag == SIG_RETURN) return;
                }
            }
            break;
        }
        case N_FUNCDEF:
            register_func(n->sval, n);
            break;
        case N_RETURN:
            return_value = n->a ? eval_expr(n->a, sc) : new_none();
            signal_flag = SIG_RETURN;
            break;
        case N_BREAK:
            signal_flag = SIG_BREAK;
            break;
        case N_CONTINUE:
            signal_flag = SIG_CONTINUE;
            break;
        case N_PASS:
            break;
        default:
            break;
    }
}

%}

%%

program:
      stmts { raiz = mknode(N_PROGRAM); raiz->a = $1; }
    ;

stmts:
      stmts stmt { $$ = append_list($1, $2); }
    | stmt { $$ = $1; }
    ;

stmt:
      simple_stmt NOVA_LINHA { $$ = $1; }
    | compound_stmt { $$ = $1; }
    ;

simple_stmt:
      expr_stmt { $$ = $1; }
    | pass_stmt { $$ = $1; }
    | break_stmt { $$ = $1; }
    | continue_stmt { $$ = $1; }
    | return_stmt { $$ = $1; }
    ;

expr_stmt:
      IDENTIFICADOR OP_ATRIBUICAO expr {
          $$ = mknode(N_ASSIGN); $$->sval = $1; $$->a = $3;
      }
    | IDENTIFICADOR augassign_op expr {
          $$ = mknode(N_AUGASSIGN); $$->sval = $1; $$->ival = $2; $$->a = $3;
      }
    | expr {
          $$ = mknode(N_EXPRSTMT); $$->a = $1;
      }
    ;

augassign_op:
      OP_ATRIBUICAO_MAIS { $$ = OP_ATRIBUICAO_MAIS; }
    | OP_ATRIBUICAO_MENOS { $$ = OP_ATRIBUICAO_MENOS; }
    | OP_ATRIBUICAO_MULTIPLICACAO { $$ = OP_ATRIBUICAO_MULTIPLICACAO; }
    | OP_ATRIBUICAO_DIVISAO { $$ = OP_ATRIBUICAO_DIVISAO; }
    | OP_ATRIBUICAO_DIVISAO_INTEIRA { $$ = OP_ATRIBUICAO_DIVISAO_INTEIRA; }
    | OP_ATRIBUICAO_MODULO { $$ = OP_ATRIBUICAO_MODULO; }
    ;

pass_stmt: PASS { $$ = mknode(N_PASS); } ;
break_stmt: BREAK { $$ = mknode(N_BREAK); } ;
continue_stmt: CONTINUE { $$ = mknode(N_CONTINUE); } ;

return_stmt:
      RETURN expr { $$ = mknode(N_RETURN); $$->a = $2; }
    | RETURN { $$ = mknode(N_RETURN); }
    ;

compound_stmt:
      if_stmt { $$ = $1; }
    | while_stmt { $$ = $1; }
    | for_stmt { $$ = $1; }
    | funcdef_stmt { $$ = $1; }
    ;

if_stmt:
      IF expr DOIS_PONTOS block elif_chain {
          $$ = mknode(N_IF); $$->a = $2; $$->b = $4; $$->c = $5;
      }
    ;

elif_chain:
      ELIF expr DOIS_PONTOS block elif_chain {
          $$ = mknode(N_IF); $$->a = $2; $$->b = $4; $$->c = $5;
      }
    | ELSE DOIS_PONTOS block { $$ = $3; }
    | /* vazio */ { $$ = NULL; }
    ;

while_stmt:
      WHILE expr DOIS_PONTOS block {
          $$ = mknode(N_WHILE); $$->a = $2; $$->b = $4;
      }
    ;

for_stmt:
      FOR IDENTIFICADOR IN expr DOIS_PONTOS block {
          $$ = mknode(N_FOR); $$->sval = $2; $$->a = $4; $$->b = $6;
      }
    ;

funcdef_stmt:
      DEF IDENTIFICADOR PARENTESES_ESQ paramlist_opt PARENTESES_DIR DOIS_PONTOS block {
          $$ = mknode(N_FUNCDEF); $$->sval = $2; $$->a = $4; $$->b = $7;
      }
    ;

paramlist_opt:
      /* vazio */ { $$ = NULL; }
    | paramlist { $$ = $1; }
    ;

paramlist:
      IDENTIFICADOR {
          Node *v = mknode(N_VAR); v->sval = $1; $$ = v;
      }
    | paramlist VIRGULA IDENTIFICADOR {
          Node *v = mknode(N_VAR); v->sval = $3; $$ = append_list($1, v);
      }
    ;

block:
      NOVA_LINHA INDENT stmts DEDENT { $$ = $3; }
    | simple_stmt NOVA_LINHA { $$ = $1; }
    ;

/* ---------- expressoes (precedencia crescente) ---------- */

expr: or_test { $$ = $1; } ;

or_test:
      or_test OR and_test { $$ = mknode(N_BOOLOP); $$->ival = OR; $$->a = $1; $$->b = $3; }
    | and_test { $$ = $1; }
    ;

and_test:
      and_test AND not_test { $$ = mknode(N_BOOLOP); $$->ival = AND; $$->a = $1; $$->b = $3; }
    | not_test { $$ = $1; }
    ;

not_test:
      NOT not_test { $$ = mknode(N_NOT); $$->a = $2; }
    | comparison { $$ = $1; }
    ;

comparison:
      arith_expr compare_op arith_expr { $$ = mknode(N_COMPARE); $$->ival = $2; $$->a = $1; $$->b = $3; }
    | arith_expr { $$ = $1; }
    ;

compare_op:
      OP_MENOR { $$ = OP_MENOR; }
    | OP_MAIOR { $$ = OP_MAIOR; }
    | OP_MENOR_IGUAL { $$ = OP_MENOR_IGUAL; }
    | OP_MAIOR_IGUAL { $$ = OP_MAIOR_IGUAL; }
    | OP_IGUAL_IGUAL { $$ = OP_IGUAL_IGUAL; }
    | OP_DIFERENTE { $$ = OP_DIFERENTE; }
    ;

arith_expr:
      arith_expr OP_MAIS term { $$ = mknode(N_BINOP); $$->ival = OP_MAIS; $$->a = $1; $$->b = $3; }
    | arith_expr OP_MENOS term { $$ = mknode(N_BINOP); $$->ival = OP_MENOS; $$->a = $1; $$->b = $3; }
    | term { $$ = $1; }
    ;

term:
      term OP_MULTIPLICACAO factor { $$ = mknode(N_BINOP); $$->ival = OP_MULTIPLICACAO; $$->a = $1; $$->b = $3; }
    | term OP_DIVISAO factor { $$ = mknode(N_BINOP); $$->ival = OP_DIVISAO; $$->a = $1; $$->b = $3; }
    | term OP_DIVISAO_INTEIRA factor { $$ = mknode(N_BINOP); $$->ival = OP_DIVISAO_INTEIRA; $$->a = $1; $$->b = $3; }
    | term OP_MODULO factor { $$ = mknode(N_BINOP); $$->ival = OP_MODULO; $$->a = $1; $$->b = $3; }
    | factor { $$ = $1; }
    ;

factor:
      OP_MENOS factor { $$ = mknode(N_UNOP); $$->ival = OP_MENOS; $$->a = $2; }
    | OP_MAIS factor { $$ = mknode(N_UNOP); $$->ival = OP_MAIS; $$->a = $2; }
    | power { $$ = $1; }
    ;

power:
      atom_trailer OP_POTENCIA factor { $$ = mknode(N_BINOP); $$->ival = OP_POTENCIA; $$->a = $1; $$->b = $3; }
    | atom_trailer { $$ = $1; }
    ;

atom_trailer:
      IDENTIFICADOR PARENTESES_ESQ arglist_opt PARENTESES_DIR {
          $$ = mknode(N_CALL); $$->sval = $1; $$->a = $3;
      }
    | atom_trailer COLCHETE_ESQ expr COLCHETE_DIR {
          $$ = mknode(N_INDEX); $$->a = $1; $$->b = $3;
      }
    | atom { $$ = $1; }
    ;

atom:
      INTEIRO { $$ = mknode(N_INT); $$->ival = $1; }
    | FLUTUANTE { $$ = mknode(N_FLOAT); $$->dval = $1; }
    | TEXTO { $$ = mknode(N_STR); $$->sval = $1; }
    | TRUE_ { $$ = mknode(N_BOOL); $$->ival = 1; }
    | FALSE_ { $$ = mknode(N_BOOL); $$->ival = 0; }
    | NONE_ { $$ = mknode(N_NONE); }
    | IDENTIFICADOR { $$ = mknode(N_VAR); $$->sval = $1; }
    | PARENTESES_ESQ expr PARENTESES_DIR { $$ = $2; }
    | COLCHETE_ESQ arglist_opt COLCHETE_DIR { $$ = mknode(N_LIST); $$->a = $2; }
    ;

arglist_opt:
      /* vazio */ { $$ = NULL; }
    | arglist { $$ = $1; }
    ;

arglist:
      expr { $$ = $1; }
    | arglist VIRGULA expr { $$ = append_list($1, $3); }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "ERRO DE SINTAXE na linha %d: %s\n", linha_atual, s);
}

int main(int argc, char **argv) {
    if (argc > 1) {
        FILE *f = fopen(argv[1], "r");
        if (!f) { perror("fopen"); return 1; }
        extern FILE *yyin;
        yyin = f;
    }
    if (yyparse() == 0 && raiz) {
        Scope *global = new_scope(NULL);
        exec_block(raiz->a, global);
    }
    return 0;
}