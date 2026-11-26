#include <stdlib.h>
#include <string.h>
#include "tabela.h"

Scope *new_scope(void) {
    return calloc(1, sizeof(Scope));
}

Value *scope_get(Scope *s, const char *name) {
    for (VarEntry *e = s->vars; e; e = e->next)
        if (strcmp(e->name, name) == 0) return e->val;
    return NULL;
}

void scope_set(Scope *s, const char *name, Value *val) {
    for (VarEntry *e = s->vars; e; e = e->next) {
        if (strcmp(e->name, name) == 0) { e->val = val; return; }
    }
    VarEntry *e = calloc(1, sizeof(VarEntry));
    e->name = strdup(name);
    e->val = val;
    e->next = s->vars;
    s->vars = e;
}
