#!/usr/bin/env python3
import importlib.util
import json
import re
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
SCRIPT = ROOT / 'scripts/check_optimization_quality.py'
spec = importlib.util.spec_from_file_location('quality', SCRIPT)
quality = importlib.util.module_from_spec(spec)
spec.loader.exec_module(quality)


class QualityTests(unittest.TestCase):
    def test_reference_wrapper_keeps_its_invocation_name(self):
        with tempfile.TemporaryDirectory() as tmp:
            script = Path(tmp) / 'shared-dispatcher'
            script.write_text('#!/bin/sh\ncase "$0" in *-ref) exit 0;; *) exit 2;; esac\n')
            script.chmod(0o755)
            wrapper = Path(tmp) / 'compiler-ref'
            wrapper.symlink_to(script)
            command = quality.executable(str(wrapper))
            self.assertEqual(command, [str(wrapper)])
            self.assertEqual(subprocess.run(command).returncode, 0)

    def test_startup_subtraction_and_empty_measurements(self):
        self.assertEqual(quality.work_ratio([100, 300], [500, 600]), 2)
        for a, b in [([4, 4], [0, 1]), ([4, 3], [0, 1]), ([0, 1], [4, 4])]:
            with self.assertRaises(ValueError):
                quality.work_ratio(a, b)

    def test_invalid_limits_and_unknown_settings_fail(self):
        original = (ROOT / 'pa33/tests/kernels/600-bounded-search.quality').read_text()
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'case.quality'
            for text in [original + 'secret_cap=3\n',
                         original.replace('level=3', 'level=0'),
                         original.replace('scenarios=0', 'scenarios=999'),
                         re.sub(r'lowir_limit=[^\n]+', 'lowir_limit=nan', original)]:
                # The last substitution uses the current published limit.
                if text == original:
                    text = '\n'.join('lowir_limit=nan' if l.startswith('lowir_limit=') else l
                                     for l in original.splitlines()) + '\n'
                path.write_text(text)
                with self.assertRaises(ValueError):
                    quality.settings(path)

    def test_oracle_rejects_compiled_wrong_program_before_counting(self):
        with tempfile.TemporaryDirectory() as tmp:
            directory = Path(tmp)
            compiler = directory / 'wrong-compiler'
            compiler.write_text('''#!/usr/bin/env python3
import pathlib, subprocess, sys
args = sys.argv[1:]
source = next(pathlib.Path(a) for a in args if a.endswith('.t'))
wrong = source.with_suffix('.wrong.cpp')
wrong.write_text('extern "C" unsigned quality_kernel(void*, unsigned) { return 0; }')
args[args.index(str(source))] = str(wrong)
sys.exit(subprocess.call(['g++', *args]))
''')
            compiler.chmod(0o755)
            result = subprocess.run(['python3', str(SCRIPT),
                                     str(ROOT / 'pa33/tests/kernels/600-bounded-search.t'),
                                     '--mode', 'native', '--compiler', str(compiler),
                                     '--host', 'g++', '--out', str(directory / 'out')],
                                    capture_output=True, text=True, timeout=30)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn('wrong result', result.stdout)
            observations = next((directory / 'out').glob('*/*/observations.jsonl')).read_text()
            self.assertNotIn('"phase": "count"', observations)

    @unittest.skipUnless(shutil.which('valgrind'), 'Cachegrind not installed')
    def test_real_alternative_compiler_and_retained_evidence(self):
        with tempfile.TemporaryDirectory() as tmp:
            result = subprocess.run(['python3', str(SCRIPT),
                                     str(ROOT / 'pa33/tests/kernels/600-bounded-search.t'),
                                     '--mode', 'native', '--compiler', 'g++', '--host', 'g++',
                                     '--out', tmp, '--scale', '64', '--timing-rounds', '1'],
                                    capture_output=True, text=True, timeout=45)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            root = next(Path(tmp).iterdir())
            data = json.loads((root / 'results.json').read_text())[0]
            self.assertGreater(data['checks'], 20)
            self.assertAlmostEqual(data['ratio'], 1, delta=.05)
            self.assertEqual(len(data['executables']), 2)
            observations = json.loads(next(root.glob('*/observations.json')).read_text())
            for phase in ('AA', 'AB'):
                self.assertTrue(any(r.get('pair') == phase for r in observations))
            self.assertTrue(all(r['status'] == 0 for r in observations))
            self.assertTrue(next(root.glob('*/inputs/kernels/600-support.h')).is_file())


class StudentDocumentsTests(unittest.TestCase):
    def audit(self, text):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            for source in (ROOT / 'docs/student-export-root').glob('*.md'):
                (root / source.name).write_text('Course documentation.\n')
            path = root / 'pa33/design-notes.md'
            path.parent.mkdir(); path.write_text(text)
            inventory = root / 'inventory'; inventory.write_text('pa33/design-notes.md\n')
            return subprocess.run(['perl', str(ROOT / 'scripts/audit_student_documents.pl'),
                                   '--root', str(root), '--manifest', str(inventory)],
                                  capture_output=True, text=True)

    def test_public_concepts_and_scaffold_paths_are_allowed(self):
        result = self.audit('Use dominance and typed operands. Limits are in the fixtures.\n'
                            'The supplied vocabulary is `dev/src/lowir_model.h`.\n')
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_design_notes_cannot_publish_private_details(self):
        for text in ['Use opt::FunctionContext.', 'See dev/src/opt/scalar_fold.cpp.',
                     'The course solution\nkeeps four private waves.',
                     'The once-cap controls admission.', 'PENNANT took fewer cycles.',
                     'See obj/wt-unroll/handoff.md.']:
            with self.subTest(text=text):
                result = self.audit(text)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn('pa33/design-notes.md:', result.stdout)


if __name__ == '__main__':
    unittest.main()
