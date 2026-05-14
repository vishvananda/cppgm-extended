#!/usr/bin/env python3

import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPT_PATH = REPO_ROOT / "scripts" / "bootstrap_trace_report.py"
DATA_DIR = Path(__file__).resolve().parent / "data" / "bootstrap_trace_report"


def load_module():
    spec = importlib.util.spec_from_file_location("bootstrap_trace_report", SCRIPT_PATH)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(module)
    return module


trace_report = load_module()


class BootstrapTraceReportTests(unittest.TestCase):
    def test_parse_darwin_linker_output(self):
        items = trace_report.parse_linker_undefineds((DATA_DIR / "darwin-link.txt").read_text())
        self.assertEqual([item.raw for item in items], ["__ZN3foo3barEv", "__ZN3baz3quxEi"])
        self.assertEqual(items[0].referencers, ["a.o"])
        self.assertEqual(items[1].referencers, ["b.o"])

    def test_parse_gnu_linker_output(self):
        items = trace_report.parse_linker_undefineds((DATA_DIR / "gnu-link.txt").read_text())
        self.assertEqual(len(items), 1)
        self.assertEqual(items[0].raw, "foo::bar()")
        self.assertEqual(items[0].referencers, ["a.o"])

    def test_parse_lld_linker_output(self):
        items = trace_report.parse_linker_undefineds((DATA_DIR / "lld-link.txt").read_text())
        self.assertEqual(len(items), 1)
        self.assertEqual(items[0].raw, "foo::baz(int)")
        self.assertEqual(items[0].referencers, ["c.o", "d.o"])

    def test_parse_duplicate_symbol_output(self):
        items = trace_report.parse_linker_duplicates(
            "duplicate symbol 'std::__1::foo()' in:\n"
            "    /tmp/a.o\n"
            "    /tmp/b.o\n"
            "duplicate symbol '___inline_signbitd' in:\n"
            "    /tmp/c.o\n"
        )
        self.assertEqual([item.raw for item in items],
                         ["std::__1::foo()", "___inline_signbitd"])
        self.assertEqual(items[0].referencers, ["a.o", "b.o"])
        self.assertEqual(items[1].referencers, ["c.o"])

    def test_parse_trace_line_supports_error_lines(self):
        event = trace_report.parse_trace_line(
            "ERROR: lowir exported symbol missing semantic owner @std____1____construct_at__ov13")
        self.assertIsNotNone(event)
        assert event is not None
        self.assertEqual(event.category, "error")
        self.assertEqual(event.fields["action"], "compiler-error")
        self.assertIn("construct_at__ov13", event.fields["text"])

    def test_load_frontier_requires_failed_link_stage(self):
        with self.assertRaises(SystemExit):
            trace_report.load_frontier_undefineds(DATA_DIR / "frontier-compile-failed.json")

    def test_load_frontier_link_stage(self):
        data, items = trace_report.load_frontier_undefineds(DATA_DIR / "frontier-link-failed.json")
        self.assertEqual(data["result"], "link-failed")
        self.assertEqual(len(items), 1)
        self.assertEqual(items[0].raw, "__ZN3foo3barEv")

    def test_load_frontier_link_issues_includes_duplicates(self):
        with tempfile.TemporaryDirectory(prefix="bootstrap-trace-report-test.") as temp_dir:
            cluster = Path(temp_dir) / "frontier-link-duplicate.json"
            cluster.write_text(json.dumps({
                "result": "link-failed",
                "active_frontier": "host-link",
                "stages": [
                    {
                        "stage": "link",
                        "returncode": 1,
                        "stdout": ("duplicate symbol 'std::__1::foo()' in:\n"
                                   "    /tmp/a.o\n"
                                   "    /tmp/b.o\n"
                                   "ld: 2 duplicate symbols\n"),
                    },
                ],
            }, indent=2))
            data, undefineds, duplicates = trace_report.load_frontier_link_issues(cluster)
            self.assertEqual(data["result"], "link-failed")
            self.assertEqual(undefineds, [])
            self.assertEqual(len(duplicates), 1)
            self.assertEqual(duplicates[0].raw, "std::__1::foo()")
            self.assertEqual(duplicates[0].referencers, ["a.o", "b.o"])

    def test_temporary_object_path_avoids_basename_collisions(self):
        with tempfile.TemporaryDirectory(prefix="bootstrap-trace-report-test.") as temp_dir:
            out_dir = Path(temp_dir)
            src_a = REPO_ROOT / "dev" / "src" / "qualified_name_parser.cpp"
            src_b = REPO_ROOT / "dev" / "src" / "template_angle_parser.cpp"
            # Same basename case via synthetic sibling path outside repo.
            other_dir = out_dir / "shadow"
            other_dir.mkdir()
            src_c = other_dir / "template_angle_parser.cpp"
            src_c.write_text("int x;\n")

            path_a = trace_report.temporary_object_path(REPO_ROOT, src_b, out_dir)
            path_b = trace_report.temporary_object_path(REPO_ROOT, src_c, out_dir)
            self.assertNotEqual(path_a, path_b)

    def test_json_output_mode(self):
        proc = subprocess.run(
            [
                sys.executable,
                str(SCRIPT_PATH),
                "--repo-root",
                str(REPO_ROOT),
                "--link-stdout",
                str(DATA_DIR / "gnu-link.txt"),
                "--json",
            ],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=True,
        )
        payload = json.loads(proc.stdout)
        self.assertIn("detail_path", payload)
        self.assertTrue(payload["sections"])
        self.assertEqual(payload["sections"][0]["kind"], "frontier_summary")
        self.assertEqual(payload["sections"][0]["data"]["undefined_symbol_count"], 1)

    def test_frontier_integration_suggests_cluster_probe_and_trace(self):
        with tempfile.TemporaryDirectory(prefix="bootstrap-trace-report-test.") as temp_dir:
            build_dir = Path(temp_dir) / "cluster.build"
            build_dir.mkdir()
            consumer_obj = build_dir / "013-consumer.o"
            provider_obj = build_dir / "042-provider.o"
            consumer_obj.write_text("")
            provider_obj.write_text("")

            cluster_data = {
                "build_dir": str(build_dir),
                "compiler": "./dev/cppgm++",
                "host_cxx": "/usr/bin/clang++",
                "jobs": 12,
                "stages": [
                    {
                        "stage": "compile",
                        "source": "dev/src/consumer.cpp",
                        "object": str(consumer_obj),
                    },
                    {
                        "stage": "compile",
                        "source": "dev/src/provider.cpp",
                        "object": str(provider_obj),
                    },
                ],
            }
            symbol = trace_report.SymbolRef("__ZN2ns3fooEi")
            symbol.demangled = "ns::foo(int)"
            symbol.referencers = ["013-consumer.o"]

            original = trace_report.find_repo_source_candidates
            trace_report.find_repo_source_candidates = (
                lambda repo_root, demangled, limit=12: ["dev/src/provider.cpp"])
            try:
                data = trace_report.frontier_integration_data(
                    REPO_ROOT,
                    Path("/tmp/frontier-link.json"),
                    cluster_data,
                    [symbol],
                    [],
                    [],
                    3,
                )
            finally:
                trace_report.find_repo_source_candidates = original

            best = data["best_candidate"]
            self.assertEqual(best["suggested_preset"], "linkage")
            self.assertEqual(best["suggested_trace_source"], "dev/src/provider.cpp")
            self.assertIn("--cluster-probe", best["provider_diff_command"])
            self.assertIn("--trace-source", best["trace_command"])
            self.assertIn("dev/src/provider.cpp", best["trace_command"])

    def test_write_prefix_persists_outputs(self):
        with tempfile.TemporaryDirectory(prefix="bootstrap-trace-report-test.") as temp_dir:
            prefix = Path(temp_dir) / "analysis"
            proc = subprocess.run(
                [
                    sys.executable,
                    str(SCRIPT_PATH),
                    "--repo-root",
                    str(REPO_ROOT),
                    "--link-stdout",
                    str(DATA_DIR / "gnu-link.txt"),
                    "--write-prefix",
                    str(prefix),
                    "--json",
                ],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                check=True,
            )
            payload = json.loads(proc.stdout)
            self.assertEqual(payload["text_path"], str(prefix) + ".txt")
            self.assertEqual(payload["json_path"], str(prefix) + ".json")
            self.assertEqual(payload["detail_path"], str(prefix) + ".details.txt")
            self.assertTrue((Path(str(prefix) + ".txt")).exists())
            self.assertTrue((Path(str(prefix) + ".json")).exists())
            self.assertTrue((Path(str(prefix) + ".details.txt")).exists())
            persisted = json.loads(Path(str(prefix) + ".json").read_text())
            self.assertEqual(
                persisted["meta"]["request"]["link_stdout"],
                str((DATA_DIR / "gnu-link.txt").resolve()),
            )
            self.assertTrue(persisted["meta"]["inputs"]["link_stdout_sha256"])
            self.assertEqual(
                Path(persisted["meta"]["tool"]["path"]).resolve(),
                SCRIPT_PATH.resolve(),
            )
            self.assertTrue(persisted["meta"]["tool"]["sha256"])

    def test_raw_trace_json_mode(self):
        with tempfile.TemporaryDirectory(prefix="bootstrap-trace-report-test.") as temp_dir:
            trace_path = Path(temp_dir) / "raw.stderr.txt"
            trace_path.write_text(
                "[output.export] action=insert symbol=@std____1____construct_at__ov13 "
                "reason=callee owner=std::__1::construct_at linkage=weak\n"
                "[output.audit] action=lowir-missing-semantic-owner "
                "symbol=@std____1____construct_at__ov13 detail=pre-lowir-owner=missing\n"
                "ERROR: lowir exported symbol missing semantic owner "
                "@std____1____construct_at__ov13\n"
            )
            proc = subprocess.run(
                [
                    sys.executable,
                    str(SCRIPT_PATH),
                    "--repo-root",
                    str(REPO_ROOT),
                    "--trace-stderr",
                    str(trace_path),
                    "--focus",
                    "construct_at",
                    "--json",
                ],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                check=True,
            )
            payload = json.loads(proc.stdout)
            self.assertTrue(payload["sections"])
            self.assertEqual(payload["sections"][0]["kind"], "focused_trace")
            data = payload["sections"][0]["data"]
            self.assertEqual(data["source"], str(trace_path.resolve()))
            self.assertEqual(data["preset"], "raw-trace")
            self.assertIn("no semantic output owner", data["likely_issue"])
            self.assertIn("semantic owner", data["compiler_errors"][0])

    def test_raw_trace_write_prefix_persists_outputs(self):
        with tempfile.TemporaryDirectory(prefix="bootstrap-trace-report-test.") as temp_dir:
            prefix = Path(temp_dir) / "raw-analysis"
            trace_path = Path(temp_dir) / "raw.stderr.txt"
            trace_path.write_text(
                "[template.resolve] action=deduction-type route=semantic-only result=ok type=int\n"
                "ERROR: synthetic compiler failure\n"
            )
            proc = subprocess.run(
                [
                    sys.executable,
                    str(SCRIPT_PATH),
                    "--repo-root",
                    str(REPO_ROOT),
                    "--trace-stderr",
                    str(trace_path),
                    "--write-prefix",
                    str(prefix),
                    "--json",
                ],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                check=True,
            )
            payload = json.loads(proc.stdout)
            self.assertEqual(payload["text_path"], str(prefix) + ".txt")
            self.assertEqual(payload["json_path"], str(prefix) + ".json")
            self.assertEqual(payload["detail_path"], str(prefix) + ".details.txt")
            persisted = json.loads(Path(str(prefix) + ".json").read_text())
            self.assertEqual(
                persisted["meta"]["request"]["trace_stderr"],
                [str(trace_path.resolve())],
            )
            self.assertEqual(
                persisted["meta"]["inputs"]["trace_stderr_sha256"][str(trace_path.resolve())],
                trace_report.file_sha256(trace_path.resolve()),
            )

    def test_trace_summary_surfaces_focus_lifecycle_and_error(self):
        events = [
            trace_report.TraceEvent(
                "symbol.linkage",
                "",
                ("action=function entity=std::__1::construct_at internal="
                 "@std____1____construct_at__ov13 object=__ZNSt3__112construct_atEv linkage=weak"),
            ),
            trace_report.TraceEvent(
                "output.export",
                "",
                ("action=insert symbol=@std____1____construct_at__ov13 reason=callee "
                 "owner=std::__1::construct_at linkage=weak"),
            ),
            trace_report.TraceEvent(
                "output.export",
                "",
                ("action=missing-closure symbol=@std____1____construct_at__ov13 "
                 "reason=exported-symbol known-function=no known-global=no "
                 "external-function=no external-object=no referenced-function=no "
                 "referenced-global=no runtime-reserved=no backend-passthrough=no"),
            ),
            trace_report.TraceEvent(
                "output.audit",
                "",
                ("action=lowir-missing-semantic-owner "
                 "symbol=@std____1____construct_at__ov13 "
                 "detail=pre-lowir-owner=missing"),
            ),
            trace_report.TraceEvent(
                "error",
                "",
                "action=compiler-error text=lowir exported symbol missing semantic owner "
                "@std____1____construct_at__ov13",
            ),
        ]
        data = trace_report.trace_summary_data(
            "dev/src/semantic_lookup.cpp",
            "full-link-root-cause",
            events,
            ["construct_at"],
        )
        self.assertTrue(data["focus_lifecycle"])
        self.assertIn("construct_at", data["focus_lifecycle"][0]["entity"])
        self.assertTrue(data["audit_trace"])
        self.assertIn("pre-lowir-owner=missing", data["audit_trace"][0])
        self.assertIn("semantic owner", data["compiler_errors"][0])
        self.assertIn("no semantic output owner", data["likely_issue"])

    def test_trace_summary_surfaces_required_definition_audit_skip(self):
        events = [
            trace_report.TraceEvent(
                "output.require",
                "",
                ("action=require-definition entity=detail::hidden_construct_at "
                 "symbol=@detail__hidden_construct_at reason=direct-call"),
            ),
            trace_report.TraceEvent(
                "output.audit",
                "",
                ("action=skip-required-definition-validation "
                 "function=detail::hidden_construct_at "
                 "symbol=@detail__hidden_construct_at "
                 "reason=template instantiation still output-dependent "
                 "detail=output-required=yes has-definition=yes "
                 "instantiation-ready=no placeholder-origin=value Args__pack1"),
            ),
        ]
        data = trace_report.trace_summary_data(
            "sample.cpp",
            "output-lifecycle",
            events,
            ["hidden_construct_at"],
        )
        self.assertTrue(data["audit_trace"])
        self.assertIn("skip-required-definition-validation", data["audit_trace"][0])
        self.assertIn("template instantiation still output-dependent", data["likely_issue"])

    def test_duplicate_link_stdout_summary_reports_duplicate_count(self):
        with tempfile.TemporaryDirectory(prefix="bootstrap-trace-report-test.") as temp_dir:
            link_stdout = Path(temp_dir) / "duplicate-link.txt"
            link_stdout.write_text(
                "duplicate symbol 'std::__1::foo()' in:\n"
                "    /tmp/a.o\n"
                "    /tmp/b.o\n"
                "ld: 2 duplicate symbols\n"
            )
            proc = subprocess.run(
                [
                    sys.executable,
                    str(SCRIPT_PATH),
                    "--repo-root",
                    str(REPO_ROOT),
                    "--link-stdout",
                    str(link_stdout),
                ],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                check=True,
            )
            self.assertIn("duplicate_symbols: 1", proc.stdout)
            self.assertIn("Top Duplicate Symbols (1)", proc.stdout)

    def test_cluster_probe_skips_diff_when_provider_missing(self):
        with tempfile.TemporaryDirectory(prefix="bootstrap-trace-report-test.") as temp_dir:
            temp_root = Path(temp_dir)
            build_dir = temp_root / "cluster.build"
            build_dir.mkdir()
            consumer_obj = build_dir / "013-consumer.o"
            consumer_obj.write_text("")

            cluster = temp_root / "cluster.json"
            cluster.write_text(json.dumps({
                "result": "link-failed",
                "active_frontier": "host-link",
                "build_dir": str(build_dir),
                "stages": [
                    {
                        "stage": "compile",
                        "source": "dev/src/consumer.cpp",
                        "object": str(consumer_obj),
                    },
                    {
                        "stage": "link",
                        "returncode": 1,
                        "stdout": ('"__ZN3foo3barEv", referenced from:\n'
                                   '    _use in 013-consumer.o\n'),
                    },
                ],
            }, indent=2))

            proc = subprocess.run(
                [
                    sys.executable,
                    str(SCRIPT_PATH),
                    "--repo-root",
                    str(REPO_ROOT),
                    "--cluster",
                    str(cluster),
                    "--cluster-probe",
                    "--focus",
                    "foo::bar",
                    "--json",
                ],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                check=True,
            )
            payload = json.loads(proc.stdout)
            probe_sections = [section for section in payload["sections"]
                              if section["kind"] == "cluster_probe"]
            self.assertEqual(len(probe_sections), 1)
            notes = probe_sections[0]["data"]["notes"]
            self.assertTrue(any("skipping provider diff" in note for note in notes))

    def test_demangle_symbols_uses_stdin(self):
        with tempfile.TemporaryDirectory(prefix="bootstrap-trace-report-test.") as temp_dir:
            cxxfilt = Path(temp_dir) / "fake-cxxfilt.sh"
            cxxfilt.write_text("#!/bin/sh\ncat\n")
            cxxfilt.chmod(0o755)

            result = trace_report.demangle_symbols(
                ["__ZN3foo3barEv", "__ZN3baz3quxEi"],
                str(cxxfilt),
            )

            self.assertEqual(result["__ZN3foo3barEv"], "__ZN3foo3barEv")
            self.assertEqual(result["__ZN3baz3quxEi"], "__ZN3baz3quxEi")

    def test_symbol_pipeline_detects_missing_semantic_declaration(self):
        data = trace_report.symbol_pipeline_data(
            "sample.cpp",
            (DATA_DIR / "pipeline-semantics.txt").read_text(),
            (DATA_DIR / "pipeline-lowir.txt").read_text(),
            ["std::__1::basic_ostream<char, std::__1::char_traits<char>>::put"],
        )

        self.assertEqual(len(data["semantic_hits"]["declaration"]), 0)
        self.assertEqual(len(data["semantic_hits"]["use"]), 1)
        self.assertEqual(len(data["lowir_hits"]["definition"]), 0)
        self.assertEqual(len(data["lowir_hits"]["reference"]), 1)
        self.assertIn("no semantic declaration/definition node was emitted",
                      data["likely_issue"])

    def test_render_symbol_pipeline_mentions_lowir_reference(self):
        data = trace_report.symbol_pipeline_data(
            "sample.cpp",
            (DATA_DIR / "pipeline-semantics.txt").read_text(),
            (DATA_DIR / "pipeline-lowir.txt").read_text(),
            ["flush"],
        )

        rendered = trace_report.render_symbol_pipeline(data)
        self.assertIn("Semantic Output", rendered)
        self.assertIn("LowIR", rendered)
        self.assertIn("ref L", rendered)
        self.assertIn("Likely issue", rendered)


if __name__ == "__main__":
    unittest.main()
