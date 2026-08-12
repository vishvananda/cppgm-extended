import importlib.util
from pathlib import Path
import unittest


REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPT_PATH = REPO_ROOT / "scripts" / "analyze_witness_provenance.py"
SPEC = importlib.util.spec_from_file_location("analyze_witness_provenance", SCRIPT_PATH)
MODULE = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(MODULE)


class AnalyzeWitnessProvenanceTest(unittest.TestCase):
    @staticmethod
    def record(record_kind, **fields):
        return {"record": record_kind, "_trace_file": "trace.jsonl", **fields}

    def test_alias_routes_own_table_and_renderer_actions(self):
        route = "alias.canonical_occurrence"
        producer = "alias.canonical_occurrence"
        records = [
            self.record(
                "source_attempt",
                producer=producer,
                upstream_route=route,
                action="inserted",
                kind="alias_use",
            ),
            self.record(
                "final_table_row",
                producers=[producer],
                upstream_routes=[route],
                kind="alias_use",
            ),
            self.record(
                "renderer_action",
                producers=[producer],
                upstream_routes=[route],
                action="removed",
                kind="alias_use",
                **{"pass": "canonicalize_locations_and_dedupe"},
            ),
            self.record(
                "final_visible",
                producers=[producer],
                upstream_routes=[route],
                kind="alias_use",
                source="test.t",
                location="test.t:1:1",
                template_name="alias",
            ),
        ]

        report = MODULE.build_report(records)
        coverage = report["alias_upstream_route_coverage"][route]
        self.assertEqual(coverage["attempts"], 1)
        self.assertEqual(coverage["inserted"], 1)
        self.assertEqual(coverage["surviving_rows"], 1)
        self.assertEqual(coverage["final_visible_rows"], 1)
        self.assertEqual(
            report["alias_renderer_ownership_by_route"][route][
                "canonicalize_locations_and_dedupe:removed"
            ],
            1,
        )

    def test_unknown_alias_route_is_visible(self):
        report = MODULE.build_report(
            [
                self.record(
                    "source_attempt",
                    producer="alias.canonical_occurrence",
                    action="inserted",
                    kind="alias_use",
                )
            ]
        )
        self.assertEqual(
            report["alias_upstream_route_coverage"]["unknown"]["attempts"],
            1,
        )

    def test_function_route_owns_attempt_table_and_visible_row(self):
        route = "function.overload_resolution"
        producer = "function.semantic_template_function"
        report = MODULE.build_report(
            [
                self.record(
                    "source_attempt",
                    producer=producer,
                    upstream_route=route,
                    action="inserted",
                    kind="function_call",
                    location="test.t:4:3",
                    template_name="f",
                ),
                self.record(
                    "final_table_row",
                    producers=[producer],
                    upstream_routes=[route],
                    kind="function_call",
                ),
                self.record(
                    "final_visible",
                    producers=[producer],
                    upstream_routes=[route],
                    kind="function_call",
                    source="test.t",
                    location="test.t:4:3",
                    template_name="f",
                ),
            ]
        )
        coverage = report["upstream_route_coverage"][route]
        self.assertEqual(coverage["attempts"], 1)
        self.assertEqual(coverage["kind:function_call"], 1)
        self.assertEqual(coverage["surviving_rows"], 1)
        self.assertEqual(coverage["final_visible_rows"], 1)
        self.assertEqual(
            report["source_attempt_decisions"][0]["location"],
            "test.t:4:3",
        )

    def test_explicit_trace_file_count_includes_empty_sessions(self):
        report = MODULE.build_report([], trace_file_count=3)
        self.assertEqual(report["trace_files"], 3)
        self.assertEqual(report["records"], 0)

    def test_single_owner_schema_has_no_arbitration_counters(self):
        report = MODULE.build_report([])
        self.assertEqual(report["schema_version"], 5)
        self.assertNotIn("replacement_matrix", report)
        self.assertNotIn("collision_matrix", report)
        function_coverage = report["site_coverage"][
            "function.semantic_template_function"
        ]
        lifecycle_coverage = report["site_coverage"][
            "lifecycle.transition_observer.01"
        ]
        for field in ("exact_duplicate", "rejected", "replaced", "enriched"):
            self.assertNotIn(field, function_coverage)
            self.assertNotIn(field, lifecycle_coverage)

    def test_lifecycle_context_is_reported(self):
        report = MODULE.build_report(
            [
                self.record(
                    "lifecycle_attempt",
                    producer="lifecycle.transition_observer.01",
                    action="inserted",
                    kind="variable_instantiation",
                    location="test.t:7:3",
                    entity="value<int>",
                    entry_origin=1,
                    closure_reason=3,
                    cause=2,
                    public_source_required=True,
                ),
            ]
        )
        self.assertEqual(
            report["lifecycle_attempt_context_summary"]["entry_origin:1"], 1
        )
        self.assertEqual(
            report["lifecycle_attempt_context_summary"][
                "public_source_required"
            ],
            1,
        )

if __name__ == "__main__":
    unittest.main()
