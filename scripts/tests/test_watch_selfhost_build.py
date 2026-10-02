#!/usr/bin/env python3

import importlib.util
import os
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch


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

    def test_source_ids_apply_filter_out_assignment(self):
        source_sets = self.parse_source_sets(
            "FRONTEND_SOURCE_IDS_tool := keep drop/path drop-one drop-two\n"
            "FRONTEND_SOURCE_IDS_tool := "
            "$(filter-out drop/path drop-%,$(FRONTEND_SOURCE_IDS_tool))\n"
        )
        self.assertEqual(source_sets["tool"], ["keep"])

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

    def test_compile_source_label_preserves_dev_src_subdirectories(self):
        self.assertEqual(
            watch.compile_source_label(
                "../dev/src/frontend/semantic/declarations/semantics.cpp",
                Path("../obj/semantics.o"),
            ),
            "frontend/semantic/declarations/semantics.cpp",
        )
        self.assertEqual(
            watch.compile_source_label(
                "/tmp/repo/dev/src/frontend/semantic/expressions/semantics.cpp",
                Path("../obj/semantics.o"),
            ),
            "frontend/semantic/expressions/semantics.cpp",
        )
        self.assertEqual(
            watch.compile_source_label(
                "../dev/pptoken.cpp",
                Path("../obj/pptoken.o"),
            ),
            "pptoken.cpp",
        )

    def test_process_capture_uses_unambiguous_elapsed_seconds(self):
        original_run_capture = watch.run_capture
        try:
            watch.run_capture = lambda command: (
                "  98765   43210       7 compiler -c source.cpp\n"
            )
            processes = watch.capture_processes()
        finally:
            watch.run_capture = original_run_capture

        self.assertEqual(len(processes), 1)
        self.assertEqual(processes[0].pid, 98765)
        self.assertEqual(processes[0].ppid, 43210)
        self.assertEqual(processes[0].elapsed_seconds, 7)
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

    def test_discovers_make_started_from_pa39_working_directory(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "pa39").mkdir()
            (root / "dev").mkdir()
            (root / "pa39" / "Makefile").touch()
            (root / "dev" / "frontend_source_sets.mk").touch()
            process = watch.ProcessInfo(
                pid=123,
                ppid=1,
                elapsed_seconds=7,
                command="make compare-pptoken-inception",
                argv=["make", "compare-pptoken-inception"],
            )

            original_process_working_directory = watch.process_working_directory
            try:
                watch.process_working_directory = lambda pid: root / "pa39"
                builds = watch.discover_build_processes([process], root)
            finally:
                watch.process_working_directory = original_process_working_directory

            self.assertEqual(builds, [process])

    def test_discovers_make_with_explicit_pa39_directory(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "pa39").mkdir()
            (root / "dev").mkdir()
            (root / "pa39" / "Makefile").touch()
            (root / "dev" / "frontend_source_sets.mk").touch()
            process = watch.ProcessInfo(
                pid=456,
                ppid=1,
                elapsed_seconds=7,
                command="make -C pa39 compare-pptoken-inception",
                argv=["make", "-C", "pa39", "compare-pptoken-inception"],
            )

            original_process_working_directory = watch.process_working_directory
            try:
                watch.process_working_directory = lambda pid: root / "pa39"
                builds = watch.discover_build_processes([process], root)
            finally:
                watch.process_working_directory = original_process_working_directory

            self.assertEqual(builds, [process])

    def test_ignores_recursive_make_behind_shell_wrapper(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "pa39").mkdir()
            (root / "dev").mkdir()
            (root / "pa39" / "Makefile").touch()
            (root / "dev" / "frontend_source_sets.mk").touch()
            parent = watch.ProcessInfo(
                pid=100,
                ppid=1,
                elapsed_seconds=7,
                command="make compare-pptoken-inception",
                argv=["make", "compare-pptoken-inception"],
            )
            wrapper = watch.ProcessInfo(
                pid=101,
                ppid=100,
                elapsed_seconds=6,
                command="/bin/sh -c make ../obj/pa39/bin/inception/pptoken-inception",
                argv=["/bin/sh", "-c", "make ../obj/pa39/bin/inception/pptoken-inception"],
            )
            child = watch.ProcessInfo(
                pid=102,
                ppid=101,
                elapsed_seconds=5,
                command="make ../obj/pa39/bin/inception/pptoken-inception",
                argv=["make", "../obj/pa39/bin/inception/pptoken-inception"],
            )

            original_process_working_directory = watch.process_working_directory
            try:
                watch.process_working_directory = lambda pid: root / "pa39"
                builds = watch.discover_build_processes(
                    [parent, wrapper, child], root
                )
            finally:
                watch.process_working_directory = original_process_working_directory

            self.assertEqual(builds, [parent])

    def test_discovers_pa39_make_below_repository_make(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "pa39").mkdir()
            (root / "dev").mkdir()
            (root / "pa39" / "Makefile").touch()
            (root / "dev" / "frontend_source_sets.mk").touch()
            outer = watch.ProcessInfo(
                pid=100,
                ppid=1,
                elapsed_seconds=7,
                command="make inception",
                argv=["make", "inception"],
            )
            wrapper = watch.ProcessInfo(
                pid=101,
                ppid=100,
                elapsed_seconds=6,
                command="/bin/sh -c make -C pa39 compare-inception",
                argv=["/bin/sh", "-c", "make -C pa39 compare-inception"],
            )
            child = watch.ProcessInfo(
                pid=102,
                ppid=101,
                elapsed_seconds=5,
                command="make -C pa39 compare-inception",
                argv=["make", "-C", "pa39", "compare-inception"],
            )

            original_process_working_directory = watch.process_working_directory
            try:
                working_directories = {
                    outer.pid: root,
                    child.pid: root / "pa39",
                }
                watch.process_working_directory = working_directories.get
                builds = watch.discover_build_processes(
                    [outer, wrapper, child], root
                )
            finally:
                watch.process_working_directory = original_process_working_directory

            self.assertEqual(builds, [child])

    def test_path_output_target_maps_to_checkpoint_basename(self):
        checkpoints = ["pptoken", "posttoken"]
        scope, label = watch.target_scope(
            "../obj/pa39/bin/inception/pptoken-inception",
            checkpoints,
            {},
        )
        self.assertEqual(scope, ["pptoken"])
        self.assertEqual(label, "pptoken")

    def test_selects_pa34_selfhost_layout_and_discovers_make(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "pa34").mkdir()
            (root / "dev").mkdir()
            (root / "pa34" / "Makefile").write_text("CHECKPOINTS = cppgm++\n")
            (root / "dev" / "frontend_source_sets.mk").touch()
            process = watch.ProcessInfo(123, 1, 7, "make -C pa34 cppgm++-self",
                                        ["make", "-C", "pa34", "cppgm++-self"])
            with patch.object(watch, "process_working_directory", return_value=root / "pa34"):
                self.assertEqual(watch.discover_build_processes([process], root), [process])
            self.assertEqual(watch.inception_directory(root), root / "pa34")

    def test_pa34_without_checkpoints_does_not_replace_pa39_layout(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for name in ("pa34", "pa39"):
                (root / name).mkdir()
            (root / "pa34" / "Makefile").write_text("all: test\n")
            (root / "pa39" / "Makefile").write_text("CHECKPOINTS = cppgm++\n")
            self.assertEqual(watch.inception_directory(root), root / "pa39")

    def test_maps_private_volume_using_host_mount_root(self):
        mounts = {
            Path("/proc/self/mountinfo"): [("7:5", Path("/"), Path("/work/private/run"))],
            Path("/proc/123/mountinfo"): [("7:5", Path("/tmp"), Path("/tmp"))],
        }
        with patch.object(watch, "read_mounts", side_effect=mounts.get):
            mappings = watch.process_path_mappings(123)
        self.assertEqual(watch.mapped_path(Path("/tmp/build/object.o"), mappings),
                         Path("/work/private/run/tmp/build/object.o"))

    def test_nested_identity_mount_overrides_parent_mapping(self):
        mappings = {Path("/tmp"): Path("/private/tmp"),
                    Path("/tmp/shared"): Path("/tmp/shared")}
        self.assertEqual(watch.mapped_path(Path("/tmp/shared/object.o"), mappings),
                         Path("/tmp/shared/object.o"))

    def test_mount_parser_unescapes_paths_and_keeps_top_mount(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "mountinfo"
            path.write_text(
                "1 0 0:1 / /tmp ro - tmpfs tmpfs ro\n"
                "2 1 7:5 /private\\040tmp /tmp rw - ext4 /dev/loop5 rw\n")
            self.assertEqual(watch.read_mounts(path),
                             [("7:5", Path("/private tmp"), Path("/tmp"))])

    def test_infers_course_compiler_default_without_explicit_cxx(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "Makefile"
            path.write_text("ifeq ($(origin CXX), default)\nCXX := ../dev/cppgm++\nendif\n")
            with patch.object(watch, "INCEPTION_MAKEFILE", path):
                self.assertEqual(watch.infer_flavor("cppgm++-self", {}), "selfhost")
                self.assertEqual(watch.infer_flavor("cppgm++-self", {"CXX": "g++"}), "host")

    def test_probe_counts_one_object_and_maps_active_output_and_depfile(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory) / "repo"
            private_tmp = Path(directory) / "private-tmp"
            (root / "pa34").mkdir(parents=True)
            (root / "pa34" / "Makefile").write_text("CHECKPOINTS = cppgm++\n")
            source = root / "dev/src/semantic/example.cpp"
            source.parent.mkdir(parents=True)
            source.write_text("// source\n")
            (root / "dev/frontend_source_sets.mk").write_text(
                "FRONTEND_TEST_RUNNER_SOURCE_ID := support/testing/test_runner\n")
            compiler = root / "dev/cppgm++"
            compiler.touch()
            header = private_tmp / "generated/header.h"
            header.parent.mkdir(parents=True)
            header.touch()
            output = private_tmp / "build/probe/selfhost/shared/release/semantic/example.o"
            output.parent.mkdir(parents=True)
            output.touch()
            depfile = output.parent.parent / ".d/semantic/example.d"
            depfile.parent.mkdir(parents=True)
            depfile.write_text("example.o: ../dev/src/semantic/example.cpp /tmp/generated/header.h\n")
            for dependency in (source, compiler, header):
                os.utime(dependency, (100, 100))
            os.utime(output, (200, 200))
            parent = watch.ProcessInfo(123, 1, 7, "make probe-self-object", [
                "make", "probe-self-object", "INCEPTION_OBJ_ROOT_BASE=/tmp/build",
                "SOURCE=../dev/src/semantic/example.cpp"])
            child = watch.ProcessInfo(124, 123, 3, "cppgm++ compile", [
                "../dev/cppgm++", "-c", "-o",
                "/tmp/build/probe/selfhost/shared/release/semantic/example.o",
                "../dev/src/semantic/example.cpp"])
            original_root = watch.REPO_ROOT
            try:
                watch.configure_repo_root(root)
                with patch.object(watch, "process_path_mappings", return_value={Path("/tmp"): private_tmp, root: root}):
                    spec = watch.build_spec_from_process(parent, ["cppgm++"], {})
                self.assertEqual(spec.flavor, "selfhost")
                self.assertEqual(spec.obj_root_base, private_tmp / "build/probe")
                self.assertEqual(watch.runner_source_path(), root / "dev/src/support/testing/test_runner.cpp")
                views = []
                for processes in ([parent], [parent, child]):
                    views.append(watch.build_view(spec, parent.pid, parent.command, {}, processes,
                                                 {p.pid: p for p in processes}, watch.process_tree(processes)))
                complete, running = views
                self.assertEqual((complete.shared_done, complete.shared_total, complete.binary_total), (1, 1, 0))
                self.assertEqual(running.shared_done, 0)
                self.assertEqual(len(running.active_tasks), 1)
                self.assertEqual(running.active_tasks[0].label, "semantic/example.cpp")
                self.assertEqual(running.active_tasks[0].output_path, output)
            finally:
                watch.configure_repo_root(original_root)


if __name__ == "__main__":
    unittest.main()
