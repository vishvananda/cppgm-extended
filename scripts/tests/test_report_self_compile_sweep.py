#!/usr/bin/env python3

import importlib.util
from pathlib import Path
import unittest


REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPT_PATH = REPO_ROOT / "scripts" / "report_self_compile_sweep.py"


def load_module():
    spec = importlib.util.spec_from_file_location("report_self_compile_sweep", SCRIPT_PATH)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(module)
    return module


report_sweep = load_module()


class ReportSelfCompileSweepTests(unittest.TestCase):
    def test_default_inception_source_list_includes_runner(self):
        files = report_sweep.source_list(
            REPO_ROOT,
            "dev/cppgm++.cpp",
            "pa39-selfhost",
            True,
        )
        self.assertIn(REPO_ROOT / "dev" / "src" / "test_runner.cpp", files)
        self.assertIn(REPO_ROOT / "dev" / "cppgm++.cpp", files)

    def test_inception_shared_object_path_matches_ladder_layout(self):
        obj = report_sweep.object_path(
            REPO_ROOT,
            REPO_ROOT / "obj" / "pa39" / "selfhost",
            0,
            REPO_ROOT / "dev" / "src" / "semantic_output.cpp",
            "pa39-selfhost",
            True,
        )
        self.assertEqual(
            obj,
            REPO_ROOT / "obj" / "pa39" / "selfhost" / "shared" / "release" / "semantic_output.o",
        )

    def test_inception_entry_object_path_matches_ladder_layout(self):
        obj = report_sweep.object_path(
            REPO_ROOT,
            REPO_ROOT / "obj" / "pa39" / "selfhost",
            0,
            REPO_ROOT / "dev" / "cppgm++.cpp",
            "pa39-selfhost",
            True,
        )
        self.assertEqual(
            obj,
            REPO_ROOT / "obj" / "pa39" / "selfhost" / "cppgm++" / "release" / "cppgm++-runner.o",
        )

    def test_inception_runner_object_path_matches_ladder_layout(self):
        obj = report_sweep.object_path(
            REPO_ROOT,
            REPO_ROOT / "obj" / "pa39" / "selfhost",
            0,
            REPO_ROOT / "dev" / "src" / "test_runner.cpp",
            "pa39-selfhost",
            True,
        )
        self.assertEqual(
            obj,
            REPO_ROOT / "obj" / "pa39" / "selfhost" / "shared" / "release" / "test_runner-enabled.o",
        )

    def test_inception_entry_compile_command_uses_runner_define_and_depfile(self):
        obj = REPO_ROOT / "obj" / "pa39" / "selfhost" / "cppgm++" / "release" / "cppgm++-runner.o"
        cmd = report_sweep.compile_command(
            REPO_ROOT,
            "./dev/cppgm++",
            REPO_ROOT / "dev" / "cppgm++.cpp",
            obj,
            "pa39-selfhost",
            True,
            "",
        )
        self.assertIn("-Dmain=test_runner_real_main", cmd)
        self.assertIn("-MMD", cmd)
        self.assertIn("-MF", cmd)
        self.assertEqual(cmd[-2:], ["-c", "dev/cppgm++.cpp"])

    def test_inception_runner_compile_command_uses_shared_runner_define(self):
        obj = REPO_ROOT / "obj" / "pa39" / "selfhost" / "shared" / "release" / "test_runner-enabled.o"
        cmd = report_sweep.compile_command(
            REPO_ROOT,
            "./dev/cppgm++",
            REPO_ROOT / "dev" / "src" / "test_runner.cpp",
            obj,
            "pa39-selfhost",
            True,
            "",
        )
        self.assertIn("-DTEST_RUNNER_ENABLE", cmd)
        self.assertEqual(cmd[-2:], ["-c", "dev/src/test_runner.cpp"])


if __name__ == "__main__":
    unittest.main()
