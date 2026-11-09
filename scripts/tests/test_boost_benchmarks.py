#!/usr/bin/env python3
"""Publication must use identical reviewed Boost cases and complete timing."""
import argparse
import copy
import hashlib
import json
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


if __name__ == '__main__':
    unittest.main()
