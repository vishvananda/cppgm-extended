#!/usr/bin/env python3

from __future__ import annotations

import importlib.util
import sys
import tempfile
import unittest
from pathlib import Path


SCRIPT = Path(__file__).parents[1] / "audit_witness_store_ownership.py"
SPEC = importlib.util.spec_from_file_location("audit_witness_store_ownership", SCRIPT)
assert SPEC and SPEC.loader
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


SESSION = """struct TemplateWitnessSession
{
  struct Nested { int value = 0; };
  std::string primary_source_file;
  std::vector<int> lifecycle_events;
  std::map<int, int> lifecycle_transition_states;
  std::set<int> public_source_definition_dependencies;
  SemanticSourceUseTable source_use_table;
  std::map<int, int> variable_source_use_results;
  std::vector<std::string> inline_namespace_names;
  std::vector<int> template_body_ranges;
  std::vector<int> template_header_contexts;
  std::map<int, int> class_source_occurrences;
  std::map<int, int> retained_enum_value_bindings;
};
"""

TEXT_OUTPUT = """// Compact closure rows intentionally omit provenance
void analyze(const Session & session) {
  for(int i = 0; i < session.lifecycle_events.size(); ++i) {
    use(session.lifecycle_events[i]);
  }
  if(!public_seen.insert(public_key).second) return;
}
"""

CALLSEMANTIC = """void finish() {
  primary_source_file;
  lifecycle_events;
  class_source_occurrences;
  completed_alias_source_occurrences_.clear();
  pending_class_uses.clear();
  pending_class_use_indices.clear();
}
"""


class WitnessStoreOwnershipAuditTests(unittest.TestCase):
    def make_root(self) -> Path:
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        root = Path(temporary.name)
        source = root / "dev/src"
        (source / "callsemantic").mkdir(parents=True)

        files = {
            "template_witness.h": SESSION,
            "callsemantic.cpp": CALLSEMANTIC,
            "witness_provenance.cpp": "primary_source_file;\n",
            "template_text_output.cpp": TEXT_OUTPUT,
            "template_api.cpp": (
                "lifecycle_transition_states;\n"
                "public_source_definition_dependencies;\n"
                "template_body_ranges;\n"
                "template_header_contexts;\n"
                "retained_enum_value_bindings;\n"
            ),
            "semantic_conversion.cpp": "public_source_definition_dependencies;\n",
            "semantic_source_use.h": (
                "inline void record_source_use(T & table, const U & use) "
                "{ table.uses.push_back(use); }\n"
            ),
            "witness_api.cpp": (
                "source_use_table;\nvariable_source_use_results.clear();\n"
            ),
            "semantic_template_function.cpp": "source_use_table;\n",
            "template_witness_renderer.cpp": (
                "source_use_table;\ninline_namespace_names;\n"
                "aliases.erase(found);\n"
            ),
            "template_instantiation.cpp": "variable_source_use_results;\n",
            "template_resolution.cpp": "class_source_occurrences;\n",
            "template_argument_semantics.cpp": (
                "class_source_occurrences;\nretained_enum_value_bindings;\n"
            ),
            "callsemantic/class_template_reference.cpp": (
                "class_source_occurrences;\n"
            ),
        }
        for relative, text in files.items():
            path = source / relative
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(text, encoding="utf-8")
        return root

    def test_current_inventory_and_named_boundaries_pass(self) -> None:
        report = MODULE.audit(self.make_root())
        self.assertEqual(report["findings"], [])
        self.assertEqual(report["obligations"]["renderer_lifecycle_session_scans"], 1)

    def test_unclassified_session_store_fails(self) -> None:
        root = self.make_root()
        path = root / "dev/src/template_witness.h"
        text = path.read_text(encoding="utf-8")
        prefix, suffix = text.rsplit("};\n", 1)
        path.write_text(
            prefix + "  std::vector<int> repair_rows;\n};\n" + suffix,
            encoding="utf-8",
        )
        findings = MODULE.audit(root)["findings"]
        self.assertTrue(any("unclassified" in item["reason"] for item in findings))

    def test_session_store_foreign_owner_fails(self) -> None:
        root = self.make_root()
        (root / "dev/src/unrelated.cpp").write_text(
            "void f() { session.variable_source_use_results; }\n",
            encoding="utf-8",
        )
        findings = MODULE.audit(root)["findings"]
        self.assertTrue(any("escaped" in item["reason"] for item in findings))

    def test_destructive_source_ledger_fails(self) -> None:
        root = self.make_root()
        path = root / "dev/src/semantic_source_use.h"
        path.write_text(
            path.read_text(encoding="utf-8").replace(
                "table.uses.push_back(use);",
                "table.uses.push_back(use); table.uses.clear();",
            ),
            encoding="utf-8",
        )
        findings = MODULE.audit(root)["findings"]
        self.assertTrue(any("destructive arbitration" in item["reason"]
                            for item in findings))

    def test_second_lifecycle_session_scan_fails(self) -> None:
        root = self.make_root()
        path = root / "dev/src/template_text_output.cpp"
        path.write_text(
            path.read_text(encoding="utf-8") +
            "void again(const Session & session) { "
            "for(int i = 0; i < session.lifecycle_events.size(); ++i) "
            "use(session.lifecycle_events[i]); }\n",
            encoding="utf-8",
        )
        findings = MODULE.audit(root)["findings"]
        self.assertTrue(any("one aggregate session scan" in item["reason"]
                            for item in findings))

    def test_zero_obligation_alias_rollback_fails(self) -> None:
        root = self.make_root()
        path = root / "dev/src/callsemantic.cpp"
        path.write_text(
            path.read_text(encoding="utf-8") +
            "completed_alias_source_occurrences_.erase(completion_key);\n",
            encoding="utf-8",
        )
        findings = MODULE.audit(root)["findings"]
        self.assertTrue(any("zero-obligation alias completion rollback" in
                            item["reason"] for item in findings))

    def test_unnamed_renderer_erase_fails(self) -> None:
        root = self.make_root()
        path = root / "dev/src/template_witness_renderer.cpp"
        path.write_text(
            path.read_text(encoding="utf-8") + "events.erase(found);\n",
            encoding="utf-8",
        )
        findings = MODULE.audit(root)["findings"]
        self.assertTrue(any("unnamed destructive" in item["reason"]
                            for item in findings))


if __name__ == "__main__":
    unittest.main()
