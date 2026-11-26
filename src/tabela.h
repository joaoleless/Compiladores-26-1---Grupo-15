#ifndef TABELA_H
#define TABELA_H

#include "valores.h"

/* Tabela de simbolos. */

typedef struct VarEntry {
    char *name;
    Value *val;
    struct VarEntry *next;
} VarEntry;

typedef struct Scope {
    VarEntry *vars;
} Scope;

/* Cria um escopo vazio. */
Scope *new_scope(void);

/* Valor atual da variavel, ou NULL se ela nunca foi atribuida. */
Value *scope_get(Scope *s, const char *name);

/* Atribui o valor a variavel, criando a entrada se ela ainda nao existe. */
void scope_set(Scope *s, const char *name, Value *val);

#endif
