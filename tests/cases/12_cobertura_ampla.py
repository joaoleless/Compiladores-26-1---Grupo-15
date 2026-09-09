# Teste de cobertura ampla: junta varios recursos da linguagem num so programa.


# --- variaveis, aritmetica e string ---
nome = "Turma"
nota1 = 7.5
nota2 = 8.0
total = 0
total += nota1
total += nota2
media = total / 2
print("Media da " + nome + ":")
print(media)

# --- condicional com elif/else ---
if media >= 9:
    print("conceito A")
elif media >= 7:
    print("conceito B")
else:
    print("conceito C")

# --- while com break/continue ---
i = 0
soma_pares = 0
while i < 10:
    i += 1
    if i % 2 != 0:
        continue
    if i > 8:
        break
    soma_pares += i
print("soma dos pares ate 8:")
print(soma_pares)

# --- listas, for-in e indexacao negativa ---
notas = [7.5, 8.0, 9.0, 6.5]
print("quantidade de notas:")
print(len(notas))
print("ultima nota:")
print(notas[-1])
for n in notas:
    if n < 7:
        print("nota baixa")

# --- funcao recursiva ---
def fibonacci(n):
    if n <= 1:
        return n
    return fibonacci(n - 1) + fibonacci(n - 2)

for i in range(6):
    print(fibonacci(i))

# --- booleanos e comparacao ---
aprovado = media >= 7 and len(notas) > 0
print(aprovado)
