# Achados dos testes de `parser.y`

Testado compilando `lexer/lexer.l` + `parser/parser.y` com flex/bison/gcc.
11 casos "felizes" cobrindo aritmética, precedência, condicionais, laços,
funções/recursão, strings, booleanos, listas/índices e escopo — **todos passam**
(veja `tests/cases/`).

Além disso, três comportamentos de borda merecem decisão do grupo:

## 1. Erro de sintaxe não altera o código de saída
```python
x =
print(x)
```
Imprime `ERRO DE SINTAXE na linha 2: syntax error` no stderr, mas o programa
termina com **exit code 0**. Em `parser.y`, o `main()` faz:
```c
if (yyparse() == 0 && raiz) { ... }
return 0;
```
Ou seja, mesmo se `yyparse()` retornar erro, o `return 0` final sempre roda.
**Sugestão:** retornar 1 quando `yyparse()` falhar, para scripts de teste/CI
conseguirem detectar falha automaticamente.

## 2. Divisão por zero não gera erro
```python
print(10 / 0)
```
Imprime `inf` em vez de lançar um erro (no Python real seria
`ZeroDivisionError`). Isso é consequência de `a / b` ser feito com `double`
puro (`new_float(a / b)`), sem checagem de `b == 0`.
**Sugestão:** decidir se isso é aceitável para o escopo do projeto ou se vale
adicionar checagem e mensagem de erro.

## 3. Variável não definida não interrompe a execução
```python
print(y)
```
Imprime `ERRO: variavel 'y' nao definida` no stderr, mas continua a execução
tratando `y` como `None` (imprime `None`). Pode ser uma escolha de design
(mais tolerante), mas vale confirmar que é intencional — do contrário, um
programa com erro grave pode "passar" nos testes por engano.

---

Nenhum desses é necessariamente errado — dependem do que o professor espera
para "tratamento de erros" no guia do interpretador. Mas são bons pontos pra
levar pra reunião do grupo e, se decidirem corrigir, pra virarem novos casos
de teste (`.py` + `.expected`) aqui em `tests/cases/`.
