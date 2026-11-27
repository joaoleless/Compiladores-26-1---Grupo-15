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

### Escopo do subconjunto

O subconjunto foi reduzido a um núcleo mínimo: `int`, `float`, `str`, `bool` e
`None`; operadores aritméticos, relacionais e lógicos; atribuição simples;
`if/else`, `while`, `break`, `continue` e `pass`; e as funções embutidas
`print`, `int`, `float` e `str`. Há um único escopo global.

Ficam **fora** do escopo: funções definidas pelo usuário, listas, `for`/`range`,
`elif` e atribuição composta. O lexer ainda tokeniza essas e outras palavras e
símbolos do Python, mas o parser os rejeita com `ERRO DE SINTAXE`.

---

## Guia Rápido (Compilação, Testes e Execução)

> **Pré-requisitos:** `gcc`, `flex`, `bison`, `make` e `python3` (ambiente Linux ou WSL recomendado).

### 1. Compilar o interpretador
```bash
make
```
Gera o executável `compilador` na raiz do projeto.

### 2. Rodar a bateria de testes automatizados
O projeto conta com **98 testes automatizados** cobrindo análise léxica, sintática, semântica e execução:
```bash
# Executa todos os 98 testes:
make test

# Ou execute por etapa isoladamente:
make test-lexer     # 28 testes unitários do lexer
make test-parser    # 70 testes do parser e interpretador
```
*(Saída esperada: `Ran 28 tests ... OK` e `Ran 70 tests ... OK`)*

### 3. Executar manualmente um programa Python
```bash
# Executar a partir de um arquivo:
./compilador caminho/para/programa.py

# Ou interativamente via terminal (digite o código e envie EOF com Ctrl+D):
./compilador
```

### 4. Limpar arquivos compilados
```bash
make clean
```

---

## Estrutura do Repositório

```text
.
├── docs/
│   ├── decisoes-tecnicas.md     # Decisões técnicas, com justificativa e alternativa descartada
│   ├── problemas-e-solucoes.md  # Problemas encontrados e soluções adotadas
│   └── sprints.md               # Ciclos de trabalho do projeto
├── lexer/
│   ├── lexer.l                  # Analisador léxico (Flex)
│   ├── suite_testes_lexer.py    # Arquivo de entrada usado nos testes do lexer
│   └── Relatorio_Testes_Lexer.md # Relatório de cobertura dos testes léxicos
├── parser/
│   └── parser.y                 # Gramática (Bison), construção da AST, análise semântica e interpretador
├── src/
│   ├── ast.c / ast.h            # Árvore sintática abstrata: tipo Node, mknode, append_list
│   ├── valores.c / valores.h    # Valores em tempo de execução: tipo Value, conversões, impressão
│   ├── tabela.c / tabela.h      # Tabela de símbolos: escopo global de variáveis
│   └── main.c                   # Ponto de entrada do compilador
├── tests/
│   ├── lexer_driver.c           # Driver isolado para testar tokens do lexer
│   ├── test_lexer.py            # Testes automatizados do lexer
│   └── test_parser.py           # Testes do parser e do interpretador
└── Makefile                     # Automação da compilação e dos testes
```

Os arquivos gerados pelo Bison e pelo Flex ficam em `parser/` e `lexer/` e não
devem ser editados manualmente.

### Documentação

| Documento | Conteúdo |
| --- | --- |
| [Decisões técnicas](docs/decisoes-tecnicas.md) | Por que cada decisão foi tomada e qual alternativa foi descartada: escopo da linguagem, interpretação da AST, escopo global, tratamento da indentação no lexer, erros léxicos |
| [Problemas encontrados e soluções](docs/problemas-e-solucoes.md) | Comportamentos que divergiam do Python, causa, solução adotada e commit de cada correção; decisões sobre erros de execução e inteiros de 64 bits |
| [Sprints](docs/sprints.md) | Ciclos de trabalho do projeto, com período, objetivo e commits entregues em cada um |

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
| `INTEIRO` | `[0-9]+` (até 64 bits; maior que isso é erro léxico) | `42`, `0`, `1000` |
| `FLUTUANTE` | `[0-9]+"."[0-9]*` ou `"."[0-9]+`, com expoente opcional `[eE][+-]?[0-9]+`; ou `[0-9]+` com expoente | `3.14`, `.5`, `1.`, `1e3`, `2.5e-1` |
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

> **Nota:** o lexer reconhece mais tokens do que o parser aceita (veja
> *Escopo do subconjunto*). Os testes e o relatório do lexer cobrem a
> tokenização completa; o subconjunto executável é definido pelo parser.

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

Caracteres não reconhecidos geram uma mensagem de erro com o número da linha. A análise prossegue (para reportar todos os erros), mas o contador `erros_lexicos` é incrementado e o executável termina com código de saída `1` **sem executar o programa**:

```c
. { fprintf(stderr, "ERRO LEXICO: caractere desconhecido '%s' na linha %d\n", yytext, linha_atual); }
```

Erros de indentação inconsistente (onde o nível de recuo não corresponde a nenhum nível anterior) também são reportados via `stderr` e contam em `erros_lexicos`.

---

## Etapa 2 — Análise Sintática e Interpretação (`parser/parser.y`)

### Visão Geral

Depois da análise léxica, o programa passa por três fases:

1. **Análise sintática** (`parser/parser.y`): o Bison verifica se a sequência de tokens segue a gramática e, em caso afirmativo, constrói uma **Árvore Sintática Abstrata (AST)**.
2. **Análise semântica** (`check_stmts`, em `parser/parser.y`): percorre a AST antes da execução e recusa o programa se encontrar `break` ou `continue` fora de laço.
3. **Interpretação** (`eval_expr`, `exec_stmt` e `exec_block`, em `parser/parser.y`): executa o programa percorrendo a AST, sem etapa de geração de código intermediário.

As estruturas usadas nessas fases ficam em módulos próprios em `src/`: a AST
(`ast.c`/`ast.h`), os valores em tempo de execução (`valores.c`/`valores.h`) e a
tabela de símbolos (`tabela.c`/`tabela.h`).

### Árvore Sintática Abstrata (`src/ast.h`)

Todos os nós da AST são instâncias de um único tipo `Node`:

```c
typedef struct Node {
    NodeType type;   // tipo do nó (enum NodeType)
    long ival;       // valor inteiro (literal int, operador, etc.)
    double dval;     // valor real (literal float)
    char *sval;      // nome de variável/função ou conteúdo de string
    struct Node *a, *b, *c; // filhos (semântica depende do tipo)
    struct Node *next;      // próximo elemento (lista de statements e argumentos)
} Node;
```

**Tipos de nós (`NodeType`):**

| Categoria | Tipos |
| --- | --- |
| Estrutural | `N_PROGRAM`, `N_BLOCK` |
| Literais | `N_INT`, `N_FLOAT`, `N_STR`, `N_BOOL`, `N_NONE` |
| Referência | `N_VAR` |
| Operações | `N_BINOP`, `N_UNOP`, `N_BOOLOP`, `N_COMPARE`, `N_NOT` |
| Atribuição | `N_ASSIGN` |
| Controle de fluxo | `N_IF`, `N_WHILE`, `N_PASS`, `N_BREAK`, `N_CONTINUE` |
| Chamadas | `N_CALL` (somente funções embutidas) |
| Instrução de expressão | `N_EXPRSTMT` |

### Gramática Implementada

#### Estrutura geral

```
program  → stmts
stmts    → stmt | stmts stmt
stmt     → simple_stmt NOVA_LINHA | compound_stmt
```

#### Instruções simples (`simple_stmt`)

```
simple_stmt → expr_stmt | pass_stmt | break_stmt | continue_stmt

expr_stmt   → IDENTIFICADOR = expr          (atribuição simples)
            | expr                           (expressão avulsa)
```

#### Instruções compostas (`compound_stmt`)

**Condicional:**
```
if_stmt → IF expr : block
        | IF expr : block ELSE : block
```

**Laço `while`:**
```
while_stmt → WHILE expr : block
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
| 9 (mais alto) | `func(args)` (apenas embutidas) | Esquerda |

### Sistema de Valores em Tempo de Execução (`src/valores.h`)

O interpretador representa todos os valores por meio do tipo `Value`:

```c
typedef enum { V_INT, V_FLOAT, V_STR, V_BOOL, V_NONE } VType;

typedef struct Value {
    VType type;
    long ival;         // V_INT, V_BOOL
    double dval;       // V_FLOAT
    char *sval;        // V_STR
} Value;
```

**Conversão de tipos:**
- Operações aritméticas entre `int` e `float` promovem o resultado para `float`.
- A função `truthy()` determina a veracidade de um valor (análogo ao comportamento do Python):
  - `0`, `0.0`, `""`, `None`, `False` → falso
  - Qualquer outro valor → verdadeiro

**Impressão de valores (`print_value`):**
- Inteiros: formato `%ld`
- Reais: menor representação que preserva o valor, como o `repr` do Python: notação fixa para expoentes de -4 a 15 (`2.0`, `0.1`, `1500.0`) e científica fora disso (`1e-05`, `1e+20`)
- Booleanos: `True` ou `False`
- `None`: `None`

### Tabela de Símbolos (`src/tabela.h`)

Existe um **único escopo global**, mantido como lista encadeada de variáveis:

```c
typedef struct Scope {
    VarEntry *vars;    // lista encadeada de variáveis
} Scope;
```

- `scope_get()` procura a variável no escopo e devolve `NULL` se ela não existir; nesse caso o interpretador interrompe a execução com erro (equivalente ao `NameError` do Python).
- `scope_set()` atualiza a variável ou cria uma nova entrada.

### Análise Semântica

Antes de executar, `check_stmts()` percorre a AST guardando se está dentro de um
`while`. Um `break` ou `continue` fora de laço gera
`ERRO SEMANTICO: 'break' fora de laco`; havendo algum erro semântico, o programa
não é executado e o código de saída é `1`, como o `SyntaxError` do Python.

Variável não definida e tipos incompatíveis continuam sendo detectados durante a
execução, quando a linha é alcançada, também como no Python. A justificativa
está em [docs/decisoes-tecnicas.md](docs/decisoes-tecnicas.md).

### Funções Embutidas

| Função | Comportamento |
| --- | --- |
| `print(*args)` | Imprime os argumentos separados por espaço, seguido de `\n` |
| `int(x)` | Converte `int`, `bool`, `float` (truncando) ou `str` com um inteiro (`" 42 "`) para inteiro; `int()` devolve `0` |
| `float(x)` | Converte número ou `str` com um número (`"3.5"`, `"1e3"`) para ponto flutuante; `float()` devolve `0.0` |
| `str(x)` | Converte para string |

### Sinalizadores de Controle de Fluxo

Para propagar `break` e `continue` pelas chamadas de `exec_stmt` e `exec_block`, o interpretador usa uma flag global:

```c
typedef enum { SIG_NONE, SIG_BREAK, SIG_CONTINUE } Signal;
static Signal signal_flag = SIG_NONE;
```

Cada nível de execução verifica `signal_flag` após executar cada instrução e interrompe o bloco se necessário.

### Operações Suportadas pelo Interpretador

#### Aritméticas

| Operação | Comportamento |
| --- | --- |
| `a + b` | Soma numérica ou concatenação de duas strings |
| `a - b` | Subtração |
| `a * b` | Multiplicação, ou repetição de string (`"ab" * 3`, `3 * "ab"`) |
| `a / b` | Divisão (sempre resulta em `float`) |
| `a // b` | Divisão inteira arredondada para baixo (`-7 // 2` é `-4`); `float` se algum operando for `float` |
| `a % b` | Módulo com o sinal do divisor (`-7 % 3` é `2`), como no Python |
| `a ** b` | Potenciação: inteira (exponenciação por quadrados) para `int ** int` com expoente ≥ 0; `pow()` da `math.h` nos demais casos |

`bool` participa das operações como inteiro (`True + 1` é `2`). Operações com
`str` ou `None` fora dos casos acima geram `TypeError`.

#### Relacionais

Suporte a `<`, `>`, `<=`, `>=`, `==`, `!=` entre números (incluindo `bool`) e entre
strings (comparação lexicográfica). Entre tipos diferentes ou com `None`, `==` e
`!=` comparam como o Python (`"a" == 0` é `False`, `None == None` é `True`), e
`<`, `>`, `<=`, `>=` geram `TypeError`.

### Erros de Execução

Como no Python, um erro de execução interrompe o programa: a mensagem vai para
`stderr`, a saída já impressa é preservada e o código de saída é `1`.

| Situação | Mensagem |
| --- | --- |
| Variável ou função não definida | `ERRO: variavel 'x' nao definida` |
| Divisão, divisão inteira ou módulo por zero | `ERRO: ZeroDivisionError: ...` |
| Operação com tipos incompatíveis (`"a" + 1`, `None < 1`) | `ERRO: TypeError: ...` |
| Conversão inválida (`int("abc")`) | `ERRO: ValueError: ...` |
| Inteiro fora de 64 bits (`2 ** 100`) | `ERRO: OverflowError: ...` |

**Limitação conhecida:** os inteiros do interpretador são de 64 bits (`long`),
enquanto o Python não tem limite. Em vez de produzir um resultado errado, o
estouro gera `OverflowError`.

---

## Testes do Analisador Léxico

Os testes automatizados são executados por `tests/test_lexer.py`. O arquivo
`tests/lexer_driver.c` expõe os tokens produzidos pelo lexer sem executar o
parser. O arquivo `lexer/suite_testes_lexer.py` continua disponível como um
programa de entrada abrangente, e o relatório completo está em
`lexer/Relatorio_Testes_Lexer.md`.

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

## Testes do Parser e do Interpretador

`tests/test_parser.py` compila programas temporários com o executável completo
e verifica a saída produzida. A suíte cobre:

- precedência, associatividade e parênteses; `**`, `/`, `//`, `%` e promoção `int`/`float`;
- operadores unários, relacionais e lógicos (retorno de operandos e curto-circuito) e veracidade (`truthy`);
- strings (concatenação, comparação, escapes), `print` com vários argumentos e conversões `int`/`float`/`str`;
- variáveis (reatribuição, mudança de tipo) e erros de execução (variável/função inexistente, divisão por zero, tipos incompatíveis, conversão inválida e estouro de inteiro), com código de saída `1`;
- `if/else` (aninhado e em linha única), `while`, `break`, `continue`, `pass` e laços aninhados;
- análise semântica: `break`/`continue` fora de laço recusados antes da execução (código de saída `1` e nenhuma saída);
- comentários, linhas em branco, expressões multi-linha entre parênteses e programa vazio;
- construções fora do subconjunto (`def`, `for`, listas, `+=`, `elif`) e outros erros sintáticos, com código de saída diferente de zero e sem saída parcial;
- erros léxicos e de indentação (código de saída `1` e nenhuma execução);
- saída de `float` idêntica à do Python (`2.0`, `0.30000000000000004`, `1e+20`);
- erros do executável (arquivo inexistente e excesso de argumentos).

`tests/test_lexer.py` usa o driver `tests/lexer_driver.c`, que imprime o nome de
cada token, e cobre: palavras reservadas (todas), identificadores, literais
numéricos e de texto, operadores (aritméticos, bitwise, compostos e *longest
match*), delimitadores, comentários, `INDENT`/`DEDENT` (múltiplos, EOF, tabs e
linhas em branco), supressão de `NOVA_LINHA` entre `()`, `[]` e `{}`, e erros
léxicos e de indentação reportados no `stderr`.

Para executar todas as suítes:

```bash
make test
```

Também é possível executar cada grupo separadamente com `make test-lexer` ou
`make test-parser`.

---

## Como Compilar e Executar

> **Pré-requisitos:** `flex`, `bison` e `gcc` instalados (ambiente Linux/WSL recomendado).

```bash
# Gerar os arquivos e compilar o executável
make

# Executar um programa Python de exemplo
./compilador meu_programa.py
```

Para remover os binários e arquivos gerados:

```bash
make clean
```

O ponto de entrada está em `src/main.c`: ele chama o parser e, se não houver
erros léxicos nem sintáticos, chama `executar_programa()` (em `parser/parser.y`),
que faz a análise semântica e executa o programa.

---

## Exemplo de Programa Suportado

```python
n = 5
resultado = 1
while n > 1:
    resultado = resultado * n
    n = n - 1
print("Fatorial de 5:", resultado)

if resultado > 100.5:
    print("grande")
else:
    print("pequeno")
```

### Recursos atualmente suportados

- Tipos: inteiro, ponto flutuante, string, booleano, `None`
- Operadores aritméticos, relacionais e lógicos
- Atribuição simples
- `if / else`
- `while` com `break`, `continue` e `pass`
- Funções embutidas: `print`, `int`, `float`, `str`
- Indentação Python-style (INDENT/DEDENT)
- Comentários com `#`
- Expressões multi-linha dentro de `(`, `[`, `{`

### Recursos ainda não implementados

- Funções definidas pelo usuário (`def`, `return`)
- Listas, indexação e `for`/`range()`
- `elif` e atribuição composta (`+=`, etc.)
- Classes e orientação a objetos
- Tratamento de exceções (`try/except/finally`)
- Importação de módulos (`import`)
- Funções anônimas (`lambda`)
- Tuplas e dicionários
- Funções aninhadas e closures
- Geração de código intermediário ou bytecode

---

## Status do Projeto

| Componente | Status |
| --- | --- |
| Analisador léxico (`lexer.l`) | Completo ✅ |
| Testes do lexer | Completo ✅ |
| Analisador sintático (`parser.y`) | Completo para o subconjunto definido ✅ |
| Análise semântica (`break`/`continue` fora de laço) | Completo ✅ |
| Interpretador (epílogo do `parser.y`) | Completo para o subconjunto definido ✅ |
| AST, valores e tabela de símbolos (`src/`) | Completo ✅ |
| Ponto de entrada (`main.c`) | Completo ✅ |
| Makefile | Completo ✅ |
| Testes do parser e interpretador | Completo ✅ |
| Tratamento de erros léxicos e sintáticos | Completo ✅ (mensagens com linha; sem coluna) |
| Tratamento de erros de execução | Completo ✅ (interrompe com código `1`; sem número de linha) |
| Cobertura de casos negativos e de todos os comandos | Completo ✅ |

Esta é a versão final do projeto. O interpretador implementa todo o
subconjunto definido em **Escopo do subconjunto**, com testes automatizados para
cada funcionalidade e para os casos de erro.

### Limitações conhecidas

- **Inteiros de 64 bits:** o Python não tem limite para inteiros; aqui, um
  resultado fora do intervalo de 64 bits gera `OverflowError` (ver
  [docs/problemas-e-solucoes.md](docs/problemas-e-solucoes.md)).
- **Mensagens de erro:** os erros léxicos e sintáticos informam a linha, mas não
  a coluna; os erros de execução não informam a linha, porque a AST não guarda a
  posição de cada nó.
- **Memória:** a AST, os valores e a tabela de símbolos não são liberados ao fim
  da execução. Como o processo termina logo em seguida, o sistema operacional
  recupera a memória, mas um uso como biblioteca exigiria essa liberação.
- **Tabs e espaços misturados:** o Python 3 recusa (`TabError`) indentação que
  mistura tabs e espaços de forma ambígua; o interpretador não faz essa
  verificação.

