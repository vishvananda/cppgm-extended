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
    "semantic_expression_text_reparse": 0,
    "semantic_template_id_text_decomposition": 0,
    "semantic_qualified_name_text_reparse": 0,
    "semantic_nttp_text_rebind": 0,
    "function_result_argument_text_reparse": 0,
    "owner_member_text_reparse": 0,
    "abi_template_component_text_reparse": 0,
    "semantic_argument_spelling_recovery": 0,
    "template_parameter_display_lookup": 0,
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
                "bool parse_dependent_type_expr_text(const std::string & text);\n"
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
            self.assertIn("parse_dependent_type_expr_text", result.stdout)
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

    def test_semantic_expression_text_reparse_is_counted(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cppgm-text-reparse-audit.") as temp_dir:
            root = Path(temp_dir)
            src = root / "dev" / "src"
            src.mkdir(parents=True)
            (src / "template_argument_semantics.cpp").write_text(
                "try_evaluate_integral_text_with_pack_scope(scope, text, value);\n"
                "find_top_level_binary_operator_token_text(text, ops, pos, op);\n"
                "parse_sizeof_pack_count_text(scope, text, value);\n"
                "lookup_integral_constant_count_text(scope, text, value);\n"
                "const string integer_pack_prefix = \"__integer_pack(\";\n"
                "rewrite_decltype_expression_pack_texts(services, scope, text);\n"
                "split_top_level_call_expression_text(text, callee, args);\n",
                encoding="utf-8",
            )
            baseline = root / "baseline.json"
            baseline.write_text(
                json.dumps({"limits": ZERO_LIMITS}),
                encoding="utf-8",
            )

            result = self.run_script(src, "--baseline", str(baseline), "--list-sites")

            self.assertEqual(result.returncode, 1)
            self.assertIn("semantic_expression_text_reparse", result.stdout)
            self.assertIn("try_evaluate_integral_text_with_pack_scope", result.stdout)
            self.assertIn("find_top_level_binary_operator_token_text", result.stdout)
            self.assertIn("parse_sizeof_pack_count_text", result.stdout)
            self.assertIn("lookup_integral_constant_count_text", result.stdout)
            self.assertIn("integer_pack_prefix", result.stdout)
            self.assertIn("rewrite_decltype_expression_pack_texts", result.stdout)
            self.assertIn("split_top_level_call_expression_text", result.stdout)

    def test_legacy_template_id_text_decomposition_is_counted(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cppgm-text-reparse-audit.") as temp_dir:
            root = Path(temp_dir)
            src = root / "dev" / "src"
            src.mkdir(parents=True)
            (src / "template_specialization.cpp").write_text(
                "semantic_utils::split_top_level_template_id_text(text, name, args);\n"
                "deduce_from_named_template_id_text(services, partial, state);\n"
                "template_id_syntax_from_component_text(text, syntax);\n"
                "template_lookup_fragment_text(lookup_name);\n"
                "template_lookup_fragment_identifier(fragment);\n"
                "split_unqualified_template_head_text(text, head);\n"
                "parse_angle_type_transform_text(text, name, args);\n",
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
            self.assertIn("template_lookup_fragment_text", result.stdout)
            self.assertIn("split_unqualified_template_head_text", result.stdout)
            self.assertIn("parse_angle_type_transform_text", result.stdout)

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

    def test_abi_template_component_text_reparse_is_counted(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cppgm-text-reparse-audit.") as temp_dir:
            root = Path(temp_dir)
            src = root / "dev" / "src"
            src.mkdir(parents=True)
            (src / "symbol_linkage.cpp").write_text(
                "parse_template_component(text, component);\n"
                "split_template_arguments(body, arguments);\n",
                encoding="utf-8",
            )
            baseline = root / "baseline.json"
            baseline.write_text(
                json.dumps({"limits": ZERO_LIMITS}),
                encoding="utf-8",
            )

            result = self.run_script(src, "--baseline", str(baseline), "--list-sites")

            self.assertEqual(result.returncode, 1)
            self.assertIn("abi_template_component_text_reparse", result.stdout)
            self.assertIn("parse_template_component", result.stdout)
            self.assertIn("split_template_arguments", result.stdout)

    def test_semantic_argument_spelling_recovery_is_counted(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cppgm-text-reparse-audit.") as temp_dir:
            root = Path(temp_dir)
            src = root / "dev" / "src"
            src.mkdir(parents=True)
            (src / "template_argument_semantics.cpp").write_text(
                "evaluate_non_type_argument_text(services, scope, text, value);\n"
                "try_parse_builtin_type_trait_text(scope, text, value);\n"
                "resolve_template_template_argument_text(scope, text, result);\n"
                "resolve_member_template_template_argument_text(scope, text, result);\n"
                "resolve_member_template_owner_type_text(scope, text, result);\n"
                "lookup_rewritten_bound_type_argument(scope, text, result);\n"
                "template_argument_text_matches_type_binding(text, type);\n"
                "deduce_array_bound_texts_from_actual_type(parameters, text, pattern, actual);\n"
                "top_level_array_bound_texts(pattern_text);\n"
                "lookup_bound_template_template_argument_by_canonical_text(scope, text);\n"
                "parse_simple_identifier_pack_expansion_text(text, name, expanded);\n"
                "annotate_template_id_type_arguments_from_matching_scope_bindings(\n"
                "    scope, decl, syntax);\n",
                encoding="utf-8",
            )
            baseline = root / "baseline.json"
            baseline.write_text(
                json.dumps({"limits": ZERO_LIMITS}),
                encoding="utf-8",
            )

            result = self.run_script(src, "--baseline", str(baseline), "--list-sites")

            self.assertEqual(result.returncode, 1)
            self.assertIn("semantic_argument_spelling_recovery", result.stdout)
            self.assertIn("evaluate_non_type_argument_text", result.stdout)
            self.assertIn("try_parse_builtin_type_trait_text", result.stdout)
            self.assertIn("resolve_template_template_argument_text", result.stdout)
            self.assertIn("resolve_member_template_template_argument_text", result.stdout)
            self.assertIn("resolve_member_template_owner_type_text", result.stdout)
            self.assertIn("lookup_rewritten_bound_type_argument", result.stdout)
            self.assertIn("template_argument_text_matches_type_binding", result.stdout)
            self.assertIn("deduce_array_bound_texts_from_actual_type", result.stdout)
            self.assertIn("top_level_array_bound_texts", result.stdout)
            self.assertIn(
                "lookup_bound_template_template_argument_by_canonical_text",
                result.stdout,
            )
            self.assertIn("parse_simple_identifier_pack_expansion_text", result.stdout)
            self.assertIn(
                "annotate_template_id_type_arguments_from_matching_scope_bindings",
                result.stdout,
            )

    def test_template_parameter_display_lookup_is_counted_across_lines(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cppgm-text-reparse-audit.") as temp_dir:
            root = Path(temp_dir)
            src = root / "dev" / "src"
            src.mkdir(parents=True)
            (src / "template_resolution.cpp").write_text(
                "parameter = find_template_parameter(\n"
                "    parameters,\n"
                "    base->named_display);\n",
                encoding="utf-8",
            )
            baseline = root / "baseline.json"
            baseline.write_text(
                json.dumps({"limits": ZERO_LIMITS}),
                encoding="utf-8",
            )

            result = self.run_script(src, "--baseline", str(baseline), "--list-sites")

            self.assertEqual(result.returncode, 1)
            self.assertIn("template_parameter_display_lookup", result.stdout)
            self.assertIn("base->named_display", result.stdout)

    def test_semantic_qualified_name_text_reparse_is_counted(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cppgm-text-reparse-audit.") as temp_dir:
            root = Path(temp_dir)
            src = root / "dev" / "src"
            src.mkdir(parents=True)
            (src / "template_argument_semantics.cpp").write_text(
                "bool refresh_substituted_member_value_expression();\n"
                "bool recover_qualified_owner_from_text();\n"
                "bool parse_out_of_class_member_qualified_name();\n"
                "void refresh_qualified_name_qualifier_template_id_texts();\n"
                "semantic_utils::split_qualified_name_text(text, name);\n",
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
            self.assertIn("parse_out_of_class_member_qualified_name", result.stdout)
            self.assertIn("refresh_qualified_name_qualifier_template_id_texts", result.stdout)
            self.assertIn("split_qualified_name_text", result.stdout)

    def test_owner_member_text_reparse_is_counted(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cppgm-text-reparse-audit.") as temp_dir:
            root = Path(temp_dir)
            src = root / "dev" / "src"
            src.mkdir(parents=True)
            (src / "template_instantiation.cpp").write_text(
                "bool result_owner_member_text_reparse = true;\n"
                "bool evaluate_qualified_member_value_from_text();\n"
                "bool split_top_level_member_expression_text();\n"
                "void collect_non_type_parameter_pack_references_from_text();\n",
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
            self.assertIn("split_top_level_member_expression_text", result.stdout)
            self.assertIn("collect_non_type_parameter_pack_references_from_text", result.stdout)

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
