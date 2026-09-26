import subprocess
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


class LexerTests(unittest.TestCase):
    def run_lexer(self, source):
        with tempfile.NamedTemporaryFile("w", suffix=".py", delete=False) as source_file:
            source_file.write(source)
            source_path = source_file.name
        try:
            result = subprocess.run(
                [str(ROOT / "lexer_driver"), source_path],
                cwd=ROOT,
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            return result.stdout.splitlines()
        finally:
            Path(source_path).unlink(missing_ok=True)

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


if __name__ == "__main__":
    unittest.main()