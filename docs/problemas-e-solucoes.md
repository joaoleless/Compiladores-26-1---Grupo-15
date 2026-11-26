# Problemas encontrados e soluções adotadas

Este documento registra os problemas encontrados durante o desenvolvimento do
interpretador e como cada um foi resolvido.

## Comportamentos divergentes do Python no interpretador (novembro de 2026)

### Como foram encontrados

Lendo `parser/parser.y`, levantamos casos em que o interpretador poderia se
comportar diferente do Python. Cada caso foi confirmado executando o mesmo
programa no `./compilador` e no `python3` e comparando a saída e o código de
saída. Antes de cada correção escrevemos um teste em `tests/test_parser.py` ou
`tests/test_lexer.py` que reproduz o problema. Cada correção está em um commit
próprio, junto com o seu teste.

### Causa comum

A maior parte dos problemas vinha da função `as_double()`, que converte `str`
e `None` para `0.0` sem avisar. Com isso, qualquer operação aritmética ou
comparação envolvendo esses tipos caía no ramo numérico e produzia um
resultado errado em vez de um erro. Além disso, erros de execução só eram
impressos, e o programa continuava com código de saída 0.

### Decisão: erro de execução interrompe o programa

**Decisão:** todo erro de execução chama `runtime_error()`, que esvazia o
`stdout`, escreve a mensagem em `stderr` e encerra com código de saída `1`.

**Por quê:** é o comportamento do Python (a exceção não tratada encerra o
programa), e um programa que continua depois de um erro produz saída sem
sentido. Também permite que os testes verifiquem o erro pelo código de saída.

**Alternativa descartada:** continuar a execução usando `None` no lugar do
valor inválido, que era o comportamento anterior. Ela escondia o erro e
gerava resultados errados nas linhas seguintes.

### Decisão: inteiros de 64 bits com `OverflowError`

**Decisão:** os inteiros continuam sendo `long` (64 bits). Toda operação que
pode estourar (`+`, `-`, `*`, `**`, `//` por `-1`, menos unário, `int()` de
`float` ou de `str`, literais inteiros) verifica o estouro e gera
`OverflowError`.

**Por quê:** o resultado errado silencioso (`2 ** 100` dava
`-9223372036854775808`) é o pior caso. Detectar o estouro custa pouco com
`__builtin_add_overflow`, `__builtin_sub_overflow` e `__builtin_mul_overflow`
do GCC.

**Alternativa descartada:** inteiros de tamanho arbitrário, como os do
Python. Exigiriam implementar aritmética de precisão múltipla (ou uma
biblioteca como a GMP) para todas as operações, um custo alto para um caso
fora do uso normal dos programas do subconjunto. A diferença ficou registrada
como limitação conhecida no README.

### Problemas e correções

| Problema | Entrada | Antes | Python / agora | Solução | Commit |
| --- | --- | --- | --- | --- | --- |
| Variável não definida não parava o programa | `print(x)` | imprimia erro, `None` e continuava; código 0 | erro, código 1 | `scope_get` chama `runtime_error()` | `b735d99` |
| Divisão por zero | `10 % 0`, `1 / 0`, `1 // 0` | crash SIGFPE (código 136), `inf`, `-9223372036854775808` | `ZeroDivisionError` | verificação do divisor antes de `/`, `//` e `%` | `4b08aaf` |
| `//` e `%` com negativos | `-7 % 3`, `7.0 // 2` | `-1`, `3` | `2`, `3.0` | C trunca em direção a zero; o Python usa piso. Ajuste do quociente e do resto quando os sinais diferem | `3bc6bf2` |
| Aritmética com `str` ou `None` | `"a" + 1`, `None + 1` | `a`, `1` | `TypeError` | só `int`, `float` e `bool` entram no ramo numérico | `301152c` |
| Concatenação truncada | `t + t` com 600 caracteres | cortava em 511 | 600 caracteres | buffer alocado com o tamanho exato no lugar de `char buf[512]` | `0117ce6` |
| Repetição de string | `"ab" * 3` | `0` | `ababab` | implementação de `str * int` e `int * str` | `639ef08` |
| Comparação entre tipos diferentes | `"a" == 0`, `None == 0`, `"a" < 1` | `True`, `True`, `True` | `False`, `False`, `TypeError` | `==`/`!=` entre tipos não comparáveis só igualam dois `None`; ordenação gera `TypeError` | `fe09166` |
| Conversão de string | `int("42")`, `float("3.5")` | `0`, `0.0` | `42`, `3.5` | `strtol`/`strtod` com validação; texto inválido gera `ValueError` | `0b58e39` |
| Estouro de inteiro | `2 ** 100`, `int(1e20)` | `-9223372036854775808` | `OverflowError` (ver decisão acima) | builtins de estouro do GCC e exponenciação por quadrados; operações inteiras deixam de passar por `double`, que perdia precisão acima de 2^53 | `0445f41` |
| Formato de `float` | `print(1500.0)` | `1.5e+03` | `1500.0` | `%g` passa para notação científica cedo; agora segue o `repr` do Python (fixa para expoentes de -4 a 15) | `9f1592a` |
| Notação científica | `1.5e3`, `1e30` | erro de sintaxe | `1500.0`, `1e+30` | expoente `[eE][+-]?[0-9]+` na regex `FLOAT` do lexer | `91f24e4` |
| Literal inteiro grande | `x = 99999999999999999999` | `atol` estourava em silêncio | erro léxico (ver decisão acima) | `strtol` com verificação de `ERANGE` | `18795db` |

### O que continua diferente do Python

- Inteiros limitados a 64 bits (`OverflowError` em vez de inteiro grande).
- Mensagens de erro de execução sem número de linha: a AST não guarda a linha
  de cada nó.
- O texto das mensagens não é idêntico ao do Python; o tipo do erro
  (`TypeError`, `ValueError`, ...) aparece na mensagem.
