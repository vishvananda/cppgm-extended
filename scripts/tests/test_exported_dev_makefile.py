#!/usr/bin/env python3

import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import time
import unittest


REPO_ROOT = Path(__file__).resolve().parents[2]
EXPORT_SCRIPT = REPO_ROOT / "scripts" / "export_student_repo.sh"
TESTS_WORKFLOW = REPO_ROOT / ".github" / "workflows" / "tests.yml"

REFERENCE_TARGETS = [
    "abimangle", "pptoken", "posttoken", "ppexpr", "preproc",
    "cppgm++", "lowir",
    "lowir2native", "lowiropt",
]

ASSIGNMENT_REFERENCE_TARGETS = {
    **{f"pa{i}": "cppgm++" for i in range(5, 8)},
    **{f"pa{i}": "cppgm++" for i in range(10, 24)},
    **{f"pa{i}": "cppgm++" for i in range(25, 33)},
    "pa1": "pptoken",
    "pa2": "posttoken",
    "pa3": "ppexpr",
    "pa4": "preproc",
    "pa8": "lowir",
    "pa9": "abimangle",
    "pa24": "lowir2native",
    "pa33": "lowiropt",
    "pa34": "lowir2native",
    "pa38": "cppgm++",
}


def exported_dev_makefile():
    script = EXPORT_SCRIPT.read_text()
    marker = "cat > \"$dest/dev/Makefile\" <<'EOF'\n"
    start = script.index(marker) + len(marker)
    end = script.index("\nEOF\n", start)
    return script[start:end] + "\n"


def shell_array(script: str, name: str) -> list[str]:
    match = re.search(rf"^{re.escape(name)}=\(\n(?P<body>.*?)^\)$", script, re.M | re.S)
    if not match:
        raise AssertionError(f"missing shell array {name}")
    return [line.strip() for line in match.group("body").splitlines() if line.strip()]


class ExportedDevMakefileTests(unittest.TestCase):
    def test_export_copies_public_documents_without_run_notes(self):
        script = EXPORT_SCRIPT.read_text()
        start = script.index('copy_tracked_paths() {')
        end = script.index('\n}\n', start) + len('\n}\n')
        documents = (REPO_ROOT / 'scripts/student_export_documents.txt').read_text().splitlines()
        self.assertEqual(documents, sorted(set(documents)))
        for relative in documents:
            self.assertTrue((REPO_ROOT / relative).is_file(), relative)
        fixtures = {
            'scripts/check_macho.py': '# optional public target checker\n',
            'scripts/run_resource_limited.py': '# public resource wrapper\n',
            'scripts/check_inline_assembly.pl': '# optional assembly checker\n',
            'dev/src/asm_model.h': '// shared assembly vocabulary\n',
            'dev/src/x86_asm_model.h': '// architectural encoding facts\n',
            'pa29/Makefile': '# optional assignment\n',
            'pa29/tests/source/100-stream.t': 'source fixture\n',
            'pa29/tests/source/100-stream.assembly-profile': 'gnu\n',
            'pa29/tests/source/100-stream.driver.cpp': 'native observer\n',
            'pa8/tests/general/100-valid.t': 'program input\n',
            'pa27/tests/behavior/100-layout.ref.inspect.plan': 'object contract\n',
            'pa27/tests/controls/200-section-token-safe.t': 'graded control\n',
            'pa30/tests/modern-cxx/compile/600-extension.t': 'optional extension\n',
            'pa30/tests/modern-cxx/compile/600-extension.cxx-standard': 'c++14\n',
            'pa30/tests/modern-cxx/compile/600-extension.ref.exit_status': 'EXIT_SUCCESS\n',
            'pa32/tests/modern-cxx/link/600-abi.t': 'optional ABI extension\n',
        }
        excluded = [
            'scripts/private_audit.py',
            # Run only by the maintainer's pending-audit checker.
            'pa28/tests/controls/100-audit-standalone-function-try.audit',
            'pa27/tests/controls/200-audit-global-variable-name.cpp',
            'pa10/plan.md', 'pa10/audit.md', 'pa10/implementation.md',
            'pa33/optimization-path-unification-plan.md',
            'pa33/IMPLEMENTATION.MD', 'pa34/maintainer/design-notes.md',
            'pa35/notes/README.md', 'pa35/run-notes.md',
            'doc/backend-review/newcomer-notes.md',
            'doc/backend-review/mutations.txt', 'doc/compiler-native-symbol-owners.tsv',
        ]
        fixtures = {relative: content.encode() for relative, content in fixtures.items()}
        with tempfile.TemporaryDirectory(prefix='export-documents.') as temp:
            source = Path(temp) / 'source'
            dest = Path(temp) / 'export'
            source.mkdir()
            dest.mkdir()
            for relative in documents:
                fixtures[relative] = (REPO_ROOT / relative).read_bytes()
            for relative, content in {**fixtures, **dict.fromkeys(excluded, b'run notes\n')}.items():
                path = source / relative
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(content)
            subprocess.run(['git', 'init', '-q', str(source)], check=True)
            subprocess.run(['git', '-C', str(source), 'add', '.'], check=True)
            subprocess.run(
                ['bash', '-euc', script[start:end] +
                 '\nrepo_root="$1"\ndest="$2"\nscript_dir="$3"\ncopy_tracked_paths .',
                 'bash', str(source), str(dest), str(EXPORT_SCRIPT.parent)], check=True,
            )
            self.assertEqual({str(path.relative_to(dest)) for path in dest.rglob('*')
                              if path.is_file()}, set(fixtures))
            for relative, content in fixtures.items():
                self.assertEqual((dest / relative).read_bytes(), content)

    def test_script_dependencies_resolve_findbin_paths(self):
        script = EXPORT_SCRIPT.read_text()
        start = script.index('check_exported_script_references() {')
        end = script.index('\n}\n', start) + len('\n}\n')
        with tempfile.TemporaryDirectory(prefix='export-dependencies.') as temp:
            dest = Path(temp)
            (dest / 'scripts').mkdir()
            (dest / 'scripts/checker.pl').write_text('my $tool = "$FindBin::Bin/worker.py";\n')
            command = ['bash', '-euc', script[start:end] +
                       '\ndest="$1"\nSTUDENT_ROOT_MAKEFILE_DROPPED_TARGETS=\n'
                       'check_exported_script_references', 'bash', str(dest)]
            missing = subprocess.run(command, capture_output=True, text=True)
            self.assertNotEqual(missing.returncode, 0, missing.stdout + missing.stderr)
            self.assertIn('worker.py', missing.stderr)
            (dest / 'scripts/worker.py').write_text('# supplied\n')
            supplied = subprocess.run(command, capture_output=True, text=True)
            self.assertEqual(supplied.returncode, 0, supplied.stdout + supplied.stderr)

    def test_pa34_export_keeps_course_without_maintainer_dependencies(self):
        script = EXPORT_SCRIPT.read_text()
        start = script.index('prune_student_pa34() {')
        end = script.index('\n}\n', start) + len('\n}\n')
        with tempfile.TemporaryDirectory(prefix='exported-pa34.') as temp:
            root = Path(temp)
            shutil.copytree(REPO_ROOT / 'pa34', root / 'pa34', symlinks=True,
                            ignore=shutil.ignore_patterns('*.my*', '*.check*', '*.ref.program'))
            shutil.copytree(REPO_ROOT / 'scripts', root / 'scripts', symlinks=True)
            shutil.copy(REPO_ROOT / 'Makefile', root / 'Makefile')
            # Driver links now reuse fixtures from their earliest owning PA.
            # Include those targets as the complete exporter does.
            for path in (root / 'pa34/tests').rglob('*'):
                if not path.is_symlink():
                    continue
                source = (REPO_ROOT / path.relative_to(root)).resolve(strict=True)
                destination = root / source.relative_to(REPO_ROOT)
                if not destination.exists():
                    destination.parent.mkdir(parents=True, exist_ok=True)
                    shutil.copy2(source, destination)
            subprocess.run(['bash', '-c', script[start:end] + '\ndest="$1"\nprune_student_pa34',
                            'bash', str(root)], check=True)
            pa = root / 'pa34'
            self.assertFalse((pa / 'maintainer').exists())
            self.assertFalse((pa / 'tests/regression').exists())
            self.assertEqual(list(pa.rglob('*.ref.ir')), [])
            for path in (pa / 'tests').rglob('*'):
                if path.is_symlink():
                    self.assertTrue(path.exists(), str(path))
            makefile = (pa / 'Makefile').read_text()
            for script_name in re.findall(r'\.\./scripts/[\w.-]+', makefile):
                self.assertTrue((pa / script_name).is_file(), script_name)
            result = subprocess.run(['make', '-n', '-C', str(pa), 'test', 'test-debuginfo',
                                     'CPPGM_SKIP_DEV_REBUILD=1'],
                                    capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('check_cppgm_native_programs.pl', result.stdout)
            self.assertIn('tests/debuginfo/o3', result.stdout)
            self.assertNotIn('census', result.stdout)
            self.assertNotIn('check_ir_envelope', result.stdout)
            for target in ('test-regression', 'test-perf', 'test-perf-refs'):
                result = subprocess.run(['make', '-n', '-C', str(pa), target],
                                        capture_output=True, text=True)
                self.assertNotEqual(result.returncode, 0, target)

    def test_export_drops_regression_lanes(self):
        script = EXPORT_SCRIPT.read_text()
        start = script.index('prune_student_regression_lanes() {')
        end = script.index('\n}\n', start) + len('\n}\n')
        with tempfile.TemporaryDirectory(prefix='exported-regression-lanes.') as temp:
            root = Path(temp)
            for pa in ('pa24', 'pa33', 'pa34', 'pa37'):
                shutil.copytree(REPO_ROOT / pa, root / pa, symlinks=True,
                                ignore=shutil.ignore_patterns('*.my*', '*.check*', '*.ref.program'))
            shutil.copytree(REPO_ROOT / 'scripts', root / 'scripts', symlinks=True)
            (root / 'dev').mkdir()
            shutil.copy(REPO_ROOT / 'dev/frontend_source_sets.mk', root / 'dev')
            shutil.copy(REPO_ROOT / 'Makefile', root / 'Makefile')
            subprocess.run(['bash', '-euo', 'pipefail', '-c',
                            script[start:end] + '\ndest="$1"\nprune_student_regression_lanes',
                            'bash', str(root)], check=True)
            # A script only the PA33 controls ran goes; one a course lane runs stays.
            self.assertFalse((root / 'scripts/check_lowir_inline_limit.pl').exists())
            self.assertTrue((root / 'scripts/check_pa24_native_contracts.pl').is_file())
            # (PA37 names its compiler build in TEST_DEPS; the others skip theirs by the flag.)
            for name, skip_build in (('pa24', 'CPPGM_SKIP_DEV_REBUILD=1'), ('pa33', 'CPPGM_SKIP_DEV_REBUILD=1'),
                                     ('pa37', 'TEST_DEPS=')):
                with self.subTest(pa=name):
                    pa = root / name
                    self.assertFalse((pa / 'maintainer').exists())
                    self.assertFalse((pa / 'tests/regression').exists())
                    makefile = (pa / 'Makefile').read_text()
                    self.assertNotIn('maintainer', makefile)
                    for script_name in re.findall(r'\.\./scripts/[\w.-]+', makefile):
                        self.assertTrue((pa / script_name).is_file(), script_name)
                    result = subprocess.run(['make', '-n', '--no-print-directory', '-C', str(pa),
                                             'test', 'ref-test', skip_build],
                                            capture_output=True, text=True)
                    self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                    self.assertRegex(result.stdout, r'scripts/run_all_tests(_common)?\.pl')
                    self.assertNotIn('regression', result.stdout)
                    result = subprocess.run(['make', '-n', '-C', str(pa), 'test-regression', skip_build],
                                            capture_output=True, text=True)
                    self.assertNotEqual(result.returncode, 0, result.stdout)

    def test_student_default_excludes_solution_regressions(self):
        script = EXPORT_SCRIPT.read_text()
        start = script.index("sanitize_student_makefile_defaults() {")
        end = script.index("\n}\n", start) + len("\n}\n")
        sanitize = script[start:end]
        start = script.index("student_makefiles=(")
        end = script.index("sanitize_student_root_makefile", start)
        selection = script[start:end]
        with tempfile.TemporaryDirectory(prefix="exported-test-defaults.") as temp:
            root = Path(temp)
            (root / "Makefile").write_text("all:\n")
            for pa in (24, 33, 34):
                source = (REPO_ROOT / f"pa{pa}/Makefile").read_text()
                # Exercise the actual file selection as well as the rewrite;
                # a correct sanitizer does nothing for an omitted assignment.
                default = next(line for line in source.splitlines()
                               if line.startswith("test:"))
                # Other course lanes the default names (PA34's kernels) are
                # silent stand-ins here; the course lane is what prints.
                others = [name for name in default.split()[1:]
                          if name not in ("test-course", "test-regression")]
                directory = root / f"pa{pa}"
                directory.mkdir()
                (directory / "Makefile").write_text(
                    default + "\n"
                    "test-course:\n\t@echo course\n"
                    "test-regression:\n\t@echo regression\n"
                    + "".join(f"{name}:\n\t@true\n" for name in others)
                )
            subprocess.run(
                ["bash", "-c", sanitize + '\ndest="$1"\n' + selection,
                 "bash", str(root)], check=True,
            )
            for pa in (24, 33, 34):
                with self.subTest(pa=pa):
                    for target, expected in (("test", "course"),
                                             ("test-regression", "regression")):
                        result = subprocess.run(
                            ["make", "-s", "-C", str(root / f"pa{pa}"), target],
                            check=True, text=True, stdout=subprocess.PIPE,
                        )
                        self.assertEqual(result.stdout.strip(), expected)

    def test_student_root_makefile_drops_maintainer_targets(self):
        # test-cells walks the course's toolchain cells and test-harness runs
        # scripts that never ship: both are maintainer-only.  The export's
        # own check greps for the names in this list, so neither may leave it.
        script = EXPORT_SCRIPT.read_text()
        match = re.search(r"^STUDENT_ROOT_MAKEFILE_DROPPED_TARGETS='([^']*)'$", script, re.M)
        self.assertIsNotNone(match, "missing STUDENT_ROOT_MAKEFILE_DROPPED_TARGETS")
        dropped = match.group(1).split()
        for target in ("test-cells", "test-harness", "test-wasm-inception", "test-wasm-inception-nobuild"):
            self.assertIn(target, dropped)
        self.assertIn('for target in $STUDENT_ROOT_MAKEFILE_DROPPED_TARGETS; do', script)
        start = script.index("sanitize_student_root_makefile() {")
        end = script.index("\n}\n", start) + len("\n}\n")
        maintainer = (REPO_ROOT / "Makefile").read_text()
        for target in ("test-cells", "test-harness", "test-wasm-inception", "test-wasm-inception-nobuild"):
            self.assertRegex(maintainer, rf"(?m)^{target}:")
        with tempfile.TemporaryDirectory(prefix="exported-root-makefile.") as temp:
            makefile = Path(temp) / "Makefile"
            shutil.copy(REPO_ROOT / "Makefile", makefile)
            (Path(temp) / "scripts").mkdir()
            shutil.copy(REPO_ROOT / "scripts/test_run_id.mk", Path(temp) / "scripts/test_run_id.mk")
            subprocess.run(
                ["bash", "-euc", script[start:end] +
                 '\nsanitize_student_root_makefile "$1"', "bash", str(makefile)],
                check=True,
            )
            exported = makefile.read_text()
            for target in dropped:
                self.assertFalse(target in exported,
                                 f"the exported root Makefile still names {target}")
            # A student still picks a toolchain by prefixing one target.
            self.assertRegex(exported, r"(?m)^with-clang-%: require-clang$")
            self.assertRegex(exported, r"(?m)^with-clang-libcxx-%: require-clang-libcxx$")
            # Only the per-assignment test-% rule is left to match the names,
            # and it stops at once: there is no such assignment.  (Not make
            # -n: test-% runs its $(MAKE) lines even then.)
            for target in ("test-cells", "test-harness"):
                result = subprocess.run(
                    ["make", "-s", "-C", temp, target],
                    capture_output=True, text=True, timeout=60,
                )
                self.assertEqual(result.returncode, 2, target + result.stdout + result.stderr)
                self.assertIn("unknown assignment: " + target.removeprefix("test-"),
                              result.stderr)

    def test_identical_relink_preserves_frontend_mtime(self):
        self.check_identical_relink(exported_dev_makefile())

    def test_solution_identical_relink_preserves_frontend_mtime(self):
        self.check_identical_relink((REPO_ROOT / "dev/Makefile").read_text())

    def check_identical_relink(self, makefile):
        with tempfile.TemporaryDirectory(prefix="exported-dev-makefile.") as temp:
            root = Path(temp)
            dev = root / "dev"
            src = dev / "src"
            src.mkdir(parents=True)
            (dev / "Makefile").write_text(makefile)
            if "include lowir_library.mk" in makefile:
                (dev / "lowir_library.mk").write_text(
                    (REPO_ROOT / "dev/lowir_library.mk").read_text()
                )

            targets = [
                "abimangle", "pptoken", "posttoken", "ppexpr",
                "preproc", "lowir", "lowiropt", "lowir2native",
                "cppgm++",
            ]
            source_sets = []
            for target in targets:
                objects = "shared" if target == "pptoken" else ""
                source_sets.append(
                    f"FRONTEND_OBJ_BASENAMES_{target} := {objects}\n"
                )
            (dev / "frontend_source_sets.mk").write_text("".join(source_sets))
            (dev / "pptoken.cpp").write_text("entry-v1\n")
            (src / "shared.cpp").write_text("shared-v1\n")

            compiler = root / "fake-cxx"
            compiler.write_text(
                "#!/usr/bin/env python3\n"
                "import hashlib, pathlib, sys\n"
                "args = sys.argv[1:]\n"
                "out = pathlib.Path(args[args.index('-o') + 1])\n"
                "inputs = [pathlib.Path(a) for a in args if pathlib.Path(a).is_file()]\n"
                "payload = b''.join(path.read_bytes() for path in inputs)\n"
                "out.parent.mkdir(parents=True, exist_ok=True)\n"
                "out.write_bytes(hashlib.sha256(payload).digest())\n"
                "if '-MF' in args:\n"
                "    dep = pathlib.Path(args[args.index('-MF') + 1])\n"
                "    dep.parent.mkdir(parents=True, exist_ok=True)\n"
                "    dep.write_text(str(out) + ':\\n')\n"
            )
            compiler.chmod(0o755)

            command = [
                "make", "-s", "-C", str(dev), "pptoken",
                "CPPGM_TEST_RUNNER=0", f"CXX={compiler}", f"CPPGM_HOST_CXX={compiler}",
                f"OBJ={root / 'obj'}", f"GENERATED={root / 'obj' / 'generated'}",
            ]
            subprocess.run(command, check=True)
            binary = dev / "pptoken"
            original_mtime = binary.stat().st_mtime_ns
            original_content = binary.read_bytes()

            result = subprocess.run(
                command, check=True, text=True, stdout=subprocess.PIPE
            )
            self.assertNotIn("LINK", result.stdout)
            self.assertEqual(original_mtime, binary.stat().st_mtime_ns)

            # GNU make can compare timestamps at one-second resolution on some
            # platforms and filesystems. Cross a full tick before touching the
            # prerequisite so this CI check is deterministic on macOS too.
            time.sleep(1.1)
            os.utime(root / "obj" / "dev" / "shared.o", None)
            result = subprocess.run(
                command, check=True, text=True, stdout=subprocess.PIPE
            )
            self.assertIn("LINK", result.stdout)
            self.assertEqual(original_content, binary.read_bytes())
            self.assertEqual(original_mtime, binary.stat().st_mtime_ns)

            result = subprocess.run(command, check=True, text=True, stdout=subprocess.PIPE)
            self.assertNotIn("LINK", result.stdout)
            self.assertEqual(original_mtime, binary.stat().st_mtime_ns)

            binary.unlink()
            subprocess.run(command, check=True, stdout=subprocess.PIPE)
            self.assertEqual(original_content, binary.read_bytes())
            rebuilt_mtime = binary.stat().st_mtime_ns
            result = subprocess.run(command, check=True, text=True, stdout=subprocess.PIPE)
            self.assertNotIn("LINK", result.stdout)

            time.sleep(1.1)
            (src / "shared.cpp").write_text("shared-v2\n")
            subprocess.run(command, check=True)
            self.assertNotEqual(original_content, binary.read_bytes())
            self.assertGreater(binary.stat().st_mtime_ns, rebuilt_mtime)

    def test_reference_bundle_and_ci_build_export_the_same_binaries(self):
        script = EXPORT_SCRIPT.read_text()
        reference_targets = shell_array(script, "reference_targets")
        self.assertEqual(reference_targets, REFERENCE_TARGETS)

        scaffold_targets = [
            pair.split(":", 1)[0]
            for pair in shell_array(script, "scaffold_pairs")
        ]
        self.assertCountEqual(scaffold_targets, REFERENCE_TARGETS)

        workflow = TESTS_WORKFLOW.read_text()
        match = re.search(r"for exe in \\\n(?P<body>.*?)\s*; do", workflow, re.S)
        self.assertIsNotNone(match, "missing host-binary upload loop")
        uploaded = match.group("body").replace("\\", " ").split()
        self.assertCountEqual(uploaded, REFERENCE_TARGETS)

    def test_assignment_reference_binary_ownership(self):
        pairs = shell_array(EXPORT_SCRIPT.read_text(), "pa_ref_pairs")
        actual = dict(pair.split(":", 1) for pair in pairs)
        self.assertEqual(actual, ASSIGNMENT_REFERENCE_TARGETS)
        self.assertEqual(len(pairs), len(ASSIGNMENT_REFERENCE_TARGETS))

    def test_export_owns_failed_stdout_diagnostics(self):
        script = EXPORT_SCRIPT.read_text()
        self.assertIn(
            'CPPGM_KEEP_FAILED_REFERENCE_STDOUT=1 make -s -C "$dest" ref-test',
            script,
        )
        self.assertIn('is_failed_stdout_diagnostic "$dest" "$path"', script)
        self.assertNotIn("strict", script)
        self.assertNotIn("witness", script)

        tracked_stdout = subprocess.run(
            [
                "git",
                "ls-files",
                "--",
                "pa[0-9]*/**/*.ref.stdout",
            ],
            cwd=REPO_ROOT,
            check=True,
            text=True,
            stdout=subprocess.PIPE,
        ).stdout.splitlines()
        for stdout_name in tracked_stdout:
            status_name = stdout_name.removesuffix(".stdout") + ".exit_status"
            status = (REPO_ROOT / status_name).read_text().strip()
            self.assertEqual(
                status,
                "EXIT_SUCCESS",
                f"failed-case diagnostic must be export-owned: {stdout_name}",
            )


if __name__ == "__main__":
    unittest.main()
