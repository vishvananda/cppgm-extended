#!/usr/bin/env python3
"""Live badges must use the published host data and report estimators."""
import json
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import library_benchmark_badges as badges


def godot():
    return dict(status='complete', metadata=dict(dirty=False, cpu='AMD EPYC test CPU', revision='a' * 40),
                compilation=dict(cppgm=dict(cpu=2), gcc=dict(cpu=6), clang=dict(cpu=8)),
                runtime={name: {ref: dict(cpu=dict(speed=value)) for ref in ('gcc', 'clang')}
                         for name, value in [('first', .5), ('second', 2)]})


class LiveBadgeTests(unittest.TestCase):
    def test_godot_uses_build_cpu_and_equal_workload_runtime_weight(self):
        values = badges.metrics('godot', godot())
        self.assertEqual(values['build-gcc'], 3)
        self.assertEqual(values['build-clang'], 4)
        self.assertAlmostEqual(values['runtime-gcc'], 1)

    def test_simd_modes_keep_their_own_compile_and_runtime_estimates(self):
        document = dict(modes={mode: dict(
            compilation={ref: dict(cpu=dict(speed=value)) for ref in ('gcc', 'clang')},
            runtime={'one': {ref: dict(cpu=dict(speed=value)) for ref in ('gcc', 'clang')}})
            for mode, value in [('scalar', 2), ('sse', 4)]})
        values = badges.metrics('simdjson', document)
        self.assertEqual(values['scalar-compile-gcc'], 2)
        self.assertEqual(values['sse-runtime-clang'], 4)

    def test_index_runtime_uses_the_reported_elapsed_workload_estimate(self):
        document = dict(compilation={ref: dict(cpu=dict(speed=3)) for ref in ('gcc', 'clang')},
                        runtime=dict(pairs={ref + ':all': dict(geometric_speed=2) for ref in ('gcc', 'clang')}))
        self.assertEqual(badges.metrics('index-cpp', document)['runtime-gcc'], 2)

    def test_host_values_provenance_and_missing_measurements(self):
        with tempfile.TemporaryDirectory() as tmp:
            directory = Path(tmp)
            (directory / 'epyc').mkdir()
            (directory / 'epyc/results.json').write_text(json.dumps(godot()))
            badges.library_badges(directory, 'godot')
            value = (directory / 'epyc-build-gcc.svg').read_text()
            self.assertIn('3.00×', value)
            self.assertIn('revision ' + 'a' * 40, value)
            self.assertIn('AMD EPYC test CPU', value)
            self.assertIn('n/m', (directory / 'github-intel-runtime-clang.svg').read_text())
            self.assertIn('rx="4"', value)
            self.assertIn('font-weight="700"', value)
            damaged = godot()
            damaged['metadata']['dirty'] = True
            (directory / 'epyc/results.json').write_text(json.dumps(damaged))
            with self.assertRaisesRegex(ValueError, 'clean'):
                badges.library_badges(directory, 'godot')
            self.assertEqual((directory / 'epyc-build-gcc.svg').read_text(), value)

    def test_public_clang_direction_and_other_host_preservation(self):
        with tempfile.TemporaryDirectory() as tmp:
            directory = Path(tmp)
            for host in ('epyc', 'intel'):
                (directory / host).mkdir()
            other = directory / 'intel/o3-generated.svg'
            other.write_text('previous dedicated Intel result')
            document = dict(programs=[dict(status='ok', cppgm=dict(run_cpu_O3=2),
                                          gcc=dict(run_cpu_O3=4), clang=dict(run_cpu_O3=6))])
            (directory / 'epyc/results.json').write_text(json.dumps(document))
            (directory / 'epyc/provenance.json').write_text(json.dumps(dict(cpu='AMD EPYC', revision='a' * 40)))
            badges.public_badges(directory, 'epyc')
            self.assertIn('2.00×', (directory / 'epyc/o3-generated.svg').read_text())
            self.assertIn('3.00×', (directory / 'epyc/o3-generated-clang.svg').read_text())
            self.assertEqual(other.read_text(), 'previous dedicated Intel result')
            self.assertIn('epyc/report.md', (directory / 'public-hosts.md').read_text())

    def test_nonpositive_runtime_cannot_become_a_badge(self):
        document = godot()
        document['runtime']['first']['gcc']['cpu']['speed'] = 0
        with self.assertRaises(ValueError):
            badges.metrics('godot', document)


if __name__ == '__main__':
    unittest.main()
