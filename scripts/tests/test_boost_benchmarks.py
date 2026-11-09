#!/usr/bin/env python3
"""Publication must use identical reviewed Boost cases and complete timing."""
import argparse
import copy
import hashlib
import json
import os
from pathlib import Path
import sys
import tempfile
import unittest
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import benchmark_report
import boost_benchmarks
import boost_ci
import boost_results
import boost_runtime


class BenchmarkTests(unittest.TestCase):
    project = 'libs/example/test'

    def document(self):
        row = dict(target=self.project + '//example.test', kind='RUN', compile_cpu=2.0,
                   compile_elapsed=2.1, test_cpu=.01, test_elapsed=.02, native_status=0, compiler_invocations=1)
        report = dict(status='passed', suites=[dict(project=self.project, passed=1, tests=[row])])
        metadata = dict(dirty=False, revision='a' * 40, cpu='AMD EPYC test processor',
                        standard_library='host libstdc++', jobs=2, cpus=[0, 1], flags=['variant=debug'])
        return dict(status='complete', archive_sha256=boost_ci.ARCHIVE_SHA256, metadata=metadata,
                    compilers={label: copy.deepcopy(report) for label in boost_benchmarks.LABELS})

    def setUp(self):
        self.selection = mock.patch.dict(boost_ci.SUITES, {self.project: 1}, clear=True)
        self.selection.start()
        self.addCleanup(self.selection.stop)

    def test_all_compilers_and_actual_measurements_required(self):
        document = self.document()
        boost_benchmarks.validate(document)
        for edit in ('compiler', 'declaration', 'compile-time', 'execution-time', 'native-outcome'):
            damaged = copy.deepcopy(document)
            if edit == 'compiler':
                damaged['compilers'].pop('clang')
            else:
                row = damaged['compilers']['gcc']['suites'][0]['tests'][0]
                if edit == 'declaration':
                    row['target'] = self.project + '//different.test'
                elif edit == 'compile-time':
                    row.pop('compile_cpu')
                elif edit == 'execution-time':
                    row.pop('test_elapsed')
                else:
                    row['native_status'] = 1
            with self.subTest(edit=edit), self.assertRaises(ValueError):
                boost_benchmarks.validate(damaged)

    def test_failure_cannot_publish_a_speed(self):
        document = self.document()
        document['compilers']['gcc']['status'] = 'failed'
        with self.assertRaises(ValueError):
            boost_benchmarks.validate(document)

    def test_debugger_anchors_preserve_original_oracles_and_reject_changed_source(self):
        original = 'int main() {\n  int value = 7;\n  // TEST_EXPR( "value", "7" )\n}\n'
        with tempfile.TemporaryDirectory() as tmp:
            source = Path(tmp)
            path = source / 'libs/json/test/printers.cpp'
            path.parent.mkdir(parents=True)
            path.write_text(original)
            with mock.patch.object(boost_benchmarks, 'DEBUG_SOURCE_SHA256',
                                   hashlib.sha256(original.encode()).hexdigest()):
                record = boost_benchmarks.debug_anchors(source)
                adapted = path.read_text()
                self.assertIn('// TEST_EXPR( "value", "7" )\n  boost_benchmark_debug_anchor = 0;', adapted)
                self.assertEqual(record['adapted_sha256'], hashlib.sha256(adapted.encode()).hexdigest())
                with self.assertRaisesRegex(ValueError, 'unexpected Boost debugger source'):
                    boost_benchmarks.debug_anchors(source)

    def test_only_debugger_target_retains_no_inline_settings(self):
        original = ('run ordinary.cpp ;\nrun printers.cpp : : : '
                    '<dependency>../src/boost_json_gdb_printers.py\n      <optimization>off ;\n')
        with tempfile.TemporaryDirectory() as tmp:
            source = Path(tmp)
            path = source / 'libs/json/test/Jamfile'
            path.parent.mkdir(parents=True)
            path.write_text(original)
            with mock.patch.object(boost_benchmarks, 'GDB_JAM_SHA256',
                                   hashlib.sha256(original.encode()).hexdigest()):
                record = boost_benchmarks.debug_build_settings(source)
                adapted = path.read_text()
                self.assertEqual(adapted, original.replace(
                    '<dependency>../src/boost_json_gdb_printers.py',
                    '<dependency>../src/boost_json_gdb_printers.py\n      <inlining>off'))
                self.assertEqual(record['adapted_sha256'], hashlib.sha256(adapted.encode()).hexdigest())
                with self.assertRaisesRegex(ValueError, 'unexpected Boost debugger build settings'):
                    boost_benchmarks.debug_build_settings(source)

    def test_enum_adaptation_is_identical_for_all_compilers_and_hash_guarded(self):
        original = 'enum test { zero, one };\nint main() { return zero; }\n'
        with tempfile.TemporaryDirectory() as tmp:
            source = Path(tmp)
            path = source / 'libs/mpl/test/issue_69.cpp'
            path.parent.mkdir(parents=True)
            path.write_text(original)
            with mock.patch.object(boost_benchmarks, 'ENUM_SOURCE_SHA256',
                                   hashlib.sha256(original.encode()).hexdigest()):
                boost_benchmarks.enum_underlying_type(source)
                self.assertEqual(path.read_text(), 'enum test : int { zero, one };\nint main() { return zero; }\n')
                with self.assertRaisesRegex(ValueError, 'unexpected Boost enum source'):
                    boost_benchmarks.enum_underlying_type(source)

    def test_publication_checks_provenance_and_preserves_other_hosts(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            artifact, target = root / 'artifact', root / 'published'
            artifact.mkdir()
            other = target / 'boost/github-intel-compile-gcc.svg'
            other.parent.mkdir(parents=True)
            other.write_text('previous Intel measurement')
            document = self.document()
            def write():
                (artifact / 'results.json').write_text(json.dumps(document))
                (artifact / 'metadata.json').write_text(json.dumps(document['metadata']))
            args = argparse.Namespace(artifact=artifact, target=target,
                                      revision='a' * 40, run_url='https://example.org/run')
            document['metadata']['dirty'] = True
            write()
            with self.assertRaisesRegex(ValueError, 'clean measured revision'):
                boost_benchmarks.publish(args)
            document['metadata']['dirty'] = False
            document['metadata']['revision'] = 'b' * 40
            write()
            with self.assertRaisesRegex(ValueError, 'clean measured revision'):
                boost_benchmarks.publish(args)
            document['metadata']['revision'] = args.revision
            write()
            boost_benchmarks.publish(args)
            self.assertEqual(other.read_text(), 'previous Intel measurement')
            self.assertIn('1.00×', (target / 'boost/epyc-compile-gcc.svg').read_text())
            history = target / 'boost/history' / args.revision / 'epyc'
            self.assertEqual(json.loads((history / 'results.json').read_text()), document)
            self.assertTrue((history / 'standard/records.json').is_file())
            self.assertTrue((target / 'boost/epyc/report.md').is_file())

    def test_dedicated_intel_publication_preserves_hosted_o3_history(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            artifact = root / 'artifact'
            artifact.mkdir()
            document = self.document()
            document['metadata']['flags'] += ['optimization=speed']
            document['metadata']['cpu'] = 'Intel Xeon E5-2696 v4'
            (artifact / 'results.json').write_text(json.dumps(document))
            (artifact / 'metadata.json').write_text(json.dumps(document['metadata']))
            target = root / 'published'
            for host in ('epyc', 'github-intel'):
                previous = target / 'boost/o3' / (host + '-compile-gcc.svg')
                previous.parent.mkdir(parents=True, exist_ok=True)
                previous.write_text('previous ' + host)
            boost_benchmarks.publish(argparse.Namespace(artifact=artifact, target=target,
                revision='a' * 40, run_url='https://example.test/run', platform='intel'))
            self.assertIn('1.00×', (target / 'boost/o3/intel-compile-gcc.svg').read_text())
            for host in ('epyc', 'github-intel'):
                self.assertEqual((target / 'boost/o3' / (host + '-compile-gcc.svg')).read_text(), 'previous ' + host)
            shared = json.loads((target / 'boost/o3/intel/standard/records.json').read_text())
            self.assertTrue(all('Dedicated Intel' in r['host']['environment'] for r in shared['records']))

    def test_shared_records_preserve_test_execution_phase_and_case_membership(self):
        with tempfile.TemporaryDirectory() as tmp:
            directory = Path(tmp)
            (directory / 'results.json').write_text(json.dumps(self.document()))
            rows = boost_results.normalize(directory, 'GitHub hosted', 'https://example.org/run',
                                           'https://example.org/details', 'https://example.org/raw')
        catalog = dict(schema=1, workloads=[dict(id='boost', name='Boost', kind='benchmark',
                       configurations=[dict(id='debug', optimizations=['O0'])])])
        benchmark_report.validate(catalog, rows)
        self.assertEqual({r['phase'] for r in rows['records']}, {'compile', 'test-execution'})
        self.assertEqual({r['coverage']['selected'] for r in rows['records']}, {1})
        self.assertTrue(all(r['speedup'] == 1 for r in rows['records']))
        self.assertEqual(rows['validations'][0]['coverage']['passed'], 1)

    @unittest.skipUnless(hasattr(os, 'pidfd_open'), 'Boost native replays run on Linux')
    def test_paired_runtime_publishes_badges_and_shared_runtime_records(self):
        document = self.document()
        document['metadata']['flags'] += ['optimization=speed']
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            artifact, target = root / 'artifact', root / 'published'
            artifact.mkdir()
            previous_debug = target / 'boost/epyc-compile-gcc.svg'
            previous_debug.parent.mkdir(parents=True)
            previous_debug.write_text('historical debug measurement')
            for report in document['compilers'].values():
                row = report['suites'][0]['tests'][0]
                row.update(native_command=['/bin/echo', 'native test output'], native_cwd=str(root))
            with mock.patch('builtins.print'):
                document['runtime'] = boost_runtime.campaign(document, artifact / 'runtime', 1)
            (artifact / 'results.json').write_text(json.dumps(document))
            (artifact / 'metadata.json').write_text(json.dumps(document['metadata']))
            args = argparse.Namespace(artifact=artifact, target=target,
                                      revision='a' * 40, run_url='https://example.org/run')
            boost_benchmarks.publish(args)
            expected = f"{document['runtime']['comparisons']['gcc']['speedup']:.2f}×"
            self.assertIn(expected, (target / 'boost/o3/epyc-runtime-gcc.svg').read_text())
            self.assertIn('n/m', (target / 'boost/o3/github-intel-runtime-gcc.svg').read_text())
            self.assertEqual(previous_debug.read_text(), 'historical debug measurement')
            normalized = json.loads((target / 'boost/o3/epyc/standard/records.json').read_text())
            runtime = [r for r in normalized['records'] if r['phase'] == 'runtime']
            self.assertEqual(len(runtime), 4)
            self.assertTrue(all(r['optimization'] == 'O3' for r in runtime))
            self.assertIn('## Native runtime replay', (target / 'boost/o3/epyc/report.md').read_text())
            next((artifact / 'runtime').glob('000-*.jsonl')).unlink()
            with self.assertRaisesRegex(ValueError, 'per-process runtime evidence'):
                boost_benchmarks.publish(args)

    def test_artifacts_exclude_sources_objects_and_build_outputs(self):
        with tempfile.TemporaryDirectory() as tmp:
            directory = Path(tmp)
            for name in ['results.json', 'cppgm/libs-example-test/invocations.jsonl',
                         'cppgm/config/user.hpp', 'source/boost/large.hpp',
                         'cppgm/build/objects/manifest.json', 'cppgm/test.o']:
                path = directory / name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text('data')
            selected = {str(p.relative_to(directory)) for p in boost_benchmarks.retained_files(directory)}
        self.assertEqual(selected, {'results.json', 'cppgm/libs-example-test/invocations.jsonl',
                                    'cppgm/config/user.hpp'})


@unittest.skipUnless(hasattr(os, 'pidfd_open'), 'Boost native replays run on Linux')
class RuntimeTests(unittest.TestCase):
    def test_child_status_and_timeout_are_preserved(self):
        with tempfile.TemporaryDirectory() as tmp:
            directory = Path(tmp)
            row = boost_runtime.execute(['/bin/sh', '-c', 'echo sample; exit 7'], directory, directory, 2)
            self.assertEqual(row['code'], 7)
            self.assertFalse(row['timed_out'])
            self.assertEqual(row['stdout_sha256'], hashlib.sha256(b'sample\n').hexdigest())
            row = boost_runtime.execute(['/bin/sleep', '10'], directory, directory, .01)
            self.assertTrue(row['timed_out'])
            self.assertLess(row['code'], 0)

    def test_paired_direction_and_suite_filtering(self):
        rows = []
        for block in range(2):
            for side, cpu in [('A', 4), ('B', 2), ('B', 2), ('A', 4)]:
                rows.append(dict(block=block, side=side, programs=[dict(target='a//one', cpu=cpu),
                                                                  dict(target='b//two', cpu=1)]))
        self.assertEqual(boost_runtime.comparison(rows, 'a')['speedup'], 2)
        self.assertAlmostEqual(boost_runtime.comparison(rows)['speedup'], 5 / 3)

    def test_native_replay_calibrates_repeats_and_excludes_debugger_and_failures(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            reports = {}
            for label in boost_runtime.LABELS:
                tests = [dict(target='libs/example/test//fixed', kind='RUN',
                              native_command=['/bin/echo', 'same'], native_cwd=str(root)),
                         dict(target='libs/example/test//varying', kind='RUN',
                              native_command=['/bin/echo', label], native_cwd=str(root)),
                         dict(target=boost_runtime.GDB, kind='RUN'),
                         dict(target='libs/example/test//failure', kind='RUN_FAIL')]
                reports[label] = dict(suites=[dict(tests=tests)])
            with mock.patch('builtins.print'):
                value = boost_runtime.campaign(dict(compilers=reports), root / 'runtime', 2)
            boost_runtime.validate(value, tests)
            self.assertEqual(value['selected'], 2)
            self.assertEqual(value['output_equality_cases'], 1)
            manifest = json.loads((root / 'runtime/manifest.json').read_text())
            for target in manifest['cases']['cppgm']:
                self.assertEqual(len({manifest['cases'][label][target]['repeats'] for label in boost_runtime.LABELS}), 1)
            batches = [json.loads(line) for line in (root / 'runtime/batches.jsonl').read_text().splitlines()]
            self.assertEqual(len(batches), 30)  # six warmups, eight A/A, sixteen paired
            for batch in batches:
                rows = [json.loads(line) for line in (root / 'runtime' / batch['observations']).read_text().splitlines()]
                self.assertEqual({r['target'] for r in rows}, {'libs/example/test//fixed', 'libs/example/test//varying'})
                self.assertTrue(all(r['code'] == 0 for r in rows))
            damaged = copy.deepcopy(value)
            damaged['comparisons']['gcc']['blocks'].pop()
            with self.assertRaisesRegex(ValueError, 'paired block'):
                boost_runtime.validate(damaged, tests)
            damaged = copy.deepcopy(value)
            damaged['cases'].pop()
            with self.assertRaisesRegex(ValueError, 'cohort'):
                boost_runtime.validate(damaged, tests)

    def test_failed_execution_keeps_observation_and_output(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            test = dict(target='libs/example/test//fail', kind='RUN',
                        native_command=['/bin/sh', '-c', 'echo failure >&2; exit 1'], native_cwd=str(root))
            document = dict(compilers={label: dict(suites=[dict(tests=[test])]) for label in boost_runtime.LABELS})
            with self.assertRaisesRegex(ValueError, 'outcome changed'):
                boost_runtime.campaign(document, root / 'runtime', 1)
            receipt = next((root / 'runtime').glob('000-*.jsonl'))
            self.assertEqual(json.loads(receipt.read_text())['code'], 1)
            self.assertEqual(next((root / 'runtime').glob('failure-*/stderr')).read_text(), 'failure\n')

    def test_matching_warmup_output_does_not_define_an_upstream_oracle(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            test = dict(target='libs/example/test//rounded-time', kind='RUN',
                        native_command=['/bin/echo', 'same'], native_cwd=str(root))
            document = dict(compilers={label: dict(suites=[dict(tests=[test])]) for label in boost_runtime.LABELS})
            rows = [dict(cpu=.03, wall=.03, code=0, timed_out=False,
                         stdout_sha256='same', stderr_sha256='0ms' if n < 6 else '1ms') for n in range(30)]
            with mock.patch.object(boost_runtime, 'execute', side_effect=rows), mock.patch('builtins.print'):
                value = boost_runtime.campaign(document, root / 'runtime', 1)
            self.assertEqual(value['status'], 'passed')
            self.assertEqual(value['output_equality_cases'], 0)

    def test_sleeping_tests_are_not_repeated_for_low_cpu_usage(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            test = dict(target='libs/example/test//timer', kind='RUN',
                        native_command=['/bin/echo', 'same'], native_cwd=str(root))
            document = dict(compilers={label: dict(suites=[dict(tests=[test])]) for label in boost_runtime.LABELS})
            row = dict(cpu=.001, wall=2, code=0, timed_out=False, stdout_sha256='a', stderr_sha256='b')
            with mock.patch.object(boost_runtime, 'execute', return_value=row), mock.patch('builtins.print'):
                value = boost_runtime.campaign(document, root / 'runtime', 1)
            self.assertEqual(value['cases'][0]['repeats'], 1)


if __name__ == '__main__':
    unittest.main()
