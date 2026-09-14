# Grupo 15 — Interpretador de Python

Repositório do Grupo 15 da disciplina **Compiladores 1**.

## Integrantes

| Integrante | Matrícula |
| --- | --- |
| João Eduardo de Souza Leles | 241025274 |
| Guilherme Lorenzi Ventura | 241011116 |
| Luiz Henrique Pallavicini | 241012329 |
| Gabriel Pereira da Silva | 221008641 |
| Rafael Silva Wasconcelos | 232030364 |

---

## Objetivo do Projeto

Construir um **interpretador de um subconjunto da linguagem Python**, desenvolvido a partir das etapas clássicas de um compilador:

1. **Análise Léxica** — tokenização do código-fonte
2. **Análise Sintática** — reconhecimento da estrutura gramatical e construção da AST
3. **Interpretação** — execução das estruturas reconhecidas diretamente sobre a AST

As ferramentas utilizadas são **Flex** (gerador de analisadores léxicos) e **Bison** (gerador de analisadores sintáticos), com código de suporte escrito em C.

---

## Estrutura do Repositório

```text
.
├── lexer/
│   ├── lexer.l                  # Analisador léxico (Flex)
│   ├── suite_testes_lexer.py    # Arquivo de entrada usado nos testes do lexer
│   └── Relatorio_Testes_Lexer.md # Relatório de cobertura dos testes léxicos
├── parser/
│   └── parser.y                 # Analisador sintático e interpretador (Bison)
├── src/
│   └── main.c                   # Ponto de entrada (a ser integrado)
├── lex.yy.c                     # Código C gerado pelo Flex (não editar manualmente)
├── compilador                   # Binário pré-compilado (Linux ELF 64-bit)
└── Makefile                     # Automação da compilação
```

---

## Etapa 1 — Análise Léxica (`lexer/lexer.l`)

### Visão Geral

O analisador léxico é implementado em Flex e é responsável por transformar o código-fonte Python em uma sequência de **tokens** que serão consumidos pelo parser. O maior desafio desta etapa é o tratamento da **indentação sensível ao contexto** do Python, que não é suportada nativamente pelos mecanismos tradicionais de tokenização.

### Tokens Reconhecidos

#### Palavras Reservadas (32 no total)

| Grupo | Palavras-chave |
| --- | --- |
| Controle de fluxo | `if`, `elif`, `else`, `while`, `for`, `break`, `continue`, `return` |
| Definição | `def`, `class`, `lambda` |
| Tratamento de exceções | `try`, `except`, `finally`, `raise`, `assert` |
| Operadores lógicos | `and`, `or`, `not`, `in`, `is` |
| Importação | `import`, `from`, `as` |
| Literais especiais | `True`, `False`, `None` |
| Avançados | `async`, `await`, `with`, `yield`, `global`, `nonlocal`, `del` |

#### Literais

| Token | Padrão Flex | Exemplo |
| --- | --- | --- |
| `INTEIRO` | `[0-9]+` | `42`, `0`, `1000` |
| `FLUTUANTE` | `[0-9]+"."[0-9]*` ou `"."[0-9]+` | `3.14`, `.5`, `1.` |
| `TEXTO` | Aspas simples ou duplas com escapes | `"olá\n"`, `'teste\'s'` |
| `IDENTIFICADOR` | `[a-zA-Z_][a-zA-Z0-9_]*` | `x`, `minha_funcao`, `_tmp` |

#### Operadores

| Categoria | Operadores |
| --- | --- |
| Aritméticos | `+` `-` `*` `/` `//` `%` `**` `@` |
| Bitwise | `&` `\|` `^` `~` `<<` `>>` |
| Relacionais | `<` `>` `<=` `>=` `==` `!=` |
| Atribuição simples | `=` |
| Atribuição composta | `+=` `-=` `*=` `/=` `//=` `%=` `**=` `&=` `\|=` `^=` `<<=` `>>=` |

#### Delimitadores

`(` `)` `[` `]` `{` `}` `,` `:` `.` `;`

#### Tokens Sintéticos (específicos do Python)

| Token | Descrição |
| --- | --- |
| `NOVA_LINHA` | Fim de uma linha lógica (não emitido dentro de parênteses/colchetes abertos) |
| `INDENT` | Aumento de nível de indentação |
| `DEDENT` | Redução de nível de indentação |

### Mecanismo de Indentação (estado `BOL`)

O controle de indentação é o componente mais sofisticado do lexer. Ele usa um **estado exclusivo `<BOL>` (Beginning Of Line)** que é ativado sempre que o lexer processa uma quebra de linha.

**Estruturas de controle:**

```c
static int indent_stack[MAX_INDENT]; // pilha dos níveis de indentação vistos
static int indent_top = 0;           // topo da pilha (nível atual)
static int pending_dedents = 0;      // DEDENTs ainda a serem emitidos
static int paren_depth = 0;          // profundidade de ( [ { abertos
```

**Funcionamento do estado `<BOL>`:**

1. Ao encontrar uma quebra de linha (`\n`) no estado `INITIAL`, o lexer emite `NOVA_LINHA`, incrementa `linha_atual` e transita para o estado `<BOL>`.
2. No estado `<BOL>`:
   - Linhas em branco e linhas contendo apenas comentários são **ignoradas** (não geram tokens).
   - Os espaços de indentação são contados via `process_indent()`.
   - Tabs são convertidos para o equivalente em espaços (regra de 8 colunas: `indent += 8 - (indent % 8)`).
3. A função `process_indent()` compara o nível atual com o topo da pilha:
   - **Maior:** empilha o novo nível e retorna `INDENT`.
   - **Menor:** desempilha níveis até encontrar o nível correspondente, configura `pending_dedents` para os DEDENTs extras, e retorna o primeiro `DEDENT`.
   - **Igual:** nenhuma ação, retorna ao estado `INITIAL`.
4. A função `yylex()` — escrita manualmente — verifica `pending_dedents > 0` antes de cada chamada ao Flex, garantindo que todos os DEDENTs pendentes sejam emitidos antes do próximo token real.

**Tratamento no EOF:**

```c
<BOL><<EOF>> {
    if (indent_top > 0) {
        pending_dedents = indent_top - 1;
        indent_top = 0;
        return DEDENT;
    }
    yyterminate();
}
```

Ao final do arquivo, se ainda houver blocos abertos na pilha, o lexer emite os DEDENTs necessários para fechar todos os escopos.

**Supressão de `NOVA_LINHA` dentro de estruturas:**

```c
\n {
    linha_atual++;
    if (paren_depth > 0) {
        /* dentro de ( [ { : quebras de linha não contam */
    } else {
        BEGIN(BOL);
        return NOVA_LINHA;
    }
}
```

Isso permite expressões multi-linha como:
```python
lista = [
    1, 2,
    3
]
```

### Processamento de Strings (`unescape`)

A função auxiliar `unescape()` em C processa o conteúdo das strings, removendo as aspas delimitadoras e convertendo as sequências de escape:

| Escape no código | Caractere resultante |
| --- | --- |
| `\n` | Quebra de linha |
| `\t` | Tabulação |
| `\\` | Barra invertida |
| `\"` | Aspas duplas |
| `\'` | Aspas simples |

### Tratamento de Erros Léxicos

Caracteres não reconhecidos geram uma mensagem de erro com o número da linha, sem interromper a análise:

```c
. { fprintf(stderr, "ERRO LEXICO: caractere desconhecido '%s' na linha %d\n", yytext, linha_atual); }
```

Erros de indentação inconsistente (onde o nível de recuo não corresponde a nenhum nível anterior) também são reportados via `stderr`.

---

## Etapa 2 — Análise Sintática e Interpretação (`parser/parser.y`)

### Visão Geral

O analisador sintático é implementado em Bison e realiza duas funções simultâneas:

1. **Reconhecimento sintático:** verifica se a sequência de tokens segue a gramática definida e, em caso afirmativo, constrói uma **Árvore Sintática Abstrata (AST)**.
2. **Interpretação:** após a construção da AST, executa o programa diretamente, sem etapa de geração de código intermediário.

### Árvore Sintática Abstrata (AST)

Todos os nós da AST são instâncias de um único tipo `Node`:

```c
typedef struct Node {
    NodeType type;   // tipo do nó (enum NodeType)
    long ival;       // valor inteiro (literal int, operador, etc.)
    double dval;     // valor real (literal float)
    char *sval;      // nome de variável/função ou conteúdo de string
    struct Node *a, *b, *c; // filhos (semântica depende do tipo)
    struct Node *next;      // próximo elemento (lista de statements, argumentos, etc.)
} Node;
```

**Tipos de nós (`NodeType`):**

| Categoria | Tipos |
| --- | --- |
| Estrutural | `N_PROGRAM`, `N_BLOCK` |
| Literais | `N_INT`, `N_FLOAT`, `N_STR`, `N_BOOL`, `N_NONE` |
| Referência e coleção | `N_VAR`, `N_LIST` |
| Operações | `N_BINOP`, `N_UNOP`, `N_BOOLOP`, `N_COMPARE`, `N_NOT` |
| Atribuição | `N_ASSIGN`, `N_AUGASSIGN` |
| Controle de fluxo | `N_IF`, `N_WHILE`, `N_FOR`, `N_PASS`, `N_BREAK`, `N_CONTINUE` |
| Funções | `N_FUNCDEF`, `N_CALL`, `N_RETURN` |
| Acesso | `N_INDEX`, `N_EXPRSTMT` |

### Gramática Implementada

#### Estrutura geral

```
program  → stmts
stmts    → stmt | stmts stmt
stmt     → simple_stmt NOVA_LINHA | compound_stmt
```

#### Instruções simples (`simple_stmt`)

```
simple_stmt → expr_stmt | pass_stmt | break_stmt | continue_stmt | return_stmt

expr_stmt   → IDENTIFICADOR = expr          (atribuição simples)
            | IDENTIFICADOR <op_atr> expr   (atribuição composta)
            | expr                           (expressão avulsa)
```

Operadores de atribuição composta suportados: `+=`, `-=`, `*=`, `/=`, `//=`, `%=`.

#### Instruções compostas (`compound_stmt`)

**Condicional:**
```
if_stmt → IF expr : block elif_chain
elif_chain → ELIF expr : block elif_chain
           | ELSE : block
           | (vazio)
```

**Laço `while`:**
```
while_stmt → WHILE expr : block
```

**Laço `for`:**
```
for_stmt → FOR IDENTIFICADOR IN expr : block
```

**Definição de função:**
```
funcdef_stmt → DEF IDENTIFICADOR ( paramlist_opt ) : block
paramlist    → IDENTIFICADOR | paramlist , IDENTIFICADOR
```

**Bloco:**
```
block → NOVA_LINHA INDENT stmts DEDENT   (bloco indentado)
      | simple_stmt NOVA_LINHA            (bloco de linha única)
```

#### Expressões (precedência crescente)

| Nível | Operação | Associatividade |
| --- | --- | --- |
| 1 (mais baixo) | `or` | Esquerda |
| 2 | `and` | Esquerda |
| 3 | `not` | Direita (prefixo) |
| 4 | `<` `>` `<=` `>=` `==` `!=` | Não-associativo |
| 5 | `+` `-` (binário) | Esquerda |
| 6 | `*` `/` `//` `%` | Esquerda |
| 7 | `-` `+` (unário) | Direita (prefixo) |
| 8 | `**` (potência) | Direita |
| 9 (mais alto) | `func(args)`, `lst[idx]` | Esquerda |

### Sistema de Valores em Tempo de Execução

O interpretador representa todos os valores por meio do tipo `Value`:

```c
typedef enum { V_INT, V_FLOAT, V_STR, V_BOOL, V_NONE, V_LIST } VType;

typedef struct Value {
    VType type;
    long ival;         // V_INT, V_BOOL
    double dval;       // V_FLOAT
    char *sval;        // V_STR
    struct Value **items; // V_LIST
    int nitems;           // V_LIST
} Value;
```

**Conversão de tipos:**
- Operações aritméticas entre `int` e `float` promovem o resultado para `float`.
- A função `truthy()` determina a veracidade de um valor (análogo ao comportamento do Python):
  - `0`, `0.0`, `""`, `[]`, `None`, `False` → falso
  - Qualquer outro valor → verdadeiro

**Impressão de valores (`print_value`):**
- Inteiros: formato `%ld`
- Reais: formato `%g` (sem zeros à direita desnecessários)
- Booleanos: `True` ou `False`
- `None`: `None`
- Listas: `[item1, item2, ...]` (recursivo)

### Gerenciamento de Escopo e Variáveis

As variáveis são gerenciadas por meio de uma cadeia de **escopos encadeados**:

```c
typedef struct Scope {
    VarEntry *vars;    // lista encadeada de variáveis locais
    struct Scope *parent; // escopo pai (para resolução de nomes)
} Scope;
```

- `scope_get()` percorre a cadeia escopo → pai → avô… até encontrar a variável.
- `scope_set()` atualiza a variável no escopo atual ou cria uma nova entrada.
- Funções são executadas em um escopo próprio (`new_scope(NULL)`) — sem acesso ao escopo global de chamada.

### Funções Definidas pelo Usuário

Funções são registradas em uma lista encadeada global:

```c
typedef struct FuncEntry {
    char *name;
    Node *def;  // nó N_FUNCDEF com parâmetros (a) e corpo (b)
    struct FuncEntry *next;
} FuncEntry;
```

Na chamada:
1. Cria-se um escopo local novo.
2. Os argumentos são avaliados no escopo do chamador e associados aos parâmetros no escopo local.
3. O corpo é executado no escopo local.
4. O valor de retorno é propagado via `return_value` e a flag `SIG_RETURN`.

### Funções Embutidas

| Função | Comportamento |
| --- | --- |
| `print(*args)` | Imprime os argumentos separados por espaço, seguido de `\n` |
| `range(stop)` | Gera lista de `0` a `stop-1` |
| `range(start, stop)` | Gera lista de `start` a `stop-1` |
| `range(start, stop, step)` | Gera lista com incremento `step` (positivo ou negativo) |
| `len(obj)` | Retorna o comprimento de uma lista ou string |
| `int(x)` | Converte para inteiro |
| `float(x)` | Converte para ponto flutuante |
| `str(x)` | Converte para string |

### Sinalizadores de Controle de Fluxo

Para propagar `break`, `continue` e `return` através da pilha de chamadas de `exec_stmt` e `exec_block`, o interpretador usa uma flag global:

```c
typedef enum { SIG_NONE, SIG_BREAK, SIG_CONTINUE, SIG_RETURN } Signal;
static Signal signal_flag = SIG_NONE;
static Value *return_value = NULL;
```

Cada nível de execução verifica `signal_flag` após executar cada instrução e interrompe o bloco se necessário.

### Operações Suportadas pelo Interpretador

#### Aritméticas

| Operação | Comportamento |
| --- | --- |
| `a + b` | Soma numérica ou concatenação de strings |
| `a - b` | Subtração |
| `a * b` | Multiplicação |
| `a / b` | Divisão (sempre resulta em `float`) |
| `a // b` | Divisão inteira (`floor`) |
| `a % b` | Módulo |
| `a ** b` | Potenciação (usa `pow()` da `math.h`) |

#### Relacionais

Suporte completo a `<`, `>`, `<=`, `>=`, `==`, `!=` para números e strings (comparação lexicográfica).

#### Acesso a Coleções

- **Indexação de listas:** `lista[i]` — suporta índices negativos (Python-style).
- **Indexação de strings:** `texto[i]` — retorna uma string de 1 caractere.
- Ambos emitem erro no `stderr` em caso de índice fora dos limites.

---

## Testes do Analisador Léxico

O arquivo `lexer/suite_testes_lexer.py` contém casos de teste para validar o comportamento do lexer. O relatório completo está em `lexer/Relatorio_Testes_Lexer.md`.

### Cenários Cobertos

| Cenário | O que testa |
| --- | --- |
| 1 — Estado `<BOL>` | Linhas em branco e comentários não geram tokens espúrios |
| 2 — INDENT/DEDENT | Pilha de indentação funciona corretamente em funções e laços |
| 3 — Strings com escapes | `unescape()` processa `\n`, `\t`, `\'`, `\\` corretamente |
| 4 — `paren_depth` | Quebras de linha dentro de `[]` e `{}` não emitem `NOVA_LINHA` |
| 5 — Operadores complexos | `+=`, `//`, `**`, `>>`, `<<=` são tokenizados corretamente |
| 6 — DEDENTs no EOF | Blocos abertos ao final do arquivo geram os DEDENTs pendentes |

### Matriz de Cobertura

| Categoria Lexical | Exemplos | Status |
| :--- | :--- | :--- |
| Palavras reservadas | `def`, `for`, `in`, `return`, `if`, `and`, `not`, `True`, `False` | Coberto ✅ |
| Literais | `INTEIRO` (0, 1, 2), `FLUTUANTE` (0.0), `TEXTO` ("...") | Coberto ✅ |
| Operadores complexos | `+=`, `//`, `**`, `>>`, `<<=` | Coberto ✅ |
| Delimitadores | `(`, `)`, `[`, `]`, `{`, `}`, `:`, `,` | Coberto ✅ |
| Tokens sintéticos | `NOVA_LINHA`, `INDENT`, `DEDENT` | Coberto ✅ |

---

## Como Compilar e Executar

> **Pré-requisitos:** `flex`, `bison` e `gcc` instalados (ambiente Linux/WSL recomendado).

```bash
# Gerar o código C do lexer
flex -o lex.yy.c lexer/lexer.l

# Gerar o código C do parser
bison -d -o y.tab.c parser/parser.y

# Compilar tudo
gcc -o compilador lex.yy.c y.tab.c -lm

# Executar um programa Python de exemplo
./compilador meu_programa.py
```

> O Makefile ainda está em construção. As etapas acima devem ser executadas manualmente por enquanto.

---

## Exemplo de Programa Suportado

```python
def fatorial(n):
    if n <= 1:
        return 1
    return n * fatorial(n - 1)

resultado = fatorial(5)
print("Fatorial de 5:", resultado)

for i in range(1, 6):
    print(i)
```

### Recursos atualmente suportados

- Tipos: inteiro, ponto flutuante, string, booleano, `None`, lista
- Operadores aritméticos, relacionais e lógicos
- Atribuição simples e composta (`+=`, `-=`, etc.)
- `if / elif / else`
- `while` com `break` e `continue`
- `for` iterando sobre listas e `range()`
- Definição e chamada de funções com parâmetros e `return`
- Indexação de listas e strings (incluindo índices negativos)
- Funções embutidas: `print`, `range`, `len`, `int`, `float`, `str`
- Indentação Python-style (INDENT/DEDENT)
- Comentários com `#`
- Expressões multi-linha dentro de `(`, `[`, `{`

### Recursos ainda não implementados

- Classes e orientação a objetos
- Tratamento de exceções (`try/except/finally`)
- Importação de módulos (`import`)
- Compreensão de listas (`[x for x in ...]`)
- Funções anônimas (`lambda`)
- Tuplas e dicionários
- Slicing (`lista[1:3]`)
- Funções aninhadas e closures
- Geração de código intermediário ou bytecode

---

## Status do Projeto

| Componente | Status |
| --- | --- |
| Analisador léxico (`lexer.l`) | Completo ✅ |
| Testes do lexer | Completo ✅ |
| Analisador sintático (`parser.y`) | Completo ✅ |
| Interpretador (embutido no parser) | Funcional ✅ |
| Ponto de entrada (`main.c`) | Pendente ⏳ |
| Makefile | Pendente ⏳ |
| Testes do parser | Em andamento 🔄 |
