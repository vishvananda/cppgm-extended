#!/usr/bin/env python3

from __future__ import annotations

import importlib.util
import sys
import tempfile
import unittest
from pathlib import Path


SCRIPT = Path(__file__).parents[1] / "audit_duplicate_ast_semantic_walks.py"
SPEC = importlib.util.spec_from_file_location("audit_duplicate_ast_semantic_walks", SCRIPT)
assert SPEC and SPEC.loader
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


STRUCTURAL_WALK = """void record_template_source_contexts_for_witness_node(
    const CppAstNode & node) const
{
  record_template_header_context_for_witness_node(node);
  record_template_body_range_for_witness_node(node);
  for(const CppAstNode & child : node.children) {
    record_template_source_contexts_for_witness_node(child);
  }
}
"""


class DuplicateAstSemanticWalkAuditTests(unittest.TestCase):
    def make_root(self, callsemantic: str, renderer: str = "") -> Path:
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        root = Path(temporary.name)
        source = root / "dev/src"
        (source / "callsemantic").mkdir(parents=True)
        (source / "callsemantic.cpp").write_text(callsemantic, encoding="utf-8")
        (source / "callsemantic/template_declaration_collector.cpp").write_text(
            "", encoding="utf-8"
        )
        (source / "template_witness_renderer.cpp").write_text(
            renderer, encoding="utf-8"
        )
        return root

    def test_structural_source_context_walk_is_named_obligation(self) -> None:
        report = MODULE.audit(self.make_root(STRUCTURAL_WALK))
        self.assertEqual(report["findings"], [])
        self.assertEqual(report["obligations"]["template_source_context_index_walk"], 1)

    def test_semantic_analysis_inside_structural_walk_fails(self) -> None:
        source = STRUCTURAL_WALK.replace(
            "record_template_header_context_for_witness_node(node);",
            "parse_template_parameters(node, parameters);",
        )
        findings = MODULE.audit(self.make_root(source))["findings"]
        self.assertTrue(any("performs semantic analysis" in item["reason"]
                            for item in findings))

    def test_unapproved_template_parameter_reanalysis_fails(self) -> None:
        source = STRUCTURAL_WALK + """
void rebuild_alias_witness(const CppAstNode & node)
{
  parse_template_parameters(node, parameters);
}
"""
        findings = MODULE.audit(self.make_root(source))["findings"]
        self.assertTrue(any("unapproved owner" in item["reason"]
                            for item in findings))

    def test_recursive_renderer_ast_walk_fails(self) -> None:
        renderer = """void render_again(const CppAstNode & node)
{
  for(const CppAstNode & child : node.children) {
    render_again(child);
  }
}
"""
        findings = MODULE.audit(self.make_root(STRUCTURAL_WALK, renderer))["findings"]
        self.assertTrue(any("renderer contains a recursive AST walk" in item["reason"]
                            for item in findings))


if __name__ == "__main__":
    unittest.main()
