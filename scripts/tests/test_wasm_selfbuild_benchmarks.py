"""Coverage and timing checks for the separate complete WASI rebuild campaign."""
import argparse
import json
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import wasm_selfbuild_benchmarks as bench


class SelfBuildTests(unittest.TestCase):
    def test_paired_cpu_ratios_use_cpu_not_wall_and_require_complete_blocks(self):
        rows = [dict(slot=slot, seconds=wall, cpu_seconds=cpu)
                for slot, wall, cpu in zip([0, 1, 1, 0], [20, 3, 5, 60], [2, 6, 10, 6])]
        self.assertEqual(bench.paired(rows, 'cpu_seconds')['speedup'], 2)
        self.assertEqual(bench.paired(rows, 'seconds')['speedup'], .1)
        with self.assertRaises(ValueError):
            bench.paired(rows[:-1], 'cpu_seconds')
        rows[0]['slot'] = 1
        with self.assertRaises(ValueError):
            bench.paired(rows, 'cpu_seconds')

    def test_producers_share_headers_feature_defines_and_aliasing_profile(self):
        flags = ['--target=wasm32-wasip1', '--sysroot=sysroot', '-resource-dir=resource',
                 '-include', 'wasi_host.h', '-D__cpp_exceptions=199711L', '-D__cpp_rtti=199711L', '-O3']
        manifest = dict(flags=flags)
        candidate, reference = bench.matched_flags(manifest), bench.matched_flags(manifest, True)
        self.assertEqual(reference[:len(candidate)], candidate)
        self.assertIn('-fno-strict-aliasing', candidate)
        self.assertIn('sysroot/include/wasm32-wasip1/eh/c++/v1', reference)
        self.assertIn('-fwasm-exceptions', reference)
        self.assertEqual(manifest['flags'], flags)

    def test_full_output_check_includes_every_object_and_final_module(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            package = root / 'package'
            inputs = package / 'inputs'
            inputs.mkdir(parents=True)
            for name in ('a.cpp', 'b.cpp'):
                (inputs / name).write_text(name)
            compiler = root / 'compiler.py'
            compiler.write_text('''#!/usr/bin/env python3
import pathlib, sys
args = sys.argv[1:]
out = pathlib.Path(args[args.index('-o')+1])
if '-c' in args:
    out.write_bytes(pathlib.Path(args[args.index('-c')+1]).read_bytes())
else:
    out.write_bytes(b''.join(pathlib.Path(p).read_bytes() for p in args if p.endswith('.o')))
''')
            compiler.chmod(0o755)
            args = argparse.Namespace(output=root, jobs=2, timeout=10)
            manifest = dict(sources=['a.cpp', 'b.cpp'], flags=[], link_flags=[])
            campaign = bench.Campaign(args, package, manifest)
            _, campaign.expected, _ = campaign.build([str(compiler)], 'gold', [])
            _, outputs, row = campaign.build([str(compiler)], 'same', [])
            self.assertEqual(set(outputs), {'a.cpp', 'b.cpp', 'cppgm.wasm'})
            self.assertEqual(row['sources'], 2)
            self.assertEqual(len(json.loads((root / 'observations.json').read_text())), 2)
            with self.assertRaisesRegex(RuntimeError, 'output set'):
                bench.outputs_equal(campaign.expected, {'cppgm.wasm': outputs['cppgm.wasm']})
            (outputs['b.cpp']).write_bytes(b'changed object')
            with self.assertRaises(RuntimeError):
                bench.outputs_equal(campaign.expected, outputs)
            _, outputs, _ = campaign.build([str(compiler)], 'again', [])
            outputs['cppgm.wasm'].write_bytes(b'changed link')
            with self.assertRaises(RuntimeError):
                bench.outputs_equal(campaign.expected, outputs)


if __name__ == '__main__':
    unittest.main()
