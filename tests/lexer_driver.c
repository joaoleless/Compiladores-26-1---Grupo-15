#include <stdio.h>
#include <stdlib.h>

#include "parser.tab.h"

extern int yylex(void);
extern FILE *yyin;
YYSTYPE yylval;

static const char *token_name(int token) {
    switch (token) {
        case IDENTIFICADOR: return "IDENTIFICADOR";
        case TEXTO: return "TEXTO";
        case INTEIRO: return "INTEIRO";
        case FLUTUANTE: return "FLUTUANTE";
        case TRUE_: return "TRUE_";
        case FALSE_: return "FALSE_";
        case NOVA_LINHA: return "NOVA_LINHA";
        case INDENT: return "INDENT";
        case DEDENT: return "DEDENT";
        case OP_ATRIBUICAO: return "OP_ATRIBUICAO";
        case OP_MAIS: return "OP_MAIS";
        case OP_DIVISAO: return "OP_DIVISAO";
        case OP_POTENCIA: return "OP_POTENCIA";
        case OP_DIVISAO_INTEIRA: return "OP_DIVISAO_INTEIRA";
        case OP_ATRIBUICAO_MAIS: return "OP_ATRIBUICAO_MAIS";
        case IF: return "IF";
        case FOR: return "FOR";
        case IN: return "IN";
        case DOIS_PONTOS: return "DOIS_PONTOS";
        case PARENTESES_ESQ: return "PARENTESES_ESQ";
        case PARENTESES_DIR: return "PARENTESES_DIR";
        case COLCHETE_ESQ: return "COLCHETE_ESQ";
        case COLCHETE_DIR: return "COLCHETE_DIR";
        case VIRGULA: return "VIRGULA";
        default: return NULL;
    }
}

int main(int argc, char **argv) {
    if (argc != 2) return 2;
    yyin = fopen(argv[1], "r");
    if (!yyin) return 1;

    int token;
    while ((token = yylex()) != 0) {
        const char *name = token_name(token);
        if (!name) {
            fprintf(stderr, "token inesperado: %d\n", token);
            fclose(yyin);
            return 1;
        }
        puts(name);
    }
    fclose(yyin);
    return 0;
}