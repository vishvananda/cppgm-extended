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
                collided_producers=[],
            ),
            self.record(
                "source_attempt",
                producer=producer,
                upstream_route=route,
                action="exact_duplicate",
                kind="alias_use",
                collided_producers=[producer],
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
        self.assertEqual(coverage["attempts"], 2)
        self.assertEqual(coverage["inserted"], 1)
        self.assertEqual(coverage["exact_duplicate"], 1)
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
                    collided_producers=[],
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
                    collided_producers=[],
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

    def test_semantic_consolidation_counts_are_aggregated(self):
        report = MODULE.build_report(
            [
                self.record(
                    "semantic_consolidation",
                    family="class_use",
                    completed_candidates=5,
                    early_repeats=2,
                    prepublication_merges=3,
                    collected_occurrences=2,
                    published_occurrences=2,
                ),
                self.record(
                    "semantic_consolidation",
                    family="class_use",
                    completed_candidates=7,
                    early_repeats=1,
                    prepublication_merges=4,
                    collected_occurrences=4,
                    published_occurrences=3,
                ),
            ]
        )
        self.assertEqual(
            report["semantic_consolidation"]["class_use"],
            {
                "completed_candidates": 12,
                "early_repeats": 3,
                "prepublication_merges": 7,
                "collected_occurrences": 6,
                "published_occurrences": 5,
            },
        )

    def test_alias_completion_and_lifecycle_context_are_reported(self):
        report = MODULE.build_report(
            [
                self.record(
                    "alias_completion",
                    operation="parameterized_resolution",
                    action="ignored_repeat",
                    location="test.t:4:9",
                    source_occurrence_id=17,
                    source_template_name="owner<T>::alias",
                    selected_template_name="alias",
                    selected_decl_location="test.t:2:7",
                    parameterized=True,
                    has_class_context=True,
                    resolved_type=False,
                ),
                self.record(
                    "lifecycle_attempt",
                    producer="lifecycle.transition_observer.01",
                    action="inserted",
                    kind="variable_instantiation",
                    location="test.t:7:3",
                    entity="value<int>",
                    collided_producers=[],
                    entry_origin=1,
                    closure_reason=3,
                    cause=2,
                    public_source_required=True,
                ),
            ]
        )
        self.assertEqual(report["alias_completion_summary"]["decisions"], 1)
        self.assertEqual(
            report["alias_completion_summary"]["action:ignored_repeat"], 1
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

    def test_class_materialization_shadow_decisions_are_reported(self):
        report = MODULE.build_report(
            [
                self.record(
                    "class_materialization_decision",
                    location="test.t:4:9",
                    template_name="Box",
                    source_occurrence_id=17,
                    source_use_mode=0,
                    typed_owner="declaration_type",
                    structured_arguments="type:dependent=no",
                    typed_materialization=True,
                    legacy_admitted=True,
                ),
                self.record(
                    "class_materialization_decision",
                    location="test.t:8:3",
                    template_name="Other",
                    source_occurrence_id=21,
                    source_use_mode=3,
                    typed_owner="none",
                    structured_arguments="",
                    typed_materialization=False,
                    legacy_admitted=True,
                ),
            ]
        )
        self.assertEqual(
            report["class_materialization_summary"],
            {
                "decisions": 2,
                "legacy_admitted": 2,
                "shadow_mismatch": 1,
                "typed_admitted": 1,
                "typed_owner:declaration_type": 1,
                "typed_owner:none": 1,
                "typed_rejected": 1,
            },
        )
        self.assertEqual(
            report["class_materialization_decisions"][0]["source_occurrence_id"],
            17,
        )


if __name__ == "__main__":
    unittest.main()
