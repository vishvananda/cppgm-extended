#!/usr/bin/env python3
"""Measurement failures must stay visible in index-cpp receipts."""

from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import index_cpp_benchmarks as bench


class IndexCppBenchmarksTests(unittest.TestCase):
    def test_parse_ignores_timestamp_but_requires_checksums_and_inventory(self):
        text = 'start: 1234\nHash::CRC32: OK in 0.320s\nSummary: 0.3201s, 1, 1, 0\n'
        self.assertEqual(bench.parse_output(text, ['Hash::CRC32']), {'Hash::CRC32': .320})
        for wrong in [text.replace('OK', 'ERR[actual=1, expected=2]'),
                      text.replace(', 1, 1, 0', ', 2, 2, 0'),
                      text.split('Summary:')[0], 'start: 1234\n',
                      text.replace('Summary:', 'Hash::CRC32: OK in 0.1s\nSummary:')]:
            with self.subTest(text=wrong), self.assertRaises(ValueError):
                bench.parse_output(wrong)
        with self.assertRaises(ValueError):
            bench.parse_output(text, ['Calculator::Ast'])

    def test_counters_reject_missing_or_multiplexed_events(self):
        good = ''.join(f'123;;{event};123456;100.00;;\n' for event in bench.COUNTERS)
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / 'counts'
            path.write_text(good)
            self.assertEqual(bench.parse_counters(path)['instructions'], 123)
            for bad in [good.replace('100.00', '70.00', 1),
                        good.replace('100.00', 'nan', 1),
                        good.replace('123;;instructions:u', '<not supported>;;instructions:u'),
                        good.splitlines()[0] + '\n']:
                path.write_text(bad)
                with self.subTest(text=bad), self.assertRaises(ValueError):
                    bench.parse_counters(path)

    def test_summary_uses_paired_block_means_with_higher_is_better(self):
        # A later block runs slower on both sides: independent minima would
        # hide that change, but each block still shows the same 2x speed.
        rows = [dict(block=block, side=side, cpu=scale * value,
                     cases={'work': scale * value}, instructions=scale * value * 100)
                for block, scale in enumerate((1, 3))
                for side, value in [('A', 4), ('B', 2), ('B', 2), ('A', 4)]]
        result = bench.summarize(rows, ['work'])
        self.assertEqual(result['loops']['work'], dict(speed=2, blocks=[2, 2]))
        self.assertEqual(result['process_cpu']['speed'], 2)
        self.assertEqual(result['geometric_speed'], 2)

    def test_calibration_warns_about_noise_without_dropping_cases(self):
        calibration = dict(loops={'stable': dict(speed=1.001), 'noisy': dict(speed=.9),
                                  'too-short': dict(speed=None)})
        self.assertEqual(bench.drift_warnings(calibration, .02), ['noisy', 'too-short'])
        self.assertEqual(len(calibration['loops']), 3)


if __name__ == '__main__':
    unittest.main()
