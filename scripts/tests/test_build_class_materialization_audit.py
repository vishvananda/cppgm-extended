import importlib.util
import json
import tempfile
import unittest
from pathlib import Path


SCRIPT = Path(__file__).parents[1] / "build_class_materialization_audit.py"
SPEC = importlib.util.spec_from_file_location("build_class_materialization_audit", SCRIPT)
assert SPEC and SPEC.loader
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class ClassMaterializationAuditTest(unittest.TestCase):
    def test_canonical_location_uses_repository_suffix(self):
        self.assertEqual(
            MODULE.canonical_location("/tmp/work/pa24/tests/a.t:3:7"),
            "pa24/tests/a.t:3:7",
        )

    def test_reports_public_row_at_rejected_location(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            prior = root / "prior.json"
            report = root / "report.json"
            clang = root / "clang"
            clang.mkdir()
            rejected = {
                "location": "/work/pa24/tests/rejected.t:4:2",
                "template_name": "Bad",
            }
            prior.write_text(
                json.dumps(
                    {
                        "late_removed_rows": [rejected] * 55,
                        "patched_clang": {
                            "class_rows_at_removed_locations": 0,
                            "compile_failures": 0,
                            "results": {},
                        },
                    }
                ),
                encoding="utf-8",
            )
            final = [
                {
                    "kind": "class_use",
                    "location": rejected["location"],
                    "template_name": "Bad",
                }
            ]
            for index in range(5):
                location = f"/work/pa24/tests/accepted.t:{index + 1}:1"
                final.append(
                    {
                        "kind": "class_use",
                        "location": location,
                        "template_name": f"Good{index}",
                    }
                )
                (clang / f"{index}.json").write_text(
                    json.dumps(
                        {
                            "events": [
                                {
                                    "kind": "class_use",
                                    "location": MODULE.canonical_location(location),
                                    "template": f"Good{index}",
                                }
                            ]
                        }
                    ),
                    encoding="utf-8",
                )
            report.write_text(
                json.dumps(
                    {
                        "public_source_ownership": final,
                        "site_coverage": {
                            "class.class_template_reference.02": {
                                "publications": 6,
                            }
                        },
                    }
                ),
                encoding="utf-8",
            )
            audit = MODULE.build_audit(prior, report, clang)
            self.assertTrue(
                any("reached CPPGM output" in failure for failure in audit["failures"])
            )

    def test_reuses_prior_ownership_audit_after_boundary_artifact_is_lost(self):
        prior = {
            "inputs": {
                "patched_clang_binary": "/toolchain/clang++",
                "patched_clang_checkout": "/toolchain/src",
            },
            "rejected": [
                {
                    "location": "pa24/tests/rejected.t:4:2",
                    "template_name": "Bad",
                    "prior_late_decision": {
                        "location": "/work/pa24/tests/rejected.t:4:2",
                        "template_name": "Bad",
                    },
                }
            ],
            "patched_clang_rejected_results": {
                "/work/pa24/tests/rejected.t": {
                    "returncode": 0,
                    "class_rows_at_removed_locations": [],
                }
            },
        }
        self.assertEqual(
            MODULE.prior_late_removed_rows(prior),
            [
                {
                    "location": "/work/pa24/tests/rejected.t:4:2",
                    "template_name": "Bad",
                }
            ],
        )
        self.assertEqual(
            MODULE.prior_patched_clang_evidence(prior),
            {
                "binary": "/toolchain/clang++",
                "checkout": "/toolchain/src",
                "class_rows_at_removed_locations": 0,
                "compile_failures": 0,
                "results": prior["patched_clang_rejected_results"],
            },
        )

    def test_reuses_prior_accepted_source_occurrence_set(self):
        accepted = [{"location": "pa24/tests/a.t:3:7", "template_name": "A"}]
        self.assertEqual(MODULE.prior_accepted_rows({"accepted": accepted}), accepted)
        self.assertEqual(MODULE.prior_accepted_rows({}), [])


if __name__ == "__main__":
    unittest.main()
