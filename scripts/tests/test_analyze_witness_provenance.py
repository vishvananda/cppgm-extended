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


if __name__ == "__main__":
    unittest.main()
