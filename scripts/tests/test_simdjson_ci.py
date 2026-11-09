#!/usr/bin/env python3
"""The simdjson report must compare paired times and reject incomplete evidence."""
import copy
from pathlib import Path
import sys
from types import SimpleNamespace
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import simdjson_ci as ci


class SimdjsonCITests(unittest.TestCase):
    def document(self):
        interval = dict(speed=2, blocks=[2, 2], calibration_before=dict(speed=1, blocks=[1]),
                        calibration_after=dict(speed=1, blocks=[1]))
        pair = {ref: {key: copy.deepcopy(interval) for key in ('cpu', 'wall')} for ref in ('gcc', 'clang')}
        return dict(status='complete', source_commit=ci.COMMIT,
                    metadata=dict(revision='a' * 40, dirty=False, cpu='AMD EPYC example',
                                  toolchains={label: 'example compiler' for label in ci.LABELS}),
                    modes={mode: dict(compilation=copy.deepcopy(pair),
                                      runtime={name: copy.deepcopy(pair) for name in ci.INPUTS},
                                      correctness={name: {label: 'b' * 64 for label in ci.LABELS} for name in ci.INPUTS})
                           for mode in ci.MODES})

    def test_block_ratios_resist_unpaired_drift(self):
        rows = [dict(block=block, side=side, cpu=cpu) for block, scale in ((0, 1), (1, 10), (2, 100))
                for side, cpu in (('A', 2 * scale), ('B', scale), ('B', scale), ('A', 2 * scale))]
        self.assertEqual(ci.summarize(rows, 'cpu')['speed'], 2)
        rows[0]['cpu'] = 0
        rows[3]['cpu'] = 0
        with self.assertRaises(ValueError):
            ci.summarize(rows, 'cpu')

    def test_cross_backend_disagreement_is_not_publishable(self):
        document = self.document()
        ci.validate(document)
        for label in ci.LABELS:
            document['modes']['sse']['correctness'][ci.INPUTS[0]][label] = 'c' * 64
        with self.assertRaisesRegex(ValueError, 'scalar/SSE'):
            ci.validate(document)

    def test_missing_and_nonfinite_measurements_are_rejected(self):
        for change in ('missing', 'nonfinite', 'failed'):
            document = self.document()
            if change == 'missing':
                del document['modes']['sse']['runtime'][ci.INPUTS[0]]
            elif change == 'nonfinite':
                document['modes']['scalar']['compilation']['gcc']['cpu']['speed'] = float('nan')
            else:
                document['status'] = 'failed'
            with self.subTest(change=change), self.assertRaises(ValueError):
                ci.validate(document)

    def test_rejected_publication_preserves_last_good_report(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            artifact = root / 'artifact'
            artifact.mkdir()
            document = self.document()
            ci.save(artifact / 'results.json', document)
            target = root / 'target'
            previous = target / 'simdjson/epyc/report.md'
            previous.parent.mkdir(parents=True)
            previous.write_text('last complete run')
            args = SimpleNamespace(artifact=artifact, target=target, revision='d' * 40, run_url='https://example.com/run')
            with self.assertRaises(ValueError):
                ci.publish(args)
            self.assertEqual(previous.read_text(), 'last complete run')

    def test_report_labels_direction_and_both_backends(self):
        text = ci.report(self.document())
        self.assertIn('reference time / cppgm time', text)
        self.assertIn('| scalar | Compile | 2.000× | 2.000× |', text)
        self.assertIn('| sse | twitter.json |', text)
        self.assertIn('vs Clang CPU', text)

    def test_publication_accepts_hosted_intel_without_replacing_epyc(self):
        for cpu, requested, platform in [('Intel Xeon Platinum 8370C', 'github', 'github-intel'),
                                         ('AMD EPYC 7763', 'github', 'epyc'),
                                         ('Intel Xeon E5-2696 v4', 'intel', 'intel')]:
            with self.subTest(cpu=cpu), tempfile.TemporaryDirectory() as temporary:
                root = Path(temporary)
                artifact = root / 'artifact'
                artifact.mkdir()
                document = self.document()
                document['metadata']['cpu'] = cpu
                ci.save(artifact / 'results.json', document)
                ci.save(artifact / 'metadata.json', document['metadata'])
                for name in ('compile-observations.jsonl', 'runtime-observations.jsonl'):
                    (artifact / name).write_text('{}\n')
                target = root / 'target'
                epyc = target / 'simdjson/epyc/report.md'
                epyc.parent.mkdir(parents=True)
                epyc.write_text('previous EPYC run')
                ci.publish(SimpleNamespace(artifact=artifact, target=target, revision='a' * 40,
                                           run_url='https://example.com/run', platform=requested))
                for destination in ('simdjson/' + platform, 'simdjson/history/' + 'a' * 40 + '/' + platform):
                    self.assertEqual((target / destination / 'results.json').read_bytes(),
                                     (artifact / 'results.json').read_bytes())
                    self.assertIn(cpu, (target / destination / 'report.md').read_text())
                if cpu.startswith('AMD EPYC'):
                    self.assertIn(cpu, epyc.read_text())
                else:
                    self.assertEqual(epyc.read_text(), 'previous EPYC run')
                self.assertIn(cpu, (target / 'simdjson/report.md').read_text())

    def test_mismatched_provenance_is_rejected_before_publication(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            artifact = root / 'artifact'
            artifact.mkdir()
            document = self.document()
            ci.save(artifact / 'results.json', document)
            ci.save(artifact / 'metadata.json', dict(document['metadata'], cpu='Intel Xeon'))
            for name in ('compile-observations.jsonl', 'runtime-observations.jsonl'):
                (artifact / name).write_text('{}\n')
            target = root / 'target'
            args = SimpleNamespace(artifact=artifact, target=target, revision='a' * 40,
                                   run_url='https://example.com/run')
            with self.assertRaisesRegex(ValueError, 'provenance'):
                ci.publish(args)
            self.assertFalse(target.exists())


if __name__ == '__main__':
    unittest.main()
