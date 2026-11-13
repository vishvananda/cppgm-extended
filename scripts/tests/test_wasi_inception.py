"""Optional inception orchestration: fixed points, failure retention and isolation."""
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]


def module(name):
    spec = importlib.util.spec_from_file_location(name, ROOT / 'scripts/experimental' / (name + '.py'))
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


lane = module('run_inception')
frozen = module('wasi_inception')


class WasiInception(unittest.TestCase):
    def test_lane_requires_all_fixed_points_and_x86_levels(self):
        args = lane.arguments([])
        phases = lane.plan(args, Path('/new-package'))
        self.assertEqual([name for name, _ in phases], ['prepare', 'wasi', 'x86', 'chromium'])
        self.assertIn('--x86-headers', phases[0][1])
        self.assertNotIn('--stages', phases[1][1])  # default builds W1, W2 and W3
        self.assertEqual(phases[2][1][-4:], ['--levels', '0', '2', '3'])
        self.assertEqual(phases[3][1][-1], 'chromium')

    def test_failure_stops_later_phases_and_retains_observations(self):
        with tempfile.TemporaryDirectory() as directory:
            package = Path(directory) / 'frozen'
            args = lane.arguments(['--directory', str(package)])
            with patch.object(lane, 'preflight'), patch.object(lane, 'invoke', side_effect=[0, 7]) as run:
                with self.assertRaisesRegex(RuntimeError, 'wasi failed'):
                    lane.run(args)
            self.assertEqual(run.call_count, 2)
            rows = json.loads(package.with_name('frozen-logs').joinpath('results.json').read_text())
            self.assertEqual([row['exit'] for row in rows], [0, 7])
            self.assertTrue(all(row['seconds'] >= 0 for row in rows))
            with patch.object(lane, 'preflight'), patch.object(lane, 'invoke') as run:
                with self.assertRaisesRegex(RuntimeError, 'new package directory'):
                    lane.run(args)
                run.assert_not_called()

    def test_existing_package_cannot_be_overwritten(self):
        with tempfile.TemporaryDirectory() as directory:
            args = lane.arguments(['--directory', directory])
            with patch.object(lane, 'preflight'), patch.object(lane, 'invoke') as run:
                with self.assertRaisesRegex(RuntimeError, 'new package directory'):
                    lane.run(args)
                run.assert_not_called()

    def test_frozen_inputs_and_runner_mutations_are_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            paths = ['inputs/source.cpp', 'native-compiler', 'bundle.bin', 'shim/index.js', 'wasi_worker.mjs']
            for name in paths:
                path = root / name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(name.encode())
            expected = lambda name: frozen.digest((root / name).read_bytes())
            manifest = {'files': [{'path': 'source.cpp', 'sha256': expected(paths[0])}],
                        'native_sha256': expected(paths[1]), 'bundle_sha256': expected(paths[2]),
                        'shim_files': {paths[3]: expected(paths[3])},
                        'runner_files': {paths[4]: expected(paths[4])}}
            frozen.save(root / 'manifest.json', manifest)
            (root / 'manifest.sha256').write_text(expected('manifest.json'))
            frozen.verify(root, manifest)
            for name in paths + ['manifest.json']:
                path = root / name
                original = path.read_bytes()
                path.write_bytes(original + b'changed')
                with self.subTest(name=name), self.assertRaisesRegex(RuntimeError, 'changed'):
                    frozen.verify(root, manifest)
                path.write_bytes(original)

    def test_byte_comparison_rejects_prefix_and_content_changes(self):
        with tempfile.TemporaryDirectory() as directory:
            a, b = [Path(directory) / name for name in ('expected', 'actual')]
            a.write_bytes(b'abc')
            for value in (b'ab', b'abcd', b'axc'):
                b.write_bytes(value)
                with self.subTest(value=value), self.assertRaisesRegex(RuntimeError, 'byte mismatch'):
                    frozen.compare(a, b)
            b.write_bytes(b'abc')
            self.assertEqual(frozen.compare(a, b)['bytes'], 3)


if __name__ == '__main__':
    unittest.main()
