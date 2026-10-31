#!/usr/bin/env python3
"""Concurrent makes keep each producer's output paired with its comparer."""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile
import textwrap
import unittest

ROOT = Path(__file__).resolve().parents[2]


class ReportIsolationTests(unittest.TestCase):
    def environment(self):
        env = dict(os.environ, CPPGM_BATCH_TESTS="0", CPPGM_TEST_RUNNER="0", CPPGM_TEST_JOBS="1")
        for key in ("MAKEFLAGS", "MFLAGS", "MAKELEVEL", "MAKEOVERRIDES", "CPPGM_TEST_RUN_ID",
                    "CPPGM_TEST_COUNTS_FILE", "CPPGM_TEST_FAILURE_MARKER", "KEEP_GOING"):
            env.pop(key, None)
        return env

    def test_overlapping_make_results_and_recursive_inheritance(self):
        for isolated in (True, False):
            with self.subTest(isolated=isolated):
                self.overlapping_make_results(isolated)

    def overlapping_make_results(self, isolated):
        with tempfile.TemporaryDirectory(prefix="report-isolation.") as directory:
            root = Path(directory)
            scripts = root / "scripts"
            scripts.mkdir()
            for name in ("test_run_id.mk", "CppgmBatchWorker.pm", "run_all_tests_common.pl",
                         "compare_results_common.pl", "run_with_timeout.pl", "pa_run_check_targets.mk"):
                shutil.copy(ROOT / "scripts" / name, scripts / name)
            (scripts / "audit_compiler_exceptions.pl").write_text("exit 0;\n")
            shutil.copy(ROOT / "Makefile", root / "Makefile")
            pa = root / "pa11"
            (pa / "tests").mkdir(parents=True)
            (root / "gate").mkdir()
            (pa / "tests/sample.t").write_text("input\n")
            (pa / "tests/sample.ref").write_text("correct\n")
            (pa / "tests/sample.ref.exit_status").write_text("EXIT_SUCCESS\n")
            (pa / "Makefile").write_text(
                "include ../scripts/pa_run_check_targets.mk\n"
                "test:\n"
                "\t@printf '%s\\n' \"$$CPPGM_TEST_COUNTS_FILE\" >\"$$TEST_GATE/inner-counts-$$TEST_SIDE\"\n"
                "\t@perl ../scripts/run_all_tests_common.pl text_t ../producer my tests\n"
                "\t@perl ../scripts/compare_results_common.pl text_t ref my tests\n"
                "\t@if [ \"$$TEST_SIDE\" = B ]; then touch \"$$TEST_GATE/B-compared\"; "
                "while [ ! -e \"$$TEST_GATE/A-complete\" ]; do sleep .01; done; fi\n"
            )
            producer = root / "producer"
            producer.write_text(textwrap.dedent("""\
                #!/usr/bin/env python3
                import os, sys, time
                from pathlib import Path
                gate = Path(os.environ['TEST_GATE'])
                side = os.environ['TEST_SIDE']
                output = Path(sys.argv[sys.argv.index('-o') + 1])
                def wait(name):
                    deadline = time.monotonic() + 10
                    while not (gate / name).exists():
                        if time.monotonic() > deadline:
                            raise RuntimeError('barrier timeout: ' + name)
                        time.sleep(.005)
                if side == 'B':
                    wait('A-written')
                output.write_text('correct\\n' if side == 'A' else 'wrong\\n')
                (gate / ('output-' + side)).write_text(str(output))
                (gate / (side + '-written')).touch()
                if side == 'A':
                    wait('B-compared')
                """))
            producer.chmod(0o755)
            record = ("namespace-record:\n"
                      "\t@printf '%s\\n' '$(CPPGM_TEST_RUN_ID)' >\"$$TEST_GATE/parent-$$TEST_SIDE\"\n"
                      "test-report-nobuild: namespace-record\n")
            processes = {}
            results = {}
            try:
                for side in ("A", "B"):
                    env = dict(self.environment(), TEST_SIDE=side, TEST_GATE=str(root / "gate"),
                               CPPGM_TEST_COUNTS_FILE=str(root / ("gate/outer-counts-" + side)))
                    command = ["make", "--no-print-directory", "test-report-nobuild", "--eval", record,
                               "ACTIVE_TEST_REPORT_PAS=pa11", "DEFAULT_BUILD_JOBS=1",
                               "TEST_REPORT_ASSIGNMENT_JOBS=1", "TEST_REPORT_SUBTEST_JOBS=1"]
                    if not isolated:
                        command.append("CPPGM_TEST_RUN_ID=")
                    processes[side] = subprocess.Popen(command, cwd=root, env=env, stdout=subprocess.PIPE,
                                                       stderr=subprocess.PIPE, text=True)
                results["A"] = processes["A"].communicate(timeout=20)
                ids = [(root / ("gate/parent-" + side)).read_text().strip() for side in ("A", "B")]
                if isolated:
                    self.assertEqual(processes["A"].returncode, 0, str(results))
                    self.assertIn("(1 / 1)", results["A"][0])
                    # A's cleanup must not remove the pending bad report's
                    # marker or its counts before B consumes them.
                    self.assertTrue((pa / (".test_failed." + ids[1])).exists())
                    inner_b = Path((root / "gate/inner-counts-B").read_text().strip())
                    self.assertEqual(inner_b.read_text(), "0 1\n")
                else:
                    # The barrier forces B to overwrite A before A compares:
                    # disabling isolation exposes the original corruption.
                    self.assertNotEqual(processes["A"].returncode, 0, str(results))
                (root / "gate/A-complete").touch()
                results["B"] = processes["B"].communicate(timeout=20)
                self.assertNotEqual(processes["B"].returncode, 0, str(results))
                self.assertIn("0 / 1 TESTS PASSED", results["B"][0])
            finally:
                (root / "gate/A-complete").touch()
                for process in processes.values():
                    if process.poll() is None:
                        process.kill()
                    process.communicate()
            if isolated:
                self.assertNotEqual(*ids)
            else:
                self.assertEqual(ids, ["", ""])
            for side, run_id in zip(("A", "B"), ids):
                output = (root / ("gate/output-" + side)).read_text()
                self.assertEqual(output, "tests/sample.my" + ("." + run_id if run_id else ""))
                expected = "correct\n" if isolated and side == "A" else "wrong\n"
                self.assertEqual((pa / output).read_text(), expected)
                expected_counts = "1 1\n" if isolated and side == "A" else "0 1\n"
                self.assertEqual((root / ("gate/outer-counts-" + side)).read_text(), expected_counts)
            self.assertEqual((pa / "tests/sample.my").exists(), not isolated)
            self.assertEqual((pa / "tests/sample.ref").read_text(), "correct\n")

    @unittest.skipUnless(shutil.which("g++") and shutil.which("bash"), "needs a host C++ compiler and bash")
    def test_host_inspection_preserves_caller_environment(self):
        with tempfile.TemporaryDirectory(prefix="report-inspect.") as directory:
            root = Path(directory)
            compiler = root / "compiler"
            compiler.write_text(
                "#!/usr/bin/env python3\nimport subprocess, sys\n"
                "for line in sys.stdin:\n"
                " out, err, inp, env, *args = line.rstrip('\\n').split('\\t')\n"
                " with open(out, 'ab') as stdout, open(err, 'ab') as stderr:\n"
                "  q = subprocess.run([" + repr(shutil.which("g++")) +
                ", '-x', 'c++', *args], stdout=stdout, stderr=stderr)\n"
                " print('EXIT_SUCCESS' if q.returncode == 0 else 'EXIT_FAILURE', flush=True)\n"
            )
            compiler.chmod(0o755)
            shell = root / "bash"
            shell.write_text(
                "#!/usr/bin/env python3\nimport os, sys\n"
                "if sys.argv[1] == '-lc':\n"
                " os.environ['INSPECTION_CALLER_VALUE'] = 'startup-overrode'\n"
                "os.execv(" + repr(shutil.which("bash")) + ", ['bash', '-c', sys.argv[2]])\n"
            )
            shell.chmod(0o755)
            (root / "sample.t").write_text("host inspection\n")
            (root / "sample.t.1").write_text("int main() { return 0; }\n")
            (root / "sample.inspect.cmd").write_text(
                'test "$INSPECTION_CALLER_VALUE" = inherited && printf preserved\n'
            )
            env = dict(self.environment(), PATH=str(root) + os.pathsep + os.environ["PATH"],
                       CPPGM_BATCH_TESTS="1", CPPGM_HOST_CXX=shutil.which("g++"),
                       INSPECTION_CALLER_VALUE="inherited")
            q = subprocess.run(["perl", str(ROOT / "scripts/run_cpphostinterop_tests_worker.pl"),
                                str(compiler), "my", str(root / "sample.t")],
                               cwd=root, env=env, capture_output=True, text=True, timeout=20)
            self.assertEqual(q.returncode, 0, q.stdout + q.stderr)
            self.assertEqual((root / "sample.my.impl.exit_status").read_text(), "0\n")
            self.assertEqual((root / "sample.my.program.exit_status").read_text(), "0\n")
            self.assertEqual((root / "sample.my.inspect").read_text(), "preserved")

    def test_explicit_suffix_and_fixed_references(self):
        def suffix(value, run_id):
            return subprocess.run(
                ["perl", "-I" + str(ROOT / "scripts"), "-MCppgmBatchWorker=result_suffix",
                 "-e", "print result_suffix($ARGV[0])", value],
                env=dict(self.environment(), CPPGM_TEST_RUN_ID=run_id), capture_output=True, text=True,
            )
        for value in ("ref", "my.already-scoped"):
            result = suffix(value, "run-1")
            self.assertEqual((result.returncode, result.stdout), (0, value))
        self.assertEqual(suffix("my", "").stdout, "my")
        self.assertEqual(suffix("check", "run-1").stdout, "check.run-1")
        self.assertEqual(suffix("pp", "run-1").stdout, "pp.run-1")
        bad = suffix("my", "../outside")
        self.assertNotEqual(bad.returncode, 0)
        self.assertIn("Invalid CPPGM_TEST_RUN_ID", bad.stderr)

    def test_gitignore_covers_scoped_results_but_keeps_references(self):
        with tempfile.TemporaryDirectory(prefix="report-ignore.") as directory:
            root = Path(directory)
            shutil.copy(ROOT / ".gitignore", root / ".gitignore")
            (root / "pa29").mkdir()
            shutil.copy(ROOT / "pa29/.gitignore", root / "pa29/.gitignore")
            subprocess.run(["git", "init", "-q", str(root)], check=True)
            outputs = ["pa1/tests/sample.my.run-1", "pa1/tests/sample.my.run-1.exit_status",
                       "pa17/tests/controls/sample.check.run-1", "pa33/tests/o1/sample.my.run-1.mir",
                       "pa29/tests/preproc/sample.pp.run-1", "pa29/tests/preproc/sample.pp.run-1.stdout",
                       ".test_counts.run-1", "pa17/.test_failed.run-1"]
            for name in outputs:
                result = subprocess.run(["git", "check-ignore", name], cwd=root, capture_output=True)
                self.assertEqual(result.returncode, 0, name)
            for name in ("pa1/tests/sample.ref", "pa33/tests/o1/sample.ref.expect",
                         "pa33/tests/o1/sample.ref.program.stdout"):
                result = subprocess.run(["git", "check-ignore", name], cwd=root, capture_output=True)
                self.assertEqual(result.returncode, 1, name)

    def test_preprocessing_retains_mode_and_uses_scoped_output(self):
        with tempfile.TemporaryDirectory(prefix="report-preproc.") as directory:
            root = Path(directory)
            (root / "sample.t").write_text("#define X 1\nX\n")
            (root / "sample.ref").write_text("preprocessed\n")
            (root / "sample.ref.exit_status").write_text("EXIT_SUCCESS\n")
            producer = root / "producer"
            producer.write_text(
                "#!/usr/bin/env python3\nimport sys\nfrom pathlib import Path\n"
                "assert '-E' in sys.argv\n"
                "Path(sys.argv[sys.argv.index('-o') + 1]).write_text('preprocessed\\n')\n"
            )
            producer.chmod(0o755)
            env = dict(self.environment(), CPPGM_TEST_RUN_ID="preproc-run", CPPGM_TEXT_INPUT_PROFILE="source")
            for command in (
                ["perl", str(ROOT / "scripts/run_all_tests_common.pl"), "text_t", str(producer), "pp", "sample.t"],
                ["perl", str(ROOT / "scripts/compare_results_common.pl"), "text_t", "ref", "pp", "sample.t"],
            ):
                result = subprocess.run(command, cwd=root, env=env, capture_output=True, text=True, timeout=20)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertEqual((root / "sample.pp.preproc-run").read_text(), "preprocessed\n")
            self.assertFalse((root / "sample.pp").exists())

    def test_envelope_reads_scoped_program_and_original_inputs(self):
        with tempfile.TemporaryDirectory(prefix="report-envelope.") as directory:
            root = Path(directory)
            (root / "sample.t").write_text("input\n")
            (root / "sample.args").write_text("17\n")
            (root / "sample.stdin").write_text("payload\n")
            (root / "sample.ref.ir").write_text("42\n")
            program = root / "sample.my.envelope-run.program"
            program.write_text("#!/bin/sh\nexit 0\n")
            program.chmod(0o755)
            valgrind = root / "valgrind"
            valgrind.write_text(
                "#!/usr/bin/env python3\nimport sys\n"
                "assert sys.argv[-2].endswith('sample.my.envelope-run.program')\n"
                "assert sys.argv[-1] == '17'\n"
                "assert sys.stdin.read() == 'payload\\n'\n"
                "print('I refs: 42', file=sys.stderr)\n"
            )
            valgrind.chmod(0o755)
            env = dict(self.environment(), CPPGM_TEST_RUN_ID="envelope-run", PATH=str(root) + os.pathsep + os.environ["PATH"])
            result = subprocess.run(
                ["perl", str(ROOT / "scripts/check_ir_envelope.pl"), "--require-sidecars", "10", str(root)],
                env=env, capture_output=True, text=True, timeout=20,
            )
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn("PASS (1/1", result.stdout)
            self.assertIn("0 skipped", result.stdout)

    def test_native_worker_and_comparer_use_the_same_run(self):
        with tempfile.TemporaryDirectory(prefix="report-native.") as directory:
            root = Path(directory)
            (root / "sample.t").write_text("input\n")
            for name, value in (("impl.exit_status", "0\n"), ("program.exit_status", "0\n"),
                                ("program.stdout", "")):
                (root / ("sample.ref." + name)).write_text(value)
            (root / "sample.ref.mir").write_text("machine_ir x86_64 linux\n")
            producer = root / "producer"
            producer.write_text(textwrap.dedent("""\
                #!/usr/bin/env python3
                from pathlib import Path
                import sys
                assert sys.argv[1:] == ['--batch-stdin']
                for line in sys.stdin:
                    stdout, stderr, stdin, environment, *args = line.rstrip('\\n').split('\\t')
                    Path(stdout).write_text('')
                    Path(stderr).write_text('')
                    Path(args[args.index('--dump-machine-ir') + 1]).write_text('machine_ir x86_64 linux\\n')
                    program = Path(args[args.index('-o') + 1])
                    program.write_text('#!/bin/sh\\nexit 0\\n')
                    program.chmod(0o755)
                    print('EXIT_SUCCESS', flush=True)
                """))
            producer.chmod(0o755)
            env = dict(self.environment(), CPPGM_TEST_RUN_ID="native-run")
            for command in (
                ["perl", str(ROOT / "scripts/run_lowir_native_tests_worker.pl"), str(producer), "my", str(root / "sample.t")],
                ["perl", str(ROOT / "scripts/compare_results_common.pl"), "mir_behavior_t", "ref", "my", str(root / "sample.t")],
            ):
                result = subprocess.run(command, cwd=root, env=env, capture_output=True, text=True, timeout=20)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertTrue((root / "sample.my.native-run.program").exists())
            self.assertFalse((root / "sample.my.program").exists())


if __name__ == "__main__":
    unittest.main()
