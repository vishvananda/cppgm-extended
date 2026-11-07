import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from run_cxx11_ci_suite import comparison, run_case, run_suite, validate_manifest
from upstream_dg_proxy import sha256
from qualify_clang_cxx11_ci_manifest import qualification_cases


FAKE = '''#!/usr/bin/env python3
import pathlib, struct, sys
args = sys.argv[1:]
if args == ['--version']:
    print('test compiler 21.1.8')
    sys.exit(0)
pathlib.Path('working-directory.txt').write_text(str(pathlib.Path.cwd()))
output = pathlib.Path(args[args.index('-o') + 1])
if '-c' in args or '-emit-obj' in args:
    source = pathlib.Path(args[args.index('-o') - 1]).read_text()
    if 'LOCATED_INTERNAL' in source:
        print('input.C:2:80: internal compiler error: Segmentation fault', file=sys.stderr)
        sys.exit(1)
    if 'INTERNAL' in source:
        print('internal compiler error', file=sys.stderr)
        sys.exit(70)
    if 'REJECT' in source:
        print('different diagnostic wording', file=sys.stderr)
        sys.exit(1)
    data = bytearray(128)
    data[:7] = b'\\x7fELF\\x02\\x01\\x01'
    struct.pack_into('<HHI', data, 16, 1, 62, 1)
    struct.pack_into('<Q', data, 40, 64)
    struct.pack_into('<HHHHHH', data, 52, 64, 0, 0, 64, 1, 0)
    output.write_bytes(data + (b'RUNTIME_FAIL' if 'RUNTIME_FAIL' in source else b''))
else:
    obj = pathlib.Path(args[0]).read_bytes()
    status = 1 if b'RUNTIME_FAIL' in obj else 0
    output.write_text('#!/bin/sh\\nexit ' + str(status) + '\\n')
    output.chmod(0o755)
'''


class Suite(unittest.TestCase):
    def test_transitive_input_hashes_and_identical_include_arguments(self):
        case = self.case()
        headers = self.root/'Inputs'
        headers.mkdir()
        header = headers/'nested.h'
        header.write_text('int nested;')
        case.update(input_files=[dict(path='Inputs/nested.h', sha256=sha256(header))],
                    include_directories=['Inputs'])
        manifest = dict(schema=1, suite='clang', cases=[case])
        validate_manifest(manifest, self.root)
        cppgm = self.run_one(case)['stages'][0]['command']
        other = self.root/'other'
        other.mkdir()
        upstream = run_case((case, str(self.root), str(self.compiler), str(self.compiler),
                             'upstream', str(other)))['stages'][0]['command']
        self.assertEqual([v for v in cppgm if v.startswith('-I')], ['-I'+str(headers)])
        self.assertEqual([v for v in cppgm if v.startswith('-I')],
                         [v for v in upstream if v.startswith('-I')])
        case['input_files'].append(dict(path=case['path'], sha256=case['sha256']))
        case['include_edges'] = [dict(source=case['path'], spelling='nested.h',
                                     resolved='Inputs/nested.h')]
        validate_manifest(manifest, self.root)
        shadow = self.root/'nested.h'
        shadow.write_text('int shadows;')
        with self.assertRaisesRegex(ValueError, 'resolution changed'):
            validate_manifest(manifest, self.root)
        shadow.unlink()
        case['include_edges'] = []
        header.write_text('int changed;')
        with self.assertRaisesRegex(ValueError, 'dependency hash'):
            validate_manifest(manifest, self.root)
        header.unlink()
        with self.assertRaises(OSError):
            validate_manifest(manifest, self.root)
        case['input_files'] = [dict(path='../escape', sha256='irrelevant')]
        with self.assertRaisesRegex(ValueError, 'outside corpus'):
            validate_manifest(manifest, self.root)
        case['input_files'] = []
        case['include_directories'] = ['../escape']
        with self.assertRaisesRegex(ValueError, 'outside corpus'):
            validate_manifest(manifest, self.root)

    def test_object_projection_does_not_execute_unmet_native_ir_or_runtime_commands(self):
        case = self.case()
        case['unmet_native_oracles'] = dict(status='not-executed',
            all_original_RUNs=['FileCheck %s', '%run %t'])
        result = self.run_one(case)
        self.assertEqual(result['status'], 'passed')
        self.assertEqual([r['name'] for r in result['stages']], ['compile'])
        self.assertEqual(case['unmet_native_oracles']['status'], 'not-executed')

    def test_strict_qualification_does_not_rewrite_normal_expectation_or_flags(self):
        self.compiler.write_text(FAKE.replace("if 'REJECT' in source:",
                                             "if 'REJECT' in source or ('STRICT_ONLY' in source "
                                             "and '-pedantic-errors' in args):"))
        case = self.case('STRICT_ONLY', flags=['-cc1'])
        case['qualification'] = dict(kind='implicit-mode-cxx11',
                                     additional_upstream_flags=['-pedantic-errors'])
        self.assertEqual(self.run_one(case, role='upstream')['status'], 'passed')
        manifest = self.root/'manifest.json'
        manifest.write_text(json.dumps(dict(schema=1, suite='clang', cases=[case])))
        original = manifest.read_bytes()
        output = self.root/'qualification'
        script = Path(__file__).resolve().parents[1]/'qualify_clang_cxx11_ci_manifest.py'
        result = subprocess.run([sys.executable, str(script), '--manifest', str(manifest),
                                 '--source-root', str(self.root), '--upstream', str(self.compiler),
                                 '--upstream-version', '21.1.8', '--output', str(output)],
                                capture_output=True, text=True)
        self.assertEqual(result.returncode, 1, result.stderr)
        self.assertEqual(manifest.read_bytes(), original)
        report = json.loads((output/'results.json').read_text())
        self.assertTrue(report['complete'])
        self.assertFalse(report['passed'])
        row, = report['result']['results']
        self.assertEqual(row['expect'], 'accept')
        self.assertEqual(row['status'], 'compile-failed')
        self.assertIn('-pedantic-errors', row['stages'][0]['command'])
        self.assertNotIn('-pedantic-errors', case['upstream_flags'])

    def test_qualification_selects_only_explicit_policies_and_preserves_original_cases(self):
        case = self.case()
        self.assertEqual(qualification_cases(dict(cases=[case])), [])
        case['qualification'] = dict(kind='implicit-mode-cxx11',
                                     additional_upstream_flags=['-pedantic-errors'])
        projected, = qualification_cases(dict(cases=[case]))
        self.assertEqual(projected['upstream_flags'], ['-pedantic-errors'])
        self.assertEqual(case['upstream_flags'], [])
        case['qualification']['kind'] = 'llvm-object-cxx11'
        llvm, = qualification_cases(dict(cases=[case]))
        self.assertEqual(llvm['upstream_flags'], ['-pedantic-errors'])
        case['qualification']['kind'] = 'local-input-cxx11'
        local, = qualification_cases(dict(cases=[case]))
        self.assertEqual(local['upstream_flags'], ['-pedantic-errors'])
        case['dialect'] = 'gnu++11'
        case['qualification'] = dict(kind='documented-gnu-cxx11', additional_upstream_flags=[],
                                     source_policy_issue='CXX11-065')
        extension, = qualification_cases(dict(cases=[case]))
        self.assertEqual(extension['upstream_flags'], [])
        case['dialect'] = 'c++11'
        case['qualification']['kind'] = 'vendor-attribute-cxx11'
        attribute, = qualification_cases(dict(cases=[case]))
        self.assertEqual(attribute['upstream_flags'], [])
        case['qualification'].update(kind='reviewed-iso-cxx11',
                                    additional_upstream_flags=['-pedantic-errors', '-Wno-comment'])
        comment, = qualification_cases(dict(cases=[case]))
        self.assertEqual(comment['upstream_flags'], ['-pedantic-errors', '-Wno-comment'])
        case['qualification']['additional_upstream_flags'] = ['-Werror']
        with self.assertRaises(ValueError):
            qualification_cases(dict(cases=[case]))

    def test_standalone_qualification_is_positive_compile_and_strict_only(self):
        case = self.case()
        case['projection_kind'] = 'standalone-llvm-only-object-acceptance'
        case['qualification'] = dict(kind='standalone-llvm-only-cxx11',
                                     additional_upstream_flags=['-pedantic-errors'])
        original = json.dumps(case)
        for dialect in ['c++11', 'gnu++11']:
            case['dialect'] = dialect
            qualified, = qualification_cases(dict(cases=[case]))
            self.assertEqual(qualified['upstream_flags'], ['-pedantic-errors'])
        case['dialect'] = 'c++11'
        self.assertEqual(json.dumps(case), original)
        for changes in [dict(expect='diagnostic'), dict(action='run'), dict(dialect='c++17'),
                        dict(projection_kind='other'),
                        dict(qualification=dict(kind='standalone-llvm-only-cxx11',
                                                additional_upstream_flags=[])),
                        dict(qualification=dict(kind='standalone-llvm-only-cxx11',
                            additional_upstream_flags=['-pedantic-errors', '-Wno-c++11-extensions']))]:
            with self.subTest(changes=changes), self.assertRaises(ValueError):
                qualification_cases(dict(cases=[dict(case, **changes)]))
        case['qualification'].update(additional_upstream_flags=['-pedantic-errors', '-Wno-comment'],
                                     source_policy_issue='review')
        qualified, = qualification_cases(dict(cases=[case]))
        self.assertEqual(qualified['upstream_flags'], ['-pedantic-errors', '-Wno-comment'])

    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.compiler = self.root / 'compiler'
        self.compiler.write_text(FAKE)
        self.compiler.chmod(0o755)

    def case(self, text='int x;', expect='accept', action='compile', flags=None):
        source = self.root / 'test.C'
        source.write_text(text)
        return dict(id='test', path='test.C', sha256=sha256(source), dialect='c++11',
                    expect=expect, action=action, candidate_flags=[], upstream_flags=flags or [])

    def run_one(self, case, role='cppgm'):
        output = self.root / 'output'
        output.mkdir()
        return run_case((case, str(self.root), str(self.compiler), str(self.compiler), role, str(output)))

    def test_diagnostic_wording_is_irrelevant(self):
        result = self.run_one(self.case('REJECT', expect='diagnostic'))
        self.assertEqual(result['status'], 'passed')
        self.assertEqual(len(result['stages']), 1)

    def test_internal_error_cannot_satisfy_diagnostic(self):
        self.assertEqual(self.run_one(self.case('INTERNAL', expect='diagnostic'))['status'], 'compiler-abnormal')

    def test_located_internal_error_cannot_satisfy_diagnostic(self):
        self.assertEqual(self.run_one(self.case('LOCATED_INTERNAL', expect='diagnostic'))['status'],
                         'compiler-abnormal')

    def test_missing_required_diagnostic_fails(self):
        self.assertEqual(self.run_one(self.case(expect='diagnostic'))['status'], 'diagnostic-missing')

    def test_runtime_obligation_is_executed(self):
        result = self.run_one(self.case('RUNTIME_FAIL', action='run'))
        self.assertEqual(result['status'], 'runtime-failed')
        self.assertEqual([row['name'] for row in result['stages']], ['compile', 'link', 'execute'])

    def test_cc1_action_is_emit_obj(self):
        result = self.run_one(self.case(flags=['-cc1']), 'upstream')
        self.assertEqual(result['status'], 'passed')
        command = result['stages'][0]['command']
        self.assertEqual(command[1:3], ['-cc1', '-std=c++11'])
        self.assertIn('-emit-obj', command)
        self.assertNotIn('-c', command)

    def test_case_working_directory_is_private_and_restored(self):
        previous = Path.cwd()
        temporary = os.environ.get('TMPDIR')
        self.assertEqual(self.run_one(self.case())['status'], 'passed')
        directory = self.root / 'output/test'
        self.assertEqual((directory / 'working-directory.txt').read_text(), str(directory))
        self.assertEqual(Path.cwd(), previous)
        self.assertEqual(os.environ.get('TMPDIR'), temporary)

    def test_worker_setup_failure_is_a_recorded_incomplete_suite(self):
        case = self.case()
        output = self.root / 'existing'
        output.mkdir()
        result = run_suite([case], self.root, str(self.compiler), str(self.compiler), 'cppgm', output, 1)
        self.assertEqual(result['executed'], 0)
        self.assertEqual(result['passed'], 0)
        self.assertTrue(result['error'])
        self.assertGreater(result['elapsed_seconds'], 0)

    def test_empty_duplicate_and_changed_manifests_cannot_pass(self):
        manifest = dict(schema=1, suite='gcc', cases=[])
        with self.assertRaises(ValueError):
            validate_manifest(manifest, self.root)
        case = self.case()
        manifest['cases'] = [case, case]
        with self.assertRaises(ValueError):
            validate_manifest(manifest, self.root)
        manifest['cases'] = [case]
        (self.root / 'test.C').write_text('changed')
        with self.assertRaises(ValueError):
            validate_manifest(manifest, self.root)

    def test_failed_or_incomplete_suites_are_noncomparable(self):
        good = dict(executed=10, passed=10, elapsed_seconds=2)
        for bad in [dict(good, executed=9), dict(good, passed=9)]:
            self.assertFalse(comparison(bad, good, 10)['comparable'])
        self.assertTrue(comparison(good, good, 10)['comparable'])

    def test_both_complete_runs_and_artifacts_survive_candidate_failure(self):
        case = self.case('REJECT')
        manifest = self.root / 'manifest.json'
        manifest.write_text(json.dumps(dict(schema=1, suite='clang', revision='pin',
                                           coverage='test', limitations=[], exclusions=[], cases=[case])))
        output = self.root / 'suite'
        script = Path(__file__).resolve().parents[1] / 'run_cxx11_ci_suite.py'
        result = subprocess.run([sys.executable, str(script), '--manifest', str(manifest),
                                 '--source-root', str(self.root), '--candidate', str(self.compiler),
                                 '--upstream', str(self.compiler), '--upstream-version', '21.1.8',
                                 '--output', str(output)], capture_output=True, text=True)
        self.assertEqual(result.returncode, 1, result.stderr)
        report = json.loads((output / 'results.json').read_text())
        self.assertEqual(report['cppgm']['executed'], 1)
        self.assertEqual(report['upstream']['executed'], 1)
        self.assertFalse(report['comparison']['comparable'])
        self.assertTrue((output / 'summary.md').is_file())


class Workflow(unittest.TestCase):
    def test_strict_qualification_failure_remains_visible_after_normal_pair(self):
        workflow = Path(__file__).resolve().parents[2]/'.github/workflows/upstream-cxx11.yml'
        text = workflow.read_text()
        qualification = text.split('      - name: Qualify implicit-mode sources with strict native C++11\n', 1)[1]
        qualification = qualification.split('      - name:', 1)[0]
        self.assertIn('continue-on-error: true', qualification)
        script = qualification.split('        run: |\n', 1)[1]
        script = '\n'.join(line[10:] for line in script.splitlines() if line.startswith('          '))
        with tempfile.TemporaryDirectory() as tmp:
            trace = Path(tmp)/'args'
            command = 'python3() { printf "%s\\0" "$@" > "$TRACE"; }\n'+script
            command = command.replace('${{ matrix.upstream }}', 'clang++-21')
            command = command.replace('${{ matrix.version }}', '21.1.8')
            subprocess.run(['bash', '-c', command], check=True,
                           env=dict(os.environ, RUNNER_TEMP=tmp, TRACE=str(trace)))
            args = trace.read_bytes().decode().rstrip('\0').split('\0')
            self.assertEqual(args[0], 'scripts/qualify_clang_cxx11_ci_manifest.py')
            self.assertEqual(args[args.index('--upstream')+1], 'clang++-21')
        gate = text.split('      - name: Require successful native C++11 qualification\n', 1)[1]
        gate = gate.split('      - name:', 1)[0]
        self.assertIn("if: always() && matrix.suite == 'clang'", gate)
        command = gate.split('        run: ', 1)[1].strip()
        for outcome, status in [('success', 0), ('failure', 1), ('skipped', 1)]:
            with self.subTest(outcome=outcome):
                result = subprocess.run(['bash', '-c', command.replace('${{ steps.qualification.outcome }}', outcome)])
                self.assertEqual(result.returncode, status)

    def test_freeze_step_enables_verify_negatives_only_for_clang(self):
        workflow = Path(__file__).resolve().parents[2] / '.github/workflows/upstream-cxx11.yml'
        step = workflow.read_text().split('      - name: Freeze language test manifest\n', 1)[1]
        step = step.split('      - name:', 1)[0].split('        run: |\n', 1)[1]
        script = '\n'.join(line[10:] for line in step.splitlines() if line.startswith('          '))
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            for suite in ['gcc', 'clang']:
                with self.subTest(suite=suite):
                    trace = root / (suite + '.args')
                    command = 'python3() { printf "%s\\0" "$@" > "$TRACE"; }\n' + script
                    command = command.replace('${{ matrix.suite }}', suite)
                    env = dict(os.environ, RUNNER_TEMP=str(root/'run temp'), TRACE=str(trace))
                    subprocess.run(['bash', '-c', command], check=True, env=env)
                    args = trace.read_bytes().decode().rstrip('\0').split('\0')
                    self.assertEqual(args[0], 'scripts/build_'+suite+'_cxx11_ci_manifest.py')
                    self.assertEqual('--verify-negatives' in args, suite == 'clang')
                    self.assertEqual('--implicit-cxx11' in args, suite == 'clang')
                    self.assertEqual('--llvm-object-projections' in args, suite == 'clang')
                    self.assertEqual('--local-inputs' in args, suite == 'clang')
                    self.assertEqual('--standalone-llvm-only-projections' in args, suite == 'clang')
                    self.assertEqual(args[args.index('--output')+1],
                                     str(root/'run temp/cxx11/manifest.json'))
                    self.assertEqual(args[args.index('--'+('llvm' if suite == 'clang' else 'gcc')+'-root')+1],
                                     'upstream-sources')


if __name__ == '__main__':
    unittest.main()
