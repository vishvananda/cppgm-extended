#!/usr/bin/env python3
"""Platform selection must agree between execution and comparison without hiding failures."""
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
SCRIPTS = ROOT / 'scripts'


class TestPlatform(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        self.env = dict(os.environ, CPPGM_TEST_PLATFORM='macos', CPPGM_TEST_RUN_ID='')
        self.env.pop('CPPGM_TEST_INCLUDE_ALL', None)
        self.env['CPPGM_TEST_SKIPS_FILE'] = str(self.root / 'skips')

    def perl(self, code, *arguments, env=None):
        return subprocess.run(['perl', '-I', str(SCRIPTS), '-MCppgmBatchWorker', '-e', code, *arguments],
                              cwd=self.root, env=env or self.env, text=True, capture_output=True)

    def fixture(self, name, restriction=None):
        base = self.root / name
        base.with_suffix('.t').write_text('input\n')
        base.with_suffix('.ref').write_text('correct\n')
        base.with_suffix('.my').write_text('wrong\n')
        for suffix in ('ref', 'my'):
            base.with_suffix(f'.{suffix}.exit_status').write_text('EXIT_SUCCESS\n')
        if restriction:
            base.with_suffix('.platforms').write_text(restriction)

    def test_execution_and_comparison_select_the_same_fixture_set(self):
        self.fixture('elf', 'linux # ELF relocation contract\n')
        self.fixture('portable')
        result = self.perl('print join("\\n", CppgmBatchWorker::collect_tests(".", qr/\\.t$/));')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn('portable.t', result.stdout)
        self.assertNotIn('elf.t', result.stdout)
        comparison = subprocess.run(['perl', str(SCRIPTS / 'compare_results_common.pl'),
                                     'text_t', 'ref', 'my', '.'], cwd=self.root, env=self.env,
                                    text=True, capture_output=True)
        self.assertNotEqual(comparison.returncode, 0)
        self.assertIn('portable', comparison.stdout + comparison.stderr)
        self.assertNotIn('elf:', comparison.stdout + comparison.stderr)

    def test_malformed_metadata_fails_even_on_an_allowed_host(self):
        for text in ('linux\n', 'linx # typo\n', '# missing platforms\n'):
            with self.subTest(text=text):
                (self.root / 'x.platforms').write_text(text)
                result = self.perl('CppgmTestPlatform::restriction("x.platforms");')
                self.assertNotEqual(result.returncode, 0)

    def test_an_inspection_exclusion_preserves_the_behavior_fixture(self):
        self.fixture('x')
        (self.root / 'x.inspect.platforms').write_text('linux # ELF COMDAT membership\n')
        selected = self.perl('print CppgmBatchWorker::collect_tests(".", qr/\\.t$/);')
        self.assertIn('x.t', selected.stdout)
        inspect = self.perl('exit(CppgmTestPlatform::inspection_supported("x") ? 1 : 0);')
        self.assertEqual(inspect.returncode, 0, inspect.stderr)
        self.assertIn('x inspection', (self.root / 'skips').read_text())

    def test_inventory_can_list_all_platforms(self):
        self.fixture('elf', 'linux # ELF contract\n')
        result = self.perl('print CppgmBatchWorker::collect_tests(".", qr/\\.t$/);',
                           env=dict(self.env, CPPGM_TEST_INCLUDE_ALL='1'))
        self.assertIn('elf.t', result.stdout)
        self.assertFalse((self.root / 'skips').exists())

    def test_discovery_without_a_report_keeps_skip_diagnostics_out_of_paths(self):
        self.fixture('elf', 'linux # ELF contract\n')
        self.fixture('portable')
        env = dict(self.env)
        env.pop('CPPGM_TEST_SKIPS_FILE')
        result = self.perl('print "$_\\n" for CppgmBatchWorker::collect_tests(".", qr/\\.t$/);',
                           env=env)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout.strip(), './portable.t')
        self.assertIn('SKIP ./elf.t: requires linux: ELF contract', result.stderr)

    def test_duplicate_skips_are_counted_once_and_never_as_passes(self):
        result = self.perl('CppgmTestPlatform::skip("x", "ELF"); '
                           'CppgmTestPlatform::skip("x", "ELF"); '
                           'CppgmTestPlatform::summary($ENV{CPPGM_TEST_SKIPS_FILE});')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn('PLATFORM SKIPS: 1 (macos)', result.stdout)
        self.assertEqual(result.stdout.count('SKIP x:'), 1)
        self.assertNotIn('PASSED', result.stdout)

    def test_supported_lanes_propagate_command_failure(self):
        args = ['perl', str(SCRIPTS / 'test_platform.pl'), 'linux', 'quality',
                'Cachegrind instruction counts', '--', 'sh', '-c', 'exit 7']
        mac = subprocess.run(args, env=self.env, capture_output=True)
        linux = subprocess.run(args, env=dict(self.env, CPPGM_TEST_PLATFORM='linux'), capture_output=True)
        self.assertEqual(mac.returncode, 0)
        self.assertEqual(linux.returncode, 7)

    def test_platform_specific_inspector_does_not_disable_execution(self):
        self.fixture('x')
        (self.root / 'x.inspect.cmd.linux').write_text('readelf probe.o\n')
        result = self.perl('print CppgmBatchWorker::collect_tests(".", qr/\\.t$/); '
                           'exit(CppgmTestPlatform::inspection_supported("x") ? 1 : 0);')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn('x.t', result.stdout)
        self.assertIn('provided only for linux', (self.root / 'skips').read_text())

    def test_native_output_oracle_preserves_status_and_output_checks(self):
        self.fixture('math')
        base = self.root / 'math'
        base.with_suffix('.oracle.macos').write_text('host-output # native libm conventions\n')
        for suffix in ('ref', 'my'):
            base.with_suffix(f'.{suffix}.impl.exit_status').write_text('0\n')
            base.with_suffix(f'.{suffix}.program.exit_status').write_text('0\n')
        base.with_suffix('.ref.program.stdout').write_text('linux formatting\n')
        base.with_suffix('.my.program.stdout').write_text('native formatting\n')
        base.with_suffix('.my.oracle.program.exit_status').write_text('0\n')
        base.with_suffix('.my.oracle.program.stdout').write_text('native formatting\n')

        def compare():
            return subprocess.run(['perl', str(SCRIPTS / 'compare_results_common.pl'),
                                   'link_program_t', 'ref', 'my', '.'], cwd=self.root,
                                  env=self.env, text=True, capture_output=True)

        good = compare()
        self.assertEqual(good.returncode, 0, good.stdout + good.stderr)
        base.with_suffix('.my.program.stdout').write_text('wrong\n')
        self.assertNotEqual(compare().returncode, 0)
        base.with_suffix('.my.program.stdout').write_text('native formatting\n')
        base.with_suffix('.my.oracle.program.exit_status').write_text('1\n')
        self.assertNotEqual(compare().returncode, 0)


if __name__ == '__main__':
    unittest.main()
