#!/usr/bin/env python3
"""Godot reports require complete builds, calibrated times and known results."""
import copy
import json
from pathlib import Path
import sys
import tempfile
from types import SimpleNamespace
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import godot_ci as ci


class GodotCITests(unittest.TestCase):
    def document(self):
        interval = dict(speed=2, blocks=[2], calibration_before=dict(speed=1, blocks=[1]),
                        calibration_after=dict(speed=1, blocks=[1]))
        pair = {ref: {key: copy.deepcopy(interval) for key in ('cpu', 'measured_wall')}
                for ref in ('gcc', 'clang')}
        return dict(status='complete', source_commit=ci.COMMIT,
                    metadata=dict(revision='a' * 40, dirty=False, cpu='AMD EPYC example',
                                  toolchains={label: 'example compiler' for label in ci.LABELS}),
                    compilation={label: dict(status=0, cpu=100, wall=50) for label in ci.LABELS},
                    runtime={name: copy.deepcopy(pair) for name in ci.WORKLOADS},
                    correctness={name: {label: ci.expected(name, 100000) for label in ci.LABELS}
                                 for name in ci.WORKLOADS})

    def test_known_workloads_have_independent_small_results(self):
        self.assertEqual(ci.expected('dispatch', 4), -1)
        self.assertEqual(ci.expected('arrays', 4), 102)
        self.assertEqual(ci.expected('strings', 4), 80)
        self.assertEqual(ci.expected('geometry', 4), 0)
        self.assertEqual(ci.expected('scene', 4), 32)

    def test_output_requires_identity_checksum_and_positive_elapsed_time(self):
        result = dict(workload='arrays', iterations=4, repeats=2, checksum=204, elapsed_usec=10)
        text = 'CPPGM_GODOT ' + json.dumps(result)
        self.assertEqual(ci.parse_output('Engine banner\n' + text, 'arrays', 4, 2), result)
        for key, value in [('checksum', 205), ('repeats', 1), ('elapsed_usec', 0), ('workload', 'scene')]:
            changed = dict(result, **{key: value})
            with self.subTest(key=key), self.assertRaises(ValueError):
                ci.parse_output('CPPGM_GODOT ' + json.dumps(changed), 'arrays', 4, 2)
        with self.assertRaises(ValueError):
            ci.parse_output(text + '\n' + text, 'arrays', 4, 2)

    def test_complete_evidence_is_required(self):
        ci.validate(self.document())
        for change in ('build', 'checksum', 'timing', 'calibration', 'missing'):
            document = self.document()
            if change == 'build':
                document['compilation']['cppgm']['status'] = 1
            elif change == 'checksum':
                for label in ci.LABELS:
                    document['correctness']['scene'][label] += 1
            elif change == 'timing':
                document['runtime']['scene']['gcc']['cpu']['speed'] = float('nan')
            elif change == 'calibration':
                document['runtime']['scene']['gcc']['cpu']['calibration_before']['blocks'] = []
            else:
                del document['runtime']['scene']
            with self.subTest(change=change), self.assertRaises(ValueError):
                ci.validate(document)

    def test_report_distinguishes_build_observations_and_paired_runtime(self):
        text = ci.report(self.document())
        self.assertIn('one clean build per compiler', text)
        self.assertIn('reference time / cppgm time: higher is better', text)
        self.assertIn('work wall time excludes engine startup', text.lower())
        self.assertIn('| geometry | 2.000× | 2.000× |', text)

    def test_dirty_or_mismatched_revision_cannot_replace_last_good_report(self):
        for dirty, revision in [(True, 'a' * 40), (False, 'b' * 40)]:
            with self.subTest(dirty=dirty), tempfile.TemporaryDirectory() as temporary:
                root = Path(temporary)
                artifact = root / 'artifact'
                artifact.mkdir()
                document = self.document()
                document['metadata']['dirty'] = dirty
                ci.save(artifact / 'results.json', document)
                target = root / 'published'
                previous = target / 'godot/epyc/report.md'
                previous.parent.mkdir(parents=True)
                previous.write_text('last good')
                with self.assertRaises(ValueError):
                    ci.publish(SimpleNamespace(artifact=artifact, target=target, revision=revision,
                                               run_url='https://example.test/run'))
                self.assertEqual(previous.read_text(), 'last good')

    def test_hosted_intel_publication_retains_epyc_and_history(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            artifact = root / 'artifact'
            artifact.mkdir()
            document = self.document()
            document['metadata']['cpu'] = 'Intel Xeon Platinum 8370C'
            ci.save(artifact / 'results.json', document)
            ci.save(artifact / 'metadata.json', document['metadata'])
            for name in ('compile-observations.jsonl', 'runtime-observations.jsonl'):
                (artifact / name).write_text('{}\n')
            target = root / 'published'
            previous = target / 'godot/epyc/report.md'
            previous.parent.mkdir(parents=True)
            previous.write_text('last EPYC')
            ci.publish(SimpleNamespace(artifact=artifact, target=target, revision='a' * 40,
                                       run_url='https://example.test/run'))
            self.assertEqual(previous.read_text(), 'last EPYC')
            for path in ('godot/github-intel/results.json', 'godot/history/' + 'a' * 40 + '/github-intel/results.json'):
                self.assertEqual((target / path).read_bytes(), (artifact / 'results.json').read_bytes())


if __name__ == '__main__':
    unittest.main()
