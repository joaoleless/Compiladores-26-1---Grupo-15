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

    def assert_runtime_error(self, source, message):
        result = self.run_program(source)
        self.assertEqual(result.returncode, 1, result.stderr)
        self.assertIn(message, result.stderr)
        return result

    def assert_syntax_error(self, source):
        result = self.run_program(source)
        self.assertNotEqual(result.returncode, 0, source)
        self.assertIn("ERRO DE SINTAXE", result.stderr, source)

    # ---------- expressoes ----------

    def test_operator_precedence_and_parentheses(self):
        self.assert_program_output(
            "print(2 + 3 * 4)\nprint((2 + 3) * 4)\nprint(10 - 4 - 3)\n",
            "14\n20\n3\n",
        )

    def test_power_is_right_associative_and_binds_tighter_than_unary_minus(self):
        self.assert_program_output(
            "print(2 ** 3 ** 2)\nprint(-2 ** 2)\n",
            "512\n-4\n",
        )

    def test_division_operators(self):
        self.assert_program_output(
            "print(7 / 2)\nprint(7 // 2)\nprint(7 % 3)\nprint(-7 // 2)\n",
            "3.5\n3\n1\n-4\n",
        )

    def test_int_float_promotion(self):
        self.assert_program_output(
            "print(1 + 2.5)\nprint(2 * 1.5)\nprint(1.5 - 1)\n",
            "3.5\n3.0\n0.5\n",
        )

    def test_float_literals(self):
        self.assert_program_output(
            "print(.5)\nprint(3.25)\nprint(1.5 * 2.0)\n",
            "0.5\n3.25\n3.0\n",
        )

    def test_float_output_matches_python_repr(self):
        self.assert_program_output(
            "print(2.0)\nprint(4 / 2)\nprint(0.1 + 0.2)\nprint(10.0 ** 20)\n"
            "print(str(3.0))\nprint(float(2))\nprint(-0.5)\n",
            "2.0\n2.0\n0.30000000000000004\n1e+20\n3.0\n2.0\n-0.5\n",
        )

    def test_unary_operators(self):
        self.assert_program_output(
            "x = 5\nprint(-x)\nprint(+x)\nprint(--x)\nprint(-1.5)\n",
            "-5\n5\n5\n-1.5\n",
        )

    def test_comparisons(self):
        self.assert_program_output(
            "print(1 < 2)\nprint(2 <= 2)\nprint(3 > 4)\nprint(4 >= 5)\n"
            "print(1 == 1.0)\nprint(1 != 2)\n",
            "True\nTrue\nFalse\nFalse\nTrue\nTrue\n",
        )

    def test_logical_operators(self):
        self.assert_program_output(
            "print(True and False)\nprint(True or False)\nprint(not True)\n"
            "print(not False and True)\n",
            "False\nTrue\nFalse\nTrue\n",
        )

    def test_and_or_return_operands_and_short_circuit(self):
        self.assert_program_output(
            "print(0 or 5)\nprint(3 and 4)\nprint(0 and undefined_name)\n"
            "print(1 or undefined_name)\n",
            "5\n4\n0\n1\n",
        )

    def test_truthiness_in_conditions(self):
        self.assert_program_output(
            "if 0:\n    print(\"zero\")\n"
            "if \"\":\n    print(\"vazia\")\n"
            "if None:\n    print(\"none\")\n"
            "if 0.0:\n    print(\"float\")\n"
            "if 7:\n    print(\"sete\")\n"
            "if \"a\":\n    print(\"texto\")\n",
            "sete\ntexto\n",
        )

    def test_floor_division_and_modulo_follow_python(self):
        self.assert_program_output(
            "print(-7 // 2)\nprint(7 // -2)\nprint(-7 % 3)\nprint(7 % -3)\n"
            "print(-7.0 % 3)\nprint(7.0 // 2)\nprint(-7.5 // 2)\nprint(7 % 3)\n",
            "-4\n-4\n2\n-2\n2.0\n3.0\n-4.0\n1\n",
        )

    def test_float_formatting_matches_python_repr(self):
        self.assert_program_output(
            "print(1500.0)\nprint(0.0001)\nprint(0.00001)\nprint(123456789.5)\n"
            "print(1000000000000000.0)\nprint(10000000000000000.0)\nprint(0.1 + 0.2)\n"
            "print(-2.5)\nprint(100.0 / 3)\n",
            "1500.0\n0.0001\n1e-05\n123456789.5\n"
            "1000000000000000.0\n1e+16\n0.30000000000000004\n"
            "-2.5\n33.333333333333336\n",
        )

    def test_scientific_notation_floats(self):
        self.assert_program_output(
            "print(1.5e3)\nprint(2.5e-1)\nprint(1e30)\nprint(1E3 + 1)\n",
            "1500.0\n0.25\n1e+30\n1001.0\n",
        )

    # ---------- tipos e strings ----------

    def test_print_booleans_and_none(self):
        self.assert_program_output(
            "print(True)\nprint(False)\nprint(None)\n",
            "True\nFalse\nNone\n",
        )

    def test_print_multiple_arguments_and_no_arguments(self):
        self.assert_program_output(
            "print(1, 2.5, \"a\", True, None)\nprint()\n",
            "1 2.5 a True None\n\n",
        )

    def test_string_concatenation_and_comparison(self):
        self.assert_program_output(
            "print(\"ab\" + \"cd\")\nprint(\"a\" < \"b\")\nprint(\"a\" == \"a\")\n"
            "print(\"a\" != \"a\")\n",
            "abcd\nTrue\nTrue\nFalse\n",
        )

    def test_string_escape_sequences_and_quotes(self):
        self.assert_program_output(
            "print(\"a\\tb\")\nprint('it\\'s')\nprint(\"dq\\\"x\")\nprint('single')\n",
            "a\tb\nit's\ndq\"x\nsingle\n",
        )

    def test_conversion_builtins(self):
        self.assert_program_output(
            "print(int(3.9))\nprint(float(2) + 0.5)\nprint(str(5) + \"x\")\n"
            "print(str(1.5))\nprint(str(True))\n",
            "3\n2.5\n5x\n1.5\nTrue\n",
        )

    def test_long_string_concatenation_is_not_truncated(self):
        result = self.run_program(
            's = "abcdefghij"\n'
            "t = s + s + s + s + s + s + s + s + s + s\n"
            "print(t + t + t + t + t + t)\n"
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, "abcdefghij" * 60 + "\n")

    def test_string_repetition(self):
        self.assert_program_output(
            'print("ab" * 3)\nprint(2 * "xy")\nprint("ab" * 0 + "|")\nprint("ab" * -1 + "|")\nprint("a" * True)\n',
            "ababab\nxyxy\n|\n|\na\n",
        )

    def test_string_repetition_by_float_is_type_error(self):
        self.assert_runtime_error('print("a" * 1.5)\n', "TypeError")

    def test_equality_between_different_types(self):
        self.assert_program_output(
            'print("a" == 0)\nprint("a" != 1)\nprint(None == 0)\nprint(None == None)\n'
            'print(None != None)\nprint("1" == 1)\nprint(1 == 1.0)\nprint(True == 1)\n',
            "False\nTrue\nFalse\nTrue\nFalse\nFalse\nTrue\nTrue\n",
        )

    def test_ordering_between_incompatible_types_is_type_error(self):
        for expr in ('"a" < 1', '1 >= "a"', "None < 1", "None <= None"):
            with self.subTest(expr=expr):
                self.assert_runtime_error(f"print({expr})\n", "TypeError")

    def test_int_and_float_convert_strings(self):
        self.assert_program_output(
            'print(int("42"))\nprint(int(" -7 "))\nprint(float("3.5"))\nprint(float("2"))\n'
            "print(int(3.9))\nprint(int(-3.9))\nprint(int(True))\nprint(int())\nprint(float())\n",
            "42\n-7\n3.5\n2.0\n3\n-3\n1\n0\n0.0\n",
        )

    def test_invalid_conversions_are_errors(self):
        cases = (
            ('int("abc")', "ValueError"),
            ('int("3.5")', "ValueError"),
            ('int("")', "ValueError"),
            ('float("x1")', "ValueError"),
            ("int(None)", "TypeError"),
            ("float(None)", "TypeError"),
        )
        for expr, error in cases:
            with self.subTest(expr=expr):
                self.assert_runtime_error(f"print({expr})\n", error)

    # ---------- variaveis ----------

    def test_variable_reassignment_and_chaining(self):
        self.assert_program_output(
            "x = 1\ny = x + 1\nx = y * 10\nprint(x, y)\n",
            "20 2\n",
        )

    def test_variable_can_change_type(self):
        self.assert_program_output(
            "x = 1\nx = \"texto\"\nprint(x)\n",
            "texto\n",
        )

    def test_undefined_variable_reports_error(self):
        self.assert_runtime_error("print(nome)\n", "variavel 'nome' nao definida")

    def test_undefined_function_reports_error(self):
        self.assert_runtime_error("nada(1)\n", "funcao 'nada' nao definida")

    # ---------- controle de fluxo ----------

    def test_if_else_branches(self):
        self.assert_program_output(
            "x = 5\n"
            "if x > 10:\n    print(\"grande\")\n"
            "else:\n    print(\"pequeno\")\n"
            "if x < 10:\n    print(\"menor\")\n",
            "pequeno\nmenor\n",
        )

    def test_nested_if(self):
        self.assert_program_output(
            "a = 1\nb = 2\n"
            "if a == 1:\n"
            "    if b == 2:\n"
            "        print(\"ambos\")\n"
            "    else:\n"
            "        print(\"so a\")\n"
            "else:\n"
            "    print(\"nenhum\")\n",
            "ambos\n",
        )

    def test_single_line_blocks(self):
        self.assert_program_output(
            "x = 1\nif x == 1: print(\"sim\")\nelse: print(\"nao\")\n",
            "sim\n",
        )

    def test_statements_after_block_run_after_dedent(self):
        self.assert_program_output(
            "if True:\n    print(1)\nprint(2)\n",
            "1\n2\n",
        )

    def test_while_accumulates(self):
        self.assert_program_output(
            "i = 0\ntotal = 0\n"
            "while i < 5:\n    total = total + i\n    i = i + 1\n"
            "print(total)\n",
            "10\n",
        )

    def test_while_false_never_runs(self):
        self.assert_program_output(
            "while False:\n    print(1)\nprint(2)\n",
            "2\n",
        )

    def test_while_break(self):
        self.assert_program_output(
            "i = 0\n"
            "while True:\n"
            "    if i == 3:\n"
            "        break\n"
            "    print(i)\n"
            "    i = i + 1\n",
            "0\n1\n2\n",
        )

    def test_while_continue(self):
        self.assert_program_output(
            "i = 0\n"
            "while i < 5:\n"
            "    i = i + 1\n"
            "    if i % 2 == 0:\n"
            "        continue\n"
            "    print(i)\n",
            "1\n3\n5\n",
        )

    def test_nested_loops_break_only_inner(self):
        self.assert_program_output(
            "i = 0\n"
            "while i < 3:\n"
            "    j = 0\n"
            "    while True:\n"
            "        if j == 2:\n"
            "            break\n"
            "        print(i, j)\n"
            "        j = j + 1\n"
            "    i = i + 1\n",
            "0 0\n0 1\n1 0\n1 1\n2 0\n2 1\n",
        )

    def test_pass_statement(self):
        self.assert_program_output(
            "if True:\n    pass\nprint(\"ok\")\n",
            "ok\n",
        )

    # ---------- formato do codigo-fonte ----------

    def test_comments_and_blank_lines(self):
        self.assert_program_output(
            "# comentario inicial\n\nx = 1  # inline\n\n# outro\nprint(x)\n",
            "1\n",
        )

    def test_multiline_expression_inside_parentheses(self):
        self.assert_program_output(
            "x = (1 +\n     2 +\n     3)\nprint(x)\n",
            "6\n",
        )

    def test_multiline_call_arguments(self):
        self.assert_program_output(
            "print(\n    1,\n    2\n)\n",
            "1 2\n",
        )

    def test_empty_and_comment_only_programs_succeed(self):
        for source in ("", "\n\n", "# so um comentario\n"):
            with self.subTest(source=source):
                result = self.run_program(source)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertEqual(result.stdout, "")

    # ---------- erros sintaticos ----------

    def test_syntax_error_returns_nonzero(self):
        result = self.run_program("if True\n    print(1)\n")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("ERRO DE SINTAXE", result.stderr)

    def test_syntax_error_reports_line_number(self):
        result = self.run_program("x = 1\ny = 2\nz = = 3\n")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("linha", result.stderr)

    def test_syntax_errors_in_core_constructs(self):
        for source in (
            "x = \n",
            "print(1\n",
            "print(1))\n",
            "while True\n    pass\n",
            "else:\n    print(1)\n",
            "if True:\nprint(1)\n",
            "1 = x\n",
            "x = 1 +\n",
            "    x = 1\n",
        ):
            with self.subTest(source=source):
                self.assert_syntax_error(source)

    def test_no_output_when_program_has_syntax_error(self):
        result = self.run_program("print(1)\nprint(\n")
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual(result.stdout, "")

    # ---------- erros lexicos ----------

    def test_lexical_error_returns_nonzero_and_does_not_execute(self):
        result = self.run_program("print(1)\nx = $ 2\n")
        self.assertEqual(result.returncode, 1)
        self.assertIn("ERRO LEXICO", result.stderr)
        self.assertIn("linha 2", result.stderr)
        self.assertEqual(result.stdout, "")

    def test_integer_literal_out_of_range_is_lexical_error(self):
        result = self.run_program("print(1)\nx = 99999999999999999999\n")
        self.assertEqual(result.returncode, 1)
        self.assertIn("ERRO LEXICO", result.stderr)
        self.assertIn("linha 2", result.stderr)
        self.assertEqual(result.stdout, "")

    def test_indentation_error_returns_nonzero(self):
        result = self.run_program("if True:\n        print(1)\n    print(2)\n")
        self.assertEqual(result.returncode, 1)
        self.assertIn("ERRO DE INDENTACAO", result.stderr)
        self.assertEqual(result.stdout, "")

    # ---------- erros de execucao ----------

    def test_runtime_error_stops_program(self):
        result = self.assert_runtime_error(
            'print("antes")\nprint(x)\nprint("depois")\n',
            "variavel 'x' nao definida",
        )
        self.assertEqual(result.stdout, "antes\n")

    def test_division_by_zero_is_runtime_error(self):
        for expr in ("1 / 0", "1 // 0", "10 % 0", "1.0 / 0.0", "1.5 % 0", "7 // 0.0"):
            with self.subTest(expr=expr):
                self.assert_runtime_error(f"print({expr})\n", "ZeroDivisionError")

    def test_arithmetic_with_non_numeric_is_type_error(self):
        for expr in ('"a" + 1', '1 + "a"', '"a" - "b"', "None + 1", "1 * None", '-"a"', "-None"):
            with self.subTest(expr=expr):
                self.assert_runtime_error(f"print({expr})\n", "TypeError")

    def test_bool_is_numeric_in_arithmetic(self):
        self.assert_program_output("print(True + 1)\nprint(-True)\n", "2\n-1\n")

    def test_integer_power_and_large_values_within_64_bits(self):
        self.assert_program_output(
            "print(2 ** 62)\nprint((-2) ** 63)\nprint(3 ** 0)\nprint(2 ** -1)\n"
            "print(9223372036854775807)\nprint(-9223372036854775807 - 1)\n"
            "print(int(-9223372036854775808.0))\nprint(3037000499 * 3037000499)\n",
            "4611686018427387904\n-9223372036854775808\n1\n0.5\n"
            "9223372036854775807\n-9223372036854775808\n"
            "-9223372036854775808\n9223372030926249001\n",
        )

    def test_integer_overflow_is_runtime_error(self):
        for expr in (
            "2 ** 63",
            "2 ** 100",
            "9223372036854775807 + 1",
            "-9223372036854775807 - 2",
            "3037000500 * 3037000500",
            "-(-9223372036854775807 - 1)",
            "(-9223372036854775807 - 1) // -1",
            "int(100000000000000000000.0)",
            'int("99999999999999999999")',
        ):
            with self.subTest(expr=expr):
                self.assert_runtime_error(f"print({expr})\n", "OverflowError")

    # ---------- erros semanticos ----------

    def test_break_or_continue_outside_loop_is_rejected_before_running(self):
        for source in (
            "print(1)\nbreak\n",
            "print(1)\ncontinue\n",
            "x = 1\nif x:\n    break\n",
            "if True:\n    pass\nelse:\n    continue\n",
        ):
            with self.subTest(source=source):
                result = self.run_program(source)
                self.assertEqual(result.returncode, 1, result.stderr)
                self.assertIn("ERRO SEMANTICO", result.stderr)
                self.assertIn("fora de laco", result.stderr)
                self.assertEqual(result.stdout, "")

    def test_break_and_continue_inside_loop_are_accepted(self):
        self.assert_program_output(
            "i = 0\n"
            "while i < 5:\n"
            "    i = i + 1\n"
            "    if i == 2:\n"
            "        continue\n"
            "    if i == 4:\n"
            "        break\n"
            "    print(i)\n"
            "if True:\n"
            "    while True:\n"
            "        break\n"
            "print(\"fim\")\n",
            "1\n3\nfim\n",
        )

    # ---------- executavel ----------

    def test_missing_input_file_fails(self):
        result = subprocess.run(
            [str(ROOT / "compilador"), "/caminho/inexistente.py"],
            cwd=ROOT, text=True, capture_output=True, check=False,
        )
        self.assertEqual(result.returncode, 1)

    def test_too_many_arguments_prints_usage(self):
        result = subprocess.run(
            [str(ROOT / "compilador"), "a.py", "b.py"],
            cwd=ROOT, text=True, capture_output=True, check=False,
        )
        self.assertEqual(result.returncode, 2)
        self.assertIn("Uso", result.stderr)


if __name__ == "__main__":
    unittest.main()
