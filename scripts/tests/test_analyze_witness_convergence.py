import importlib.util
from pathlib import Path
import sys
import tempfile
import unittest


REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPT_PATH = REPO_ROOT / "scripts" / "analyze_witness_convergence.py"
SPEC = importlib.util.spec_from_file_location("analyze_witness_convergence", SCRIPT_PATH)
MODULE = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


class AnalyzeWitnessConvergenceTest(unittest.TestCase):
    def test_absolute_location_disambiguates_shared_test_basename(self):
        record = {
            "location": "/repo/pa23/tests/general/shared.t:4:3",
            "source": "/tmp/shared.t.1.jsonl",
        }
        self.assertTrue(
            MODULE._record_belongs_to_test(
                record, "pa23/tests/general/shared.t"
            )
        )
        self.assertFalse(
            MODULE._record_belongs_to_test(
                record, "pa24/tests/general/shared.t"
            )
        )

    def test_parse_source_and_lifecycle_events(self):
        events = MODULE.parse_witness(
            """translation-unit
  alias-use at tests/general/sample.t:4:3
    template A
    bind #1 = int source=explicit
template-closure-events
  variable-instantiation
    entity value<int>
"""
        )
        self.assertEqual(len(events), 2)
        self.assertEqual(events[0].family, "alias_use")
        self.assertEqual(events[0].template_name, "A")
        self.assertEqual(events[1].family, "lifecycle")
        self.assertEqual(events[1].entity, "value<int>")

    def test_classify_changed_missing_and_unexpected(self):
        expected = MODULE.parse_witness(
            """translation-unit
  alias-use at tests/general/sample.t:4:3
    template A
    bind #1 = int source=explicit
  class-use at tests/general/sample.t:8:3
    template C
    selected primary
"""
        )
        actual = MODULE.parse_witness(
            """translation-unit
  alias-use at tests/general/sample.t:4:3
    template A
    bind #1 = long source=explicit
  function-call at tests/general/sample.t:10:3
    template f
    selected f
"""
        )
        occurrences = MODULE.classify_events(expected, actual)
        self.assertEqual(
            [item["classification"] for item in occurrences],
            ["changed", "missing_expected", "unexpected_actual"],
        )

    def test_lifecycle_classification_uses_normalized_fact_sets(self):
        expected = MODULE.parse_witness(
            """translation-unit
template-closure-events
  ensure-definition
    entity make<int>
  function-instantiation
    entity make<int>
"""
        )
        actual = MODULE.parse_witness(
            """translation-unit
template-closure-events
  require-definition
    entity make<int>
  ensure-definition
    entity make<int>
  function-instantiation
    entity make<int>
  function-instantiation
    entity make<int>
"""
        )
        self.assertEqual(MODULE.classify_events(expected, actual), [])

    def test_lifecycle_classification_warns_only_for_extra_demand(self):
        expected = MODULE.parse_witness("translation-unit\n")
        actual = MODULE.parse_witness(
            """translation-unit
template-closure-events
  require-definition
    entity extra<int>
"""
        )
        occurrences = MODULE.classify_events(expected, actual)
        self.assertEqual(len(occurrences), 1)
        self.assertEqual(
            occurrences[0]["classification"], "additional_definition_demand"
        )
        self.assertEqual(occurrences[0]["entity"], "extra<int>")

    def test_lifecycle_classification_keeps_terminals_distinct(self):
        expected = MODULE.parse_witness(
            """translation-unit
template-closure-events
  class-instantiation
    entity box<int>
"""
        )
        actual = MODULE.parse_witness(
            """translation-unit
template-closure-events
  variable-instantiation
    entity box<int>::value
"""
        )
        occurrences = MODULE.classify_events(expected, actual)
        self.assertEqual(
            [item["classification"] for item in occurrences],
            ["missing_expected", "unexpected_actual"],
        )

    def test_report_counts_mismatching_outputs_and_routes(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            tests = root / "pa19" / "tests" / "general"
            tests.mkdir(parents=True)
            (tests / "sample.ref.witness").write_text(
                """translation-unit
  alias-use at tests/general/sample.t:4:3
    template A
""",
                encoding="utf-8",
            )
            (tests / "sample.my.witness").write_text(
                "translation-unit\n", encoding="utf-8"
            )
            report = MODULE.build_report(
                root,
                ("pa19",),
                {
                    "source_publications": [
                        {
                            "kind": "alias_use",
                            "location": "tests/general/sample.t:4:3",
                            "template_name": "A",
                            "producer": "alias.canonical_occurrence",
                            "upstream_route": "alias.canonical_occurrence",
                            "source": "/tmp/pa19/sample.t.1.jsonl",
                        }
                    ]
                },
            )
            self.assertEqual(report["references"], 1)
            self.assertEqual(report["mismatching_outputs"], 1)
            self.assertEqual(report["family_summary"]["alias_use"]["tests"], 1)
            provenance = report["tests"][0]["occurrences"][0]["provenance"]
            self.assertEqual(
                provenance["semantic_routes"], ["alias.canonical_occurrence"]
            )

    def test_report_counts_warning_only_output_as_matching(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            tests = root / "pa19" / "tests" / "general"
            tests.mkdir(parents=True)
            (tests / "sample.ref.witness").write_text(
                "translation-unit\n", encoding="utf-8"
            )
            (tests / "sample.my.witness").write_text(
                """translation-unit
template-closure-events
  ensure-definition
    entity extra<int>
""",
                encoding="utf-8",
            )
            report = MODULE.build_report(root, ("pa19",))
            self.assertEqual(report["matching_outputs"], 1)
            self.assertEqual(report["mismatching_outputs"], 0)
            self.assertEqual(report["warning_outputs"], 1)
            self.assertEqual(len(report["warnings"]), 1)

    def test_retired_materialization_shadow_inventory_is_absent(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            tests = root / "pa19" / "tests" / "general"
            tests.mkdir(parents=True)
            witness = "translation-unit\n"
            (tests / "sample.ref.witness").write_text(witness, encoding="utf-8")
            (tests / "sample.my.witness").write_text(witness, encoding="utf-8")
            report = MODULE.build_report(root, ("pa19",), {})
            self.assertEqual(report["schema_version"], 2)
            self.assertNotIn("class_materialization_candidate_summary", report)
            self.assertNotIn("class_materialization_candidates", report)

if __name__ == "__main__":
    unittest.main()
