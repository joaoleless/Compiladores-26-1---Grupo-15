/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "parser/parser.y"

#define _POSIX_C_SOURCE 200809L
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
    N_INT, N_FLOAT, N_STR, N_BOOL, N_NONE, N_VAR,
    N_BINOP, N_UNOP, N_BOOLOP, N_COMPARE, N_NOT,
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

typedef enum { V_INT, V_FLOAT, V_STR, V_BOOL, V_NONE } VType;

typedef struct Value {
    VType type;
    long ival;
    double dval;
    char *sval;
} Value;

static Value *new_int(long v)    { Value *x = calloc(1,sizeof(Value)); x->type=V_INT; x->ival=v; return x; }
static Value *new_float(double v){ Value *x = calloc(1,sizeof(Value)); x->type=V_FLOAT; x->dval=v; return x; }
static Value *new_str(const char *s){ Value *x = calloc(1,sizeof(Value)); x->type=V_STR; x->sval=strdup(s); return x; }
static Value *new_bool(int b)    { Value *x = calloc(1,sizeof(Value)); x->type=V_BOOL; x->ival=b?1:0; return x; }
static Value *new_none(void)     { Value *x = calloc(1,sizeof(Value)); x->type=V_NONE; return x; }

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
    }
    return 0;
}

/* Formata um double como o Python: menor representacao que preserva o valor,
   sempre com ponto decimal (ex.: 2.0, 0.1, 1e+16). */
static void format_float(double d, char *buf, size_t size) {
    for (int prec = 1; prec <= 17; prec++) {
        snprintf(buf, size, "%.*g", prec, d);
        if (strtod(buf, NULL) == d) break;
    }
    if (!strpbrk(buf, ".einEIN")) {
        size_t n = strlen(buf);
        if (n + 2 < size) { buf[n] = '.'; buf[n + 1] = '0'; buf[n + 2] = '\0'; }
    }
}

static void print_value(Value *v) {
    switch (v->type) {
        case V_INT: printf("%ld", v->ival); break;
        case V_FLOAT: { char buf[64]; format_float(v->dval, buf, sizeof(buf)); printf("%s", buf); break; }
        case V_STR: printf("%s", v->sval); break;
        case V_BOOL: printf("%s", v->ival ? "True" : "False"); break;
        case V_NONE: printf("None"); break;
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
} Scope;

static Scope *new_scope(void) {
    return calloc(1, sizeof(Scope));
}

static Value *scope_get(Scope *s, const char *name) {
    for (VarEntry *e = s->vars; e; e = e->next)
        if (strcmp(e->name, name) == 0) return e->val;
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

#line 217 "parser.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "parser.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_IDENTIFICADOR = 3,              /* IDENTIFICADOR  */
  YYSYMBOL_TEXTO = 4,                      /* TEXTO  */
  YYSYMBOL_INTEIRO = 5,                    /* INTEIRO  */
  YYSYMBOL_FLUTUANTE = 6,                  /* FLUTUANTE  */
  YYSYMBOL_FALSE_ = 7,                     /* FALSE_  */
  YYSYMBOL_NONE_ = 8,                      /* NONE_  */
  YYSYMBOL_TRUE_ = 9,                      /* TRUE_  */
  YYSYMBOL_AND = 10,                       /* AND  */
  YYSYMBOL_AS = 11,                        /* AS  */
  YYSYMBOL_ASSERT = 12,                    /* ASSERT  */
  YYSYMBOL_ASYNC = 13,                     /* ASYNC  */
  YYSYMBOL_AWAIT = 14,                     /* AWAIT  */
  YYSYMBOL_BREAK = 15,                     /* BREAK  */
  YYSYMBOL_CLASS = 16,                     /* CLASS  */
  YYSYMBOL_CONTINUE = 17,                  /* CONTINUE  */
  YYSYMBOL_DEF = 18,                       /* DEF  */
  YYSYMBOL_DEL = 19,                       /* DEL  */
  YYSYMBOL_ELIF = 20,                      /* ELIF  */
  YYSYMBOL_ELSE = 21,                      /* ELSE  */
  YYSYMBOL_EXCEPT = 22,                    /* EXCEPT  */
  YYSYMBOL_FINALLY = 23,                   /* FINALLY  */
  YYSYMBOL_FOR = 24,                       /* FOR  */
  YYSYMBOL_FROM = 25,                      /* FROM  */
  YYSYMBOL_GLOBAL = 26,                    /* GLOBAL  */
  YYSYMBOL_IF = 27,                        /* IF  */
  YYSYMBOL_IMPORT = 28,                    /* IMPORT  */
  YYSYMBOL_IN = 29,                        /* IN  */
  YYSYMBOL_IS = 30,                        /* IS  */
  YYSYMBOL_LAMBDA = 31,                    /* LAMBDA  */
  YYSYMBOL_NONLOCAL = 32,                  /* NONLOCAL  */
  YYSYMBOL_NOT = 33,                       /* NOT  */
  YYSYMBOL_OR = 34,                        /* OR  */
  YYSYMBOL_PASS = 35,                      /* PASS  */
  YYSYMBOL_RAISE = 36,                     /* RAISE  */
  YYSYMBOL_RETURN = 37,                    /* RETURN  */
  YYSYMBOL_TRY = 38,                       /* TRY  */
  YYSYMBOL_WHILE = 39,                     /* WHILE  */
  YYSYMBOL_WITH = 40,                      /* WITH  */
  YYSYMBOL_YIELD = 41,                     /* YIELD  */
  YYSYMBOL_OP_MAIS = 42,                   /* OP_MAIS  */
  YYSYMBOL_OP_MENOS = 43,                  /* OP_MENOS  */
  YYSYMBOL_OP_POTENCIA = 44,               /* OP_POTENCIA  */
  YYSYMBOL_OP_MULTIPLICACAO = 45,          /* OP_MULTIPLICACAO  */
  YYSYMBOL_OP_DIVISAO_INTEIRA = 46,        /* OP_DIVISAO_INTEIRA  */
  YYSYMBOL_OP_DIVISAO = 47,                /* OP_DIVISAO  */
  YYSYMBOL_OP_MODULO = 48,                 /* OP_MODULO  */
  YYSYMBOL_OP_MATRIZ_MULT = 49,            /* OP_MATRIZ_MULT  */
  YYSYMBOL_OP_DESLOC_ESQUERDA = 50,        /* OP_DESLOC_ESQUERDA  */
  YYSYMBOL_OP_DESLOC_DIREITA = 51,         /* OP_DESLOC_DIREITA  */
  YYSYMBOL_OP_E_BIT = 52,                  /* OP_E_BIT  */
  YYSYMBOL_OP_OU_BIT = 53,                 /* OP_OU_BIT  */
  YYSYMBOL_OP_XOU_BIT = 54,                /* OP_XOU_BIT  */
  YYSYMBOL_OP_NAO_BIT = 55,                /* OP_NAO_BIT  */
  YYSYMBOL_OP_MENOR_IGUAL = 56,            /* OP_MENOR_IGUAL  */
  YYSYMBOL_OP_MAIOR_IGUAL = 57,            /* OP_MAIOR_IGUAL  */
  YYSYMBOL_OP_IGUAL_IGUAL = 58,            /* OP_IGUAL_IGUAL  */
  YYSYMBOL_OP_DIFERENTE = 59,              /* OP_DIFERENTE  */
  YYSYMBOL_OP_MENOR = 60,                  /* OP_MENOR  */
  YYSYMBOL_OP_MAIOR = 61,                  /* OP_MAIOR  */
  YYSYMBOL_OP_ATRIBUICAO_MAIS = 62,        /* OP_ATRIBUICAO_MAIS  */
  YYSYMBOL_OP_ATRIBUICAO_MENOS = 63,       /* OP_ATRIBUICAO_MENOS  */
  YYSYMBOL_OP_ATRIBUICAO_POTENCIA = 64,    /* OP_ATRIBUICAO_POTENCIA  */
  YYSYMBOL_OP_ATRIBUICAO_MULTIPLICACAO = 65, /* OP_ATRIBUICAO_MULTIPLICACAO  */
  YYSYMBOL_OP_ATRIBUICAO_DIVISAO_INTEIRA = 66, /* OP_ATRIBUICAO_DIVISAO_INTEIRA  */
  YYSYMBOL_OP_ATRIBUICAO_DIVISAO = 67,     /* OP_ATRIBUICAO_DIVISAO  */
  YYSYMBOL_OP_ATRIBUICAO_MODULO = 68,      /* OP_ATRIBUICAO_MODULO  */
  YYSYMBOL_OP_ATRIBUICAO_E_BIT = 69,       /* OP_ATRIBUICAO_E_BIT  */
  YYSYMBOL_OP_ATRIBUICAO_OU_BIT = 70,      /* OP_ATRIBUICAO_OU_BIT  */
  YYSYMBOL_OP_ATRIBUICAO_XOU_BIT = 71,     /* OP_ATRIBUICAO_XOU_BIT  */
  YYSYMBOL_OP_ATRIBUICAO_DESLOC_ESQUERDA = 72, /* OP_ATRIBUICAO_DESLOC_ESQUERDA  */
  YYSYMBOL_OP_ATRIBUICAO_DESLOC_DIREITA = 73, /* OP_ATRIBUICAO_DESLOC_DIREITA  */
  YYSYMBOL_OP_ATRIBUICAO = 74,             /* OP_ATRIBUICAO  */
  YYSYMBOL_PARENTESES_ESQ = 75,            /* PARENTESES_ESQ  */
  YYSYMBOL_PARENTESES_DIR = 76,            /* PARENTESES_DIR  */
  YYSYMBOL_COLCHETE_ESQ = 77,              /* COLCHETE_ESQ  */
  YYSYMBOL_COLCHETE_DIR = 78,              /* COLCHETE_DIR  */
  YYSYMBOL_CHAVE_ESQ = 79,                 /* CHAVE_ESQ  */
  YYSYMBOL_CHAVE_DIR = 80,                 /* CHAVE_DIR  */
  YYSYMBOL_VIRGULA = 81,                   /* VIRGULA  */
  YYSYMBOL_DOIS_PONTOS = 82,               /* DOIS_PONTOS  */
  YYSYMBOL_PONTO = 83,                     /* PONTO  */
  YYSYMBOL_PONTO_E_VIRGULA = 84,           /* PONTO_E_VIRGULA  */
  YYSYMBOL_SETA = 85,                      /* SETA  */
  YYSYMBOL_NOVA_LINHA = 86,                /* NOVA_LINHA  */
  YYSYMBOL_INDENT = 87,                    /* INDENT  */
  YYSYMBOL_DEDENT = 88,                    /* DEDENT  */
  YYSYMBOL_YYACCEPT = 89,                  /* $accept  */
  YYSYMBOL_program = 90,                   /* program  */
  YYSYMBOL_stmts = 91,                     /* stmts  */
  YYSYMBOL_stmt = 92,                      /* stmt  */
  YYSYMBOL_simple_stmt = 93,               /* simple_stmt  */
  YYSYMBOL_expr_stmt = 94,                 /* expr_stmt  */
  YYSYMBOL_pass_stmt = 95,                 /* pass_stmt  */
  YYSYMBOL_break_stmt = 96,                /* break_stmt  */
  YYSYMBOL_continue_stmt = 97,             /* continue_stmt  */
  YYSYMBOL_compound_stmt = 98,             /* compound_stmt  */
  YYSYMBOL_if_stmt = 99,                   /* if_stmt  */
  YYSYMBOL_while_stmt = 100,               /* while_stmt  */
  YYSYMBOL_block = 101,                    /* block  */
  YYSYMBOL_expr = 102,                     /* expr  */
  YYSYMBOL_or_test = 103,                  /* or_test  */
  YYSYMBOL_and_test = 104,                 /* and_test  */
  YYSYMBOL_not_test = 105,                 /* not_test  */
  YYSYMBOL_comparison = 106,               /* comparison  */
  YYSYMBOL_compare_op = 107,               /* compare_op  */
  YYSYMBOL_arith_expr = 108,               /* arith_expr  */
  YYSYMBOL_term = 109,                     /* term  */
  YYSYMBOL_factor = 110,                   /* factor  */
  YYSYMBOL_power = 111,                    /* power  */
  YYSYMBOL_atom_trailer = 112,             /* atom_trailer  */
  YYSYMBOL_atom = 113,                     /* atom  */
  YYSYMBOL_arglist_opt = 114,              /* arglist_opt  */
  YYSYMBOL_arglist = 115                   /* arglist  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  48
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   185

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  89
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  27
/* YYNRULES -- Number of rules.  */
#define YYNRULES  65
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  98

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   343


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   184,   184,   185,   189,   190,   194,   195,   199,   200,
     201,   202,   206,   209,   214,   215,   216,   219,   220,   224,
     227,   233,   239,   240,   245,   248,   249,   253,   254,   258,
     259,   263,   264,   268,   269,   270,   271,   272,   273,   277,
     278,   279,   283,   284,   285,   286,   287,   291,   292,   293,
     297,   298,   302,   305,   309,   310,   311,   312,   313,   314,
     315,   316,   320,   321,   325,   326
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "IDENTIFICADOR",
  "TEXTO", "INTEIRO", "FLUTUANTE", "FALSE_", "NONE_", "TRUE_", "AND", "AS",
  "ASSERT", "ASYNC", "AWAIT", "BREAK", "CLASS", "CONTINUE", "DEF", "DEL",
  "ELIF", "ELSE", "EXCEPT", "FINALLY", "FOR", "FROM", "GLOBAL", "IF",
  "IMPORT", "IN", "IS", "LAMBDA", "NONLOCAL", "NOT", "OR", "PASS", "RAISE",
  "RETURN", "TRY", "WHILE", "WITH", "YIELD", "OP_MAIS", "OP_MENOS",
  "OP_POTENCIA", "OP_MULTIPLICACAO", "OP_DIVISAO_INTEIRA", "OP_DIVISAO",
  "OP_MODULO", "OP_MATRIZ_MULT", "OP_DESLOC_ESQUERDA", "OP_DESLOC_DIREITA",
  "OP_E_BIT", "OP_OU_BIT", "OP_XOU_BIT", "OP_NAO_BIT", "OP_MENOR_IGUAL",
  "OP_MAIOR_IGUAL", "OP_IGUAL_IGUAL", "OP_DIFERENTE", "OP_MENOR",
  "OP_MAIOR", "OP_ATRIBUICAO_MAIS", "OP_ATRIBUICAO_MENOS",
  "OP_ATRIBUICAO_POTENCIA", "OP_ATRIBUICAO_MULTIPLICACAO",
  "OP_ATRIBUICAO_DIVISAO_INTEIRA", "OP_ATRIBUICAO_DIVISAO",
  "OP_ATRIBUICAO_MODULO", "OP_ATRIBUICAO_E_BIT", "OP_ATRIBUICAO_OU_BIT",
  "OP_ATRIBUICAO_XOU_BIT", "OP_ATRIBUICAO_DESLOC_ESQUERDA",
  "OP_ATRIBUICAO_DESLOC_DIREITA", "OP_ATRIBUICAO", "PARENTESES_ESQ",
  "PARENTESES_DIR", "COLCHETE_ESQ", "COLCHETE_DIR", "CHAVE_ESQ",
  "CHAVE_DIR", "VIRGULA", "DOIS_PONTOS", "PONTO", "PONTO_E_VIRGULA",
  "SETA", "NOVA_LINHA", "INDENT", "DEDENT", "$accept", "program", "stmts",
  "stmt", "simple_stmt", "expr_stmt", "pass_stmt", "break_stmt",
  "continue_stmt", "compound_stmt", "if_stmt", "while_stmt", "block",
  "expr", "or_test", "and_test", "not_test", "comparison", "compare_op",
  "arith_expr", "term", "factor", "power", "atom_trailer", "atom",
  "arglist_opt", "arglist", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-76)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int8 yypact[] =
{
      62,   -37,   -76,   -76,   -76,   -76,   -76,   -76,   -76,   -76,
     103,   103,   -76,   103,   110,   110,   103,     8,    62,   -76,
     -75,   -76,   -76,   -76,   -76,   -76,   -76,   -76,   -76,   -21,
      25,   -76,   -76,    82,    14,   -76,   -76,     0,   -76,   103,
     103,   -34,   -31,   -76,   -30,   -76,   -76,   -23,   -76,   -76,
     -76,   103,   103,   110,   110,   -76,   -76,   -76,   -76,   -76,
     -76,   110,   110,   110,   110,   110,   110,   -76,   -76,   -20,
     -18,    12,    12,   -76,    25,   -76,    14,    14,   -17,   -76,
     -76,   -76,   -76,   -76,   -76,   103,   -14,   -22,    53,   -76,
     -76,    62,   -76,    -7,    -3,    12,   -76,   -76
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       3,    60,    56,    54,    55,    58,    59,    57,    15,    16,
       0,     0,    14,     0,     0,     0,     0,     0,     2,     5,
       0,     8,     9,    10,    11,     7,    17,    18,    13,    24,
      26,    28,    30,    32,    41,    46,    49,    51,    53,     0,
      62,    60,     0,    29,     0,    48,    47,     0,     1,     4,
       6,     0,     0,     0,     0,    35,    36,    37,    38,    33,
      34,     0,     0,     0,     0,     0,     0,    12,    64,     0,
      63,     0,     0,    61,    25,    27,    39,    40,    31,    42,
      44,    43,    45,    50,    52,     0,     0,     0,    19,    21,
      65,     0,    23,     0,     0,     0,    22,    20
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -76,   -76,   -15,   -11,   -49,   -76,   -76,   -76,   -76,   -76,
     -76,   -76,   -62,    18,   -76,    27,    -2,   -76,   -76,    19,
      -5,    28,   -76,   -76,   -76,   -76,   -76
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    88,    28,    29,    30,    31,    32,    61,    33,
      34,    35,    36,    37,    38,    69,    70
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
       1,     2,     3,     4,     5,     6,     7,    49,    48,    43,
      89,    50,     8,    51,     9,     1,     2,     3,     4,     5,
       6,     7,    87,    87,    10,    53,    54,     8,    42,     9,
      11,    44,    12,    97,    47,    52,    13,    39,    40,    14,
      15,    40,    45,    46,    66,    11,    87,    12,    76,    77,
      75,    71,    72,    73,    14,    15,    84,    67,    68,    62,
      63,    64,    65,    85,    92,     1,     2,     3,     4,     5,
       6,     7,    16,    91,    93,    95,    94,     8,    74,     9,
      78,     0,     0,    49,     0,    96,     0,    16,     0,    10,
      79,    80,    81,    82,    83,    11,     0,    12,    86,     0,
       0,    13,     0,    90,    14,    15,    41,     2,     3,     4,
       5,     6,     7,    41,     2,     3,     4,     5,     6,     7,
       0,     0,     0,     0,    53,    54,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    16,    55,    56,
      57,    58,    59,    60,     0,    14,    15,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    16,     0,
       0,     0,     0,     0,     0,    16
};

static const yytype_int8 yycheck[] =
{
       3,     4,     5,     6,     7,     8,     9,    18,     0,    11,
      72,    86,    15,    34,    17,     3,     4,     5,     6,     7,
       8,     9,    71,    72,    27,    42,    43,    15,    10,    17,
      33,    13,    35,    95,    16,    10,    39,    74,    75,    42,
      43,    75,    14,    15,    44,    33,    95,    35,    53,    54,
      52,    82,    82,    76,    42,    43,    76,    39,    40,    45,
      46,    47,    48,    81,    86,     3,     4,     5,     6,     7,
       8,     9,    75,    87,    21,    82,    91,    15,    51,    17,
      61,    -1,    -1,    94,    -1,    88,    -1,    75,    -1,    27,
      62,    63,    64,    65,    66,    33,    -1,    35,    86,    -1,
      -1,    39,    -1,    85,    42,    43,     3,     4,     5,     6,
       7,     8,     9,     3,     4,     5,     6,     7,     8,     9,
      -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    33,    75,    56,    57,
      58,    59,    60,    61,    -1,    42,    43,    -1,    -1,    -1,
      -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    75,    -1,
      -1,    -1,    -1,    -1,    -1,    75
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,     9,    15,    17,
      27,    33,    35,    39,    42,    43,    75,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   102,   103,
     104,   105,   106,   108,   109,   110,   111,   112,   113,    74,
      75,     3,   102,   105,   102,   110,   110,   102,     0,    92,
      86,    34,    10,    42,    43,    56,    57,    58,    59,    60,
      61,   107,    45,    46,    47,    48,    44,   102,   102,   114,
     115,    82,    82,    76,   104,   105,   109,   109,   108,   110,
     110,   110,   110,   110,    76,    81,    86,    93,   101,   101,
     102,    87,    86,    21,    91,    82,    88,   101
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    89,    90,    90,    91,    91,    92,    92,    93,    93,
      93,    93,    94,    94,    95,    96,    97,    98,    98,    99,
      99,   100,   101,   101,   102,   103,   103,   104,   104,   105,
     105,   106,   106,   107,   107,   107,   107,   107,   107,   108,
     108,   108,   109,   109,   109,   109,   109,   110,   110,   110,
     111,   111,   112,   112,   113,   113,   113,   113,   113,   113,
     113,   113,   114,   114,   115,   115
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     0,     2,     1,     2,     1,     1,     1,
       1,     1,     3,     1,     1,     1,     1,     1,     1,     4,
       7,     4,     4,     2,     1,     3,     1,     3,     1,     2,
       1,     3,     1,     1,     1,     1,     1,     1,     1,     3,
       3,     1,     3,     3,     3,     3,     1,     2,     2,     1,
       3,     1,     4,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     0,     1,     1,     3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* program: stmts  */
#line 184 "parser/parser.y"
            { raiz = mknode(N_PROGRAM); raiz->a = (yyvsp[0].node); }
#line 1421 "parser.tab.c"
    break;

  case 3: /* program: %empty  */
#line 185 "parser/parser.y"
                           { raiz = mknode(N_PROGRAM); }
#line 1427 "parser.tab.c"
    break;

  case 4: /* stmts: stmts stmt  */
#line 189 "parser/parser.y"
                 { (yyval.node) = append_list((yyvsp[-1].node), (yyvsp[0].node)); }
#line 1433 "parser.tab.c"
    break;

  case 5: /* stmts: stmt  */
#line 190 "parser/parser.y"
           { (yyval.node) = (yyvsp[0].node); }
#line 1439 "parser.tab.c"
    break;

  case 6: /* stmt: simple_stmt NOVA_LINHA  */
#line 194 "parser/parser.y"
                             { (yyval.node) = (yyvsp[-1].node); }
#line 1445 "parser.tab.c"
    break;

  case 7: /* stmt: compound_stmt  */
#line 195 "parser/parser.y"
                    { (yyval.node) = (yyvsp[0].node); }
#line 1451 "parser.tab.c"
    break;

  case 8: /* simple_stmt: expr_stmt  */
#line 199 "parser/parser.y"
                { (yyval.node) = (yyvsp[0].node); }
#line 1457 "parser.tab.c"
    break;

  case 9: /* simple_stmt: pass_stmt  */
#line 200 "parser/parser.y"
                { (yyval.node) = (yyvsp[0].node); }
#line 1463 "parser.tab.c"
    break;

  case 10: /* simple_stmt: break_stmt  */
#line 201 "parser/parser.y"
                 { (yyval.node) = (yyvsp[0].node); }
#line 1469 "parser.tab.c"
    break;

  case 11: /* simple_stmt: continue_stmt  */
#line 202 "parser/parser.y"
                    { (yyval.node) = (yyvsp[0].node); }
#line 1475 "parser.tab.c"
    break;

  case 12: /* expr_stmt: IDENTIFICADOR OP_ATRIBUICAO expr  */
#line 206 "parser/parser.y"
                                       {
          (yyval.node) = mknode(N_ASSIGN); (yyval.node)->sval = (yyvsp[-2].str); (yyval.node)->a = (yyvsp[0].node);
      }
#line 1483 "parser.tab.c"
    break;

  case 13: /* expr_stmt: expr  */
#line 209 "parser/parser.y"
           {
          (yyval.node) = mknode(N_EXPRSTMT); (yyval.node)->a = (yyvsp[0].node);
      }
#line 1491 "parser.tab.c"
    break;

  case 14: /* pass_stmt: PASS  */
#line 214 "parser/parser.y"
                { (yyval.node) = mknode(N_PASS); }
#line 1497 "parser.tab.c"
    break;

  case 15: /* break_stmt: BREAK  */
#line 215 "parser/parser.y"
                  { (yyval.node) = mknode(N_BREAK); }
#line 1503 "parser.tab.c"
    break;

  case 16: /* continue_stmt: CONTINUE  */
#line 216 "parser/parser.y"
                        { (yyval.node) = mknode(N_CONTINUE); }
#line 1509 "parser.tab.c"
    break;

  case 17: /* compound_stmt: if_stmt  */
#line 219 "parser/parser.y"
              { (yyval.node) = (yyvsp[0].node); }
#line 1515 "parser.tab.c"
    break;

  case 18: /* compound_stmt: while_stmt  */
#line 220 "parser/parser.y"
                 { (yyval.node) = (yyvsp[0].node); }
#line 1521 "parser.tab.c"
    break;

  case 19: /* if_stmt: IF expr DOIS_PONTOS block  */
#line 224 "parser/parser.y"
                                {
          (yyval.node) = mknode(N_IF); (yyval.node)->a = (yyvsp[-2].node); (yyval.node)->b = (yyvsp[0].node);
      }
#line 1529 "parser.tab.c"
    break;

  case 20: /* if_stmt: IF expr DOIS_PONTOS block ELSE DOIS_PONTOS block  */
#line 227 "parser/parser.y"
                                                       {
          (yyval.node) = mknode(N_IF); (yyval.node)->a = (yyvsp[-5].node); (yyval.node)->b = (yyvsp[-3].node); (yyval.node)->c = (yyvsp[0].node);
      }
#line 1537 "parser.tab.c"
    break;

  case 21: /* while_stmt: WHILE expr DOIS_PONTOS block  */
#line 233 "parser/parser.y"
                                   {
          (yyval.node) = mknode(N_WHILE); (yyval.node)->a = (yyvsp[-2].node); (yyval.node)->b = (yyvsp[0].node);
      }
#line 1545 "parser.tab.c"
    break;

  case 22: /* block: NOVA_LINHA INDENT stmts DEDENT  */
#line 239 "parser/parser.y"
                                     { (yyval.node) = (yyvsp[-1].node); }
#line 1551 "parser.tab.c"
    break;

  case 23: /* block: simple_stmt NOVA_LINHA  */
#line 240 "parser/parser.y"
                             { (yyval.node) = (yyvsp[-1].node); }
#line 1557 "parser.tab.c"
    break;

  case 24: /* expr: or_test  */
#line 245 "parser/parser.y"
              { (yyval.node) = (yyvsp[0].node); }
#line 1563 "parser.tab.c"
    break;

  case 25: /* or_test: or_test OR and_test  */
#line 248 "parser/parser.y"
                          { (yyval.node) = mknode(N_BOOLOP); (yyval.node)->ival = OR; (yyval.node)->a = (yyvsp[-2].node); (yyval.node)->b = (yyvsp[0].node); }
#line 1569 "parser.tab.c"
    break;

  case 26: /* or_test: and_test  */
#line 249 "parser/parser.y"
               { (yyval.node) = (yyvsp[0].node); }
#line 1575 "parser.tab.c"
    break;

  case 27: /* and_test: and_test AND not_test  */
#line 253 "parser/parser.y"
                            { (yyval.node) = mknode(N_BOOLOP); (yyval.node)->ival = AND; (yyval.node)->a = (yyvsp[-2].node); (yyval.node)->b = (yyvsp[0].node); }
#line 1581 "parser.tab.c"
    break;

  case 28: /* and_test: not_test  */
#line 254 "parser/parser.y"
               { (yyval.node) = (yyvsp[0].node); }
#line 1587 "parser.tab.c"
    break;

  case 29: /* not_test: NOT not_test  */
#line 258 "parser/parser.y"
                   { (yyval.node) = mknode(N_NOT); (yyval.node)->a = (yyvsp[0].node); }
#line 1593 "parser.tab.c"
    break;

  case 30: /* not_test: comparison  */
#line 259 "parser/parser.y"
                 { (yyval.node) = (yyvsp[0].node); }
#line 1599 "parser.tab.c"
    break;

  case 31: /* comparison: arith_expr compare_op arith_expr  */
#line 263 "parser/parser.y"
                                       { (yyval.node) = mknode(N_COMPARE); (yyval.node)->ival = (yyvsp[-1].ival); (yyval.node)->a = (yyvsp[-2].node); (yyval.node)->b = (yyvsp[0].node); }
#line 1605 "parser.tab.c"
    break;

  case 32: /* comparison: arith_expr  */
#line 264 "parser/parser.y"
                 { (yyval.node) = (yyvsp[0].node); }
#line 1611 "parser.tab.c"
    break;

  case 33: /* compare_op: OP_MENOR  */
#line 268 "parser/parser.y"
               { (yyval.ival) = OP_MENOR; }
#line 1617 "parser.tab.c"
    break;

  case 34: /* compare_op: OP_MAIOR  */
#line 269 "parser/parser.y"
               { (yyval.ival) = OP_MAIOR; }
#line 1623 "parser.tab.c"
    break;

  case 35: /* compare_op: OP_MENOR_IGUAL  */
#line 270 "parser/parser.y"
                     { (yyval.ival) = OP_MENOR_IGUAL; }
#line 1629 "parser.tab.c"
    break;

  case 36: /* compare_op: OP_MAIOR_IGUAL  */
#line 271 "parser/parser.y"
                     { (yyval.ival) = OP_MAIOR_IGUAL; }
#line 1635 "parser.tab.c"
    break;

  case 37: /* compare_op: OP_IGUAL_IGUAL  */
#line 272 "parser/parser.y"
                     { (yyval.ival) = OP_IGUAL_IGUAL; }
#line 1641 "parser.tab.c"
    break;

  case 38: /* compare_op: OP_DIFERENTE  */
#line 273 "parser/parser.y"
                   { (yyval.ival) = OP_DIFERENTE; }
#line 1647 "parser.tab.c"
    break;

  case 39: /* arith_expr: arith_expr OP_MAIS term  */
#line 277 "parser/parser.y"
                              { (yyval.node) = mknode(N_BINOP); (yyval.node)->ival = OP_MAIS; (yyval.node)->a = (yyvsp[-2].node); (yyval.node)->b = (yyvsp[0].node); }
#line 1653 "parser.tab.c"
    break;

  case 40: /* arith_expr: arith_expr OP_MENOS term  */
#line 278 "parser/parser.y"
                               { (yyval.node) = mknode(N_BINOP); (yyval.node)->ival = OP_MENOS; (yyval.node)->a = (yyvsp[-2].node); (yyval.node)->b = (yyvsp[0].node); }
#line 1659 "parser.tab.c"
    break;

  case 41: /* arith_expr: term  */
#line 279 "parser/parser.y"
           { (yyval.node) = (yyvsp[0].node); }
#line 1665 "parser.tab.c"
    break;

  case 42: /* term: term OP_MULTIPLICACAO factor  */
#line 283 "parser/parser.y"
                                   { (yyval.node) = mknode(N_BINOP); (yyval.node)->ival = OP_MULTIPLICACAO; (yyval.node)->a = (yyvsp[-2].node); (yyval.node)->b = (yyvsp[0].node); }
#line 1671 "parser.tab.c"
    break;

  case 43: /* term: term OP_DIVISAO factor  */
#line 284 "parser/parser.y"
                             { (yyval.node) = mknode(N_BINOP); (yyval.node)->ival = OP_DIVISAO; (yyval.node)->a = (yyvsp[-2].node); (yyval.node)->b = (yyvsp[0].node); }
#line 1677 "parser.tab.c"
    break;

  case 44: /* term: term OP_DIVISAO_INTEIRA factor  */
#line 285 "parser/parser.y"
                                     { (yyval.node) = mknode(N_BINOP); (yyval.node)->ival = OP_DIVISAO_INTEIRA; (yyval.node)->a = (yyvsp[-2].node); (yyval.node)->b = (yyvsp[0].node); }
#line 1683 "parser.tab.c"
    break;

  case 45: /* term: term OP_MODULO factor  */
#line 286 "parser/parser.y"
                            { (yyval.node) = mknode(N_BINOP); (yyval.node)->ival = OP_MODULO; (yyval.node)->a = (yyvsp[-2].node); (yyval.node)->b = (yyvsp[0].node); }
#line 1689 "parser.tab.c"
    break;

  case 46: /* term: factor  */
#line 287 "parser/parser.y"
             { (yyval.node) = (yyvsp[0].node); }
#line 1695 "parser.tab.c"
    break;

  case 47: /* factor: OP_MENOS factor  */
#line 291 "parser/parser.y"
                      { (yyval.node) = mknode(N_UNOP); (yyval.node)->ival = OP_MENOS; (yyval.node)->a = (yyvsp[0].node); }
#line 1701 "parser.tab.c"
    break;

  case 48: /* factor: OP_MAIS factor  */
#line 292 "parser/parser.y"
                     { (yyval.node) = mknode(N_UNOP); (yyval.node)->ival = OP_MAIS; (yyval.node)->a = (yyvsp[0].node); }
#line 1707 "parser.tab.c"
    break;

  case 49: /* factor: power  */
#line 293 "parser/parser.y"
            { (yyval.node) = (yyvsp[0].node); }
#line 1713 "parser.tab.c"
    break;

  case 50: /* power: atom_trailer OP_POTENCIA factor  */
#line 297 "parser/parser.y"
                                      { (yyval.node) = mknode(N_BINOP); (yyval.node)->ival = OP_POTENCIA; (yyval.node)->a = (yyvsp[-2].node); (yyval.node)->b = (yyvsp[0].node); }
#line 1719 "parser.tab.c"
    break;

  case 51: /* power: atom_trailer  */
#line 298 "parser/parser.y"
                   { (yyval.node) = (yyvsp[0].node); }
#line 1725 "parser.tab.c"
    break;

  case 52: /* atom_trailer: IDENTIFICADOR PARENTESES_ESQ arglist_opt PARENTESES_DIR  */
#line 302 "parser/parser.y"
                                                              {
          (yyval.node) = mknode(N_CALL); (yyval.node)->sval = (yyvsp[-3].str); (yyval.node)->a = (yyvsp[-1].node);
      }
#line 1733 "parser.tab.c"
    break;

  case 53: /* atom_trailer: atom  */
#line 305 "parser/parser.y"
           { (yyval.node) = (yyvsp[0].node); }
#line 1739 "parser.tab.c"
    break;

  case 54: /* atom: INTEIRO  */
#line 309 "parser/parser.y"
              { (yyval.node) = mknode(N_INT); (yyval.node)->ival = (yyvsp[0].ival); }
#line 1745 "parser.tab.c"
    break;

  case 55: /* atom: FLUTUANTE  */
#line 310 "parser/parser.y"
                { (yyval.node) = mknode(N_FLOAT); (yyval.node)->dval = (yyvsp[0].dval); }
#line 1751 "parser.tab.c"
    break;

  case 56: /* atom: TEXTO  */
#line 311 "parser/parser.y"
            { (yyval.node) = mknode(N_STR); (yyval.node)->sval = (yyvsp[0].str); }
#line 1757 "parser.tab.c"
    break;

  case 57: /* atom: TRUE_  */
#line 312 "parser/parser.y"
            { (yyval.node) = mknode(N_BOOL); (yyval.node)->ival = 1; }
#line 1763 "parser.tab.c"
    break;

  case 58: /* atom: FALSE_  */
#line 313 "parser/parser.y"
             { (yyval.node) = mknode(N_BOOL); (yyval.node)->ival = 0; }
#line 1769 "parser.tab.c"
    break;

  case 59: /* atom: NONE_  */
#line 314 "parser/parser.y"
            { (yyval.node) = mknode(N_NONE); }
#line 1775 "parser.tab.c"
    break;

  case 60: /* atom: IDENTIFICADOR  */
#line 315 "parser/parser.y"
                    { (yyval.node) = mknode(N_VAR); (yyval.node)->sval = (yyvsp[0].str); }
#line 1781 "parser.tab.c"
    break;

  case 61: /* atom: PARENTESES_ESQ expr PARENTESES_DIR  */
#line 316 "parser/parser.y"
                                         { (yyval.node) = (yyvsp[-1].node); }
#line 1787 "parser.tab.c"
    break;

  case 62: /* arglist_opt: %empty  */
#line 320 "parser/parser.y"
                  { (yyval.node) = NULL; }
#line 1793 "parser.tab.c"
    break;

  case 63: /* arglist_opt: arglist  */
#line 321 "parser/parser.y"
              { (yyval.node) = (yyvsp[0].node); }
#line 1799 "parser.tab.c"
    break;

  case 64: /* arglist: expr  */
#line 325 "parser/parser.y"
           { (yyval.node) = (yyvsp[0].node); }
#line 1805 "parser.tab.c"
    break;

  case 65: /* arglist: arglist VIRGULA expr  */
#line 326 "parser/parser.y"
                           { (yyval.node) = append_list((yyvsp[-2].node), (yyvsp[0].node)); }
#line 1811 "parser.tab.c"
    break;


#line 1815 "parser.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 329 "parser/parser.y"


/* ===================== Sinalizacao de break/continue ===================== */

typedef enum { SIG_NONE, SIG_BREAK, SIG_CONTINUE } Signal;
static Signal signal_flag = SIG_NONE;

/* ===================== Interpretador ===================== */

static Value *eval_expr(Node *n, Scope *sc);
static void exec_stmt(Node *n, Scope *sc);
static void exec_block(Node *block, Scope *sc);

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
            case V_FLOAT: format_float(v->dval, buf, sizeof(buf)); return new_str(buf);
            case V_BOOL: return new_str(v->ival ? "True" : "False");
            case V_NONE: return new_str("None");
            case V_STR: return new_str(v->sval);
        }
    }

    fprintf(stderr, "ERRO: funcao '%s' nao definida\n", name);
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

void executar_programa(void) {
    if (raiz) {
        Scope *global = new_scope();
        exec_block(raiz->a, global);
    }
}
