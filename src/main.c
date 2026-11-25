#include <stdio.h>

int yyparse(void);
void executar_programa(void);
extern int erros_lexicos;
extern FILE *yyin;

int main(int argc, char **argv) {
	if (argc > 2) {
		fprintf(stderr, "Uso: %s [arquivo]\n", argv[0]);
		return 2;
	}

	if (argc == 2) {
		yyin = fopen(argv[1], "r");
		if (!yyin) {
			perror(argv[1]);
			return 1;
		}
	}

	int status = yyparse();
	if (argc == 2 && yyin) {
		fclose(yyin);
	}

	if (status != 0) return 1;
	if (erros_lexicos > 0) return 1;

	executar_programa();
	return 0;
}