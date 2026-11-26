CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L -Iparser -Isrc
LDFLAGS = -lm

PARSER_C = parser/parser.tab.c
PARSER_H = parser/parser.tab.h
LEXER_C = lexer/lex.yy.c

.PHONY: all clean test test-lexer test-parser

all: compilador

compilador: src/main.o src/ast.o src/valores.o src/tabela.o parser/parser.tab.o lexer/lex.yy.o
	$(CC) $^ $(LDFLAGS) -o $@

src/main.o: src/main.c
	$(CC) $(CFLAGS) -c $< -o $@

src/ast.o: src/ast.c src/ast.h
	$(CC) $(CFLAGS) -c $< -o $@

src/valores.o: src/valores.c src/valores.h
	$(CC) $(CFLAGS) -c $< -o $@

src/tabela.o: src/tabela.c src/tabela.h src/valores.h
	$(CC) $(CFLAGS) -c $< -o $@

parser/parser.tab.o: $(PARSER_C) $(PARSER_H) src/ast.h src/valores.h src/tabela.h
	$(CC) $(CFLAGS) -c $(PARSER_C) -o $@

lexer/lex.yy.o: $(LEXER_C) $(PARSER_H)
	$(CC) $(CFLAGS) -c $(LEXER_C) -o $@

$(PARSER_C) $(PARSER_H): parser/parser.y
	bison -d -o $(PARSER_C) parser/parser.y

$(LEXER_C): lexer/lexer.l $(PARSER_H)
	flex -o $(LEXER_C) lexer/lexer.l

lexer_driver: tests/lexer_driver.o lexer/lex.yy.o
	$(CC) $^ -o $@

tests/lexer_driver.o: tests/lexer_driver.c $(PARSER_H)
	$(CC) $(CFLAGS) -c $< -o $@

test-lexer: lexer_driver
	python3 tests/test_lexer.py

test-parser: compilador
	python3 tests/test_parser.py

test: test-lexer test-parser

clean:
	rm -f compilador lexer_driver src/*.o parser/*.o lexer/*.o tests/*.o $(PARSER_C) $(PARSER_H) $(LEXER_C)
