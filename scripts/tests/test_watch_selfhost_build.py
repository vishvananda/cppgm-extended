#!/usr/bin/env python3

import importlib.util
from pathlib import Path
import sys
import tempfile
import unittest


REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPT_PATH = REPO_ROOT / "scripts" / "watch_selfhost_build.py"


def load_module():
    spec = importlib.util.spec_from_file_location("watch_selfhost_build", SCRIPT_PATH)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


watch = load_module()


class WatchSelfhostBuildTests(unittest.TestCase):
    def parse_source_sets(self, text):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "frontend_source_sets.mk"
            path.write_text(text)
            return watch.parse_frontend_source_sets(path)

    def test_parses_legacy_basename_lists_and_references(self):
        source_sets = self.parse_source_sets(
            "FRONTEND_OBJ_BASENAMES_base := lexer lexical\n"
            "FRONTEND_OBJ_BASENAMES_tool := "
            "$(FRONTEND_OBJ_BASENAMES_base) parser\n"
        )
        self.assertEqual(source_sets["tool"], ["lexer", "lexical", "parser"])

    def test_source_ids_override_legacy_list_for_same_target(self):
        source_sets = self.parse_source_sets(
            "FRONTEND_OBJ_BASENAMES_tool := legacy\n"
            "FRONTEND_SOURCE_IDS_common := frontend/tokens/lexer\n"
            "FRONTEND_SOURCE_IDS_tool := "
            "$(FRONTEND_SOURCE_IDS_common) frontend/syntax/parser\n"
        )
        self.assertEqual(
            source_sets["tool"],
            ["frontend/tokens/lexer", "frontend/syntax/parser"],
        )

    def test_shared_object_path_preserves_source_id(self):
        object_root = Path("/tmp/obj/selfhost")
        self.assertEqual(
            watch.shared_object_path(
                object_root, "frontend/semantic/model/types"
            ),
            object_root
            / "shared"
            / "release"
            / "frontend"
            / "semantic"
            / "model"
            / "types.o",
        )
        self.assertEqual(
            watch.shared_depfile_path(
                object_root, "frontend/semantic/model/types"
            ),
            object_root
            / "shared"
            / "release"
            / ".d"
            / "frontend"
            / "semantic"
            / "model"
            / "types.d",
        )

    def test_process_capture_parses_portable_elapsed_time(self):
        original_run_capture = watch.run_capture
        captured_command = []
        try:
            def fake_run_capture(command):
                captured_command.extend(command)
                return (
                    "  98765   43210 1-02:03:04 compiler -c source.cpp\n"
                )

            watch.run_capture = fake_run_capture
            processes = watch.capture_processes()
        finally:
            watch.run_capture = original_run_capture

        self.assertEqual(
            captured_command,
            ["ps", "-axo", "pid=,ppid=,etime=,command="],
        )
        self.assertEqual(len(processes), 1)
        self.assertEqual(processes[0].pid, 98765)
        self.assertEqual(processes[0].ppid, 43210)
        self.assertEqual(processes[0].elapsed_seconds, 93784)
        self.assertEqual(processes[0].argv, ["compiler", "-c", "source.cpp"])

    def test_repository_root_is_inferred_from_pa39_process_cwd(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "pa39").mkdir()
            (root / "dev").mkdir()
            (root / "pa39" / "Makefile").touch()
            (root / "dev" / "frontend_source_sets.mk").touch()

            self.assertEqual(
                watch.repository_root_from_working_directory(root / "pa39"),
                root.resolve(),
            )

    def test_root_inception_make_does_not_hide_pa39_build(self):
        processes = [
            watch.ProcessInfo(
                pid=100,
                ppid=1,
                elapsed_seconds=12,
                command="make inception",
                argv=["make", "inception"],
            ),
            watch.ProcessInfo(
                pid=101,
                ppid=100,
                elapsed_seconds=10,
                command="make -C pa39 compare-cppgm++-inception",
                argv=["make", "-C", "pa39", "compare-cppgm++-inception"],
            ),
            watch.ProcessInfo(
                pid=102,
                ppid=101,
                elapsed_seconds=8,
                command="make -C pa39 nested",
                argv=["make", "-C", "pa39", "nested"],
            ),
        ]

        roots = watch.discover_build_processes(processes)

        self.assertEqual([process.pid for process in roots], [101])


if __name__ == "__main__":
    unittest.main()
