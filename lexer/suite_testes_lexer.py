# Teste 1: Comentários, linhas em branco e imports (Ignorar BOL, Tokens Básicos)

import sys
from os import path

# Teste 2: Indentação básica (INDENT/DEDENT), Tipos Literais e Operadores
def calcular_media(notas):
    soma = 0.0          # FLOAT
    total = 0           # INTEIRO
    for n in notas:
        soma += n       # OP_ATRIBUICAO_MAIS
        total += 1
    
    media = soma / total # OP_DIVISAO
    return media

# Teste 3: Strings com escapes e Operadores Lógicos
mensagem = "A media e:\n"
is_valid = True and not False
if is_valid:
    pass

# Teste 4: Profundidade de parênteses/chaves (Ignorar NOVA_LINHA interna)
lista_aninhada = [
    1, 2, 
    3, {
        "chave": 'valor_escape\'aqui'
    }
]

# Teste 5: Operadores complexos bitwise e de atribuição
a = 10 >> 1
a <<= 2
b = a ** 2
c = b // 3

# Teste 6: Multiplos DEDENTs acionados no final do arquivo (<<EOF>>)
if a > 0:
    if b > 0:
        if c > 0:
            print("Fim do teste")