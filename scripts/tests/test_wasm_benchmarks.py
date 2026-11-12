"""Maintainer checks for the Perl measurements and normalized publication."""
import argparse
import copy
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import benchmark_publication
import benchmark_report
import wasm_benchmark_results as producer

ROOT = Path(__file__).resolve().parents[2]


def document(level='O3'):
    def estimate(ratio):
        return dict(candidate=1., reference=ratio, speedup=ratio, ratios=[ratio]*3)
    phases = {}
    for phase in ('compile', 'link', 'startup', *producer.CASES):
        phases[phase] = {mode: {metric: estimate(1. if mode == 'AA' else 1.5)
                               for metric in ('seconds', 'cpu_seconds', 'setup_seconds')}
                         for mode in ('AA', 'AB')}
    config = dict(id='scalar', phases=phases,
                  cases=[[k, count, seed, 17] for k in range(4) for count in (1, 3)
                         for seed in (17, 42, 0x80000000, 0xffffffff)],
                  module_sizes={side: dict(module=2000, code=1000, data=0) for side in ('cppgm', 'clang')},
                  module_sha256={side: 'b'*64 for side in ('cppgm', 'clang')})
    other = copy.deepcopy(config)
    other['id'] = 'simd128'
    metadata = dict(revision='a'*40, dirty=False, cpu='Intel unit-test CPU', architecture='x86_64',
                    operating_system='Linux', affinity=[0], source_sha256='b'*64,
                    compiler_sha256='c'*64, clang_sha256='d'*64, engine_sha256='e'*64,
                    engine=dict(version='50.0.1'), optimization=level,
                    flags=dict(cppgm=['-'+level], clang=['-'+level, '-msimd128']))
    return dict(schema=1, status='complete', metadata=metadata,
                configurations=[config, other], warnings=[])


class WasmBenchmarkTests(unittest.TestCase):
    def perl(self, code, *args):
        return subprocess.run(['perl', '-I', str(ROOT / 'scripts'), '-MCppgmWasmBenchmark',
                               '-MJSON::PP', '-e', code, *map(str, args)], capture_output=True, text=True)

    def test_independent_unsigned_oracles_preserve_wrapped_integer_products(self):
        result = self.perl('print CppgmWasmBenchmark::oracle($_,3,17), "\\n" for 0..3')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout.splitlines(), ['2528231397', '2828970141', '1174712506', '1615917772'])
        result = self.perl('print CppgmWasmBenchmark::mul32(0xffffffff,0xffffffff)')
        self.assertEqual(result.stdout, '1')

    def test_paired_estimator_rejects_missing_and_misordered_observations(self):
        code = 'my $v=decode_json($ARGV[0]); print encode_json(CppgmWasmBenchmark::summarize($v,"seconds"))'
        rows = [dict(slot=slot, seconds=value) for slot, value in zip([0, 1, 1, 0], [2, 3, 5, 6])]
        result = self.perl(code, json.dumps(rows))
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(json.loads(result.stdout)['speedup'], 1)
        for invalid in (rows[:-1], [rows[1], rows[0], rows[2], rows[3]]):
            self.assertNotEqual(self.perl(code, json.dumps(invalid)).returncode, 0)

    def test_shared_report_keeps_sizes_setup_and_unavailable_gcc_explicit(self):
        normalized = producer.normalized([document()], 'unit test', 'https://example.org/run',
                                         'https://example.org/report', 'https://example.org/raw')
        catalog = json.loads((ROOT / 'benchmarks/catalog.json').read_text())
        benchmark_report.validate(catalog, normalized)
        self.assertTrue(all(r['status'] == 'not-applicable' and r['speedup'] is None
                            for r in normalized['records'] if r['baseline'] == 'gcc'))
        sizes = [r for r in normalized['records'] if r['metric'] == 'bytes' and r['baseline'] == 'clang']
        self.assertEqual(len(sizes), 6)
        self.assertTrue(all(r['candidate_seconds'] is None for r in sizes))
        self.assertEqual([r['candidate_value'] for r in sizes if r['test'] == 'data-size'], [0, 0])
        self.assertEqual(len([r for r in normalized['records'] if r['test'].endswith('-input-preparation')]), 16)

    def test_loader_rejects_mixed_hosts_inputs_and_incomplete_oracles(self):
        with tempfile.TemporaryDirectory() as directory:
            paths = [Path(directory) / name for name in ('O0', 'O3')]
            documents = [document('O0'), document('O3')]
            for path, value in zip(paths, documents):
                path.mkdir()
                (path / 'results.json').write_text(json.dumps(value))
                (path / 'metadata.json').write_text(json.dumps(value['metadata']))
            self.assertEqual(len(producer.load(paths)), 2)
            for field in ('cpu', 'source_sha256', 'compiler_sha256', 'engine_sha256'):
                value = copy.deepcopy(documents[1])
                value['metadata'][field] = 'different'
                (paths[1] / 'results.json').write_text(json.dumps(value))
                (paths[1] / 'metadata.json').write_text(json.dumps(value['metadata']))
                with self.subTest(field=field), self.assertRaisesRegex(ValueError, 'mixed compiler/input/host/engine'):
                    producer.load(paths)
            value = copy.deepcopy(documents[0])
            value['configurations'][0]['cases'][0] = value['configurations'][0]['cases'][1]
            (paths[0] / 'results.json').write_text(json.dumps(value))
            with self.assertRaisesRegex(ValueError, 'oracle coverage'):
                producer.load(paths[:1])

    def test_publication_is_compact_and_preserves_host_and_revision(self):
        with tempfile.TemporaryDirectory() as directory:
            target = Path(directory)
            args = argparse.Namespace(target=target, revision='a'*40, platform='github', run_url='https://example.org/run')
            producer.publish(args, [document('O0'), document('O3')])
            self.assertTrue((target / 'wasm/history' / ('a'*40) / 'github-intel/standard/wasm.md').is_file())
            svg = (target / 'wasm/github-intel-simd128-runtime-clang.svg').read_text()
            self.assertIn('1.50', svg)
            self.assertIn('CI Intel', svg)
            self.assertLess(benchmark_publication.prepare(target)['bytes'], 2*1024*1024)
            dirty = document()
            dirty['metadata']['dirty'] = True
            with self.assertRaisesRegex(ValueError, 'clean measured'):
                producer.publish(args, [dirty])

    def test_initial_badges_are_placeholders_without_invented_measurements(self):
        with tempfile.TemporaryDirectory() as directory:
            target = Path(directory)
            producer.seed(target)
            self.assertEqual(len(list((target / 'wasm').glob('*.svg'))), 12)
            self.assertIn('Not measured yet', (target / 'wasm/report.md').read_text())
            for path in (target / 'wasm').glob('*.svg'):
                self.assertIn('n/m', path.read_text())
            self.assertFalse(list(target.rglob('metadata.json')))


if __name__ == '__main__':
    unittest.main()
