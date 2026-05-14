#!/usr/bin/env python3

import importlib.util
import json
from pathlib import Path
import tempfile
import unittest


REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPT_PATH = REPO_ROOT / "scripts" / "report_bootstrap_frontier.py"
TRACE_TOOL_PATH = REPO_ROOT / "scripts" / "bootstrap_trace_report.py"


def load_module():
    spec = importlib.util.spec_from_file_location("report_bootstrap_frontier", SCRIPT_PATH)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(module)
    return module


report_frontier = load_module()


class ReportBootstrapFrontierTests(unittest.TestCase):
    def test_pa36_source_list_includes_runner_when_requested(self):
        files = report_frontier.layout_source_list(
            REPO_ROOT,
            "dev/cppgm++.cpp",
            "pa37-selfhost",
            True,
        )
        self.assertIn(REPO_ROOT / "dev" / "src" / "test_runner.cpp", files)
        self.assertIn(REPO_ROOT / "dev" / "cppgm++.cpp", files)

    def test_pa36_link_object_path_matches_ladder_layout(self):
        obj = report_frontier.layout_object_path(
            REPO_ROOT,
            REPO_ROOT / "obj" / "pa37" / "selfhost",
            0,
            REPO_ROOT / "dev" / "src" / "machine_object.cpp",
            "pa37-selfhost",
            True,
        )
        self.assertEqual(
            obj,
            REPO_ROOT / "obj" / "pa37" / "selfhost" / "shared" / "release" / "machine_object.o",
        )

    def test_signal_name_for_negative_returncode(self):
        self.assertEqual(report_frontier.signal_name_for_returncode(-11), "SIGSEGV")
        self.assertEqual(report_frontier.signal_name_for_returncode(0), "")

    def test_rotate_output_family_moves_current_to_previous(self):
        with tempfile.TemporaryDirectory(prefix="report-bootstrap-frontier-test.") as temp_dir:
            temp_root = Path(temp_dir)
            current_prefix = temp_root / "frontier.trace"
            previous_prefix = temp_root / "frontier.trace.prev"
            text_path = Path(str(current_prefix) + ".txt")
            detail_path = Path(str(current_prefix) + ".details.txt")
            json_path = Path(str(current_prefix) + ".json")
            text_path.write_text("current text\n")
            detail_path.write_text("current detail\n")
            json_path.write_text("current json\n")

            moved = report_frontier.rotate_output_family(
                str(current_prefix),
                str(previous_prefix),
                [".txt", ".details.txt", ".json"],
            )

            self.assertFalse(text_path.exists())
            self.assertFalse(detail_path.exists())
            self.assertFalse(json_path.exists())
            self.assertEqual((temp_root / "frontier.trace.prev.txt").read_text(), "current text\n")
            self.assertEqual((temp_root / "frontier.trace.prev.details.txt").read_text(),
                             "current detail\n")
            self.assertEqual((temp_root / "frontier.trace.prev.json").read_text(),
                             "current json\n")
            self.assertEqual(set(moved.keys()), {".txt", ".details.txt", ".json"})

    def test_load_cached_trace_analysis_accepts_matching_sidecar(self):
        with tempfile.TemporaryDirectory(prefix="report-bootstrap-frontier-test.") as temp_dir:
            temp_root = Path(temp_dir)
            cluster_json = temp_root / "frontier.json"
            cluster_json.write_text(json.dumps({"result": "link-failed"}) + "\n")

            prefix = temp_root / "frontier.trace"
            text_path = Path(str(prefix) + ".txt")
            detail_path = Path(str(prefix) + ".details.txt")
            json_path = Path(str(prefix) + ".json")
            text_path.write_text("summary\n")
            detail_path.write_text("details\n")
            json_path.write_text(json.dumps({
                "meta": {
                    "request": {
                        "cluster": str(cluster_json.resolve()),
                    },
                    "inputs": {
                        "cluster_sha256": report_frontier.file_sha256(cluster_json),
                    },
                    "tool": {
                        "path": str(TRACE_TOOL_PATH.resolve()),
                        "sha256": report_frontier.file_sha256(TRACE_TOOL_PATH),
                    },
                },
            }) + "\n")

            cached = report_frontier.load_cached_trace_analysis(
                str(prefix),
                cluster_json,
                TRACE_TOOL_PATH,
            )
            self.assertEqual(cached["text"], str(text_path))
            self.assertEqual(cached["detail"], str(detail_path))
            self.assertEqual(cached["json"], str(json_path))

    def test_load_cached_trace_analysis_rejects_stale_cluster_digest(self):
        with tempfile.TemporaryDirectory(prefix="report-bootstrap-frontier-test.") as temp_dir:
            temp_root = Path(temp_dir)
            cluster_json = temp_root / "frontier.json"
            cluster_json.write_text(json.dumps({"result": "link-failed"}) + "\n")

            prefix = temp_root / "frontier.trace"
            Path(str(prefix) + ".txt").write_text("summary\n")
            Path(str(prefix) + ".details.txt").write_text("details\n")
            Path(str(prefix) + ".json").write_text(json.dumps({
                "meta": {
                    "request": {
                        "cluster": str(cluster_json.resolve()),
                    },
                    "inputs": {
                        "cluster_sha256": "stale",
                    },
                    "tool": {
                        "path": str(TRACE_TOOL_PATH.resolve()),
                        "sha256": report_frontier.file_sha256(TRACE_TOOL_PATH),
                    },
                },
            }) + "\n")

            cached = report_frontier.load_cached_trace_analysis(
                str(prefix),
                cluster_json,
                TRACE_TOOL_PATH,
            )
            self.assertEqual(cached, {})

    def test_load_trace_frontier_snapshot_extracts_duplicate_frontier(self):
        with tempfile.TemporaryDirectory(prefix="report-bootstrap-frontier-test.") as temp_dir:
            temp_root = Path(temp_dir)
            trace_json = temp_root / "frontier.trace.json"
            trace_json.write_text(json.dumps({
                "sections": [
                    {
                        "kind": "frontier_summary",
                        "data": {
                            "undefined_symbol_count": 0,
                            "duplicate_symbol_count": 12,
                            "primary_issue_kind": "duplicate",
                            "family_counts": {
                                "hosted-libc++/ABI": 10,
                                "compiler/provider-or-signature-drift": 2,
                            },
                            "symbols": [
                                {
                                    "demangled": "std::__1::allocator_arg",
                                    "family": "hosted-libc++/ABI",
                                },
                            ],
                        },
                    },
                    {
                        "kind": "frontier_integration",
                        "data": {
                            "issue_kind": "duplicate",
                            "candidate_count": 12,
                            "candidates": [
                                {
                                    "demangled": "std::__1::allocator_arg",
                                    "family": "hosted-libc++/ABI",
                                },
                            ],
                            "best_candidate": {
                                "demangled": "std::__1::allocator_arg",
                                "family": "hosted-libc++/ABI",
                            },
                        },
                    },
                ],
            }) + "\n")

            snapshot = report_frontier.load_trace_frontier_snapshot(str(trace_json))
            self.assertEqual(snapshot["issue_kind"], "duplicate")
            self.assertEqual(snapshot["undefined_symbols"], 0)
            self.assertEqual(snapshot["duplicate_symbols"], 12)
            self.assertEqual(snapshot["candidate_count"], 12)
            self.assertEqual(snapshot["best_candidate"]["demangled"],
                             "std::__1::allocator_arg")
            self.assertEqual(snapshot["symbols"][0]["demangled"],
                             "std::__1::allocator_arg")

    def test_collect_crash_artifacts_captures_exact_and_noargs_reports(self):
        report = {
            "stages": [
                {
                    "stage": "self-compile-smoke",
                    "returncode": -11,
                    "cmd": ["/tmp/cppgm++-self", "-o", "/tmp/bootstrap-ret0", "/tmp/bootstrap-ret0.cpp"],
                },
            ],
        }
        debugger = {"path": "/usr/bin/lldb", "kind": "lldb"}

        original_run = report_frontier.run
        original_capture = report_frontier.capture_debugger_backtrace
        captured_cmds = []

        def fake_run(cmd, cwd, env):
            return {
                "cmd": list(cmd),
                "returncode": -11,
                "stdout": "",
                "duration_sec": 0.01,
            }

        def fake_capture(debugger_info, cmd, cwd, env, output_path):
            captured_cmds.append(list(cmd))
            return {
                "path": str(output_path),
                "debugger": debugger_info["path"],
                "returncode": 0,
                "timed_out": False,
                "top_frames": ["frame #0: fake"],
            }

        report_frontier.run = fake_run
        report_frontier.capture_debugger_backtrace = fake_capture
        try:
            with tempfile.TemporaryDirectory(prefix="report-bootstrap-frontier-test.") as temp_dir:
                crash = report_frontier.collect_crash_artifacts(
                    report,
                    str(Path(temp_dir) / "frontier"),
                    Path(temp_dir),
                    {},
                    debugger,
                )
        finally:
            report_frontier.run = original_run
            report_frontier.capture_debugger_backtrace = original_capture

        self.assertEqual(crash["stage"], "self-compile-smoke")
        self.assertEqual(crash["signal_name"], "SIGSEGV")
        self.assertEqual(captured_cmds[0],
                         ["/tmp/cppgm++-self", "-o", "/tmp/bootstrap-ret0", "/tmp/bootstrap-ret0.cpp"])
        self.assertEqual(captured_cmds[1], ["/tmp/cppgm++-self"])
        self.assertEqual(crash["self_binary_noargs_probe"]["signal_name"], "SIGSEGV")
        self.assertTrue(
            crash["exact_command_backtrace"]["path"].endswith(".self-compile-smoke.crash.txt"))
        self.assertTrue(
            crash["self_binary_noargs_backtrace"]["path"].endswith(".self-binary-noargs.crash.txt"))

    def test_active_failure_stage_prefers_active_frontier(self):
        report = {
            "active_frontier": "dev/src/late.cpp",
            "stages": [
                {
                    "stage": "compile",
                    "source": "dev/src/early.cpp",
                    "returncode": 1,
                },
                {
                    "stage": "compile",
                    "source": "dev/src/late.cpp",
                    "returncode": 1,
                },
            ],
        }
        stage = report_frontier.active_failure_stage(report)
        self.assertEqual(stage["source"], "dev/src/late.cpp")

    def test_collect_failure_output_artifacts_writes_full_sidecar(self):
        report = {
            "active_frontier": "host-link",
            "stages": [
                {
                    "stage": "link",
                    "returncode": 1,
                    "duration_sec": 0.25,
                    "cmd": ["/usr/bin/clang++", "-o", "cppgm++-self"],
                    "stdout": "undefined reference to foo\nmore detail\n",
                },
            ],
        }
        with tempfile.TemporaryDirectory(prefix="report-bootstrap-frontier-test.") as temp_dir:
            prefix = str(Path(temp_dir) / "frontier")
            artifact = report_frontier.collect_failure_output_artifacts(report, prefix)
            self.assertEqual(artifact["stage"], "link")
            sidecar = Path(artifact["path"])
            self.assertTrue(sidecar.exists())
            text = sidecar.read_text()
            self.assertIn("stage: link", text)
            self.assertIn("returncode: 1", text)
            self.assertIn("undefined reference to foo", text)


if __name__ == "__main__":
    unittest.main()
