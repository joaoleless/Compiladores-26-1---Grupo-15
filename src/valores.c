#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "valores.h"

Value *new_int(long v)    { Value *x = calloc(1,sizeof(Value)); x->type=V_INT; x->ival=v; return x; }
Value *new_float(double v){ Value *x = calloc(1,sizeof(Value)); x->type=V_FLOAT; x->dval=v; return x; }
Value *new_str(const char *s){ Value *x = calloc(1,sizeof(Value)); x->type=V_STR; x->sval=strdup(s); return x; }
Value *new_bool(int b)    { Value *x = calloc(1,sizeof(Value)); x->type=V_BOOL; x->ival=b?1:0; return x; }
Value *new_none(void)     { Value *x = calloc(1,sizeof(Value)); x->type=V_NONE; return x; }

double as_double(Value *v) {
    switch (v->type) {
        case V_INT: return (double)v->ival;
        case V_FLOAT: return v->dval;
        case V_BOOL: return (double)v->ival;
        default: return 0.0;
    }
}

/* Valor inteiro de int/bool (bool e subtipo de int, como no Python). */
long as_long(Value *v) {
    return (v->type == V_INT || v->type == V_BOOL) ? v->ival : (long)as_double(v);
}

int is_floaty(Value *v) { return v->type == V_FLOAT; }

int is_numeric(Value *v) {
    return v->type == V_INT || v->type == V_FLOAT || v->type == V_BOOL;
}

/* Nome do tipo como o Python mostra nas mensagens de TypeError. */
const char *type_name(Value *v) {
    switch (v->type) {
        case V_INT: return "int";
        case V_FLOAT: return "float";
        case V_STR: return "str";
        case V_BOOL: return "bool";
        case V_NONE: return "NoneType";
    }
    return "?";
}

int truthy(Value *v) {
    switch (v->type) {
        case V_INT: return v->ival != 0;
        case V_FLOAT: return v->dval != 0.0;
        case V_BOOL: return v->ival != 0;
        case V_STR: return v->sval && v->sval[0] != '\0';
        case V_NONE: return 0;
    }
    return 0;
}

/* Formata um double como o Python: menor representacao que preserva o valor,
   sempre com ponto decimal (ex.: 2.0, 0.1, 1e+16). */
void format_float(double d, char *buf, size_t size) {
    /* 1) menor numero de digitos significativos que preserva o valor */
    int prec;
    for (prec = 1; prec <= 17; prec++) {
        snprintf(buf, size, "%.*e", prec - 1, d);
        if (strtod(buf, NULL) == d) break;
    }
    if (prec > 17) prec = 17;
    /* 2) como o repr do Python: notacao fixa se -4 <= expoente < 16 */
    const char *e = strchr(buf, 'e');
    if (e) {
        int exp10 = atoi(e + 1);
        if (exp10 >= -4 && exp10 < 16) {
            int decimals = prec - 1 - exp10;
            snprintf(buf, size, "%.*f", decimals > 0 ? decimals : 0, d);
        }
    }
    if (!strpbrk(buf, ".einEIN")) {
        size_t n = strlen(buf);
        if (n + 2 < size) { buf[n] = '.'; buf[n + 1] = '0'; buf[n + 2] = '\0'; }
    }
}

void print_value(Value *v) {
    switch (v->type) {
        case V_INT: printf("%ld", v->ival); break;
        case V_FLOAT: { char buf[64]; format_float(v->dval, buf, sizeof(buf)); printf("%s", buf); break; }
        case V_STR: printf("%s", v->sval); break;
        case V_BOOL: printf("%s", v->ival ? "True" : "False"); break;
        case V_NONE: printf("None"); break;
    }
}
