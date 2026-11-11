#!/usr/bin/env python3
"""Check the self-checking controls lane (scripts/check_controls.pl) and its
Makefile routes with a stand-in frontend and reference backend."""

from pathlib import Path
import importlib.util
import json
import shutil
import os
import re
from unittest.mock import call, patch
import subprocess
import sys
import tempfile
import textwrap
import unittest


REPO_ROOT = Path(__file__).resolve().parents[2]
CHECKER = REPO_ROOT / "scripts/check_controls.pl"

# The stand-in frontend "lowers" a source to the status its main returns and
# the text a `// prints:` line names; `REJECTED` in the source is an error,
# `INTERNAL_ERROR` stops the frontend with an internal compiler error (status
# 70), `CRASHES` makes the frontend abort and `ABORTS` makes the program abort.
FRONTEND = textwrap.dedent("""\
    #!/usr/bin/env python3
    import os, pathlib, re, sys
    args = sys.argv[1:]
    assert args[:2] == ['--emit-lowir', '-O0'], args
    source = pathlib.Path(args[-1]).read_text()
    if 'REJECTED' in source:
        sys.exit(1)
    if 'INTERNAL_ERROR' in source:
        sys.exit(70)
    if 'CRASHES' in source:
        os.abort()
    status = 'abort' if 'ABORTS' in source else re.search(r'return (\\d+);', source).group(1)
    printed = re.search(r'// prints: (.*)', source)
    out = pathlib.Path(args[args.index('-o') + 1])
    out.write_text(status + '\\n' + (printed.group(1) if printed else ''))
    """)
# The stand-in backend turns that "LowIR" into a program with that behavior.
BACKEND = textwrap.dedent("""\
    #!/usr/bin/env python3
    import pathlib, sys
    args = sys.argv[1:]
    assert args[0] == '-O0', args
    status, printed = pathlib.Path(args[-1]).read_text().split('\\n', 1)
    program = pathlib.Path(args[args.index('-o') + 1])
    ending = 'kill -ABRT $$' if status == 'abort' else f'exit {status}'
    program.write_text('#!/bin/sh\\n' + (f'printf "%s\\\\n" {printed!r}\\n' if printed else '') +
                       ending + '\\n')
    program.chmod(0o755)
    """)


class CheckControlsTests(unittest.TestCase):
    def setUp(self):
        temp = tempfile.TemporaryDirectory(prefix="check-controls.")
        self.addCleanup(temp.cleanup)
        self.root = Path(temp.name)
        (self.root / "scripts").symlink_to(REPO_ROOT / "scripts")
        (self.root / "dev").symlink_to(REPO_ROOT / "dev")
        self.frontend = self.root / "cppgm++"
        self.frontend.write_text(FRONTEND)
        self.frontend.chmod(0o755)
        self.backend = self.root / "lowir2native-ref"
        self.backend.write_text(BACKEND)
        self.backend.chmod(0o755)
        self.controls = self.root / "pa13/tests/controls"
        self.controls.mkdir(parents=True)

    def control(self, name, text, **sidecars):
        path = self.controls / name
        path.write_text(textwrap.dedent(text))
        for suffix, data in sidecars.items():
            path.with_suffix("." + suffix).write_text(data)
        return path

    def check(self, *args):
        return subprocess.run(["perl", str(CHECKER), *map(str, args)],
                              capture_output=True, text=True, cwd=self.root)

    def run_checker(self, target):
        return self.check(self.frontend, self.backend, target)

    def test_optional_assembly_sidecar_owns_a_section_named_fixture(self):
        ordinary = self.control("200-section-token-safe.t", "int main() { return 0; }")
        optional = self.control("100-module-empty-section-stack.t", 'asm(".popsection");')
        optional.with_suffix(".assembly-profile").write_text("gnu-reject\n")
        result = subprocess.run(
            ["perl", str(REPO_ROOT / "scripts/check_pa27_section_contract.pl"),
             "--list", str(self.controls)], capture_output=True, text=True, check=False,
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout.splitlines(), [str(ordinary)])
        assembly = subprocess.run(
            ["perl", str(REPO_ROOT / "scripts/check_inline_assembly.pl"),
             "--list", str(self.controls)], capture_output=True, text=True, check=False,
        )
        self.assertEqual(assembly.returncode, 0, assembly.stderr)
        self.assertEqual(assembly.stdout.splitlines(), [str(optional)])

    def test_directory_grades_stated_outcomes_and_skips_focused_controls(self):
        self.control("100-pass.t", "// N3485 x: rule; expect exit 7\nint main() { return 7; }\n")
        self.control("100-reject.t", "// N3485 x: ill-formed; must be rejected\nREJECTED\n")
        self.control("530-focused-property.t", "int main() { return 3; }\n")
        result = self.run_checker(self.controls)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("PASS (2/2)", result.stdout)

    def test_wrong_status_and_accepted_rejection_fail(self):
        self.control("100-wrong.t", "// expect exit 0\nint main() { return 1; }\n")
        self.control("100-accepted.t", "// expect reject\nint main() { return 0; }\n")
        result = self.run_checker(self.controls)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("100-wrong.t: FAIL: program exited with status 1, expected exit status 0",
                      result.stderr)
        self.assertIn("100-accepted.t: FAIL: frontend accepted", result.stderr)

    def test_a_signal_is_not_an_exit_status(self):
        # SIGABRT gives the shell's 134, but a program it kills did not exit 134.
        self.control("100-exits.t", "// expect exit 134\nint main() { return 134; }\n")
        self.assertEqual(self.run_checker(self.controls).returncode, 0)
        self.control("100-aborts.t", "// expect exit 134\nint main() { ABORTS; }\n")
        result = self.run_checker(self.controls)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("100-aborts.t: FAIL: program was killed by signal 6 (SIGABRT), "
                      "expected exit status 134", result.stderr)
        self.assertNotIn("100-exits.t", result.stderr)

    def test_a_crashing_frontend_does_not_reject(self):
        self.control("100-crash.t", "// expect reject\nCRASHES\n")
        result = self.run_checker(self.controls)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("100-crash.t: FAIL: frontend was killed by signal 6 (SIGABRT) "
                      "instead of rejecting", result.stderr)

    def test_an_internal_compiler_error_does_not_reject(self):
        self.control("100-internal.t", "// expect reject\nINTERNAL_ERROR\n")
        result = self.run_checker(self.controls)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("100-internal.t: FAIL: frontend stopped with an internal compiler "
                      "error (status 70) instead of rejecting", result.stderr)

    def test_sidecar_states_status_and_exact_output(self):
        self.control("100-printed.t", "// prints: hello\nint main() { return 3; }\n",
                     expect="exit 3\n", stdout="hello\n")
        self.assertEqual(self.run_checker(self.controls).returncode, 0)
        (self.controls / "100-printed.stdout").write_text("goodbye\n")
        result = self.run_checker(self.controls)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("output differs", result.stderr)

    def test_compile_acceptance_validates_lowir_without_linking(self):
        fixture = self.control("200-types.t", "int function() { return 0; }\n",
                               expect="accept\n")
        reader = self.root / "lowir-ref"
        reader.write_text(
            "#!/usr/bin/env python3\nimport pathlib, sys\n"
            "source = pathlib.Path(sys.argv[-1]).read_text()\n"
            "sys.exit(0 if source.startswith('0\\n') else 1)\n")
        reader.chmod(0o755)
        self.backend.write_text("#!/bin/sh\nexit 99\n")
        with patch.dict(os.environ, {"CPPGM_CONTROL_LOWIR_APP": str(reader)}):
            result = self.run_checker(fixture)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            fixture.write_text("int function() { return 1; }\n")
            result = self.run_checker(fixture)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("reference LowIR reader exited with status 1", result.stderr)

    def test_control_passes_its_language_mode_without_changing_other_controls(self):
        fixture = self.control("200-modern.t", "// expect exit 0\nint main() { return 0; }\n")
        fixture.with_suffix(".cxx-standard").write_text("c++17\n")
        self.frontend.write_text(FRONTEND.replace(
            "source = pathlib.Path(args[-1]).read_text()",
            "assert '-std=c++17' in args, args\nsource = pathlib.Path(args[-1]).read_text()"))
        result = self.run_checker(fixture)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        fixture.with_suffix(".cxx-standard").write_text("c++11 c++17\n")
        result = self.run_checker(fixture)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("Expected one C++ standard", result.stderr)
        fixture.with_suffix(".cxx-standard").unlink()
        self.frontend.write_text(FRONTEND.replace(
            "source = pathlib.Path(args[-1]).read_text()",
            "assert not any(arg.startswith('-std=') for arg in args), args\n"
            "source = pathlib.Path(args[-1]).read_text()"))
        result = self.run_checker(fixture)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_a_named_control_must_state_an_outcome(self):
        focused = self.control("530-focused-property.t", "int main() { return 3; }\n")
        stated = self.control("100-pass.t", "// expect exit 7\nint main() { return 7; }\n")
        self.assertNotEqual(self.run_checker(focused).returncode, 0)
        self.assertNotEqual(self.check("--owns", focused).returncode, 0)
        self.assertEqual(self.check("--owns", stated).returncode, 0)
        self.assertEqual(self.run_checker(stated).returncode, 0)

    def test_frontend_resource_limits_are_inherited(self):
        self.frontend.write_text(FRONTEND.replace(
            "source = pathlib.Path(args[-1]).read_text()",
            "import resource\n"
            "assert resource.getrlimit(resource.RLIMIT_CPU) == (2, 3)\n"
            "if sys.platform != 'darwin':\n"
            "    assert resource.getrlimit(resource.RLIMIT_AS) == (268435456, 268435456)\n"
            "assert resource.getrlimit(resource.RLIMIT_FSIZE) == (1048576, 1048576)\n"
            "source = pathlib.Path(args[-1]).read_text()"))
        control = self.control("100-bounded.t", "// expect exit 0\nint main() { return 0; }\n",
                               limits=json.dumps(dict(cpu_seconds=2, address_space_bytes=268435456,
                                                      file_bytes=1048576)))
        result = self.run_checker(control)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_darwin_memory_gate_is_reported_without_skipping_correctness(self):
        control = self.control("100-bounded.t", "// expect exit 0\nint main() { return 0; }\n",
                               limits=json.dumps(dict(cpu_seconds=2, address_space_bytes=268435456,
                                                      file_bytes=1048576)))
        skips = self.root / "skips"
        with patch.dict(os.environ, CPPGM_TEST_PLATFORM="macos", CPPGM_TEST_SKIPS_FILE=str(skips)):
            result = self.run_checker(control)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("address-space ceiling", skips.read_text())
        self.assertIn("correctness and CPU/file limits still run", skips.read_text())

    def test_platform_resource_selection_preserves_cpu_and_file_limits(self):
        spec = importlib.util.spec_from_file_location("resource_limiter", REPO_ROOT / "scripts/run_resource_limited.py")
        limiter = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(limiter)
        path = self.root / "limits.json"
        path.write_text(json.dumps(dict(cpu_seconds=2, address_space_bytes=268435456, file_bytes=1048576)))
        for platform in ("linux", "darwin"):
            with self.subTest(platform=platform), \
                    patch.object(limiter.sys, "platform", platform), \
                    patch.object(limiter.sys, "argv", ["limiter", str(path), "--", "/bin/true"]), \
                    patch.object(limiter.resource, "setrlimit") as limits, \
                    patch.object(limiter.signal, "signal"), \
                    patch.object(limiter.os, "execvp") as execute:
                limiter.main()
                limits.assert_has_calls([call(limiter.resource.RLIMIT_CPU, (2, 3))])
                self.assertIn(call(limiter.resource.RLIMIT_FSIZE, (1048576, 1048576)), limits.call_args_list)
                address = call(limiter.resource.RLIMIT_AS, (268435456, 268435456))
                self.assertEqual(address in limits.call_args_list, platform == "linux")
                execute.assert_called_once_with("/bin/true", ["/bin/true"])

    def test_resource_exhaustion_fails_a_positive_control(self):
        control = self.control("100-bounded.t", "// expect exit 0\nint main() { return 0; }\n",
                               limits=json.dumps(dict(cpu_seconds=1, address_space_bytes=268435456,
                                                      file_bytes=1048576)))
        self.frontend.write_text("#!/usr/bin/env python3\nwhile True: pass\n")
        result = self.run_checker(control)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("frontend was killed by signal", result.stderr)
        if sys.platform != "darwin":
            self.frontend.write_text("#!/usr/bin/env python3\nx = bytearray(536870912)\n")
            result = self.run_checker(control)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("MemoryError", result.stderr)
        self.frontend.write_text("#!/usr/bin/env python3\nimport sys\n"
                                 "open(sys.argv[sys.argv.index('-o') + 1], 'wb').write(b'x' * 2097152)\n")
        result = self.run_checker(control)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("frontend", result.stderr)

    def test_invalid_resource_limits_and_bounded_rejections_fail(self):
        control = self.control("100-bounded.t", "// expect exit 0\nint main() { return 0; }\n",
                               limits='{"cpu_seconds": 1}')
        result = self.run_checker(control)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("resource limit setup failed", result.stderr)
        control.write_text("// expect reject\nREJECTED\n")
        result = self.run_checker(control)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("resource limits require a positive exit control", result.stderr)

    def make(self, pa, *args):
        return subprocess.run(
            ["make", "-s", "-C", str(self.root / pa), *args, "RUN_CHECK_DEPS=", "TEST_DEPS=", "CPPGM_TEST_RUN_ID=",
             f"CPPGM_TEST_APP={self.frontend}", f"NATIVE_REFERENCE_APP={self.backend}"],
            capture_output=True, text=True)

    def test_lowir_assignment_routes_controls_to_the_checker(self):
        shutil.copy(REPO_ROOT / "pa13/Makefile", self.root / "pa13/Makefile")
        self.control("100-pass.t", "// expect exit 7\nint main() { return 7; }\n")
        check = self.make("pa13", "check", "TEST=tests/controls/100-pass.t")
        self.assertEqual(check.returncode, 0, check.stdout + check.stderr)
        ref = self.make("pa13", "ref-test", "TEST=tests/controls/100-pass.t")
        self.assertIn("no reference sidecar", ref.stdout)
        self.assertEqual(self.make("pa13", "test-controls").returncode, 0)
        self.control("100-wrong.t", "// expect exit 0\nint main() { return 1; }\n")
        self.assertNotEqual(self.make("pa13", "test-controls").returncode, 0)

    def test_control_shape_counts_failures_in_quiet_reports(self):
        shutil.copy(REPO_ROOT / "pa13/Makefile", self.root / "pa13/Makefile")
        self.control("100-pass.t", "// expect exit 0\nint main() { return 0; }\n")
        self.control("100-reject.t", "// expect reject\nREJECTED\n")
        self.control("100-fail.t", "// expect exit 0\nint main() { return 1; }\n")
        counts = self.root / "counts.txt"
        with patch.dict(os.environ, {"KEEP_GOING": "1", "CPPGM_REPORT_QUIET": "1",
                                     "CPPGM_TEST_COUNTS_FILE": str(counts)}):
            result = self.make("pa13", "test-controls")
        self.assertEqual(counts.read_text(), "2 3\n")
        self.assertTrue((self.root / "pa13/.test_failed").exists())
        self.assertIn("pa13 tests/controls: FAIL (2/3)", result.stdout)
        self.assertIn("100-fail.t: ERROR: control predicate failed", result.stdout)

    def test_pa12_routes_stated_outcomes_away_from_its_focused_checker(self):
        pa12 = self.root / "pa12/tests/controls"
        pa12.mkdir(parents=True)
        shutil.copy(REPO_ROOT / "pa12/Makefile", self.root / "pa12/Makefile")
        (pa12 / "100-pass.t").write_text("// expect exit 7\nint main() { return 7; }\n")
        check = self.make("pa12", "check", "TEST=tests/controls/100-pass.t")
        self.assertEqual(check.returncode, 0, check.stdout + check.stderr)
        (pa12 / "100-unstated.t").write_text("int main() { return 7; }\n")
        focused = self.make("pa12", "check", "TEST=tests/controls/100-unstated.t")
        self.assertNotEqual(focused.returncode, 0)
        self.assertIn("No control predicate owns", focused.stdout + focused.stderr)

    def test_object_extents_keep_direct_and_indirect_requirements_in_their_owners(self):
        # The stand-ins isolate checker predicates; the course controls also
        # execute through the real frontend/backend in test-controls.
        self.frontend.write_text(textwrap.dedent("""\
            #!/usr/bin/env python3
            import pathlib, sys
            args = sys.argv[1:]
            pathlib.Path(args[args.index('-o') + 1]).write_text(pathlib.Path(args[-1]).read_text())
            """))
        self.backend.write_text(textwrap.dedent("""\
            #!/usr/bin/env python3
            import pathlib, sys
            args = sys.argv[1:]
            program = pathlib.Path(args[args.index('-o') + 1])
            program.write_text('#!/bin/sh\\nexit 0\\n')
            program.chmod(0o755)
            """))
        lowir = (
            "function @result(%out : ptr [pass=indirect_result, object_bytes=24]) -> void {}\n"
            "function @reference(%value : ptr [pass=by_address, object_bytes=24]) -> i64 {}\n"
            "function @member(%this : ptr [object_bytes=24]) -> i64 {}\n"
            "function @unused(%this : ptr [object_bytes=24]) -> i64 {}\n"
            "function @pointer_value(%value : ptr) -> i64 {}\n"
            "function @main() -> i32 {\n  %answer = call i64 @member(%object)\n}\n"
        )
        direct = self.root / "pa12/tests/controls/200-parameter-object-extent-boundary.t"
        indirect = self.root / "pa22/tests/controls/300-member-pointer-object-extent-boundary.t"
        for pa, control in [("pa12", direct), ("pa22", indirect)]:
            control.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy(REPO_ROOT / pa / "Makefile", self.root / pa / "Makefile")
            control.write_text(lowir)
        self.assertEqual(self.make("pa12", "check", f"TEST=tests/controls/{direct.name}").returncode, 0)
        missing = self.make("pa22", "check", f"TEST=tests/controls/{indirect.name}")
        self.assertNotEqual(missing.returncode, 0)
        self.assertIn("indirect member call lost its object extent", missing.stdout + missing.stderr)
        indirect.write_text(lowir.replace("  %answer", "  %indirect = call i64 %callee(ptr [object_bytes=24] %object)\n  %answer"))
        self.assertEqual(self.make("pa22", "check", f"TEST=tests/controls/{indirect.name}").returncode, 0)
        direct.write_text(lowir.replace("@member(%this : ptr [object_bytes=24])", "@member(%this : ptr)"))
        missing = self.make("pa12", "check", f"TEST=tests/controls/{direct.name}")
        self.assertNotEqual(missing.returncode, 0)
        self.assertIn("direct member call lost its object extent", missing.stdout + missing.stderr)


class ZeroBoundedControlTests(unittest.TestCase):
    def setUp(self):
        temp = tempfile.TemporaryDirectory(prefix="zero-bounded-control.")
        self.addCleanup(temp.cleanup)
        self.root = Path(temp.name)
        self.source = (REPO_ROOT / "pa33/tests/regression/controls/"
                       "500-zero-bounded-signed-range.t").read_text()
        self.fixture = self.root / "500-zero-bounded-signed-range.t"
        self.fixture.write_text(self.source)
        self.optimizer = self.root / "optimizer"
        self.optimizer.write_text(textwrap.dedent("""\
            #!/usr/bin/env python3
            import pathlib, sys
            args = sys.argv[1:]
            source = (pathlib.Path(args[-1]) if args[0] == '-O0' else
                      pathlib.Path(__file__).parent / 'optimized.lowir')
            pathlib.Path(args[args.index('-o') + 1]).write_text(source.read_text())
            """))
        self.driver = self.root / "driver"
        self.driver.write_text(textwrap.dedent("""\
            #!/usr/bin/env python3
            import pathlib, sys
            args = sys.argv[1:]
            output = pathlib.Path(args[args.index('-o') + 1])
            output.write_text('#!/bin/sh\\nexit 0\\n')
            output.chmod(0o755)
            """))
        for tool in (self.optimizer, self.driver):
            tool.chmod(0o755)

    def check(self, decision, unsafe_negative=False):
        output = self.source
        for name in ("fold_zero_bounded", "fold_with_accept_phi"):
            pattern = r"function @" + name + r"\(.*?^\}"
            body = re.search(pattern, output, re.M | re.S).group()
            replacement = re.sub(
                r"  block \^lower:\n.*?(?=  block \^accept:)",
                "  block ^lower:\n    " + decision + "\n\n", body,
                flags=re.S).replace("^upper: 1", "^lower: 1")
            output = output.replace(body, replacement)
        if unsafe_negative:
            output = output.replace("cmp lt i32 %lower_value, 0",
                                    "cmp ult i32 %lower_value, 128")
        (self.root / "optimized.lowir").write_text(output)
        return subprocess.run([
            "perl", str(REPO_ROOT / "scripts/check_lowir_zero_bounded_range.pl"),
            str(self.optimizer), str(self.driver), str(self.fixture)],
            capture_output=True, text=True, timeout=30)

    def test_accepts_switch_and_equivalent_unsigned_predicates(self):
        switch = "switch %value, ^reject, " + ", ".join(
            f"{value}:^accept" for value in range(128))
        decisions = [switch]
        for predicate, bound, taken, other in (
                ("ult", 128, "accept", "reject"),
                ("ule", 127, "accept", "reject"),
                ("uge", 128, "reject", "accept"),
                ("ugt", 127, "reject", "accept")):
            decisions.append(f"%range = cmp {predicate} i32 %value, {bound}\n"
                             f"    branch %range, ^{taken}, ^{other}")
        for decision in decisions:
            with self.subTest(decision=decision[:50]):
                result = self.check(decision)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_rejects_wrong_bounds_signed_tests_and_destinations(self):
        for predicate, bound, taken, other in (
                ("ult", 129, "accept", "reject"),
                ("ule", 128, "accept", "reject"),
                ("uge", 127, "reject", "accept"),
                ("lt", 128, "accept", "reject"),
                ("ult", 128, "reject", "accept")):
            with self.subTest(predicate=predicate, bound=bound, taken=taken):
                decision = (f"%range = cmp {predicate} i32 %value, {bound}\n"
                            f"    branch %range, ^{taken}, ^{other}")
                result = self.check(decision)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("did not combine the signed bounds", result.stderr)

    def test_switch_holes_and_unsafe_negative_control_still_fail(self):
        switch = "switch %value, ^reject, " + ", ".join(
            f"{value}:^accept" for value in [*range(127), 128])
        self.assertNotEqual(self.check(switch).returncode, 0)
        decision = "%range = cmp ult i32 %value, 128\n    branch %range, ^accept, ^reject"
        result = self.check(decision, unsafe_negative=True)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("combined unsafe bounds", result.stderr)


if __name__ == "__main__":
    unittest.main()
