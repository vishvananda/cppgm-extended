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
EXPECT = REPO_ROOT / "scripts" / "expect_ir.pl"

POINTER_CHOICE = """function @choose(%condition : i1, %a : ptr, %b : ptr) -> i64 {
  block ^entry:
    branch %condition, ^left, ^right
  block ^left:
    jump ^join
  block ^right:
    jump ^join
  block ^join:
    %address = phi ptr [^left: %a, ^right: %b]
    %value = load i64 %address
    return i64 %value
}
"""

SELECT_CHOICE = """function @choose(%condition : i1, %a : ptr, %b : ptr) -> i64 {
  block ^entry:
    %address = select ptr %condition, %a, %b
    %value = load i64 %address
    return i64 %value
}
"""

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

    def test_source_value_outcomes_keep_authored_checks_without_budgets(self):
        reference = """global @value : i32 [storage=readonly] = 7
function @constant_read() -> i32 {
  block ^entry:
    return i32 7
}
"""
        predicate = "# explicit-outcomes\n\nnone(load) in @constant_read\n"
        for owner in ("pa10", "pa16"):
            with self.subTest(owner=owner):
                base = self.fixture(f"{owner}/tests/general/value")
                base.with_suffix(".ref").write_text(reference)
                self.sidecar(base).write_text(predicate)
                self.assertEqual(self.audit().returncode, 0)
                self.regenerate(base.parent)
                self.assertEqual(self.sidecar(base).read_text(), predicate)
                candidate = self.root / "candidate.lowir"
                for text, status in [
                    (reference, 0),
                    (reference.replace("    return i32 7",
                                       "    %value = load i32 @value\n"
                                       "    return i32 %value"), 1),
                ]:
                    candidate.write_text(text)
                    result = subprocess.run(
                        ["perl", str(EXPECT), str(candidate), str(self.sidecar(base))],
                        capture_output=True, text=True)
                    self.assertEqual(result.returncode, status, result.stdout + result.stderr)

    def lowir_choice(self) -> Path:
        base = self.fixture("pa32/tests/o2/x")
        base.with_suffix(".t").write_text(POINTER_CHOICE)
        base.with_name("x.ref").write_text(SELECT_CHOICE)
        self.sidecar(base).write_text("instructions <= 8\ncount(load) <= 1\n")
        return base

    def test_default_outcomes_still_follow_the_reference(self):
        base = self.lowir_choice()
        self.regenerate(base.parent)
        self.assertIn("none(phi)", self.sidecar(base).read_text())
        self.assertEqual(self.audit().returncode, 0)

    def test_explicit_outcomes_allow_an_equivalent_phi_and_keep_bounds(self):
        base = self.lowir_choice()
        self.regenerate(base.parent, "--add-budgets")
        with self.sidecar(base).open("a") as stream:
            stream.write("\n# explicit-outcomes\n")
        self.assertNotEqual(self.audit().returncode, 0)
        self.regenerate(base.parent)
        sidecar = self.sidecar(base)
        contract = sidecar.read_text()
        self.assertNotIn("none(phi)", contract)
        self.assertIn("# budget", contract)
        self.assertIn("instructions <= 8", contract)
        self.assertIn("count(load) <= 1", contract)
        self.assertEqual(self.audit().returncode, 0)
        for text, status in [(SELECT_CHOICE, 0), (POINTER_CHOICE, 0),
                             (POINTER_CHOICE.replace("    return i64 %value",
                                                     "    %extra = load i64 %address\n"
                                                     "    return i64 %value"), 1)]:
            candidate = self.root / "candidate.lowir"
            candidate.write_text(text)
            result = subprocess.run(["perl", str(EXPECT), str(candidate), str(sidecar)],
                                    capture_output=True, text=True)
            self.assertEqual(result.returncode, status, result.stdout + result.stderr)
        self.regenerate(base.parent)
        self.assertEqual(sidecar.read_text(), contract)

    def test_outcome_policy_does_not_hide_a_generated_block_header(self):
        base = self.lowir_choice()
        self.regenerate(base.parent, "--add-budgets")
        original = self.sidecar(base).read_text()
        for text in ["# explicit-outcomes\n" + original,
                     original.replace("instructions <=", "# explicit-outcomes\ninstructions <=", 1),
                     original + "# explicit-outcomes\n"]:
            with self.subTest(text=text):
                self.sidecar(base).write_text(text)
                self.regenerate(base.parent)
                updated = self.sidecar(base).read_text()
                self.assertTrue(updated.startswith("# explicit-outcomes\n\n# budget"))
                self.assertNotIn("none(phi)", updated)
                self.assertIn("instructions <= 8", updated)
                self.assertEqual(self.audit().returncode, 0)

    def test_indexed_field_contract_accepts_a_branched_single_load(self):
        fixture = REPO_ROOT / "pa32/tests/o2/500-indexed-bit-loads.t"
        text = fixture.read_text()
        for name in ("shifted", "inverted", "high_bit"):
            pattern = re.compile(r"(function @" + name + r"\([^}]+)\n}")
            match = pattern.search(text)
            self.assertIsNotNone(match)
            original = match.group(0)
            replacement = re.sub(r"    %va = load typed i64 %a\n", "", original)
            replacement = re.sub(r"    %vb = load typed i64 %b\n", "", replacement)
            replacement = replacement.replace(
                "    %v = phi i64 [^left: %va, ^right: %vb]",
                "    %address = phi ptr [^left: %a, ^right: %b]\n"
                "    %v = load typed i64 %address")
            self.assertNotEqual(replacement, original)
            self.assertIn("%address = phi ptr", replacement)
            text = text.replace(original, replacement)
        candidate = self.root / "field-choice.lowir"
        candidate.write_text(text)
        sidecar = fixture.with_name(fixture.stem + ".ref.expect")
        result = subprocess.run(["perl", str(EXPECT), str(candidate), str(sidecar)],
                                capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        result = subprocess.run(["perl", str(EXPECT), str(fixture), str(sidecar)],
                                capture_output=True, text=True)
        self.assertEqual(result.returncode, 1)
        self.assertIn("count(load)", result.stdout)

    def test_explicit_outcomes_do_not_disable_machine_contracts(self):
        base = self.machine_fixture()
        contract = subprocess.check_output(
            ["perl", str(REPO_ROOT / "scripts/make_ir_contract.pl"),
             "--memory", "--preserve", str(base.with_name("x.ref.mir"))], text=True)
        with self.sidecar(base).open("a") as stream:
            stream.write("\n# explicit-outcomes\n\n" + contract)
        self.regenerate(base.parent)
        self.assertIn(contract.strip(), self.sidecar(base).read_text())
        self.assertEqual(self.audit().returncode, 0)
        base.with_name("x.ref.mir").write_text("machine_ir\n")
        self.assertNotEqual(self.audit().returncode, 0)

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
