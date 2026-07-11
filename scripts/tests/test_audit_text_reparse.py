#!/usr/bin/env python3

from __future__ import annotations

import json
import importlib.util
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPT = REPO_ROOT / "scripts" / "audit_text_reparse.py"
SPEC = importlib.util.spec_from_file_location("audit_text_reparse", SCRIPT)
assert SPEC is not None
AUDIT = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
sys.modules[SPEC.name] = AUDIT
SPEC.loader.exec_module(AUDIT)
ZERO_LIMITS = {
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
    "semantic_template_id_text_decomposition": 0,
    "semantic_qualified_name_text_reparse": 0,
    "semantic_nttp_text_rebind": 0,
    "function_result_argument_text_reparse": 0,
    "owner_member_text_reparse": 0,
    "added_semantic_text_reparse": 0,
}


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
                json.dumps({"limits": ZERO_LIMITS}),
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
                json.dumps({"limits": ZERO_LIMITS}),
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
                json.dumps({"limits": ZERO_LIMITS}),
                encoding="utf-8",
            )

            result = self.run_script(src, "--baseline", str(baseline), "--list-sites")

            self.assertEqual(result.returncode, 1)
            self.assertIn("semantic_nttp_text_rebind", result.stdout)
            self.assertIn("try_analyze_non_type_template_member_pointer_text", result.stdout)
            self.assertIn("ctx.lookup_value", result.stdout)
            self.assertIn("ctx.lookup_functions", result.stdout)
            self.assertIn("rebound_text", result.stdout)

    def test_legacy_template_id_text_decomposition_is_counted(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cppgm-text-reparse-audit.") as temp_dir:
            root = Path(temp_dir)
            src = root / "dev" / "src"
            src.mkdir(parents=True)
            (src / "template_specialization.cpp").write_text(
                "semantic_utils::split_top_level_template_id_text(text, name, args);\n"
                "deduce_from_named_template_id_text(services, partial, state);\n"
                "template_id_syntax_from_component_text(text, syntax);\n",
                encoding="utf-8",
            )
            baseline = root / "baseline.json"
            baseline.write_text(
                json.dumps({"limits": ZERO_LIMITS}),
                encoding="utf-8",
            )

            result = self.run_script(src, "--baseline", str(baseline), "--list-sites")

            self.assertEqual(result.returncode, 1)
            self.assertIn("semantic_template_id_text_decomposition", result.stdout)
            self.assertIn("split_top_level_template_id_text", result.stdout)
            self.assertIn("deduce_from_named_template_id_text", result.stdout)
            self.assertIn("template_id_syntax_from_component_text", result.stdout)

    def test_function_result_argument_text_reparse_is_counted(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cppgm-text-reparse-audit.") as temp_dir:
            root = Path(temp_dir)
            src = root / "dev" / "src"
            src.mkdir(parents=True)
            (src / "template_instantiation.cpp").write_text(
                "std::string instantiated_result_argument_text(const Arg & arg);\n"
                "bool result_non_type_argument_text_reparse = true;\n",
                encoding="utf-8",
            )
            baseline = root / "baseline.json"
            baseline.write_text(
                json.dumps({"limits": ZERO_LIMITS}),
                encoding="utf-8",
            )

            result = self.run_script(src, "--baseline", str(baseline), "--list-sites")

            self.assertEqual(result.returncode, 1)
            self.assertIn("function_result_argument_text_reparse", result.stdout)
            self.assertIn("instantiated_result_argument_text", result.stdout)
            self.assertIn("result_non_type_argument_text_reparse", result.stdout)

    def test_semantic_qualified_name_text_reparse_is_counted(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cppgm-text-reparse-audit.") as temp_dir:
            root = Path(temp_dir)
            src = root / "dev" / "src"
            src.mkdir(parents=True)
            (src / "template_argument_semantics.cpp").write_text(
                "bool refresh_substituted_member_value_expression();\n"
                "bool recover_qualified_owner_from_text();\n",
                encoding="utf-8",
            )
            baseline = root / "baseline.json"
            baseline.write_text(
                json.dumps({"limits": ZERO_LIMITS}),
                encoding="utf-8",
            )

            result = self.run_script(src, "--baseline", str(baseline), "--list-sites")

            self.assertEqual(result.returncode, 1)
            self.assertIn("semantic_qualified_name_text_reparse", result.stdout)
            self.assertIn("refresh_substituted_member_value_expression", result.stdout)
            self.assertIn("recover_qualified_owner_from_text", result.stdout)

    def test_owner_member_text_reparse_is_counted(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cppgm-text-reparse-audit.") as temp_dir:
            root = Path(temp_dir)
            src = root / "dev" / "src"
            src.mkdir(parents=True)
            (src / "template_instantiation.cpp").write_text(
                "bool result_owner_member_text_reparse = true;\n"
                "bool evaluate_qualified_member_value_from_text();\n",
                encoding="utf-8",
            )
            baseline = root / "baseline.json"
            baseline.write_text(
                json.dumps({"limits": ZERO_LIMITS}),
                encoding="utf-8",
            )

            result = self.run_script(src, "--baseline", str(baseline), "--list-sites")

            self.assertEqual(result.returncode, 1)
            self.assertIn("owner_member_text_reparse", result.stdout)
            self.assertIn("result_owner_member_text_reparse", result.stdout)
            self.assertIn("evaluate_qualified_member_value_from_text", result.stdout)

    def test_pack_owner_member_text_scan_is_counted(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cppgm-text-reparse-audit.") as temp_dir:
            root = Path(temp_dir)
            src = root / "dev" / "src"
            src.mkdir(parents=True)
            (src / "template_argument_semantics.cpp").write_text(
                "bool pattern_mentions_bound_type_pack_value_member();\n",
                encoding="utf-8",
            )
            baseline = root / "baseline.json"
            baseline.write_text(
                json.dumps({"limits": ZERO_LIMITS}),
                encoding="utf-8",
            )

            result = self.run_script(src, "--baseline", str(baseline), "--list-sites")

            self.assertEqual(result.returncode, 1)
            self.assertIn("owner_member_text_reparse", result.stdout)
            self.assertIn("pattern_mentions_bound_type_pack_value_member", result.stdout)

    def test_added_resolve_template_arguments_bridge_is_counted(self) -> None:
        category = next(
            item for item in AUDIT.CATEGORIES
            if item.name == "added_semantic_text_reparse"
        )

        self.assertTrue(category.diff_only)
        self.assertRegex(
            "template_api::resolve_template_arguments(",
            category.pattern,
        )
        self.assertRegex(
            "template_argument_semantics::resolve_template_arguments(",
            category.pattern,
        )


if __name__ == "__main__":
    unittest.main()
