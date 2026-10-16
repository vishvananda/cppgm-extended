#!/usr/bin/env python3
"""Exercise bootstrap generations and reference-track gates without a Rust compiler."""
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
SPEC = importlib.util.spec_from_file_location("bootstrap", ROOT / "scripts/reference_bootstrap.py")
BOOT = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(BOOT)


@unittest.skipUnless(sys.platform == "linux" and Path("/usr/bin/g++").is_file(), "Linux GCC reference track")
class BootstrapTests(unittest.TestCase):
    def setUp(self):
        temp = tempfile.TemporaryDirectory(prefix="reference-bootstrap.")
        self.addCleanup(temp.cleanup)
        self.root = Path(temp.name)
        self.bundle = self.root / "source"
        for name in ("dev", "scripts", "shared", *[f"pa{n}" for n in range(1, 35)]):
            (self.bundle / name).mkdir(parents=True)
        # This tiny compiler surrogate delegates to g++ only in this test fixture.
        # VARIANT changes first-generation bytes but not compilation behavior.
        (self.bundle / "dev/tool.cpp").write_text('''#include <unistd.h>
#ifndef VARIANT
#define VARIANT 0
#endif
volatile int variant = VARIANT;
int main(int argc, char **argv) { (void)argc; argv[0] = const_cast<char *>("/usr/bin/g++"); execv(argv[0], argv); return 127; }
''')
        (self.bundle / "pa34/Makefile").write_text('''OBJ=$(INCEPTION_OBJ_ROOT_BASE)/selfhost
BIN=$(INCEPTION_OBJ_ROOT_BASE)/bin/selfhost
all:
\tmkdir -p $(OBJ) $(BIN)
\t$(CXX) -std=gnu++11 -O1 -MMD -MF $(OBJ)/tool.d -c ../dev/tool.cpp -o $(OBJ)/tool.o
\t$(CPPGM_HOST_CXX) $(OBJ)/tool.o -o $(BIN)/cppgm++-self
\tfor tool in pptoken posttoken ppexpr preproc abimangle lowir lowiropt lowir2native; do cp $(BIN)/cppgm++-self $(BIN)/$$tool-self; done
''')
        for n in range(1, 34):
            (self.bundle / f"pa{n}/Makefile").write_text('''test test-debuginfo:
\t@test -x "$(CPPGM_TEST_APP)"
\t@test -x "$(CPPGM_LOWIR2NATIVE_APP)"
\t@echo '1 1' >> "$$CPPGM_TEST_COUNTS_FILE"
''')
        self.manifest()
        self.candidate = self.root / "candidate"
        self.candidate.write_text('#!/bin/sh\nexec /usr/bin/g++ -DVARIANT=1 "$@"\n')
        self.candidate.chmod(0o755)
        self.boot = BOOT.Bootstrap(self.bundle, self.root / "build", "g++", 2)

    def manifest(self):
        files = {p.relative_to(self.bundle).as_posix(): BOOT.sha(p)
                 for p in self.bundle.rglob("*") if p.is_file() and p.name != "reference-bundle.json"}
        BOOT.write_json(self.bundle / "reference-bundle.json", {"commit": "fixture", "files": files})

    def test_student_allows_distinct_first_generation_and_stabilizes(self):
        with self.boot.locked():
            pair = self.boot.pair(self.candidate)
            self.assertTrue(pair["complete"])
            self.assertNotEqual(pair["baseline_receipt"]["objects"], pair["candidate_receipt"]["objects"])
            self.assertTrue(self.boot.final(self.candidate, "student"))
            self.assertFalse((self.bundle / "dev/cppgm++").exists())
            self.assertFalse((self.boot.work / "dev").is_symlink())
            BOOT.verify_bundle(self.bundle)

    def test_port_rejects_distinct_first_generation(self):
        with self.boot.locked():
            self.assertFalse(self.boot.final(self.candidate, "port"))

    def test_failed_compile_cannot_reuse_saved_objects(self):
        with self.boot.locked():
            pair = self.boot.pair(self.candidate)
            old = Path(pair["candidate"])
            self.candidate.write_text("#!/bin/sh\nexit 1\n")
            failed = self.boot.pair(self.candidate)
            self.assertFalse(failed["complete"])
            self.assertEqual(failed["candidate_receipt"]["objects"], {})
            self.assertTrue(old.is_file())

    def test_corrupt_cached_tool_is_rejected(self):
        with self.boot.locked():
            pair = self.boot.pair(self.candidate)
            result = pair["candidate_receipt"]
            path = Path(result["path"])
            (path / "bin/lowir-self").unlink()
            self.assertFalse(self.boot.valid(path, result["inputs"]))
            recovered = self.boot.pair(self.candidate)
            self.assertTrue(recovered["complete"])
            self.assertTrue((path / "bin/lowir-self").is_file())

    def test_dependency_directory_and_changed_header(self):
        directory = self.root / "objects"
        (directory / ".d").mkdir(parents=True)
        header = self.root / "host-header.h"
        header.write_text("before\n")
        (directory / ".d/tool.d").write_text(f"tool.o: {header}\n")
        deps = BOOT.dependencies(directory)
        self.assertEqual(deps, {str(header): BOOT.sha(header)})
        header.write_text("after\n")
        self.assertNotEqual(deps, BOOT.dependencies(directory))

    def test_progress_rejects_stale_receipts_and_reuses_unchanged_groups(self):
        shutil.copyfile(self.boot.host, self.candidate)
        scripts = ROOT / "pa35/scripts"
        options = {"reference-bundle": str(self.bundle), "reference-out": str(self.root / "build"),
                   "compiler": str(self.candidate), "lowir2native": str(self.candidate)}
        expression = 'use JSON::PP; print PA35::reference_identity(decode_json($ARGV[0]));'
        identity = subprocess.check_output(["perl", "-I", str(scripts), "-MPA35", "-e", expression,
                                            json.dumps(options)], text=True)
        values = self.root / "values"
        values.mkdir()
        groups = {g: [] for g in ("correctness", "speed", "codegen")}
        for line in (ROOT / "pa35/gates.txt").read_text().splitlines():
            if line.startswith("#") or not line.strip():
                continue
            group, name, metric, op, limit, *rest = line.split()
            groups[group].append(f"{metric}\t{limit}\n")
        for group, lines in groups.items():
            path = values / f"values-{group}.tsv"
            path.write_text("".join(lines))
            BOOT.write_json(values / f"receipt-{group}.json", {"identity": identity, "values": BOOT.sha(path)})
        # Refreshing identical correctness must not invalidate later measurements.
        os.utime(values / "values-correctness.tsv", (2000000000, 2000000000))
        command = ["perl", str(scripts / "progress.pl"), "--out", str(values)]
        for key, value in options.items():
            command += ["--" + key, value]
        passed = subprocess.run(command, capture_output=True, text=True)
        self.assertEqual(passed.returncode, 0, passed.stderr + passed.stdout)
        with self.candidate.open("ab") as stream:
            stream.write(b"changed compiler")
        stale = subprocess.run(command, capture_output=True, text=True)
        self.assertEqual(stale.returncode, 1, stale.stderr)
        self.assertIn("stale or missing reference provenance", stale.stdout)
        self.assertIn("earlier groups missing, stale or failing", stale.stdout)

    def test_cli_rejects_build_wrappers(self):
        result = subprocess.run(["python3", str(ROOT / "scripts/reference_bootstrap.py"), "pair",
                                 "--bundle", str(self.bundle), "--compiler", str(self.candidate),
                                 "--out", str(self.root / "build")], capture_output=True, text=True)
        self.assertEqual(result.returncode, 2)
        self.assertIn("actual Linux release executable", result.stderr)

    def test_inventory_extra_missing_and_changed_are_rejected(self):
        expected = {"exit": 0, "objects": {"a.o": "x"}, "binaries": dict.fromkeys(BOOT.TOOLS, "y")}
        self.assertTrue(BOOT.same(expected, expected))
        for objects in ({}, {"a.o": "x", "b.o": "z"}, {"a.o": "different"}):
            self.assertFalse(BOOT.same(expected, dict(expected, objects=objects)))
        self.assertFalse(BOOT.same(expected, dict(expected, exit=1)))
        self.assertFalse(BOOT.same(expected, dict(expected, binaries={})))

    def test_changed_source_and_added_source_are_rejected(self):
        source = self.bundle / "dev/tool.cpp"
        original = source.read_bytes()
        source.write_text("changed\n")
        with self.assertRaisesRegex(ValueError, "changed"):
            BOOT.verify_bundle(self.bundle)
        source.write_bytes(original)
        (self.bundle / "dev/extra.cpp").write_text("extra\n")
        with self.assertRaisesRegex(ValueError, "inventory"):
            BOOT.verify_bundle(self.bundle)

    def test_working_source_copy_cannot_drift(self):
        with self.boot.locked():
            (self.boot.work / "dev/tool.cpp").write_text("changed working source\n")
            BOOT.verify_bundle(self.bundle)
            with self.assertRaisesRegex(ValueError, "working reference source changed"):
                self.boot.generation("B", self.boot.host)

    def test_rebuilt_helpers_are_bound_for_every_stage(self):
        for stage in range(1, 34):
            bindings = dict(p.split("=", 1) for p in BOOT.bindings(Path("/generation"), stage, Path("/host")))
            for name in ("CPPGM_DRIVER_APP", "LOWIROPT_APP", "LOWIR2NATIVE_APP", "NATIVE_REFERENCE_APP",
                         "CPPGM_LOWIR_BEHAVIOR_APP", "CPPGM_CPPGM_APP", "CPPGM_LOWIROPT_APP", "CPPGM_LOWIR2NATIVE_APP"):
                self.assertTrue(bindings[name].startswith("/generation/"), name)

    def test_suite_failure_marker_and_missing_counts_fail(self):
        with self.boot.locked():
            result = self.boot.generation("B", self.boot.host)
            makefile = self.boot.work / "pa1/Makefile"
            makefile.write_text("test:\n\ttouch .test_failed\n\techo '1 1' >> \"$$CPPGM_TEST_COUNTS_FILE\"\n")
            self.assertFalse(self.boot.suites(result))
            makefile.write_text("test:\n\ttrue\n")
            self.assertFalse(self.boot.suites(result))


class ReferenceGateTests(unittest.TestCase):
    def test_check_routes_candidate_and_codegen_separately(self):
        # Execute check.pl with a recording PA35 module, avoiding slow simulation.
        with tempfile.TemporaryDirectory(prefix="reference-gates.") as temp:
            root = Path(temp)
            scripts = root / "pa35/scripts"
            scripts.mkdir(parents=True)
            shutil.copy(ROOT / "pa35/scripts/check.pl", scripts / "check.pl")
            shutil.copy(ROOT / "pa35/gates.txt", root / "pa35/gates.txt")
            module = r'''package PA35;
use JSON::PP; use File::Path qw(make_path); use File::Basename qw(dirname);
our $PA35 = 'ROOT/pa35'; our $ROOT = 'ROOT';
sub reference_options { ('reference-bundle=s','reference-out=s','compiler=s','lowir2native=s') }
sub reference_identity { 'identity' }
sub reference_pair { ('/host-built-reference', '/candidate-built-reference') }
sub sha256_file { 'hash' }
sub slurp { open my $f, '<', $_[0] or die $!; local $/; <$f> }
sub spit { make_path(dirname($_[0])); open my $f, '>', $_[0] or die $!; print $f $_[1] }
sub workload_args { ($_[0], '-o', $_[1]) }
sub timed_run { {status=>0,user=>1,output=>''} }
sub run_capture {
  my @a=@_; open my $f,'>>','ROOT/commands'; print $f encode_json(\@a),"\n"; close $f;
  my ($script)=grep {/\.pl$/} @a;
  if ($script =~ /measure/) {
    my ($i)=grep {$a[$_] eq '--out'} 0..$#a; my $out=$a[$i+1];
    spit("$out/summary.tsv", $ENV{FIXTURE_MODE} eq 'missing-identical' ? "level\tmetric\ta\tb\tratio\n"
      : "level\tmetric\ta\tb\tratio\nO0\tidentical\t1\t1\t1\nO3\tidentical\t1\t1\t1\n");
    spit("$out/summary.txt", 'text bytes: (1.000x)');
  }
  return (($ENV{FIXTURE_MODE} eq 'failed-command' ? 1 : 0), '1/1 tests pass');
}
1;'''.replace("ROOT", str(root))
            # Preserve the Perl variable name; only placeholder path strings vary.
            module = module.replace("our $" + str(root), "our $ROOT")
            (scripts / "PA35.pm").write_text(module)
            compiler = root / "rust-compiler"
            compiler.write_text("fixture")
            command = ["perl", str(scripts / "check.pl"), "--reference-bundle", str(root),
                       "--compiler", str(compiler), "--lowir2native", str(compiler)]
            for group in ("correctness", "codegen"):
                subprocess.run(command + ["--group", group], capture_output=True, check=False)
            calls = [json.loads(line) for line in (root / "commands").read_text().splitlines()]
            measure = next(c for c in calls if any("measure.pl" in str(a) for a in c))
            self.assertEqual(measure[measure.index("--seed") + 1], "/host-built-reference")
            self.assertEqual(measure[measure.index("--self") + 1], "/candidate-built-reference")
            for script in ("behavior.pl", "generated.pl", "kernels.pl"):
                call = next(c for c in calls if any(script in str(a) for a in c))
                self.assertEqual(call[call.index("--compiler") + 1], str(compiler))
            for mode in ("failed-command", "missing-identical"):
                env = dict(os.environ, FIXTURE_MODE=mode)
                result = subprocess.run(command + ["--group", "codegen"], env=env, capture_output=True, text=True)
                self.assertEqual(result.returncode, 1, result.stderr)
                self.assertIn("identical\t0", (root / "pa35/obj/check/values-codegen.tsv").read_text())
            result = subprocess.run(command + ["--group", "correctness"], env=dict(os.environ, FIXTURE_MODE="failed-command"),
                                    capture_output=True, text=True)
            self.assertEqual(result.returncode, 1, result.stderr)
            self.assertIn("behavior-failures\terror", (root / "pa35/obj/check/values-correctness.tsv").read_text())


if __name__ == "__main__":
    unittest.main()
