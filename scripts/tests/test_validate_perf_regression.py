import contextlib
import importlib.util
import io
from pathlib import Path
import re
from types import SimpleNamespace
import unittest


REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPT_PATH = REPO_ROOT / "scripts" / "validate_perf_regression.py"
SPEC = importlib.util.spec_from_file_location("validate_perf_regression", SCRIPT_PATH)
MODULE = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(MODULE)


class ValidatePerfRegressionTest(unittest.TestCase):
    def test_default_workload_uses_frozen_project_headers(self):
        include_dir = "benchmarks/self_compile/stable/include"
        self.assertIn("-I", MODULE.DEFAULT_COMMAND)
        include_index = MODULE.DEFAULT_COMMAND.index("-I")
        self.assertEqual(MODULE.DEFAULT_COMMAND[include_index + 1], include_dir)
        self.assertNotIn("dev/src", MODULE.DEFAULT_COMMAND)

    def test_frozen_project_include_closure_is_complete(self):
        include_dir = REPO_ROOT / "benchmarks/self_compile/stable/include"
        source = REPO_ROOT / "benchmarks/self_compile/stable/semantic_overload.cpp"
        project_files = [source, *sorted(include_dir.iterdir())]
        include_pattern = re.compile(r'^\s*#\s*include\s+"([^"]+)"')

        missing = []
        for path in project_files:
            if not path.is_file():
                continue
            for line in path.read_text().splitlines():
                match = include_pattern.match(line)
                if match and not (include_dir / match.group(1)).is_file():
                    missing.append(f"{path.name}: {match.group(1)}")

        self.assertEqual(missing, [])

    def test_workload_comparison_ignores_only_output_path(self):
        baseline = ["./dev/cppgm++", "-I", "frozen", "-c", "-o", "/tmp/a.o", "input.cpp"]
        candidate = ["./dev/cppgm++", "-I", "frozen", "-c", "-o", "/tmp/b.o", "input.cpp"]
        changed_headers = [
            "./dev/cppgm++", "-I", "dev/src", "-c", "-o", "/tmp/b.o", "input.cpp"
        ]

        self.assertEqual(
            MODULE.normalized_workload_command(baseline),
            MODULE.normalized_workload_command(candidate),
        )
        self.assertNotEqual(
            MODULE.normalized_workload_command(baseline),
            MODULE.normalized_workload_command(changed_headers),
        )

        summary = {
            key: {"median": 1}
            for key in (
                "instructions_retired",
                "maximum_resident_set_size",
                "peak_memory_footprint",
            )
        }
        args = SimpleNamespace(
            instruction_tolerance=0.01,
            rss_tolerance=0.03,
            footprint_tolerance=0.03,
        )
        with contextlib.redirect_stdout(io.StringIO()):
            failures = MODULE.compare_reports(
                {"command": baseline, "summary": summary},
                {"command": changed_headers, "summary": summary},
                args,
            )
        self.assertTrue(
            any("benchmark workload command differs" in failure for failure in failures)
        )


if __name__ == "__main__":
    unittest.main()
