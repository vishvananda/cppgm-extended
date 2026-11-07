#!/usr/bin/env python3
"""Boundary regressions; optional native dg integration uses external helpers."""
import json
import os
from pathlib import Path
import shutil
import signal
import subprocess
import sys
import tempfile
import time
import unittest

REPO = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO / 'scripts'))
import run_upstream_dg as runner
import upstream_dg_proxy as proxy


# Linux piped core handlers can delay waitpid despite RLIMIT_CORE=0. Keep this
# synthetic crash local to its child process and retain the real signal status.
CRASH_CODE = '''import os, signal, sys
if sys.platform.startswith('linux'):
    import ctypes
    assert ctypes.CDLL(None).prctl(4, 0, 0, 0, 0) == 0  # PR_SET_DUMPABLE
os.kill(os.getpid(), signal.SIGSEGV)
'''


class BoundaryTests(unittest.TestCase):
    def config(self, root, **values):
        config = dict(mode='candidate', dialect='c++11', candidate=sys.executable,
                      host=shutil.which('g++') or '/nonexistent', log=str(root / 'calls'),
                      source_root=str(root), timeout_seconds=3, cpu_seconds=2,
                      address_space_bytes=268435456, file_bytes=1048576)
        config.update(values)
        return config

    def test_ignored_flags_and_conflicting_dialects_are_blocked(self):
        config = self.config(Path('/tmp'))
        for flag in ['-Wall', '-pedantic-errors', '-fimaginary-flag', '-flto', '-m32',
                     '-std=c++14', '-std=gnu++11', '@options']:
            with self.subTest(flag=flag), self.assertRaises(proxy.Blocked):
                proxy.translate(['input.C', flag], config)

    def test_host_arguments_preserve_library_order(self):
        args = ['-std=c++11', '-lfirst', 'one.o', '-L', 'directory', '-lsecond', 'two.o']
        translated = proxy.translate(args, self.config(Path('/tmp'), mode='host'))
        self.assertEqual(translated[0], args)

    def test_only_severity_location_prefix_changes(self):
        raw = 'ERROR: path.C:4:7: original diagnostic\ncontinuation\nERROR: std::bad_alloc\n'
        self.assertEqual(proxy.normalize_diagnostics(raw),
                         'path.C:4:7: error: original diagnostic\ncontinuation\nERROR: std::bad_alloc\n')

    def test_expected_error_pass_cannot_hide_abnormal_or_blocked_invocation(self):
        native = [dict(status='PASS', text='expected error')]
        execution = dict(classification='success', returncode=0)
        for classification, expected in [('abnormal', 'compiler-abnormal'),
                                         ('timeout', 'compiler-abnormal'),
                                         ('blocked', 'harness-blocked')]:
            call = dict(phase='test', role='test', classification=classification,
                        incoming=['negative.C'], reason='unsupported option')
            result = runner.disposition(native, [call], execution)
            self.assertEqual(result['status'], expected)

    def test_initialization_blocker_is_retained_without_disposing_unrelated_case(self):
        call = dict(phase='initialization', role='probe', classification='blocked', reason='offload')
        self.assertEqual(runner.disposition([dict(status='PASS')], [call],
                                          dict(classification='success', returncode=0))['status'],
                         'native-assertions-passed')

    def test_unavailable_checker_never_becomes_pass(self):
        result = runner.disposition([dict(status='PASS')], [],
                                    dict(classification='success', returncode=0), ['assembly unavailable'])
        self.assertEqual(result['status'], 'harness-blocked')

    def test_native_framework_error_is_not_attributed_to_compiler(self):
        native = [dict(status='FAIL'), dict(status='ERROR')]
        result = runner.disposition(native, [], dict(classification='rejection', returncode=1))
        self.assertEqual(result['status'], 'harness-unresolved')

    def test_resource_errors_signals_and_internal_exit_are_abnormal(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            for index, code in enumerate([
                    "import sys; print('ERROR: std::bad_alloc', file=sys.stderr); sys.exit(1)",
                    'import sys; sys.exit(70)',
                    CRASH_CODE]):
                result, _, _ = proxy.execute([sys.executable, '-c', code], root / str(index),
                                             self.config(root))
                self.assertEqual(result['classification'], 'abnormal')
                if index == 2:
                    self.assertEqual(result['returncode'], -signal.SIGSEGV)

    def test_located_ice_and_ordinary_error_mention_are_distinct(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            for index, (message, expected) in enumerate([
                    ('input.C:2:80: internal compiler error: Segmentation fault', 'abnormal'),
                    ('cc1plus: internal compiler error: Segmentation fault', 'abnormal'),
                    ('input.C:2:1: error: #error internal compiler error', 'rejection')]):
                code = 'import sys; print(' + repr(message) + ', file=sys.stderr); sys.exit(1)'
                result, _, _ = proxy.execute([sys.executable, '-c', code], root / str(index),
                                             self.config(root))
                self.assertEqual(result['classification'], expected)

    def test_timeout_kills_descendant_process_group(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            marker = root / 'leaked'
            started = root / 'started'
            # A grandchild would write the marker if only the direct child died.
            code = ('import subprocess,sys,time; '
                    'subprocess.Popen([sys.executable,"-c",' +
                    repr('import time; from pathlib import Path; Path(' + repr(str(started)) +
                         ').write_text("ready"); time.sleep(2); Path(' + repr(str(marker)) +
                         ').write_text("leaked")') + ']); time.sleep(20)')
            result, _, _ = proxy.execute([sys.executable, '-c', code], root / 'timeout',
                                         self.config(root, timeout_seconds=1))
            self.assertEqual(result['classification'], 'timeout')
            self.assertTrue(started.is_file(), 'descendant must start before the timeout')
            time.sleep(1.2)
            self.assertFalse(marker.exists())


NATIVE = all(os.environ.get(key) for key in ['CPPGM_DG_RUNTEST', 'CPPGM_DG_TESTSUITE', 'CPPGM_DG_LIBRARY'])


@unittest.skipUnless(NATIVE and shutil.which('g++'), 'set CPPGM_DG_RUNTEST/TESTSUITE/LIBRARY for native integration')
class NativeTests(unittest.TestCase):
    def setUp(self):
        retained = os.environ.get('CPPGM_DG_TEST_OUTPUT')
        if retained:
            Path(retained).mkdir(parents=True, exist_ok=True)
            self.root = Path(tempfile.mkdtemp(prefix=self._testMethodName + '-', dir=retained))
        else:
            temporary = tempfile.TemporaryDirectory()
            self.addCleanup(temporary.cleanup)
            self.root = Path(temporary.name)
        (self.root / 'cases').mkdir()
        self.compiler = self.root / 'fake-compiler'
        self.compiler.write_text('''#!/usr/bin/env python3
import os, pathlib, signal, sys, time
sources = [pathlib.Path(arg) for arg in sys.argv[1:] if pathlib.Path(arg).suffix in {'.C','.cc','.c'} and pathlib.Path(arg).is_file()]
text = '\\n'.join(path.read_text() for path in sources)
if 'FAKE_CRASH' in text or 'FAKE_TIMEOUT' in text or 'FAKE_INTERNAL' in text:
    print('ERROR: ' + str(sources[0]) + ':1:1: marker', file=sys.stderr, flush=True)
    if 'FAKE_CRASH' in text: exec(''' + repr(CRASH_CODE) + ''')
    if 'FAKE_TIMEOUT' in text: time.sleep(20)
    sys.exit(70)
if 'FAKE_NO_OUTPUT' in text: sys.exit(0)
os.execv(''' + repr(shutil.which('g++')) + ''', ['g++', '-fdiagnostics-plain-output', *sys.argv[1:]])
''')
        self.compiler.chmod(0o755)

    def run_case(self, source, mode='candidate', timeout=3, defaults='adapted'):
        case = self.root / 'cases' / 'input.C'
        case.write_text(source)
        out = self.root / ('result-' + mode)
        command = [sys.executable, '-B', str(REPO / 'scripts' / 'run_upstream_dg.py'),
                   '--testsuite', os.environ['CPPGM_DG_TESTSUITE'],
                   '--dejagnu-library', os.environ['CPPGM_DG_LIBRARY'],
                   '--runtest', os.environ['CPPGM_DG_RUNTEST'],
                   '--candidate', str(self.compiler), '--case', str(case),
                   '--source-root', str(case.parent), '--mode', mode, '--defaults', defaults,
                   '--timeout-seconds', str(timeout), '--case-timeout-seconds', '20', '--output', str(out)]
        result = subprocess.run(command, capture_output=True, text=True, timeout=30)
        self.assertTrue((out / 'result.json').is_file(), result.stderr)
        return json.loads((out / 'result.json').read_text())

    def test_conditional_diagnostics_on_both_routes(self):
        source = '''// { dg-do compile }
int value = missing; // { dg-error "not declared" "" { target c++11 } }
// { dg-error "must never appear" "" { target c++14 } }
'''
        for mode in ['host', 'candidate']:
            result = self.run_case(source, mode)
            self.assertEqual(result['status'], 'native-assertions-passed', result)
            self.assertGreaterEqual(result['native_counts'].get('PASS', 0), 2)

    def test_skip_uses_native_target_evaluation(self):
        result = self.run_case('''// { dg-do compile }
// { dg-skip-if "synthetic skip" { *-*-* } { "*" } { "" } }
int value;
''')
        self.assertEqual(result['status'], 'native-unsupported', result)
        self.assertFalse(any(c.get('role') == 'test' for c in result['calls']))

    def test_xfail_is_not_rewritten_as_plain_pass(self):
        result = self.run_case('''// { dg-do compile }
int value; // { dg-error "expected but absent" "" { xfail *-*-* } }
''')
        self.assertEqual(result['native_counts'].get('XFAIL'), 1, result)
        self.assertEqual(result['status'], 'native-assertions-passed', result)

    def test_companion_source_and_hosted_execution(self):
        (self.root / 'cases' / 'helper.C').write_text('int helper() { return 7; }\n')
        result = self.run_case('''// { dg-do run }
// { dg-additional-sources "helper.C" }
int helper(); int main() { return helper() != 7; }
''')
        self.assertEqual(result['status'], 'native-assertions-passed', result)
        calls = [c for c in result['calls'] if c.get('role') == 'test']
        self.assertEqual(len(calls), 1, result)
        self.assertEqual(len(calls[0]['steps']), 3, result)
        self.assertTrue(all(step['command'][0] == str(self.compiler) for step in calls[0]['steps'][:2]))
        self.assertEqual(calls[0]['steps'][2]['command'][0], str(Path(shutil.which('g++')).resolve()))

    def test_crash_overrides_native_expected_error_pass(self):
        result = self.run_case('// FAKE_CRASH { dg-error "marker" "" { target *-*-* } 0 }\n')
        self.assertEqual(result['status'], 'compiler-abnormal', result)
        self.assertGreaterEqual(result['native_counts'].get('PASS', 0), 1, result)

    def test_timeout_overrides_native_expected_error_pass(self):
        result = self.run_case('// FAKE_TIMEOUT { dg-error "marker" "" { target *-*-* } 0 }\n', timeout=0.5)
        self.assertEqual(result['status'], 'compiler-abnormal', result)
        self.assertEqual(result['abnormal'][0]['classification'], 'timeout', result)
        self.assertGreaterEqual(result['native_counts'].get('PASS', 0), 1, result)

    def test_internal_error_overrides_native_expected_error_pass(self):
        result = self.run_case('// FAKE_INTERNAL { dg-error "marker" "" { target *-*-* } 0 }\n')
        self.assertEqual(result['status'], 'compiler-abnormal', result)
        self.assertGreaterEqual(result['native_counts'].get('PASS', 0), 1, result)

    def test_unknown_flag_is_blocked_even_if_native_expects_its_error(self):
        result = self.run_case('''// { dg-options "-funknown-adapter-option" }
// { dg-error "adapter unavailable" "" { target *-*-* } 0 }
int value;
''')
        self.assertEqual(result['status'], 'harness-blocked', result)
        self.assertIn('unverified compiler option', ' '.join(result['reasons']))

    def test_assembly_checker_is_an_explicit_coverage_gap(self):
        result = self.run_case('''// { dg-do compile }
// { dg-final { scan-assembler "main" } }
int main() {}
''')
        self.assertEqual(result['status'], 'harness-blocked', result)
        self.assertIn('unavailable dg-final', ' '.join(result['reasons']))

    def test_success_without_artifact_is_abnormal(self):
        result = self.run_case('// FAKE_NO_OUTPUT\nint value;\n')
        self.assertEqual(result['status'], 'compiler-abnormal', result)

    def test_native_pedantic_default_is_not_silently_ignored(self):
        result = self.run_case('int value;\n', defaults='native')
        self.assertEqual(result['status'], 'harness-blocked', result)
        self.assertIn('-pedantic-errors', ' '.join(result['reasons']))

    def test_newer_dialect_is_not_run_as_cxx11(self):
        result = self.run_case('// { dg-options "-std=c++14" }\nint value;\n')
        self.assertEqual(result['status'], 'harness-blocked', result)
        self.assertIn('dialect outside', ' '.join(result['reasons']))

    def test_effective_target_does_not_inherit_host_flag_support(self):
        result = self.run_case('// { dg-require-effective-target fpic }\nint value;\n')
        self.assertEqual(result['status'], 'harness-blocked', result)
        self.assertIn('-fpic', ' '.join(result['reasons']))

    def test_assembly_inspecting_effective_target_does_not_read_elf_as_text(self):
        result = self.run_case('// { dg-require-effective-target tls_native }\nint value;\n')
        self.assertEqual(result['status'], 'harness-blocked', result)
        self.assertIn('probe interface unavailable', ' '.join(result['reasons']))

    def test_runtime_crash_overrides_expected_runtime_failure(self):
        result = self.run_case('''// { dg-do run }
// { dg-shouldfail "synthetic runtime failure" }
int main() { __builtin_trap(); }
''')
        self.assertEqual(result['status'], 'runtime-abnormal', result)
        self.assertGreaterEqual(result['native_counts'].get('PASS', 0), 1, result)

    def test_runtime_timeout_has_its_own_record(self):
        result = self.run_case('// { dg-do run }\nint main() { for (;;) {} }\n', timeout=2)
        self.assertEqual(result['status'], 'runtime-abnormal', result)
        self.assertTrue(any(c.get('role') == 'runtime' and c['classification'] == 'timeout'
                            for c in result['calls']), result)

    def test_expected_nonzero_runtime_exit_is_preserved(self):
        result = self.run_case('''// { dg-do run }
// { dg-shouldfail "ordinary exit 70 is a program outcome" }
int main() { return 70; }
''')
        self.assertEqual(result['status'], 'native-assertions-passed', result)
        runtime = [c for c in result['calls'] if c.get('role') == 'runtime']
        self.assertEqual(runtime[0]['exitcode'], 70, result)
        self.assertEqual(runtime[0]['classification'], 'rejection', result)


if __name__ == '__main__':
    unittest.main()
