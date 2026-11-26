%{
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

int yylex(void);
void yyerror(const char *s);
extern int linha_atual;

#include "ast.h"
#include "valores.h"
#include "tabela.h"

/* ===================== Erros de execucao ===================== */

static _Noreturn void runtime_error(const char *fmt, ...) {
    fflush(stdout);
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "ERRO: ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    exit(1);
}

/* Os inteiros do interpretador sao de 64 bits (long); o Python nao tem limite.
   Em vez de dar um resultado errado, o estouro interrompe a execucao. */
static _Noreturn void int_overflow(void) {
    runtime_error("OverflowError: inteiro fora do intervalo de 64 bits");
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
%type <node> expr_stmt pass_stmt break_stmt continue_stmt
%type <node> if_stmt while_stmt arglist_opt arglist
%type <node> expr or_test and_test not_test comparison comp_chain arith_expr term factor power atom_trailer atom
%type <ival> compare_op

%%

program:
      stmts { raiz = mknode(N_PROGRAM); raiz->a = $1; }
    | /* programa vazio */ { raiz = mknode(N_PROGRAM); }
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
    ;

expr_stmt:
      IDENTIFICADOR OP_ATRIBUICAO expr {
          $$ = mknode(N_ASSIGN); $$->sval = $1; $$->a = $3;
      }
    | expr {
          $$ = mknode(N_EXPRSTMT); $$->a = $1;
      }
    ;

pass_stmt: PASS { $$ = mknode(N_PASS); } ;
break_stmt: BREAK { $$ = mknode(N_BREAK); } ;
continue_stmt: CONTINUE { $$ = mknode(N_CONTINUE); } ;

compound_stmt:
      if_stmt { $$ = $1; }
    | while_stmt { $$ = $1; }
    ;

if_stmt:
      IF expr DOIS_PONTOS block {
          $$ = mknode(N_IF); $$->a = $2; $$->b = $4;
      }
    | IF expr DOIS_PONTOS block ELSE DOIS_PONTOS block {
          $$ = mknode(N_IF); $$->a = $2; $$->b = $4; $$->c = $7;
      }
    ;

while_stmt:
      WHILE expr DOIS_PONTOS block {
          $$ = mknode(N_WHILE); $$->a = $2; $$->b = $4;
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

/* Comparacoes podem ser encadeadas como no Python: a < b <= c equivale a
   (a < b) and (b <= c), avaliando b uma unica vez. O no N_COMPARE guarda o
   primeiro operando em a e, em b, a lista de pares (operador, operando)
   como nos N_CMPITEM encadeados por next. */
comparison:
      arith_expr comp_chain { $$ = mknode(N_COMPARE); $$->a = $1; $$->b = $2; }
    | arith_expr { $$ = $1; }
    ;

comp_chain:
      compare_op arith_expr {
          $$ = mknode(N_CMPITEM); $$->ival = $1; $$->a = $2;
      }
    | comp_chain compare_op arith_expr {
          Node *item = mknode(N_CMPITEM); item->ival = $2; item->a = $3;
          $$ = append_list($1, item);
      }
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

/* ===================== Sinalizacao de break/continue ===================== */

typedef enum { SIG_NONE, SIG_BREAK, SIG_CONTINUE } Signal;
static Signal signal_flag = SIG_NONE;

/* ===================== Interpretador ===================== */

static Value *eval_expr(Node *n, Scope *sc);
static void exec_stmt(Node *n, Scope *sc);
static void exec_block(Node *block, Scope *sc);

/* Verdadeiro se o resto da string so tem espacos (o Python aceita " 42 "). */
static int only_spaces(const char *p) {
    while (isspace((unsigned char)*p)) p++;
    return *p == '\0';
}

static const char *op_symbol(long op) {
    switch (op) {
        case OP_MAIS: return "+";
        case OP_MENOS: return "-";
        case OP_MULTIPLICACAO: return "*";
        case OP_DIVISAO: return "/";
        case OP_DIVISAO_INTEIRA: return "//";
        case OP_MODULO: return "%";
        case OP_POTENCIA: return "**";
        case OP_MENOR: return "<";
        case OP_MAIOR: return ">";
        case OP_MENOR_IGUAL: return "<=";
        case OP_MAIOR_IGUAL: return ">=";
    }
    return "?";
}

/* Resultado (0 ou 1) de l op r, com as regras do Python para cada tipo. */
static int compare_values(long op, Value *l, Value *r) {
    if (l->type == V_STR && r->type == V_STR) {
        int cmp = strcmp(l->sval, r->sval);
        switch (op) {
            case OP_MENOR: return cmp < 0;
            case OP_MAIOR: return cmp > 0;
            case OP_MENOR_IGUAL: return cmp <= 0;
            case OP_MAIOR_IGUAL: return cmp >= 0;
            case OP_IGUAL_IGUAL: return cmp == 0;
            case OP_DIFERENTE: return cmp != 0;
        }
    }
    if (!is_numeric(l) || !is_numeric(r)) {
        /* Tipos diferentes (ou None): == e != comparam identidade de
           tipo/valor; <, >, <=, >= nao sao suportados, como no Python. */
        int equal = (l->type == V_NONE && r->type == V_NONE);
        if (op == OP_IGUAL_IGUAL) return equal;
        if (op == OP_DIFERENTE) return !equal;
        runtime_error("TypeError: '%s' nao suportado entre '%s' e '%s'",
                      op_symbol(op), type_name(l), type_name(r));
    }
    double a = as_double(l), b = as_double(r);
    switch (op) {
        case OP_MENOR: return a < b;
        case OP_MAIOR: return a > b;
        case OP_MENOR_IGUAL: return a <= b;
        case OP_MAIOR_IGUAL: return a >= b;
        case OP_IGUAL_IGUAL: return a == b;
        case OP_DIFERENTE: return a != b;
    }
    return 0;
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
    if (strcmp(name, "int") == 0) {
        if (!args) return new_int(0);
        Value *v = eval_expr(args, sc);
        switch (v->type) {
            case V_INT: case V_BOOL: return new_int(v->ival);
            case V_FLOAT:
                /* -2^63 e exato em double; 2^63 ja nao cabe em long. */
                if (!(v->dval >= -9223372036854775808.0 && v->dval < 9223372036854775808.0))
                    int_overflow();
                return new_int((long) v->dval);
            case V_STR: {
                char *end;
                errno = 0;
                long x = strtol(v->sval, &end, 10);
                if (end == v->sval || !only_spaces(end))
                    runtime_error("ValueError: literal invalido para int(): '%s'", v->sval);
                if (errno == ERANGE) int_overflow();
                return new_int(x);
            }
            case V_NONE:
                runtime_error("TypeError: int() nao aceita '%s'", type_name(v));
        }
        return new_none();
    }
    if (strcmp(name, "float") == 0) {
        if (!args) return new_float(0.0);
        Value *v = eval_expr(args, sc);
        switch (v->type) {
            case V_INT: case V_BOOL: case V_FLOAT: return new_float(as_double(v));
            case V_STR: {
                char *end;
                double x = strtod(v->sval, &end);
                if (end == v->sval || !only_spaces(end))
                    runtime_error("ValueError: nao foi possivel converter para float: '%s'", v->sval);
                return new_float(x);
            }
            case V_NONE:
                runtime_error("TypeError: float() nao aceita '%s'", type_name(v));
        }
        return new_none();
    }
    if (strcmp(name, "str") == 0) {
        Value *v = eval_expr(args, sc);
        char buf[128];
        switch (v->type) {
            case V_INT: snprintf(buf, sizeof(buf), "%ld", v->ival); return new_str(buf);
            case V_FLOAT: format_float(v->dval, buf, sizeof(buf)); return new_str(buf);
            case V_BOOL: return new_str(v->ival ? "True" : "False");
            case V_NONE: return new_str("None");
            case V_STR: return new_str(v->sval);
        }
    }

    runtime_error("funcao '%s' nao definida", name);
}

static Value *eval_expr(Node *n, Scope *sc) {
    if (!n) return new_none();
    switch (n->type) {
        case N_INT: return new_int(n->ival);
        case N_FLOAT: return new_float(n->dval);
        case N_STR: return new_str(n->sval);
        case N_BOOL: return new_bool(n->ival);
        case N_NONE: return new_none();
        case N_VAR: {
            Value *v = scope_get(sc, n->sval);
            if (!v) runtime_error("variavel '%s' nao definida", n->sval);
            return v;
        }
        case N_CALL:
            return call_function(n->sval, n->a, sc);
        case N_NOT:
            return new_bool(!truthy(eval_expr(n->a, sc)));
        case N_UNOP: {
            Value *v = eval_expr(n->a, sc);
            if (!is_numeric(v))
                runtime_error("TypeError: operando invalido para '%s' unario: '%s'",
                              op_symbol(n->ival), type_name(v));
            if (n->ival == OP_MENOS) {
                if (is_floaty(v)) return new_float(-as_double(v));
                long x = as_long(v);
                if (x == LONG_MIN) int_overflow();
                return new_int(-x);
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
            /* a op1 b op2 c ...: para no primeiro par falso (curto-circuito),
               e cada operando do meio e avaliado uma unica vez */
            Value *l = eval_expr(n->a, sc);
            for (Node *item = n->b; item; item = item->next) {
                Value *r = eval_expr(item->a, sc);
                if (!compare_values(item->ival, l, r)) return new_bool(0);
                l = r;
            }
            return new_bool(1);
        }
        case N_BINOP: {
            Value *l = eval_expr(n->a, sc);
            Value *r = eval_expr(n->b, sc);

            if (n->ival == OP_MAIS && l->type == V_STR && r->type == V_STR) {
                size_t la = strlen(l->sval), lb = strlen(r->sval);
                char *buf = malloc(la + lb + 1);
                memcpy(buf, l->sval, la);
                memcpy(buf + la, r->sval, lb + 1);
                Value *res = new_str(buf);
                free(buf);
                return res;
            }

            /* Repeticao de string: str * int ou int * str (bool conta como int). */
            if (n->ival == OP_MULTIPLICACAO
                && ((l->type == V_STR && (r->type == V_INT || r->type == V_BOOL))
                    || (r->type == V_STR && (l->type == V_INT || l->type == V_BOOL)))) {
                Value *s = l->type == V_STR ? l : r;
                long times = as_long(l->type == V_STR ? r : l);
                size_t len = strlen(s->sval);
                if (times <= 0 || len == 0) return new_str("");
                if ((size_t)times > (SIZE_MAX - 1) / len)
                    runtime_error("MemoryError: string repetida grande demais");
                char *buf = malloc(len * (size_t)times + 1);
                for (long i = 0; i < times; i++) memcpy(buf + (size_t)i * len, s->sval, len);
                buf[len * (size_t)times] = '\0';
                Value *res = new_str(buf);
                free(buf);
                return res;
            }

            if (!is_numeric(l) || !is_numeric(r))
                runtime_error("TypeError: operandos invalidos para '%s': '%s' e '%s'",
                              op_symbol(n->ival), type_name(l), type_name(r));

            int floaty = is_floaty(l) || is_floaty(r);
            double a = as_double(l), b = as_double(r);

            if ((n->ival == OP_DIVISAO || n->ival == OP_DIVISAO_INTEIRA || n->ival == OP_MODULO)
                && b == 0.0)
                runtime_error("ZeroDivisionError: divisao por zero");

            switch (n->ival) {
                case OP_MAIS:
                case OP_MENOS:
                case OP_MULTIPLICACAO: {
                    if (floaty) {
                        if (n->ival == OP_MAIS) return new_float(a + b);
                        if (n->ival == OP_MENOS) return new_float(a - b);
                        return new_float(a * b);
                    }
                    long x = as_long(l), y = as_long(r), res;
                    int overflow = n->ival == OP_MAIS ? __builtin_add_overflow(x, y, &res)
                                 : n->ival == OP_MENOS ? __builtin_sub_overflow(x, y, &res)
                                 : __builtin_mul_overflow(x, y, &res);
                    if (overflow) int_overflow();
                    return new_int(res);
                }
                case OP_DIVISAO: return new_float(a / b);
                /* Python arredonda // para baixo (piso) e o resto de % tem o sinal
                   do divisor; em C, / e % truncam em direcao a zero. */
                case OP_DIVISAO_INTEIRA: {
                    if (floaty) return new_float(floor(a / b));
                    long x = as_long(l), y = as_long(r);
                    if (x == LONG_MIN && y == -1) int_overflow();
                    long q = x / y;
                    if ((x % y != 0) && ((x < 0) != (y < 0))) q--;
                    return new_int(q);
                }
                case OP_MODULO: {
                    if (floaty) {
                        double m = fmod(a, b);
                        if (m != 0.0 && ((m < 0) != (b < 0))) m += b;
                        return new_float(m);
                    }
                    long x = as_long(l), y = as_long(r);
                    if (y == -1) return new_int(0); /* LONG_MIN % -1 e indefinido em C */
                    long m = x % y;
                    if (m != 0 && ((m < 0) != (y < 0))) m += y;
                    return new_int(m);
                }
                case OP_POTENCIA: {
                    long e = as_long(r);
                    if (floaty || e < 0) return new_float(pow(a, b));
                    /* Exponenciacao inteira por quadrados, detectando estouro. */
                    long base = as_long(l), res = 1;
                    while (e > 0) {
                        if ((e & 1) && __builtin_mul_overflow(res, base, &res)) int_overflow();
                        e >>= 1;
                        if (e > 0 && __builtin_mul_overflow(base, base, &base)) int_overflow();
                    }
                    return new_int(res);
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
            }
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

void yyerror(const char *s) {
    fprintf(stderr, "ERRO DE SINTAXE na linha %d: %s\n", linha_atual, s);
}

/* ===================== Analise semantica ===================== */

/* Percorre a AST antes da execucao. Por enquanto verifica break/continue
   fora de laco, que o Python rejeita antes de executar qualquer linha. */
static int erros_semanticos = 0;

static void check_stmts(Node *s, int dentro_de_laco) {
    for (; s; s = s->next) {
        switch (s->type) {
            case N_BREAK:
            case N_CONTINUE:
                if (!dentro_de_laco) {
                    fprintf(stderr, "ERRO SEMANTICO: '%s' fora de laco\n",
                            s->type == N_BREAK ? "break" : "continue");
                    erros_semanticos++;
                }
                break;
            case N_IF:
                check_stmts(s->b, dentro_de_laco);
                check_stmts(s->c, dentro_de_laco);
                break;
            case N_WHILE:
                check_stmts(s->b, 1);
                break;
            default:
                break;
        }
    }
}

void executar_programa(void) {
    if (raiz) {
        check_stmts(raiz->a, 0);
        if (erros_semanticos > 0) exit(1);
        Scope *global = new_scope();
        exec_block(raiz->a, global);
    }
}