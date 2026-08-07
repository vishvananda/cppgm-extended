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
        route = "alias.resolved_instantiation"
        producer = "alias.callsemantic.02"
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
                    producer="alias.callsemantic.02",
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


if __name__ == "__main__":
    unittest.main()
