import subprocess
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


class LexerTests(unittest.TestCase):
    def run_lexer_full(self, source):
        with tempfile.NamedTemporaryFile("w", suffix=".py", delete=False) as source_file:
            source_file.write(source)
            source_path = source_file.name
        try:
            return subprocess.run(
                [str(ROOT / "lexer_driver"), source_path],
                cwd=ROOT,
                text=True,
                capture_output=True,
                check=False,
            )
        finally:
            Path(source_path).unlink(missing_ok=True)

    def run_lexer(self, source):
        result = self.run_lexer_full(source)
        self.assertEqual(result.returncode, 0, result.stderr)
        return result.stdout.splitlines()

    def test_indent_dedent_and_newlines(self):
        tokens = self.run_lexer("if True:\n    x = 1\ny = 2\n")
        self.assertEqual(
            tokens,
            ["IF", "TRUE_", "DOIS_PONTOS", "NOVA_LINHA", "INDENT",
             "IDENTIFICADOR", "OP_ATRIBUICAO", "INTEIRO", "NOVA_LINHA",
             "DEDENT", "IDENTIFICADOR", "OP_ATRIBUICAO", "INTEIRO",
             "NOVA_LINHA"],
        )

    def test_strings_and_parenthesized_newlines(self):
        tokens = self.run_lexer('text = "linha\\n\\\"aspas"\nvalues = [\n    1,\n    2\n]\n')
        self.assertEqual(tokens.count("NOVA_LINHA"), 2)
        self.assertIn("TEXTO", tokens)
        self.assertEqual(tokens.count("INDENT"), 0)
        self.assertEqual(tokens.count("DEDENT"), 0)

    def test_reserved_words(self):
        words = {
            "if": "IF", "elif": "ELIF", "else": "ELSE", "while": "WHILE",
            "for": "FOR", "break": "BREAK", "continue": "CONTINUE",
            "return": "RETURN", "def": "DEF", "class": "CLASS",
            "lambda": "LAMBDA", "try": "TRY", "except": "EXCEPT",
            "finally": "FINALLY", "raise": "RAISE", "assert": "ASSERT",
            "and": "AND", "or": "OR", "not": "NOT", "in": "IN", "is": "IS",
            "import": "IMPORT", "from": "FROM", "as": "AS",
            "True": "TRUE_", "False": "FALSE_", "None": "NONE_",
            "async": "ASYNC", "await": "AWAIT", "with": "WITH",
            "yield": "YIELD", "global": "GLOBAL", "nonlocal": "NONLOCAL",
            "del": "DEL", "pass": "PASS",
        }
        for word, token in words.items():
            with self.subTest(word=word):
                self.assertEqual(self.run_lexer(word + "\n"), [token, "NOVA_LINHA"])

    def test_keyword_prefix_is_identifier(self):
        tokens = self.run_lexer("iffy = classe + passe\n")
        self.assertEqual(
            tokens,
            ["IDENTIFICADOR", "OP_ATRIBUICAO", "IDENTIFICADOR", "OP_MAIS",
             "IDENTIFICADOR", "NOVA_LINHA"],
        )

    def test_keywords_are_case_sensitive(self):
        self.assertEqual(
            self.run_lexer("true none If\n"),
            ["IDENTIFICADOR", "IDENTIFICADOR", "IDENTIFICADOR", "NOVA_LINHA"],
        )

    def test_numeric_literals(self):
        self.assertEqual(
            self.run_lexer("42 0 3.14 .5 1.\n"),
            ["INTEIRO", "INTEIRO", "FLUTUANTE", "FLUTUANTE", "FLUTUANTE",
             "NOVA_LINHA"],
        )

    def test_scientific_notation_floats(self):
        self.assertEqual(
            self.run_lexer("1e3 1.5e3 .5E-2 2.e+10 1e\n"),
            ["FLUTUANTE", "FLUTUANTE", "FLUTUANTE", "FLUTUANTE",
             "INTEIRO", "IDENTIFICADOR", "NOVA_LINHA"],
        )

    def test_string_literals_single_and_double_quotes(self):
        self.assertEqual(
            self.run_lexer("\"abc\" 'abc' \"\" 'it\\'s'\n"),
            ["TEXTO", "TEXTO", "TEXTO", "TEXTO", "NOVA_LINHA"],
        )

    def test_identifiers(self):
        self.assertEqual(
            self.run_lexer("x _tmp minha_var2 __a__\n"),
            ["IDENTIFICADOR"] * 4 + ["NOVA_LINHA"],
        )

    def test_arithmetic_and_comparison_operators(self):
        source = "+ - * / // % ** < > <= >= == != =\n"
        self.assertEqual(
            self.run_lexer(source),
            ["OP_MAIS", "OP_MENOS", "OP_MULTIPLICACAO", "OP_DIVISAO",
             "OP_DIVISAO_INTEIRA", "OP_MODULO", "OP_POTENCIA", "OP_MENOR",
             "OP_MAIOR", "OP_MENOR_IGUAL", "OP_MAIOR_IGUAL",
             "OP_IGUAL_IGUAL", "OP_DIFERENTE", "OP_ATRIBUICAO", "NOVA_LINHA"],
        )

    def test_bitwise_operators(self):
        self.assertEqual(
            self.run_lexer("& | ^ ~ << >> @\n"),
            ["OP_E_BIT", "OP_OU_BIT", "OP_XOU_BIT", "OP_NAO_BIT",
             "OP_DESLOC_ESQUERDA", "OP_DESLOC_DIREITA", "OP_MATRIZ_MULT",
             "NOVA_LINHA"],
        )

    def test_compound_assignment_operators(self):
        source = "+= -= *= /= //= %= **= &= |= ^= <<= >>=\n"
        self.assertEqual(
            self.run_lexer(source),
            ["OP_ATRIBUICAO_MAIS", "OP_ATRIBUICAO_MENOS",
             "OP_ATRIBUICAO_MULTIPLICACAO", "OP_ATRIBUICAO_DIVISAO",
             "OP_ATRIBUICAO_DIVISAO_INTEIRA", "OP_ATRIBUICAO_MODULO",
             "OP_ATRIBUICAO_POTENCIA", "OP_ATRIBUICAO_E_BIT",
             "OP_ATRIBUICAO_OU_BIT", "OP_ATRIBUICAO_XOU_BIT",
             "OP_ATRIBUICAO_DESLOC_ESQUERDA", "OP_ATRIBUICAO_DESLOC_DIREITA",
             "NOVA_LINHA"],
        )

    def test_longest_match_operators(self):
        self.assertEqual(
            self.run_lexer("a**b a*b a//b a/b\n"),
            ["IDENTIFICADOR", "OP_POTENCIA", "IDENTIFICADOR",
             "IDENTIFICADOR", "OP_MULTIPLICACAO", "IDENTIFICADOR",
             "IDENTIFICADOR", "OP_DIVISAO_INTEIRA", "IDENTIFICADOR",
             "IDENTIFICADOR", "OP_DIVISAO", "IDENTIFICADOR", "NOVA_LINHA"],
        )

    def test_delimiters(self):
        self.assertEqual(
            self.run_lexer("( ) [ ] { } , : . ; ->\n"),
            ["PARENTESES_ESQ", "PARENTESES_DIR", "COLCHETE_ESQ",
             "COLCHETE_DIR", "CHAVE_ESQ", "CHAVE_DIR", "VIRGULA",
             "DOIS_PONTOS", "PONTO", "PONTO_E_VIRGULA", "SETA",
             "NOVA_LINHA"],
        )

    def test_inline_comment_is_ignored(self):
        self.assertEqual(
            self.run_lexer("x = 1  # comentario com 'aspas' e ( parenteses\n"),
            ["IDENTIFICADOR", "OP_ATRIBUICAO", "INTEIRO", "NOVA_LINHA"],
        )

    def test_blank_and_comment_only_lines_do_not_affect_indentation(self):
        source = "if x:\n\n    # comentario\n    y = 1\n\n# fim\nz = 2\n"
        tokens = self.run_lexer(source)
        self.assertEqual(tokens.count("INDENT"), 1)
        self.assertEqual(tokens.count("DEDENT"), 1)
        self.assertEqual(tokens.count("NOVA_LINHA"), 3)

    def test_multiple_dedents_at_once(self):
        source = "if a:\n    if b:\n        c\nd\n"
        tokens = self.run_lexer(source)
        self.assertEqual(tokens.count("INDENT"), 2)
        i = tokens.index("DEDENT")
        self.assertEqual(tokens[i:i + 3], ["DEDENT", "DEDENT", "IDENTIFICADOR"])

    def test_open_blocks_are_closed_at_eof(self):
        tokens = self.run_lexer("if a:\n    if b:\n        c\n")
        self.assertEqual(tokens.count("INDENT"), 2)
        self.assertEqual(tokens.count("DEDENT"), 2)
        self.assertEqual(tokens[-2:], ["DEDENT", "DEDENT"])

    def test_dedent_to_intermediate_level(self):
        source = "if a:\n    if b:\n        c\n    d\n"
        tokens = self.run_lexer(source)
        self.assertEqual(tokens.count("INDENT"), 2)
        self.assertEqual(tokens.count("DEDENT"), 2)
        i = tokens.index("DEDENT")
        self.assertEqual(tokens[i:i + 2], ["DEDENT", "IDENTIFICADOR"])

    def test_tab_indentation(self):
        tokens = self.run_lexer("if a:\n\tb\n")
        self.assertEqual(tokens.count("INDENT"), 1)
        self.assertEqual(tokens.count("DEDENT"), 1)

    def test_newline_suppressed_inside_all_bracket_kinds(self):
        for opening, closing in ("()", "[]", "{}"):
            with self.subTest(bracket=opening):
                tokens = self.run_lexer(f"x = {opening}\n1,\n2\n{closing}\n")
                self.assertEqual(tokens.count("NOVA_LINHA"), 1)
                self.assertEqual(tokens.count("INDENT"), 0)

    def test_nested_brackets_suppress_newlines_until_balanced(self):
        tokens = self.run_lexer("x = ([\n1\n])\ny = 2\n")
        self.assertEqual(tokens.count("NOVA_LINHA"), 2)

    def test_unknown_character_reports_line_and_continues(self):
        result = self.run_lexer_full("a = 1\nb = $ 2\nc = 3\n")
        self.assertEqual(result.returncode, 1)
        self.assertIn("ERRO LEXICO", result.stderr)
        self.assertIn("'$'", result.stderr)
        self.assertIn("linha 2", result.stderr)
        self.assertEqual(result.stdout.splitlines().count("INTEIRO"), 3)

    def test_inconsistent_dedent_is_reported(self):
        result = self.run_lexer_full("if a:\n        b\n    c\n")
        self.assertEqual(result.returncode, 1)
        self.assertIn("ERRO DE INDENTACAO", result.stderr)
        self.assertIn("linha 3", result.stderr)

    def test_consistent_indentation_has_no_errors(self):
        result = self.run_lexer_full("if a:\n    b\n    c\nd\n")
        self.assertEqual(result.stderr, "")

    def test_empty_file_produces_no_tokens(self):
        self.assertEqual(self.run_lexer(""), [])

    def test_file_without_trailing_newline(self):
        self.assertEqual(
            self.run_lexer("x = 1"),
            ["IDENTIFICADOR", "OP_ATRIBUICAO", "INTEIRO", "NOVA_LINHA"],
        )


if __name__ == "__main__":
    unittest.main()
