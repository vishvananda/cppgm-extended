#!/usr/bin/env python3

from __future__ import annotations

import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPT = REPO_ROOT / "scripts" / "audit_text_reparse.py"


class AuditTextReparseTests(unittest.TestCase):
    def run_script(self, root: Path, *args: str) -> subprocess.CompletedProcess[str]:
        return subprocess.run(
            [sys.executable, str(SCRIPT), "--root", str(root), *args],
            cwd=REPO_ROOT,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=False,
        )

    def test_template_argument_text_shape_deduction_is_counted(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cppgm-text-reparse-audit.") as temp_dir:
            root = Path(temp_dir)
            src = root / "dev" / "src"
            src.mkdir(parents=True)
            (src / "template_resolution.cpp").write_text(
                "auto deduce_template_argument_text_shape = [] { return false; };\n"
                "trace << \"deduce-text-shape-template-mismatch\";\n"
                "trace << \"deduce-template-arg-text-shape-fail\";\n"
                "trace << \"template argument spelling diagnostic only\";\n",
                encoding="utf-8",
            )
            baseline = root / "baseline.json"
            baseline.write_text(
                json.dumps({
                    "limits": {
                        "qualified_name_string_parse": 0,
                        "template_id_string_parse": 0,
                        "expression_fragment_parse": 0,
                        "type_fragment_parse": 0,
                        "semantic_type_text_bridge": 0,
                        "translation_unit_fragment_parse": 0,
                        "ast_text_rebuild": 0,
                        "source_line_recovery": 0,
                        "template_argument_text_shape_deduction": 0,
                        "semantic_template_fragment_reparse": 0,
                        "semantic_text_tokenizer_reparse": 0,
                        "manual_template_argument_text_parse": 0,
                        "semantic_nttp_text_rebind": 0,
                    }
                }),
                encoding="utf-8",
            )

            result = self.run_script(src, "--baseline", str(baseline), "--list-sites")

            self.assertEqual(result.returncode, 1)
            self.assertIn("template_argument_text_shape_deduction", result.stdout)
            self.assertIn("deduce_template_argument_text_shape", result.stdout)
            self.assertIn("deduce-text-shape-template-mismatch", result.stdout)
            self.assertIn("deduce-template-arg-text-shape-fail", result.stdout)
            self.assertNotIn("template argument spelling diagnostic only", result.stdout)

    def test_semantic_fragment_and_manual_text_reparse_are_counted(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cppgm-text-reparse-audit.") as temp_dir:
            root = Path(temp_dir)
            src = root / "dev" / "src"
            src.mkdir(parents=True)
            (src / "template_resolution.cpp").write_text(
                "bool parse_type_argument_text_syntax(const std::string & text);\n"
                "PPTokenizer pp_tokens(input.rdbuf());\n"
                "PostTokenizer post_tokens(pp_tokens);\n"
                "RecogTokenizer recog_tokens(post_tokens);\n"
                "split_top_level_function_type_argument_text(text, result, params);\n",
                encoding="utf-8",
            )
            baseline = root / "baseline.json"
            baseline.write_text(
                json.dumps({
                    "limits": {
                        "qualified_name_string_parse": 0,
                        "template_id_string_parse": 0,
                        "expression_fragment_parse": 0,
                        "type_fragment_parse": 0,
                        "semantic_type_text_bridge": 0,
                        "translation_unit_fragment_parse": 0,
                        "ast_text_rebuild": 0,
                        "source_line_recovery": 0,
                        "template_argument_text_shape_deduction": 0,
                        "semantic_template_fragment_reparse": 0,
                        "semantic_text_tokenizer_reparse": 0,
                        "manual_template_argument_text_parse": 0,
                        "semantic_nttp_text_rebind": 0,
                    }
                }),
                encoding="utf-8",
            )

            result = self.run_script(src, "--baseline", str(baseline), "--list-sites")

            self.assertEqual(result.returncode, 1)
            self.assertIn("semantic_template_fragment_reparse", result.stdout)
            self.assertIn("parse_type_argument_text_syntax", result.stdout)
            self.assertIn("semantic_text_tokenizer_reparse", result.stdout)
            self.assertIn("PPTokenizer pp_tokens", result.stdout)
            self.assertIn("PostTokenizer post_tokens", result.stdout)
            self.assertIn("RecogTokenizer recog_tokens", result.stdout)
            self.assertIn("manual_template_argument_text_parse", result.stdout)
            self.assertIn("split_top_level_function_type_argument_text", result.stdout)

    def test_semantic_nttp_text_rebind_is_counted(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cppgm-text-reparse-audit.") as temp_dir:
            root = Path(temp_dir)
            src = root / "dev" / "src"
            src.mkdir(parents=True)
            (src / "semantic_expression.cpp").write_text(
                "bool try_analyze_non_type_template_member_pointer_text();\n"
                "auto x = ctx.lookup_value(scope, binding.non_type_template_argument_text);\n"
                "auto y = ctx.lookup_functions(scope, binding.non_type_template_argument_text, policy);\n"
                "std::string rebound_text = binding.non_type_template_argument_text;\n",
                encoding="utf-8",
            )
            baseline = root / "baseline.json"
            baseline.write_text(
                json.dumps({
                    "limits": {
                        "qualified_name_string_parse": 0,
                        "template_id_string_parse": 0,
                        "expression_fragment_parse": 0,
                        "type_fragment_parse": 0,
                        "semantic_type_text_bridge": 0,
                        "translation_unit_fragment_parse": 0,
                        "ast_text_rebuild": 0,
                        "source_line_recovery": 0,
                        "template_argument_text_shape_deduction": 0,
                        "semantic_template_fragment_reparse": 0,
                        "semantic_text_tokenizer_reparse": 0,
                        "manual_template_argument_text_parse": 0,
                        "semantic_nttp_text_rebind": 0,
                    }
                }),
                encoding="utf-8",
            )

            result = self.run_script(src, "--baseline", str(baseline), "--list-sites")

            self.assertEqual(result.returncode, 1)
            self.assertIn("semantic_nttp_text_rebind", result.stdout)
            self.assertIn("try_analyze_non_type_template_member_pointer_text", result.stdout)
            self.assertIn("ctx.lookup_value", result.stdout)
            self.assertIn("ctx.lookup_functions", result.stdout)
            self.assertIn("rebound_text", result.stdout)


if __name__ == "__main__":
    unittest.main()
