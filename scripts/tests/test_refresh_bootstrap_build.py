#!/usr/bin/env python3

import importlib.util
from pathlib import Path
import tempfile
import unittest


REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPT_PATH = REPO_ROOT / "scripts" / "refresh_bootstrap_build.py"


def load_module():
    spec = importlib.util.spec_from_file_location("refresh_bootstrap_build", SCRIPT_PATH)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(module)
    return module


refresh_bootstrap = load_module()


class RefreshBootstrapBuildTests(unittest.TestCase):
    def test_parse_dependency_text_handles_dev_make_relative_paths(self):
        with tempfile.TemporaryDirectory(prefix="refresh-bootstrap-test.") as temp_dir:
            repo_root = Path(temp_dir)
            text = (
                "../obj/release/callsemantic.o: ../dev/src/callsemantic.cpp \\\n"
                "  ../dev/src/callsemantic.h ../dev/src/callsem_output.h\n"
            )
            deps = refresh_bootstrap.parse_dependency_text(
                text,
                repo_root,
                repo_root / "dev",
            )
            self.assertEqual(
                deps,
                {
                    "dev/src/callsemantic.cpp",
                    "dev/src/callsemantic.h",
                    "dev/src/callsem_output.h",
                },
            )

    def test_affected_sources_include_header_reverse_closure(self):
        source_paths = [
            "dev/src/callsemantic.cpp",
            "dev/src/semantic_expression.cpp",
            "dev/cppgm++.cpp",
        ]
        reverse = {
            "dev/src/callsem_output.h": {
                "dev/src/callsemantic.cpp",
                "dev/src/semantic_expression.cpp",
            },
        }
        affected, no_effect, fanout = refresh_bootstrap.affected_sources(
            REPO_ROOT,
            source_paths,
            [],
            ["dev/src/callsem_output.h"],
            reverse,
        )
        self.assertEqual(
            affected,
            [
                "dev/src/callsemantic.cpp",
                "dev/src/semantic_expression.cpp",
            ],
        )
        self.assertEqual(no_effect, [])
        self.assertEqual(fanout["dev/src/callsem_output.h"], 2)

    def test_affected_sources_keep_explicit_source_even_without_dep_hit(self):
        source_paths = [
            "dev/src/callsemantic.cpp",
            "dev/src/semantic_expression.cpp",
            "dev/cppgm++.cpp",
        ]
        affected, no_effect, fanout = refresh_bootstrap.affected_sources(
            REPO_ROOT,
            source_paths,
            ["dev/cppgm++.cpp"],
            ["docs/notes.md"],
            {},
        )
        self.assertEqual(affected, ["dev/cppgm++.cpp"])
        self.assertEqual(no_effect, ["docs/notes.md"])
        self.assertEqual(fanout["docs/notes.md"], 0)

    def test_recommended_followup_escalates_for_hot_paths_and_high_fanout(self):
        followup, reasons = refresh_bootstrap.recommended_followup(
            ["dev/src/callsemantic.cpp", "dev/src/shared.h"],
            {
                "dev/src/callsemantic.cpp": 1,
                "dev/src/shared.h": refresh_bootstrap.HIGH_FANOUT_LIMIT,
            },
        )
        self.assertEqual(followup, "full-frontier")
        self.assertIn("hot-path:dev/src/callsemantic.cpp", reasons)
        self.assertIn("high-fanout:dev/src/shared.h", reasons)


if __name__ == "__main__":
    unittest.main()
