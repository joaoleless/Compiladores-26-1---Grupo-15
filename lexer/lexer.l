%{
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "parser.tab.h"

/* Renomeia a função gerada pelo flex para "flex_scan".
   yylex() de verdade é escrito à mão lá embaixo e faz o controle
   de INDENT/DEDENT antes de delegar para flex_scan(). */
#define YY_DECL int flex_scan(void)

int linha_atual = 1;
int erros_lexicos = 0; /* erros lexicos/de indentacao encontrados */

/* --- controle de indentação --- */
#define MAX_INDENT 128
static int indent_stack[MAX_INDENT];
static int indent_top = 0;      /* indent_stack[indent_top] = nivel atual */
static int pending_dedents = 0; /* DEDENTs extras a devolver nas proximas chamadas */
static int paren_depth = 0;     /* profundidade de ( [ {  -> ignora NEWLINE dentro */
static int last_indent_action = 0; /* 0=nada, 1=INDENT, 2=DEDENT (preenchido por process_indent) */
static int ultimo_token = 0;   /* ultimo token devolvido ao parser (0 = nenhum ainda) */

/* calcula a indentacao de "espacos" (tabs contam 8) e atualiza a pilha de indentacao */
static void process_indent(const char *espacos) {
    int indent = 0;
    for (const char *p = espacos; *p; p++) {
        if (*p == '\t') indent += 8 - (indent % 8);
        else indent += 1;
    }
    last_indent_action = 0;
    if (indent > indent_stack[indent_top]) {
        if (indent_top + 1 >= MAX_INDENT) {
            fprintf(stderr, "ERRO: indentacao aninhada demais na linha %d\n", linha_atual);
            exit(1);
        }
        indent_stack[++indent_top] = indent;
        last_indent_action = 1;
    } else if (indent < indent_stack[indent_top]) {
        int count = 0;
        while (indent_top > 0 && indent < indent_stack[indent_top]) {
            indent_top--;
            count++;
        }
        if (indent != indent_stack[indent_top]) {
            fprintf(stderr, "ERRO DE INDENTACAO na linha %d\n", linha_atual);
            erros_lexicos++;
        }
        pending_dedents = count - 1;
        last_indent_action = 2;
    }
    /* igual ao nivel atual: last_indent_action fica 0, nada a fazer */
}

static char *unescape(const char *s) {
    /* recebe o texto com aspas (ex: "abc\n") e devolve o conteudo sem aspas,
       com escapes basicos resolvidos */
    int len = strlen(s);
    char *out = malloc(len + 1);
    int oi = 0;
    for (int i = 1; i < len - 1; i++) { /* pula aspas nas pontas */
        if (s[i] == '\\' && i + 1 < len - 1) {
            i++;
            switch (s[i]) {
                case 'n': out[oi++] = '\n'; break;
                case 't': out[oi++] = '\t'; break;
                case '\\': out[oi++] = '\\'; break;
                case '"': out[oi++] = '"'; break;
                case '\'': out[oi++] = '\''; break;
                default: out[oi++] = s[i]; break;
            }
        } else {
            out[oi++] = s[i];
        }
    }
    out[oi] = '\0';
    return out;
}
%}

%option noyywrap
%option yylineno
%option noinput
%option nounput
%x BOL

DIGIT       [0-9]
LETTER      [a-zA-Z_]
ID          {LETTER}({LETTER}|{DIGIT})*
INTEGER     {DIGIT}+
EXP         [eE][+-]?{DIGIT}+
FLOAT       ({DIGIT}+"."{DIGIT}*|"."{DIGIT}+){EXP}?|{DIGIT}+{EXP}
STRING      \"([^\"\\]|\\.)*\"|\'([^\'\\]|\\.)*\'

%%

    /* ---------- estado BOL: inicio de linha logica, calcula indentacao ---------- */
<BOL>[ \t]*"#".*\n     { linha_atual++; /* linha so com comentario: ignora */ }
<BOL>[ \t]*\n          { linha_atual++; /* linha em branco: ignora */ }
<BOL><<EOF>>           {
                            if (indent_top > 0) {
                                pending_dedents = indent_top - 1;
                                indent_top = 0;
                                return DEDENT;
                            }
                            yyterminate();
                        }
    /* nota: usamos [ \t]+ (1 ou mais) e um fallback <BOL>. separado, porque o flex
       nao dispara de forma confiavel uma regra que so casa string vazia logo no
       inicio do escaneamento (caso de uma linha sem nenhuma indentacao). */
<BOL>[ \t]+            { process_indent(yytext); BEGIN(INITIAL);
                          if (last_indent_action == 1) return INDENT;
                          if (last_indent_action == 2) return DEDENT;
                        }
<BOL>.                 {
                            yyless(0); /* devolve o caractere: ele sera lido de novo em INITIAL */
                            process_indent("");
                            BEGIN(INITIAL);
                            if (last_indent_action == 1) return INDENT;
                            if (last_indent_action == 2) return DEDENT;
                        }

    /* ---------- palavras reservadas ---------- */
"False"     { return FALSE_; }
"None"      { return NONE_; }
"True"      { return TRUE_; }
"and"       { return AND; }
"as"        { return AS; }
"assert"    { return ASSERT; }
"async"     { return ASYNC; }
"await"     { return AWAIT; }
"break"     { return BREAK; }
"class"     { return CLASS; }
"continue"  { return CONTINUE; }
"def"       { return DEF; }
"del"       { return DEL; }
"elif"      { return ELIF; }
"else"      { return ELSE; }
"except"    { return EXCEPT; }
"finally"   { return FINALLY; }
"for"       { return FOR; }
"from"      { return FROM; }
"global"    { return GLOBAL; }
"if"        { return IF; }
"import"    { return IMPORT; }
"in"        { return IN; }
"is"        { return IS; }
"lambda"    { return LAMBDA; }
"nonlocal"  { return NONLOCAL; }
"not"       { return NOT; }
"or"        { return OR; }
"pass"      { return PASS; }
"raise"     { return RAISE; }
"return"    { return RETURN; }
"try"       { return TRY; }
"while"     { return WHILE; }
"with"      { return WITH; }
"yield"     { return YIELD; }

{ID}        { yylval.str = strdup(yytext); return IDENTIFICADOR; }
{FLOAT}     { yylval.dval = atof(yytext);   return FLUTUANTE; }
{INTEGER}   {
                errno = 0;
                yylval.ival = strtol(yytext, NULL, 10);
                if (errno == ERANGE) {
                    fprintf(stderr, "ERRO LEXICO: inteiro '%s' fora do intervalo de 64 bits na linha %d\n", yytext, linha_atual);
                    erros_lexicos++;
                }
                return INTEIRO;
            }
{STRING}    { yylval.str = unescape(yytext); return TEXTO; }

"**="       { return OP_ATRIBUICAO_POTENCIA; }
"//="       { return OP_ATRIBUICAO_DIVISAO_INTEIRA; }
"<<="       { return OP_ATRIBUICAO_DESLOC_ESQUERDA; }
">>="       { return OP_ATRIBUICAO_DESLOC_DIREITA; }
"+="        { return OP_ATRIBUICAO_MAIS; }
"-="        { return OP_ATRIBUICAO_MENOS; }
"*="        { return OP_ATRIBUICAO_MULTIPLICACAO; }
"/="        { return OP_ATRIBUICAO_DIVISAO; }
"%="        { return OP_ATRIBUICAO_MODULO; }
"&="        { return OP_ATRIBUICAO_E_BIT; }
"|="        { return OP_ATRIBUICAO_OU_BIT; }
"^="        { return OP_ATRIBUICAO_XOU_BIT; }
"**"        { return OP_POTENCIA; }
"//"        { return OP_DIVISAO_INTEIRA; }
"<<"        { return OP_DESLOC_ESQUERDA; }
">>"        { return OP_DESLOC_DIREITA; }
"<="        { return OP_MENOR_IGUAL; }
">="        { return OP_MAIOR_IGUAL; }
"=="        { return OP_IGUAL_IGUAL; }
"!="        { return OP_DIFERENTE; }
"->"        { return SETA; }

"+"         { return OP_MAIS; }
"-"         { return OP_MENOS; }
"*"         { return OP_MULTIPLICACAO; }
"/"         { return OP_DIVISAO; }
"%"         { return OP_MODULO; }
"@"         { return OP_MATRIZ_MULT; }
"&"         { return OP_E_BIT; }
"|"         { return OP_OU_BIT; }
"^"         { return OP_XOU_BIT; }
"~"         { return OP_NAO_BIT; }
"<"         { return OP_MENOR; }
">"         { return OP_MAIOR; }
"="         { return OP_ATRIBUICAO; }

"("         { paren_depth++; return PARENTESES_ESQ; }
")"         { if (paren_depth > 0) paren_depth--; return PARENTESES_DIR; }
"["         { paren_depth++; return COLCHETE_ESQ; }
"]"         { if (paren_depth > 0) paren_depth--; return COLCHETE_DIR; }
"{"         { paren_depth++; return CHAVE_ESQ; }
"}"         { if (paren_depth > 0) paren_depth--; return CHAVE_DIR; }
","         { return VIRGULA; }
":"         { return DOIS_PONTOS; }
"."         { return PONTO; }
";"         { return PONTO_E_VIRGULA; }

"#".*       { /* comentario no meio da linha: ignora */ }

[ \t]+      { /* espacos fora do inicio de linha: ignora */ }

\n          {
                linha_atual++;
                if (paren_depth > 0) {
                    /* dentro de ( [ {  quebras de linha nao contam */
                } else {
                    BEGIN(BOL);
                    return NOVA_LINHA;
                }
            }

<<EOF>>     {
                /* ultima linha sem '\n' no fim do arquivo: o Python considera a linha
                   encerrada, entao emitimos a NOVA_LINHA que faltou e voltamos ao
                   estado BOL, que fecha os blocos abertos com DEDENT. */
                if (paren_depth == 0 && ultimo_token != 0 && ultimo_token != NOVA_LINHA) {
                    BEGIN(BOL);
                    return NOVA_LINHA;
                }
                if (indent_top > 0) {
                    pending_dedents = indent_top - 1;
                    indent_top = 0;
                    return DEDENT;
                }
                yyterminate();
            }

.           { fprintf(stderr, "ERRO LEXICO: caractere desconhecido '%s' na linha %d\n", yytext, linha_atual); erros_lexicos++; }

%%

int yylex(void) {
    static int primeira_chamada = 1;
    if (primeira_chamada) {
        primeira_chamada = 0;
        BEGIN(BOL);
    }
    if (pending_dedents > 0) {
        pending_dedents--;
        return ultimo_token = DEDENT;
    }
    return ultimo_token = flex_scan();
}