#!/usr/bin/env python3

from pathlib import Path
import json
import subprocess
import sys
import tempfile
import unittest


REPO_ROOT = Path(__file__).resolve().parents[2]
COMPARE = REPO_ROOT / "pa24" / "scripts" / "compare_results.pl"


class Pa24MirModeTest(unittest.TestCase):
    def write_success_case(self, root, ref_mir, my_mir):
        test = root / "case.t"
        test.write_text("function @main() -> i32 {\n}\n", encoding="utf-8")
        for suffix in ("ref", "my"):
            (root / f"case.{suffix}.impl.exit_status").write_text(
                "0\n", encoding="utf-8"
            )
            (root / f"case.{suffix}.program.exit_status").write_text(
                "0\n", encoding="utf-8"
            )
            (root / f"case.{suffix}.program.stdout").write_text(
                "ok\n", encoding="utf-8"
            )
        (root / "case.ref.mir").write_text(ref_mir, encoding="utf-8")
        (root / "case.my.mir").write_text(my_mir, encoding="utf-8")
        return test

    def compare(self, test):
        return subprocess.run(
            ["perl", str(COMPARE), "ref", "my", str(test)],
            cwd=REPO_ROOT,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )

    def test_behavior_retains_but_does_not_compare_reference_mir(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary) / "behavior"
            root.mkdir()
            test = self.write_success_case(root, "reference mir\n", "student mir\n")
            result = self.compare(test)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_behavior_requires_informational_reference_mir(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary) / "behavior"
            root.mkdir()
            test = self.write_success_case(root, "reference mir\n", "student mir\n")
            (root / "case.ref.mir").unlink()
            result = self.compare(test)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("missing informational reference machine IR", result.stdout)

    def test_strict_still_compares_reference_mir(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary) / "strict"
            root.mkdir()
            test = self.write_success_case(root, "reference mir\n", "student mir\n")
            result = self.compare(test)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("machine IR dumps do not match", result.stdout)

    @unittest.skipUnless(sys.platform.startswith("linux"), "ELF control lane")
    def test_native_control_applies_compiler_resource_limits(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            test = root / "case.t"
            test.write_text("unused by stand-in backend\n")
            limits = dict(cpu_seconds=1, address_space_bytes=134217728, file_bytes=1048576)
            test.with_suffix(".limits").write_text(json.dumps(limits))
            backend = root / "backend"
            backend.write_text(
                "#!/usr/bin/env python3\n"
                "import resource, sys\n"
                "assert resource.getrlimit(resource.RLIMIT_CPU) == (1, 2)\n"
                "assert resource.getrlimit(resource.RLIMIT_AS) == (134217728, 134217728)\n"
                "assert resource.getrlimit(resource.RLIMIT_FSIZE) == (1048576, 1048576)\n"
                "print('observed all three compiler resource limits', file=sys.stderr)\n"
                "sys.exit(23)\n"
            )
            backend.chmod(0o755)
            result = subprocess.run(
                ["perl", str(REPO_ROOT / "scripts/check_pa24_native_contracts.pl"),
                 str(backend), str(test)], capture_output=True, text=True,
            )
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("native compile failed", result.stderr)
            self.assertIn("observed all three compiler resource limits", result.stderr)

    @unittest.skipUnless(sys.platform.startswith("linux"), "ELF control lane")
    def test_native_control_rejects_invalid_resource_limits(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            test = root / "case.t"
            test.write_text("unused by stand-in backend\n")
            test.with_suffix(".limits").write_text(json.dumps(dict(
                cpu_seconds=0, address_space_bytes=134217728, file_bytes=1048576)))
            result = subprocess.run(
                ["perl", str(REPO_ROOT / "scripts/check_pa24_native_contracts.pl"),
                 "/bin/true", str(test)], capture_output=True, text=True,
            )
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("resource limit setup failed", result.stderr)


if __name__ == "__main__":
    unittest.main()
