# Relatório de Cobertura de Testes: Lexer Python-like

## 1. Objetivo
Este relatório acompanha o arquivo `suite_testes_lexer.py` gerado, detalhando a estratégia de cobertura para validar todas as regras léxicas do analisador construído em Flex.

---

## 2. Detalhamento dos Cenários de Teste

### Cenário 1: Limpeza e Estado Inicial (`<BOL>`)
Valida se as regras no início da linha (estado BOL) ignoram corretamente linhas em branco e comentários, não gerando falsos tokens de indentação.
```python
import sys
from os import path
```

### Cenário 2: Controle de Indentação (INDENT/DEDENT)
Testa a mecânica da pilha `indent_stack`. O lexer deve emitir um token **INDENT** ao entrar no corpo da função e no laço `for`, e emitir **DEDENT** corretamente quando a indentação retroceder no fechamento do laço.

### Cenário 3: Processamento de Strings (Avançado)
Testa a captura do token `STRING` e a validação da função `unescape()` escrita em C, garantindo que os escapes como `\n` e aspas aninhadas `\'` sejam filtrados corretamente.

### Cenário 4: Profundidade de Parênteses (`paren_depth`)
Um dos pontos cruciais do código: quebras de linha dentro de colchetes `[]` e chaves `{}` **não** devem emitir o token `NOVA_LINHA`. O contador `paren_depth` precisa ser incrementado e decrementado em sincronia.
```python
lista_aninhada = [
    1, 2, 
    3
]
```

### Cenário 5: Fechamento Seguro (EOF e Pending Dedents)
Quando o arquivo termina sem que a indentação retorne a zero, a regra `<<EOF>>` deve emitir repetidamente tokens **DEDENT** com base na variável `pending_dedents` até esvaziar a pilha de escopos abertos.

---

## 3. Matriz de Cobertura de Tokens

| Categoria Lexical | Exemplos Testados | Estado Esperado |
| :--- | :--- | :--- |
| **Controle de Fluxo e Palavras Reservadas** | `def`, `for`, `in`, `return`, `if`, `and`, `not`, `True`, `False` | Coberto ✅ |
| **Tipos de Dados Literais** | `INTEIRO` (0, 1, 2), `FLOAT` (0.0), `TEXTO` ("...") | Coberto ✅ |
| **Operadores Complexos** | `+=`, `//`, `**`, `>>`, `<<=` | Coberto ✅ |
| **Delimitadores e Agrupadores** | `(`, `)`, `[`, `]`, `{`, `}`, `:`, `,` | Coberto ✅ |
| **Tokens Sintéticos (Python-style)** | `NOVA_LINHA`, `INDENT`, `DEDENT` | Coberto ✅ |