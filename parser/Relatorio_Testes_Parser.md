# Relatório de Cobertura de Testes: Parser/Interpretador Python-like

## 1. Objetivo
Este relatório acompanha a suíte de testes em `tests/` (script `run_tests.sh` +
casos em `tests/cases/`), detalhando a estratégia de cobertura para validar
a gramática, a construção da AST e a execução implementadas em `parser.y`.

Diferente do lexer, o `parser.y` deste projeto concentra três
responsabilidades no mesmo arquivo: as regras sintáticas (Bison), a
montagem da árvore de sintaxe abstrata (`mknode`, `Node`) e o interpretador
que percorre essa árvore (`eval_expr`, `exec_stmt`, `exec_block`). Por isso,
os testes aqui são de **caixa-preta end-to-end**: cada caso é um programa
`.py` completo, executado pelo binário compilado, com a saída padrão
comparada a um arquivo `.expected`.

---

## 2. Detalhamento dos Cenários de Teste

### Cenário 1: Aritmética e Precedência de Operadores
Valida a cadeia de regras `arith_expr → term → factor → power → atom`, que
implementa a precedência crescente (soma/subtração < multiplicação/divisão
< potência), além do operador unário `-` e da associatividade à direita de
`**`.
```python
print(2 + 3 * 4)      # 14 - multiplicação antes da soma
print(2 ** 3 ** 2)     # 512 - ** associa à direita: 2 ** (3 ** 2)
print(-2 ** 2)          # -4 - unário tem precedência menor que **
```

### Cenário 2: Estruturas Condicionais (`if_stmt`/`elif_chain`)
Testa a regra recursiva `elif_chain`, que encadeia `elif`s e resolve o
`else` final, gerando nós `N_IF` aninhados corretamente.
```python
if x > 10:
    print("maior")
elif x == 5:
    print("igual")
else:
    print("menor")
```

### Cenário 3: Laços (`while_stmt`, `for_stmt`) e Sinalização de Fluxo
Valida `while` e `for ... in`, incluindo a máquina de sinalização
(`SIG_BREAK`, `SIG_CONTINUE`, `SIG_RETURN`) usada por `exec_block` para
propagar `break`/`continue`/`return` através de blocos aninhados.
```python
i = 0
while i < 10:
    i += 1
    if i == 3:
        continue
    if i == 6:
        break
    print(i)
```

### Cenário 4: Funções, Recursão e Escopo (`funcdef_stmt`, `Scope`)
Testa `def`, chamada de função (`N_CALL`), `return`, recursão e o
isolamento de escopo (`new_scope`/`scope_get`/`scope_set`) — confirma que
alterar um parâmetro dentro da função não vaza para a variável do chamador.
```python
def fatorial(n):
    if n <= 1:
        return 1
    return n * fatorial(n - 1)

print(fatorial(5))  # 120
```

### Cenário 5: Tipos Compostos e Builtins (Listas, Strings, `len`)
Valida `N_LIST`, indexação (`N_INDEX`, inclusive índices negativos) e as
funções nativas (`print`, `len`, `range`, `int`, `float`, `str`).
```python
lista = [1, 2, 3, 4, 5]
print(lista[-1])   # 5
print(len(lista))  # 5
for v in lista:
    print(v)
```

### Cenário 6: Booleanos e Comparação de Strings
Testa `or_test`/`and_test`/`not_test`/`comparison`, incluindo curto-circuito
de `and`/`or` e comparação lexicográfica de strings (`strcmp` dentro de
`N_COMPARE`).
```python
print(True and False)
print("abc" < "abd")
```

### Cenário 7: Cobertura Ampla (programa único)
No mesmo espírito do `suite_testes_lexer.py` do lexer — um único programa
que combina vários recursos da linguagem para simular um caso de uso
realista — o arquivo `12_cobertura_ampla.py` junta comentários, aritmética,
`if/elif/else`, `while` com `break`/`continue`, listas e indexação
negativa, `for ... in`, uma função recursiva (Fibonacci) e operadores
booleanos, tudo em um só script. A diferença é que aqui a saída também é
comparada automaticamente com `12_cobertura_ampla.expected`, então o
resultado não depende de inspeção manual.

---

## 3. Matriz de Cobertura

| Categoria da Gramática | Regras/Nós Testados | Estado Esperado |
| :--- | :--- | :--- |
| **Expressões aritméticas** | `arith_expr`, `term`, `factor`, `power` (`+ - * / // % **`) | Coberto ✅ |
| **Expressões relacionais/lógicas** | `comparison`, `or_test`, `and_test`, `not_test` | Coberto ✅ |
| **Atribuição** | `OP_ATRIBUICAO`, `augassign_op` (`+= -= *= /= //= %=`) | Coberto ✅ |
| **Controle de fluxo** | `if_stmt`, `elif_chain`, `while_stmt`, `for_stmt`, `break`, `continue` | Coberto ✅ |
| **Funções** | `funcdef_stmt`, `paramlist`, `return_stmt`, recursão, escopo local | Coberto ✅ |
| **Tipos e literais** | `INTEIRO`, `FLUTUANTE`, `TEXTO`, `TRUE_`/`FALSE_`/`NONE_`, `N_LIST` | Coberto ✅ |
| **Indexação e builtins** | `N_INDEX` (positivo/negativo), `print`, `len`, `range` | Coberto ✅ |
| **Tratamento de erros** | Erro de sintaxe, variável indefinida, divisão por zero | Comportamento documentado ⚠️ |

---

## 4. Achados (comportamentos de borda)

Três pontos identificados durante os testes que não quebram os cenários
"felizes", mas merecem decisão do grupo sobre se são bugs a corrigir ou
comportamento aceito para o escopo do projeto:

1. **Erro de sintaxe não altera o exit code**: `main()` sempre retorna `0`,
   mesmo quando `yyparse()` falha — dificulta detectar falha em scripts/CI.
2. **Divisão por zero não é tratada**: `10 / 0` retorna `inf` (aritmética de
   ponto flutuante pura), em vez de um erro, como faria o Python real.
3. **Variável indefinida não interrompe a execução**: imprime erro no
   `stderr`, mas segue tratando a variável como `None` e continua rodando.

Detalhes completos, com trechos de código explicando a causa de cada um,
estão em `tests/ACHADOS.md`.

---

## 5. Como reproduzir

```bash
sudo apt install flex bison build-essential -y   # se ainda não tiver
chmod +x tests/run_tests.sh
./tests/run_tests.sh
```

O script compila `lexer/lexer.l` + `parser/parser.y` do zero e roda todos
os casos em `tests/cases/`, reportando PASS/FAIL por caso e o total no final.
