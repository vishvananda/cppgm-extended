#!/usr/bin/env python3
"""Check the PA35 harness without the course compiler: the gate table, the
progress ladder (progress.pl), the release check's margins (calibrate.pl
--from), the frozen workload's manifest, and the behavior and generated
runners driven with the host compiler standing in for cppgm++."""

from pathlib import Path
import os
import re
import shutil
import subprocess
import tempfile
import unittest


REPO_ROOT = Path(__file__).resolve().parents[2]
PA35 = REPO_ROOT / "pa35"
SCRIPTS = PA35 / "scripts"
CALIBRATE = PA35 / "maintainer/calibrate.pl"


def read_gates():
    gates = []
    for line in (PA35 / "gates.txt").read_text().splitlines():
        if not line.strip() or line.lstrip().startswith("#"):
            continue
        fields = line.split()
        gate = dict(zip(("group", "name", "metric", "op", "limit"), fields[:5]))
        gate["limit"] = float(gate["limit"])
        gate["from"] = float(fields[5]) if len(fields) > 5 else None
        gate["step"] = float(fields[6]) if len(fields) > 6 else None
        gates.append(gate)
    return gates


def rungs(gate):
    """progress.pl's rung thresholds: limit x (1 + step)^k up to `from`."""
    if gate["op"] != "<=" or not gate["from"] or not gate["step"]:
        return [gate["limit"]]
    thresholds = [gate["limit"]]
    while thresholds[-1] < gate["from"] * (1 - 1e-12):
        thresholds.append(thresholds[-1] * (1 + gate["step"]))
    return thresholds


def write_values(directory, values, gates, groups=("correctness", "speed", "codegen")):
    for group in groups:
        rows = "".join(f"{g['metric']}\t{values[g['metric']]}\n" for g in gates if g["group"] == group)
        (directory / f"values-{group}.tsv").write_text(rows)


def passing_values(gates):
    """Every gate at its limit (==) or well inside it (<=)."""
    return {g["metric"]: (int(g["limit"]) if g["op"] == "==" else g["limit"] / 1.5) for g in gates}


class GateTableTests(unittest.TestCase):
    def test_seventeen_gates_in_three_ordered_groups(self):
        gates = read_gates()
        self.assertEqual(len(gates), 17)
        order = [g["group"] for g in gates]
        self.assertEqual(sorted(set(order), key=order.index), ["correctness", "speed", "codegen"])
        self.assertEqual(order, sorted(order, key=["correctness", "speed", "codegen"].index))
        self.assertEqual(len({g["name"] for g in gates}), 17)
        self.assertEqual(len({g["metric"] for g in gates}), 17)

    def test_every_measured_gate_has_a_ladder(self):
        for gate in read_gates():
            if gate["op"] == "==":
                self.assertIsNone(gate["from"], gate["name"])
                continue
            self.assertEqual(gate["op"], "<=", gate["name"])
            self.assertGreater(gate["from"], gate["limit"], gate["name"])
            self.assertGreater(gate["step"], 0, gate["name"])
            self.assertLessEqual(gate["step"], 0.05, gate["name"])


class ProgressTests(unittest.TestCase):
    def setUp(self):
        temp = tempfile.TemporaryDirectory(prefix="pa35-progress.")
        self.addCleanup(temp.cleanup)
        self.dir = Path(temp.name)
        self.gates = read_gates()

    def progress(self):
        return subprocess.run(["perl", str(SCRIPTS / "progress.pl"), "--out", str(self.dir)],
                              capture_output=True, text=True)

    def test_nothing_measured_passes_no_rung(self):
        result = self.progress()
        self.assertEqual(result.returncode, 1)
        total = sum(len(rungs(g)) for g in self.gates)
        self.assertIn(f"===== TEST SUMMARY: 0 / {total} TESTS PASSED =====", result.stdout)

    def test_all_gates_pass_every_rung(self):
        write_values(self.dir, passing_values(self.gates), self.gates)
        result = self.progress()
        self.assertEqual(result.returncode, 0, result.stdout)
        total = sum(len(rungs(g)) for g in self.gates)
        self.assertIn(f"===== ALL TESTS PASSED SUCCESSFULLY! ({total}/{total}) =====", result.stdout)

    def test_a_value_between_rungs_passes_the_rungs_above_it(self):
        values = passing_values(self.gates)
        gate = next(g for g in self.gates if g["metric"] == "O0-own")
        values["O0-own"] = gate["limit"] * 1.2
        write_values(self.dir, values, self.gates)
        result = self.progress()
        self.assertEqual(result.returncode, 1)
        expected = sum(1 for t in rungs(gate) if values["O0-own"] <= t * (1 + 1e-12))
        self.assertIn(f"{gate['name']}: FAIL ({expected}/{len(rungs(gate))})", result.stdout)

    def test_a_group_older_than_an_earlier_group_is_not_counted(self):
        write_values(self.dir, passing_values(self.gates), self.gates)
        codegen = self.dir / "values-codegen.tsv"
        speed = self.dir / "values-speed.tsv"
        os.utime(codegen, (speed.stat().st_mtime - 100,) * 2)
        result = self.progress()
        self.assertEqual(result.returncode, 1)
        self.assertIn("measured before the earlier groups", result.stdout)
        for gate in self.gates:
            if gate["group"] == "codegen":
                self.assertIn(f"{gate['name']}: FAIL (0/{len(rungs(gate))})", result.stdout)


class CalibrateTests(unittest.TestCase):
    def setUp(self):
        temp = tempfile.TemporaryDirectory(prefix="pa35-calibrate.")
        self.addCleanup(temp.cleanup)
        self.dir = Path(temp.name)
        self.gates = read_gates()
        self.out = self.dir / "calibration.tsv"

    def calibrate(self, values, student=None):
        """Runs calibrate.pl on a course build with `values` and a student
        build (speed and codegen groups) with `student` (default: the same)."""
        for build, given in (("course", values), ("student", student or values)):
            (self.dir / build).mkdir(exist_ok=True)
            groups = ("correctness", "speed", "codegen") if build == "course" else ("speed", "codegen")
            write_values(self.dir / build, given, self.gates, groups)
        return subprocess.run(["perl", str(CALIBRATE), "--from", str(self.dir), "--out", str(self.out)],
                              capture_output=True, text=True)

    def gate_rows(self):
        """gate name -> (course status, student status)."""
        rows = {}
        for line in self.out.read_text().splitlines():
            if line.startswith("gate\t"):
                fields = line.split("\t")
                rows[fields[1]] = (fields[7], fields[10])
        return rows

    def test_values_well_inside_the_limits_pass(self):
        result = self.calibrate(passing_values(self.gates))
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        rows = self.gate_rows()
        self.assertEqual(len(rows), 17)
        self.assertTrue(all(row == ("pass", "pass") for row in rows.values()), rows)
        self.assertIn("RELEASE CHECK PASSED (17/17)", result.stdout)
        text = self.out.read_text()
        for key in ("date", "source", "host compiler", "libstdc++"):
            self.assertRegex(text, rf"(?m)^meta\t{re.escape(key)}\t\S")

    def test_a_value_inside_the_limit_but_not_the_margin_is_short(self):
        values = passing_values(self.gates)
        gate = next(g for g in self.gates if g["metric"] == "O0-proxy")
        values["O0-proxy"] = gate["limit"] / 1.001
        result = self.calibrate(values)
        self.assertEqual(result.returncode, 1)
        self.assertEqual(self.gate_rows()[gate["name"]], ("SHORT", "short"))
        self.assertIn("RELEASE CHECK FAILED: 1 of 17 gates", result.stdout)

    def test_the_student_build_reports_its_margins_and_fails_only_a_gate(self):
        values = passing_values(self.gates)
        student = dict(values)
        gate = next(g for g in self.gates if g["metric"] == "O3-proxy")
        student["O3-proxy"] = gate["limit"] / 1.001
        result = self.calibrate(values, student)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(self.gate_rows()[gate["name"]], ("pass", "short"))
        student["O3-proxy"] = gate["limit"] * 1.001
        result = self.calibrate(values, student)
        self.assertEqual(result.returncode, 1)
        self.assertEqual(self.gate_rows()[gate["name"]], ("pass", "FAIL"))

    def test_the_student_build_uses_the_exported_makefile_flags(self):
        export = (REPO_ROOT / "scripts/export_student_repo.sh").read_text()
        heredoc = export[export.index("cat > \"$dest/dev/Makefile\""):]
        exported = re.search(r"^CC_FLAGS \?= (.*?) \$\(CPPGM_STDLIB_FLAGS\)$", heredoc, re.M).group(1)
        self.assertNotIn("HOST_ALLOC_LIBS", heredoc[:heredoc.index("\nEOF\n")])
        calibrate = CALIBRATE.read_text()
        self.assertIn(f"'CC_FLAGS={exported}', 'HOST_ALLOC_LIBS='", calibrate)

    def test_a_failing_or_missing_gate_fails(self):
        values = passing_values(self.gates)
        values["behavior-failures"] = 1
        values["text"] = "missing"
        result = self.calibrate(values)
        self.assertEqual(result.returncode, 1)
        rows = self.gate_rows()
        self.assertEqual(rows["behavior"][0], "FAIL")
        self.assertEqual(rows["text-size"][0], "FAIL")


class WorkloadAndRunnerTests(unittest.TestCase):
    def test_scripts_compile(self):
        for script in sorted(SCRIPTS.glob("*.pl")) + [CALIBRATE]:
            result = subprocess.run(["perl", "-c", str(script)], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, f"{script}: {result.stderr}")

    def test_workload_matches_its_manifest(self):
        result = subprocess.run(["perl", f"-I{SCRIPTS}", "-MPA35", "-e", "print PA35::verify_workload()"],
                                capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        listed = len((PA35 / "workload/MANIFEST.sha256").read_text().splitlines())
        self.assertEqual(result.stdout, str(listed))

    @unittest.skipUnless(shutil.which("g++"), "needs g++")
    def test_behavior_and_generated_runners_with_the_host_compiler(self):
        host = shutil.which("g++")
        programs = sorted((PA35 / "tests/behavior").glob("*.cpp"))[:2]
        result = subprocess.run(["perl", str(SCRIPTS / "behavior.pl"), "--compiler", host, "--levels", "O0,O2",
                                 "--jobs", "2"] + [str(p) for p in programs],
                                capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("behavior: 2/2 tests pass at O0 O2", result.stdout)
        result = subprocess.run(["perl", str(SCRIPTS / "generated.pl"), "--compiler", host, "--seeds", "1",
                                 "--levels", "O1", "--jobs", "2"], capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("generated: 5/5 programs pass at O1", result.stdout)


if __name__ == "__main__":
    unittest.main()
