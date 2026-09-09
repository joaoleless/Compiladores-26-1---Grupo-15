def soma(a, b):
    return a + b

print(soma(2, 3))

def fatorial(n):
    if n <= 1:
        return 1
    return n * fatorial(n - 1)

print(fatorial(5))
