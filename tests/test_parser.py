import subprocess
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


class ParserInterpreterTests(unittest.TestCase):
    def run_program(self, source):
        with tempfile.NamedTemporaryFile("w", suffix=".py", delete=False) as source_file:
            source_file.write(source)
            source_path = source_file.name
        try:
            return subprocess.run(
                [str(ROOT / "compilador"), source_path],
                cwd=ROOT,
                text=True,
                capture_output=True,
                check=False,
            )
        finally:
            Path(source_path).unlink(missing_ok=True)

    def assert_program_output(self, source, expected):
        result = self.run_program(source)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, expected)

    def test_assignment_and_expressions(self):
        self.assert_program_output(
            "result = 2 + 3 * 4\nprint(result)\n",
            "14\n",
        )

    def test_conditionals_and_loops(self):
        self.assert_program_output(
            "total = 0\n"
            "value = 1\n"
            "while value < 4:\n"
            "    total = total + value\n"
            "    value = value + 1\n"
            "if total == 6:\n"
            "    print(total)\n"
            "else:\n"
            "    print(0)\n",
            "6\n",
        )

    def test_float_arithmetic(self):
        self.assert_program_output(
            "print(1.5 + 2)\nprint(7 / 2)\n",
            "3.5\n3.5\n",
        )

    def test_removed_constructs_are_syntax_errors(self):
        for source in (
            "def f():\n    return 1\n",
            "for i in range(3):\n    print(i)\n",
            "x = [1, 2]\n",
            "x = 1\nx += 1\n",
            "if True:\n    print(1)\nelif False:\n    print(2)\n",
        ):
            result = self.run_program(source)
            self.assertNotEqual(result.returncode, 0, source)
            self.assertIn("ERRO DE SINTAXE", result.stderr, source)

    def test_syntax_error_returns_nonzero(self):
        result = self.run_program("if True\n    print(1)\n")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("ERRO DE SINTAXE", result.stderr)


if __name__ == "__main__":
    unittest.main()