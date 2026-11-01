#!/usr/bin/env python3
"""A sidecar that no longer matches its reference must fail the audit."""
from pathlib import Path
import re
import subprocess
import tempfile
import unittest


REPO_ROOT = Path(__file__).resolve().parents[2]
AUDIT = REPO_ROOT / "scripts" / "audit_ir_expectations.pl"
REGENERATE = REPO_ROOT / "scripts" / "regenerate_ir_expectations.pl"

# A real dump: the generators read its format, and it is large enough for the
# two tolerances to give different bounds.
MACHINE_IR = max((REPO_ROOT / "pa33" / "tests" / "o1").glob("*.ref.mir"),
                 key=lambda path: path.stat().st_size).read_text()


class AuditIrExpectationsTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)

    def fixture(self, lane: str) -> Path:
        base = self.root / lane
        base.parent.mkdir(parents=True)
        base.with_suffix(".t").write_text("# fixture\n")
        return base

    def sidecar(self, base: Path) -> Path:
        return base.with_name(base.name + ".ref.expect")

    def machine_fixture(self, lane: str = "pa33/tests/o1/x") -> Path:
        base = self.fixture(lane)
        base.with_name("x.ref.mir").write_text(MACHINE_IR)
        self.sidecar(base).write_text("")
        self.regenerate(base.parent, "--add-budgets")
        return base

    def regenerate(self, root: Path, *arguments: str) -> None:
        subprocess.run(["perl", str(REGENERATE), *arguments, "--write", str(root)],
                       check=True, capture_output=True, text=True)

    def audit(self, *arguments: str) -> subprocess.CompletedProcess[str]:
        return subprocess.run(["perl", str(AUDIT), "--root", str(self.root), *arguments],
                              capture_output=True, text=True, check=False)

    def bound(self, base: Path) -> int:
        line = next(line for line in self.sidecar(base).read_text().splitlines()
                    if line.startswith("instructions <="))
        return int(line.split()[-1])

    def set_bound(self, base: Path, bound: int) -> None:
        self.sidecar(base).write_text(re.sub(r"^instructions <= \d+$", f"instructions <= {bound}",
                                             self.sidecar(base).read_text(), flags=re.M))

    def test_a_current_sidecar_passes(self):
        self.machine_fixture()
        self.assertEqual(self.audit().returncode, 0)

    def test_a_stale_sidecar_fails_and_write_repairs_it(self):
        base = self.machine_fixture()
        current = self.sidecar(base).read_text()
        self.set_bound(base, self.bound(base) - 1)
        stale = self.sidecar(base).read_text()

        result = self.audit()
        self.assertEqual(result.returncode, 1)
        self.assertIn("x.ref.expect", result.stdout)
        self.assertIn("--write", result.stderr)
        self.assertEqual(self.sidecar(base).read_text(), stale)

        self.assertEqual(self.audit("--write").returncode, 0)
        self.assertEqual(self.sidecar(base).read_text(), current)
        self.assertEqual(self.audit().returncode, 0)

    def test_a_loose_bound_is_kept(self):
        base = self.machine_fixture()
        self.set_bound(base, self.bound(base) + 5)
        self.assertEqual(self.audit().returncode, 0)

    def test_hand_written_lines_are_kept(self):
        base = self.machine_fixture()
        with self.sidecar(base).open("a") as stream:
            stream.write("\n# why\ncount(call) == 0\n")
        self.assertEqual(self.audit().returncode, 0)

    def test_pa24_uses_its_own_tolerance(self):
        base = self.machine_fixture("pa24/tests/o1/x")
        narrow = self.bound(base)
        self.assertEqual(self.audit().returncode, 1)
        self.assertEqual(self.audit("--write").returncode, 0)
        self.assertGreater(self.bound(base), narrow)

    def test_an_assignment_without_a_tolerance_fails(self):
        self.machine_fixture("pa40/tests/o1/x")
        result = self.audit()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("no tolerance", result.stderr)

    def backend(self) -> Path:
        path = self.root / "backend"
        dump = self.root / "machine-ir"
        dump.write_text(MACHINE_IR)
        path.write_text(f"""#!/bin/sh
printf '%s\\n' "$*" >> {self.root}/backend.log
while [ "$1" != --dump-machine-ir ]; do shift; done
cp {dump} "$2"
""")
        path.chmod(0o755)
        return path

    def behavior_fixture(self) -> Path:
        base = self.machine_fixture("pa33/tests/behavior/o2/x")
        base.with_name("x.ref.mir").unlink()
        base.with_name("x.flags").write_text("--a --b\n")
        return base

    def test_a_behavior_sidecar_is_checked_with_the_backend(self):
        base = self.behavior_fixture()
        backend = str(self.backend())
        self.assertEqual(self.audit("--backend", backend).returncode, 0)
        arguments = (self.root / "backend.log").read_text()
        self.assertIn("-O2 --a --b --dump-machine-ir", arguments)
        self.assertFalse(base.with_name("x.ref.mir").exists())

        self.set_bound(base, self.bound(base) - 1)
        self.assertEqual(self.audit("--backend", backend).returncode, 1)

    def test_a_behavior_sidecar_is_skipped_without_the_backend(self):
        base = self.behavior_fixture()
        self.set_bound(base, self.bound(base) - 1)
        result = self.audit()
        self.assertEqual(result.returncode, 0)
        self.assertIn("no reference output", result.stdout)

    def test_a_failing_backend_fails_the_audit(self):
        self.behavior_fixture()
        result = self.audit("--backend", "/bin/false")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("failed on", result.stderr)


if __name__ == "__main__":
    unittest.main()
