%{
#include <stdio.h>
#include <stdlib.h>

int linha_atual = 1;
%}

%option noyywrap
%option yylineno 

DIGIT       [0-9]
LETTER      [a-zA-Z_]
ID          {LETTER}({LETTER}|{DIGIT})*
INTEGER     {DIGIT}+
FLOAT       {DIGIT}+"."{DIGIT}*|"."{DIGIT}+
STRING      \"([^\"\\]|\\.)*\"|\'([^\'\\]|\\.)*\'

%%

"False"     { printf("PR_FALSO\n"); }
"None"      { printf("PR_NULO\n"); }
"True"      { printf("PR_VERDADEIRO\n"); }
"and"       { printf("PR_E\n"); }
"as"        { printf("PR_COMO\n"); }
"assert"    { printf("PR_AFIRMAR\n"); }
"async"     { printf("PR_ASSINCRONO\n"); }
"await"     { printf("PR_ESPERAR\n"); }
"break"     { printf("PR_PARAR\n"); }
"class"     { printf("PR_CLASSE\n"); }
"continue"  { printf("PR_CONTINUAR\n"); }
"def"       { printf("PR_DEF\n"); }
"del"       { printf("PR_EXCLUIR\n"); }
"elif"      { printf("PR_SENAO_SE\n"); }
"else"      { printf("PR_SENAO\n"); }
"except"    { printf("PR_EXCECAO\n"); }
"finally"   { printf("PR_FINALMENTE\n"); }
"for"       { printf("PR_PARA\n"); }
"from"      { printf("PR_DE\n"); }
"global"    { printf("PR_GLOBAL\n"); }
"if"        { printf("PR_SE\n"); }
"import"    { printf("PR_IMPORTAR\n"); }
"in"        { printf("PR_EM\n"); }
"is"        { printf("PR_EH\n"); }
"lambda"    { printf("PR_LAMBDA\n"); }
"nonlocal"  { printf("PR_NAO_LOCAL\n"); }
"not"       { printf("PR_NAO\n"); }
"or"        { printf("PR_OU\n"); }
"pass"      { printf("PR_PASSAR\n"); }
"raise"     { printf("PR_LEVANTAR\n"); }
"return"    { printf("PR_RETORNAR\n"); }
"try"       { printf("PR_TENTAR\n"); }
"while"     { printf("PR_ENQUANTO\n"); }
"with"      { printf("PR_COM\n"); }
"yield"     { printf("PR_PRODUZIR\n"); }

{ID}        { printf("IDENTIFICADOR(%s)\n", yytext); }
{FLOAT}     { printf("FLUTUANTE(%s)\n", yytext); }
{INTEGER}   { printf("INTEIRO(%s)\n", yytext); }
{STRING}    { printf("TEXTO(%s)\n", yytext); }

"+"         { printf("OP_MAIS\n"); }
"-"         { printf("OP_MENOS\n"); }
"**"        { printf("OP_POTENCIA\n"); }
"*"         { printf("OP_MULTIPLICACAO\n"); }
"//"        { printf("OP_DIVISAO_INTEIRA\n"); }
"/"         { printf("OP_DIVISAO\n"); }
"%"         { printf("OP_MODULO\n"); }
"@"         { printf("OP_MATRIZ_MULT\n"); }
"<<"        { printf("OP_DESLOC_ESQUERDA\n"); }
">>"        { printf("OP_DESLOC_DIREITA\n"); }
"&"         { printf("OP_E_BIT\n"); }
"|"         { printf("OP_OU_BIT\n"); }
"^"         { printf("OP_XOU_BIT\n"); }
"~"         { printf("OP_NAO_BIT\n"); }
"<="        { printf("OP_MENOR_IGUAL\n"); }
">="        { printf("OP_MAIOR_IGUAL\n"); }
"=="        { printf("OP_IGUAL_IGUAL\n"); }
"!="        { printf("OP_DIFERENTE\n"); }
"<"         { printf("OP_MENOR\n"); }
">"         { printf("OP_MAIOR\n"); }
"+="        { printf("OP_ATRIBUICAO_MAIS\n"); }
"-="        { printf("OP_ATRIBUICAO_MENOS\n"); }
"**="       { printf("OP_ATRIBUICAO_POTENCIA\n"); }
"*="        { printf("OP_ATRIBUICAO_MULTIPLICACAO\n"); }
"//="       { printf("OP_ATRIBUICAO_DIVISAO_INTEIRA\n"); }
"/="        { printf("OP_ATRIBUICAO_DIVISAO\n"); }
"%="        { printf("OP_ATRIBUICAO_MODULO\n"); }
"&="        { printf("OP_ATRIBUICAO_E_BIT\n"); }
"|="        { printf("OP_ATRIBUICAO_OU_BIT\n"); }
"^="        { printf("OP_ATRIBUICAO_XOU_BIT\n"); }
"<<="       { printf("OP_ATRIBUICAO_DESLOC_ESQUERDA\n"); }
">>="       { printf("OP_ATRIBUICAO_DESLOC_DIREITA\n"); }
"="         { printf("OP_ATRIBUICAO\n"); }
"("         { printf("PARENTESES_ESQ\n"); }
")"         { printf("PARENTESES_DIR\n"); }
"["         { printf("COLCHETE_ESQ\n"); }
"]"         { printf("COLCHETE_DIR\n"); }
"{"         { printf("CHAVE_ESQ\n"); }
"}"         { printf("CHAVE_DIR\n"); }
","         { printf("VIRGULA\n"); }
":"         { printf("DOIS_PONTOS\n"); }
"."         { printf("PONTO\n"); }
";"         { printf("PONTO_E_VIRGULA\n"); }
"->"        { printf("SETA\n"); }

"#".*       { }

[ \t]+      { }
\n          { linha_atual++; printf("NOVA_LINHA\n"); }

.           { printf("ERRO LÉXICO: Caractere desconhecido '%s' na linha %d\n", yytext, linha_atual); }

%%

int main(int argc, char **argv) {
    printf("Iniciando analisador lexico...\n");
    yylex();
    printf("\nAnalise concluida com sucesso.\n");
    return 0;
}
