#!/usr/bin/env python3
"""Frozen internal timing must not change the cohort or overwrite another host."""
import copy
import io
import json
from pathlib import Path
import shutil
import sys
import tempfile
import tarfile
from types import SimpleNamespace
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import internal_cpp_benchmarks as bench


class InternalBenchmarkTests(unittest.TestCase):
    def baseline(self):
        return json.loads((bench.ROOT / 'benchmarks/internal/baseline.json').read_text())

    def test_cohort_keeps_original_qualified_sources_and_modes(self):
        cohort = json.loads(bench.COHORT.read_text())
        self.assertEqual(cohort['source_revision'], bench.SOURCE)
        self.assertEqual(len(cohort['cases']), 5614)
        self.assertEqual(sum(len(c['units']) for c in cohort['cases']), 5724)
        self.assertEqual(len({c['id'] for c in cohort['cases']}), 5614)
        self.assertEqual({c['dialect'] for c in cohort['cases']}, {'gnu++11', 'c++11', 'c++14', 'c++17'})
        for case in cohort['cases']:
            self.assertTrue(case['path'].startswith('pa'))
            for unit in case['units']:
                self.assertEqual(len(unit['sha256']), 64)
                self.assertNotIn('..', Path(unit['path']).parts)

    def test_incomplete_or_dirty_results_never_publish(self):
        for mutate in (lambda d: d.update(status='failed'),
                       lambda d: d['metadata'].update(dirty=True),
                       lambda d: d['compilation']['gcc'].update(accepted=5723),
                       lambda d: d['compilation']['clang'].update(units=5723),
                       lambda d: d['compilation']['cppgm'].update(cpu_seconds=0)):
            data = copy.deepcopy(self.baseline())
            mutate(data)
            with self.assertRaises(ValueError):
                bench.validate(data)

    def test_historical_seed_is_labeled_and_retained_when_fresh_data_exists(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            bench.seed(root)
            directory = root / 'internal'
            self.assertIn('Historical local snapshot', (directory / 'intel/report.md').read_text())
            self.assertIn('4.46×', (directory / 'intel-compile-gcc.svg').read_text())
            self.assertIn('>Intel</text>', (directory / 'intel-compile-gcc.svg').read_text())
            self.assertIn('>CI Intel</text>', (directory / 'github-intel-compile-gcc.svg').read_text())
            self.assertIn('n/m', (directory / 'epyc-compile-gcc.svg').read_text())
            data = self.baseline()
            data['metadata']['revision'] = 'b' * 40
            (directory / 'intel/results.json').write_text(json.dumps(data))
            bench.seed(root)
            self.assertEqual(json.loads((directory / 'intel/results.json').read_text())['metadata']['revision'], 'b' * 40)

    def test_single_case_records_real_object_cpu_and_source_hash(self):
        if not shutil.which('g++'):
            self.skipTest('g++ is required')
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            source = root / 'case.cpp'
            source.write_text('int answer() { return 42; }\n')
            case = dict(id='sample', path='case.cpp', dialect='gnu++11', flags=[],
                        units=[dict(path='case.cpp', sha256=bench.digest(source))])
            limits = dict(cpu_seconds=30, timeout_seconds=30, address_space_bytes=2*1024**3,
                          file_bytes=256*1024**2, compiler_error_scan=True)
            result = bench.compile_case((case, shutil.which('g++'), [], root, root, limits))
            self.assertTrue(result['accepted'])
            self.assertGreater(result['stages'][0]['cpu_seconds'], 0)
            self.assertEqual(result['stages'][0]['source_sha256'], bench.digest(source))
            self.assertEqual(len(result['stages'][0]['object_sha256']), 64)
            self.assertFalse((root / 'sample/0.o').exists())
            source.write_text('changed\n')
            case['id'] = 'damaged'
            with self.assertRaisesRegex(ValueError, 'source checksum'):
                bench.compile_case((case, shutil.which('g++'), [], root, root, limits))

    def test_report_is_compile_only_and_names_the_fixed_cohort(self):
        report = bench.report(self.baseline())
        self.assertIn('5,614 fixture groups / 5,724 translation units', report)
        self.assertIn('no application-runtime score', report)
        self.assertIn(bench.SOURCE, report)

    def test_verified_archive_keeps_companions_and_skips_harness_sources(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            cache = root / 'cache'
            cache.mkdir()
            archive = cache / (bench.SOURCE + '.tar.gz')
            with tarfile.open(archive, 'w:gz') as bundle:
                for name in ['checkout/pa10/tests/case.t', 'checkout/pa10/tests/case.h',
                             'checkout/scripts/tests/harness.py']:
                    member = tarfile.TarInfo(name)
                    member.size = 6
                    bundle.addfile(member, io.BytesIO(b'input\n'))
            with patch.object(bench, 'ARCHIVE_SHA256', bench.digest(archive)):
                bench.fetch_inputs(cache, root / 'source')
            self.assertEqual((root / 'source/pa10/tests/case.h').read_bytes(), b'input\n')
            self.assertFalse((root / 'source/scripts').exists())
            archive.write_bytes(b'changed archive')
            with self.assertRaisesRegex(ValueError, 'archive checksum'):
                bench.fetch_inputs(cache, root / 'other')

    def test_published_totals_must_match_every_unit_observation(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            case = dict(id='sample', path='case.t', units=[dict(path='case.t', sha256='a' * 64)])
            row = dict(id='sample', path='case.t', accepted=True, stages=[dict(
                unit='case.t', source_sha256='a' * 64, accepted=True, returncode=0,
                classification='success', object_sha256='b' * 64, object_bytes=64, cpu_seconds=1.25)])
            data = dict(compilation={role: dict(groups=1, units=1, accepted=1, cpu_seconds=1.25) for role in bench.ROLES})
            for role in bench.ROLES:
                (root / role).mkdir()
                (root / role / 'observations.jsonl').write_text(json.dumps(row) + '\n')
            bench.verify_observations(root, data, dict(cases=[case]))
            data['compilation']['cppgm']['cpu_seconds'] = 1.5
            with self.assertRaisesRegex(ValueError, 'disagree'):
                bench.verify_observations(root, data, dict(cases=[case]))

    def test_shared_records_keep_this_source_projection_separate(self):
        import benchmark_report
        import internal_cpp_results
        data = self.baseline()
        data['metadata']['cohort_sha256'] = bench.digest(bench.COHORT)
        records = internal_cpp_results.normalize(data, 'Dedicated Intel; Linux',
                    'https://example.org/run', 'https://example.org/report', 'https://example.org/raw')
        catalog = json.loads((bench.ROOT / 'benchmarks/catalog.json').read_text())
        benchmark_report.validate(catalog, records)
        self.assertEqual(len(records['records']), 2)
        self.assertEqual(records['records'][0]['coverage']['selected'], 5724)
        self.assertTrue(all(r['phase'] == 'compile' for r in records['records']))


if __name__ == '__main__':
    unittest.main()
