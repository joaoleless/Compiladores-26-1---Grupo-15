# Decisões técnicas

Cada decisão traz o que foi decidido, por que, e a alternativa que o grupo
deixou de lado.

## 1. Reduzir o escopo da linguagem

**Decisão:** em 06/11/2026 (commit `3d3929e`) o subconjunto de Python foi
reduzido a `int`, `float`, `str`, `bool` e `None`; operadores aritméticos,
relacionais e lógicos; atribuição simples; `if/else`, `while`, `break`,
`continue` e `pass`; e as funções embutidas `print`, `int`, `float` e `str`.
Saíram funções definidas pelo usuário (`def`/`return`), listas, `for`/`range`,
`elif` e atribuição composta (`+=` etc.).

**Por quê:** o escopo inicial tinha crescido mais rápido do que o domínio do
grupo sobre a implementação. Preferimos um núcleo menor, em que todos entendem
cada parte, para aprender e aplicar na prática o conteúdo da disciplina; para
esse objetivo, um escopo tão grande não era necessário.

**Alternativa descartada:** manter as funcionalidades que já existiam no código.
O interpretador ficaria maior, mas com partes que o grupo ainda não dominava.

**Consequência:** o lexer continua reconhecendo todas as palavras reservadas e
símbolos do Python, mas o parser rejeita o que está fora do subconjunto com
`ERRO DE SINTAXE`. Os testes cobrem essa rejeição.

## 2. Interpretar diretamente a AST

**Decisão:** o parser (Bison) constrói uma árvore sintática abstrata (AST) e o
interpretador a executa percorrendo os nós (`eval_expr` para expressões,
`exec_stmt` e `exec_block` para comandos). Não há código intermediário.

**Por quê:** é a forma mais direta de executar o programa depois da análise
sintática, e corresponde à etapa "interpretação da AST" do guia do professor.
Cada tipo de nó tem uma regra de execução própria, fácil de ler e de testar.

**Alternativa descartada:** gerar código intermediário (por exemplo, código de
três endereços ou bytecode) e executá-lo numa máquina virtual. Acrescentaria
uma tradução e uma máquina virtual sem benefício para o escopo do projeto.

## 3. Um único escopo global

**Decisão:** todas as variáveis ficam num único escopo global, uma lista
encadeada de pares nome/valor (`scope_get` e `scope_set`).

**Por quê:** sem funções definidas pelo usuário (decisão 1), não existe nenhum
ponto do programa que crie um escopo novo. Em Python, `if` e `while` não criam
escopo.

**Alternativa descartada:** escopos encadeados (um por chamada de função), que
existiam antes da redução de escopo e foram removidos junto com `def`.

## 4. Indentação tratada no lexer, com estado `BOL` e pilha de níveis

**Decisão:** o lexer tem um estado `BOL` (início de linha). Nele, mede a
indentação da linha e compara com a pilha de níveis (`indent_stack`): se
aumentou, empilha e emite `INDENT`; se diminuiu, desempilha até achar o nível e
emite um `DEDENT` para cada nível removido; se o nível não existir na pilha, é
erro de indentação. Linhas em branco e só com comentário são ignoradas.

**Por quê:** é como o próprio Python faz. Com `INDENT` e `DEDENT` como tokens, a
gramática do Bison trata um bloco como se fosse delimitado por chaves
(`block: NOVA_LINHA INDENT stmts DEDENT`), sem precisar saber de colunas.

**Alternativa descartada:** tratar a indentação no parser. A gramática do Bison
é livre de contexto e não tem como comparar a coluna de uma linha com a da
anterior.

## 5. Tabulação avança até o próximo múltiplo de 8

**Decisão:** ao medir a indentação, um tab avança a coluna até o próximo
múltiplo de 8 (`indent += 8 - (indent % 8)`).

**Por quê:** é a regra do tokenizador do Python. Assim, um mesmo arquivo é
indentado do mesmo jeito no nosso interpretador e no Python.

**Alternativa descartada:** contar o tab como uma coluna, ou como 4. O mesmo
arquivo poderia ter blocos diferentes no nosso interpretador e no Python.

**Diferença conhecida:** o Python 3 recusa (com `TabError`) alguns arquivos que
misturam tabs e espaços de forma ambígua; o nosso interpretador não faz essa
verificação.

## 6. `yylex()` escrito à mão para emitir vários `DEDENT`

**Decisão:** a função de varredura gerada pelo Flex foi renomeada para
`flex_scan()` (`#define YY_DECL`), e o `yylex()` que o Bison chama foi escrito à
mão: antes de chamar `flex_scan()`, ele devolve os `DEDENT`s pendentes guardados
em `pending_dedents`.

**Por quê:** uma ação do Flex devolve um único token, mas sair de vários níveis
de uma vez (por exemplo, do fim de dois `if` aninhados de volta à coluna 0)
precisa gerar vários `DEDENT`s. A ação emite o primeiro e guarda quantos faltam;
o `yylex()` devolve os outros nas chamadas seguintes, sem ler entrada nova.

**Alternativa descartada:** devolver o texto à entrada com `yyless` para o Flex
reprocessar a mesma linha várias vezes, o que espalha a lógica pelas regras e é
mais difícil de acompanhar.

## 7. Quebras de linha ignoradas dentro de parênteses

**Decisão:** o lexer conta a profundidade de `(`, `[` e `{` (`paren_depth`). Com
profundidade maior que zero, o `\n` não gera `NOVA_LINHA` nem volta ao estado
`BOL`.

**Por quê:** é a junção implícita de linhas do Python: uma expressão entre
parênteses pode ocupar várias linhas, e a indentação dessas linhas não importa.

**Alternativa descartada:** tratar isso na gramática, aceitando `NOVA_LINHA`
dentro de expressões, o que deixaria todas as regras de expressão mais
complicadas.

## 8. Erros léxicos não param a análise, mas impedem a execução

**Decisão:** ao encontrar um caractere desconhecido ou uma indentação
inconsistente, o lexer informa o erro com a linha, conta em `erros_lexicos` e
continua. Se houver qualquer erro léxico, o programa não é executado e o código
de saída é 1.

**Por quê:** mostrar todos os erros de uma vez é mais útil do que parar no
primeiro, e executar um programa com erro léxico produziria resultados sem
sentido.

**Alternativa descartada:** encerrar no primeiro erro léxico.

## 9. Erros de execução e inteiros de 64 bits

As decisões sobre erros de execução (interromper o programa com código de saída
1) e sobre o limite de 64 bits dos inteiros (`OverflowError`) estão
justificadas em [problemas-e-solucoes.md](problemas-e-solucoes.md).
