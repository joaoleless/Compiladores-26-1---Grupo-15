#include <stdio.h>
#include <stdlib.h>

#include "parser.tab.h"

extern int yylex(void);
extern FILE *yyin;
extern int erros_lexicos;
YYSTYPE yylval;

static const char *token_name(int token) {
    switch (token) {
        case IDENTIFICADOR: return "IDENTIFICADOR";
        case TEXTO: return "TEXTO";
        case INTEIRO: return "INTEIRO";
        case FLUTUANTE: return "FLUTUANTE";
        case FALSE_: return "FALSE_";
        case NONE_: return "NONE_";
        case TRUE_: return "TRUE_";
        case AND: return "AND";
        case AS: return "AS";
        case ASSERT: return "ASSERT";
        case ASYNC: return "ASYNC";
        case AWAIT: return "AWAIT";
        case BREAK: return "BREAK";
        case CLASS: return "CLASS";
        case CONTINUE: return "CONTINUE";
        case DEF: return "DEF";
        case DEL: return "DEL";
        case ELIF: return "ELIF";
        case ELSE: return "ELSE";
        case EXCEPT: return "EXCEPT";
        case FINALLY: return "FINALLY";
        case FOR: return "FOR";
        case FROM: return "FROM";
        case GLOBAL: return "GLOBAL";
        case IF: return "IF";
        case IMPORT: return "IMPORT";
        case IN: return "IN";
        case IS: return "IS";
        case LAMBDA: return "LAMBDA";
        case NONLOCAL: return "NONLOCAL";
        case NOT: return "NOT";
        case OR: return "OR";
        case PASS: return "PASS";
        case RAISE: return "RAISE";
        case RETURN: return "RETURN";
        case TRY: return "TRY";
        case WHILE: return "WHILE";
        case WITH: return "WITH";
        case YIELD: return "YIELD";
        case OP_MAIS: return "OP_MAIS";
        case OP_MENOS: return "OP_MENOS";
        case OP_POTENCIA: return "OP_POTENCIA";
        case OP_MULTIPLICACAO: return "OP_MULTIPLICACAO";
        case OP_DIVISAO_INTEIRA: return "OP_DIVISAO_INTEIRA";
        case OP_DIVISAO: return "OP_DIVISAO";
        case OP_MODULO: return "OP_MODULO";
        case OP_MATRIZ_MULT: return "OP_MATRIZ_MULT";
        case OP_DESLOC_ESQUERDA: return "OP_DESLOC_ESQUERDA";
        case OP_DESLOC_DIREITA: return "OP_DESLOC_DIREITA";
        case OP_E_BIT: return "OP_E_BIT";
        case OP_OU_BIT: return "OP_OU_BIT";
        case OP_XOU_BIT: return "OP_XOU_BIT";
        case OP_NAO_BIT: return "OP_NAO_BIT";
        case OP_MENOR_IGUAL: return "OP_MENOR_IGUAL";
        case OP_MAIOR_IGUAL: return "OP_MAIOR_IGUAL";
        case OP_IGUAL_IGUAL: return "OP_IGUAL_IGUAL";
        case OP_DIFERENTE: return "OP_DIFERENTE";
        case OP_MENOR: return "OP_MENOR";
        case OP_MAIOR: return "OP_MAIOR";
        case OP_ATRIBUICAO_MAIS: return "OP_ATRIBUICAO_MAIS";
        case OP_ATRIBUICAO_MENOS: return "OP_ATRIBUICAO_MENOS";
        case OP_ATRIBUICAO_POTENCIA: return "OP_ATRIBUICAO_POTENCIA";
        case OP_ATRIBUICAO_MULTIPLICACAO: return "OP_ATRIBUICAO_MULTIPLICACAO";
        case OP_ATRIBUICAO_DIVISAO_INTEIRA: return "OP_ATRIBUICAO_DIVISAO_INTEIRA";
        case OP_ATRIBUICAO_DIVISAO: return "OP_ATRIBUICAO_DIVISAO";
        case OP_ATRIBUICAO_MODULO: return "OP_ATRIBUICAO_MODULO";
        case OP_ATRIBUICAO_E_BIT: return "OP_ATRIBUICAO_E_BIT";
        case OP_ATRIBUICAO_OU_BIT: return "OP_ATRIBUICAO_OU_BIT";
        case OP_ATRIBUICAO_XOU_BIT: return "OP_ATRIBUICAO_XOU_BIT";
        case OP_ATRIBUICAO_DESLOC_ESQUERDA: return "OP_ATRIBUICAO_DESLOC_ESQUERDA";
        case OP_ATRIBUICAO_DESLOC_DIREITA: return "OP_ATRIBUICAO_DESLOC_DIREITA";
        case OP_ATRIBUICAO: return "OP_ATRIBUICAO";
        case PARENTESES_ESQ: return "PARENTESES_ESQ";
        case PARENTESES_DIR: return "PARENTESES_DIR";
        case COLCHETE_ESQ: return "COLCHETE_ESQ";
        case COLCHETE_DIR: return "COLCHETE_DIR";
        case CHAVE_ESQ: return "CHAVE_ESQ";
        case CHAVE_DIR: return "CHAVE_DIR";
        case VIRGULA: return "VIRGULA";
        case DOIS_PONTOS: return "DOIS_PONTOS";
        case PONTO: return "PONTO";
        case PONTO_E_VIRGULA: return "PONTO_E_VIRGULA";
        case SETA: return "SETA";
        case NOVA_LINHA: return "NOVA_LINHA";
        case INDENT: return "INDENT";
        case DEDENT: return "DEDENT";
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
    return erros_lexicos > 0 ? 1 : 0;
}