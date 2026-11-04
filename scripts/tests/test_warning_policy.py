#!/usr/bin/env python3
"""Warnings in compiler sources must fail strict builds, including cached builds."""

from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[2]


class WarningPolicyTests(unittest.TestCase):
    def test_enabling_policy_rebuilds_and_rejects_warnings(self):
        compilers = [path for name in ('g++', 'clang++') if (path := shutil.which(name))]
        if not compilers:
            self.skipTest('no host compiler available')
        for compiler in compilers:
            with self.subTest(compiler=compiler), tempfile.TemporaryDirectory() as directory:
                work = Path(directory)
                header = work / 'warning.h'
                header.write_text('#warning CPPGM_WARNING_POLICY_PROBE\n')
                obj = work / 'obj'
                target = obj / 'dev/support/unicode/utf8.o'
                args = ['make', '-s', '-C', str(ROOT / 'dev'), str(target),
                        'CXX=' + compiler, 'CPPGM_HOST_CXX=' + compiler,
                        'CPPGM_STDLIB_FLAGS=', 'OBJ=' + str(obj),
                        'CC_FLAGS=-std=gnu++11 -Wall -O0 -include ' + str(header)]
                for strict in (0, 1):
                    result = subprocess.run(
                        args + ['CPPGM_WARNINGS_AS_ERRORS=' + str(strict)],
                        capture_output=True, text=True, timeout=60)
                    output = result.stdout + result.stderr
                    self.assertIn('CPPGM_WARNING_POLICY_PROBE', output)
                    if strict:
                        self.assertNotEqual(result.returncode, 0, output)
                        self.assertIn('error:', output)
                    else:
                        self.assertEqual(result.returncode, 0, output)
                        self.assertTrue(target.is_file())


if __name__ == '__main__':
    unittest.main()
