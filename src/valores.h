#ifndef VALORES_H
#define VALORES_H

#include <stddef.h>

/* Valores em tempo de execucao: cada valor tem um tipo e usa o campo
   correspondente (ival para int/bool, dval para float, sval para str). */

typedef enum { V_INT, V_FLOAT, V_STR, V_BOOL, V_NONE } VType;

typedef struct Value {
    VType type;
    long ival;
    double dval;
    char *sval;
} Value;

Value *new_int(long v);
Value *new_float(double v);
Value *new_str(const char *s);
Value *new_bool(int b);
Value *new_none(void);

/* Valor numerico como double (str e None viram 0.0). */
double as_double(Value *v);
/* Valor inteiro de int/bool (bool e subtipo de int, como no Python). */
long as_long(Value *v);
int is_floaty(Value *v);
/* int, float ou bool. */
int is_numeric(Value *v);
/* Nome do tipo como o Python mostra nas mensagens de TypeError. */
const char *type_name(Value *v);
/* Veracidade do valor, como no Python. */
int truthy(Value *v);
/* Formata um double como o repr do Python. */
void format_float(double d, char *buf, size_t size);
/* Imprime o valor como o print do Python (sem quebra de linha). */
void print_value(Value *v);

#endif
