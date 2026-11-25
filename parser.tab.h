/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_PARSER_TAB_H_INCLUDED
# define YY_YY_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    IDENTIFICADOR = 258,           /* IDENTIFICADOR  */
    TEXTO = 259,                   /* TEXTO  */
    INTEIRO = 260,                 /* INTEIRO  */
    FLUTUANTE = 261,               /* FLUTUANTE  */
    FALSE_ = 262,                  /* FALSE_  */
    NONE_ = 263,                   /* NONE_  */
    TRUE_ = 264,                   /* TRUE_  */
    AND = 265,                     /* AND  */
    AS = 266,                      /* AS  */
    ASSERT = 267,                  /* ASSERT  */
    ASYNC = 268,                   /* ASYNC  */
    AWAIT = 269,                   /* AWAIT  */
    BREAK = 270,                   /* BREAK  */
    CLASS = 271,                   /* CLASS  */
    CONTINUE = 272,                /* CONTINUE  */
    DEF = 273,                     /* DEF  */
    DEL = 274,                     /* DEL  */
    ELIF = 275,                    /* ELIF  */
    ELSE = 276,                    /* ELSE  */
    EXCEPT = 277,                  /* EXCEPT  */
    FINALLY = 278,                 /* FINALLY  */
    FOR = 279,                     /* FOR  */
    FROM = 280,                    /* FROM  */
    GLOBAL = 281,                  /* GLOBAL  */
    IF = 282,                      /* IF  */
    IMPORT = 283,                  /* IMPORT  */
    IN = 284,                      /* IN  */
    IS = 285,                      /* IS  */
    LAMBDA = 286,                  /* LAMBDA  */
    NONLOCAL = 287,                /* NONLOCAL  */
    NOT = 288,                     /* NOT  */
    OR = 289,                      /* OR  */
    PASS = 290,                    /* PASS  */
    RAISE = 291,                   /* RAISE  */
    RETURN = 292,                  /* RETURN  */
    TRY = 293,                     /* TRY  */
    WHILE = 294,                   /* WHILE  */
    WITH = 295,                    /* WITH  */
    YIELD = 296,                   /* YIELD  */
    OP_MAIS = 297,                 /* OP_MAIS  */
    OP_MENOS = 298,                /* OP_MENOS  */
    OP_POTENCIA = 299,             /* OP_POTENCIA  */
    OP_MULTIPLICACAO = 300,        /* OP_MULTIPLICACAO  */
    OP_DIVISAO_INTEIRA = 301,      /* OP_DIVISAO_INTEIRA  */
    OP_DIVISAO = 302,              /* OP_DIVISAO  */
    OP_MODULO = 303,               /* OP_MODULO  */
    OP_MATRIZ_MULT = 304,          /* OP_MATRIZ_MULT  */
    OP_DESLOC_ESQUERDA = 305,      /* OP_DESLOC_ESQUERDA  */
    OP_DESLOC_DIREITA = 306,       /* OP_DESLOC_DIREITA  */
    OP_E_BIT = 307,                /* OP_E_BIT  */
    OP_OU_BIT = 308,               /* OP_OU_BIT  */
    OP_XOU_BIT = 309,              /* OP_XOU_BIT  */
    OP_NAO_BIT = 310,              /* OP_NAO_BIT  */
    OP_MENOR_IGUAL = 311,          /* OP_MENOR_IGUAL  */
    OP_MAIOR_IGUAL = 312,          /* OP_MAIOR_IGUAL  */
    OP_IGUAL_IGUAL = 313,          /* OP_IGUAL_IGUAL  */
    OP_DIFERENTE = 314,            /* OP_DIFERENTE  */
    OP_MENOR = 315,                /* OP_MENOR  */
    OP_MAIOR = 316,                /* OP_MAIOR  */
    OP_ATRIBUICAO_MAIS = 317,      /* OP_ATRIBUICAO_MAIS  */
    OP_ATRIBUICAO_MENOS = 318,     /* OP_ATRIBUICAO_MENOS  */
    OP_ATRIBUICAO_POTENCIA = 319,  /* OP_ATRIBUICAO_POTENCIA  */
    OP_ATRIBUICAO_MULTIPLICACAO = 320, /* OP_ATRIBUICAO_MULTIPLICACAO  */
    OP_ATRIBUICAO_DIVISAO_INTEIRA = 321, /* OP_ATRIBUICAO_DIVISAO_INTEIRA  */
    OP_ATRIBUICAO_DIVISAO = 322,   /* OP_ATRIBUICAO_DIVISAO  */
    OP_ATRIBUICAO_MODULO = 323,    /* OP_ATRIBUICAO_MODULO  */
    OP_ATRIBUICAO_E_BIT = 324,     /* OP_ATRIBUICAO_E_BIT  */
    OP_ATRIBUICAO_OU_BIT = 325,    /* OP_ATRIBUICAO_OU_BIT  */
    OP_ATRIBUICAO_XOU_BIT = 326,   /* OP_ATRIBUICAO_XOU_BIT  */
    OP_ATRIBUICAO_DESLOC_ESQUERDA = 327, /* OP_ATRIBUICAO_DESLOC_ESQUERDA  */
    OP_ATRIBUICAO_DESLOC_DIREITA = 328, /* OP_ATRIBUICAO_DESLOC_DIREITA  */
    OP_ATRIBUICAO = 329,           /* OP_ATRIBUICAO  */
    PARENTESES_ESQ = 330,          /* PARENTESES_ESQ  */
    PARENTESES_DIR = 331,          /* PARENTESES_DIR  */
    COLCHETE_ESQ = 332,            /* COLCHETE_ESQ  */
    COLCHETE_DIR = 333,            /* COLCHETE_DIR  */
    CHAVE_ESQ = 334,               /* CHAVE_ESQ  */
    CHAVE_DIR = 335,               /* CHAVE_DIR  */
    VIRGULA = 336,                 /* VIRGULA  */
    DOIS_PONTOS = 337,             /* DOIS_PONTOS  */
    PONTO = 338,                   /* PONTO  */
    PONTO_E_VIRGULA = 339,         /* PONTO_E_VIRGULA  */
    SETA = 340,                    /* SETA  */
    NOVA_LINHA = 341,              /* NOVA_LINHA  */
    INDENT = 342,                  /* INDENT  */
    DEDENT = 343                   /* DEDENT  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 147 "parser/parser.y"

    long ival;
    double dval;
    char *str;
    struct Node *node;

#line 159 "parser.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_PARSER_TAB_H_INCLUDED  */
