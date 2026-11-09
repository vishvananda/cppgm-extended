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
import godot_results
import benchmark_report


class GodotCITests(unittest.TestCase):
    def observations(self):
        return [dict(workload=name, phase='AB-' + ref, label=label, cpu=cpu, measured_wall=cpu / 2)
                for name in ci.WORKLOADS for ref in ('gcc', 'clang')
                for label, cpu in [(ref, 2), ('cppgm', 1)]]

    def document(self):
        interval = dict(speed=2, blocks=[2], calibration_before=dict(speed=1, blocks=[1]),
                        calibration_after=dict(speed=1, blocks=[1]))
        pair = {ref: {key: copy.deepcopy(interval) for key in ('cpu', 'measured_wall')}
                for ref in ('gcc', 'clang')}
        return dict(status='complete', source_commit=ci.COMMIT,
                    metadata=dict(revision='a' * 40, dirty=False, cpu='AMD EPYC example',
                                  cpus='0,1', runtime_cpu='0', flags=ci.BUILD_FLAGS,
                                  workload_hashes={'workload.gd': 'b' * 64},
                                  toolchains={label: 'example compiler' for label in ci.LABELS}),
                    compilation={label: dict(status=0, cpu=100, wall=50, command=['scons', '-j2'])
                                 for label in ci.LABELS},
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
            (artifact / 'compile-observations.jsonl').write_text('')
            (artifact / 'runtime-observations.jsonl').write_text(
                ''.join(json.dumps(row) + '\n' for row in self.observations()))
            target = root / 'published'
            previous = target / 'godot/epyc/report.md'
            previous.parent.mkdir(parents=True)
            previous.write_text('last EPYC')
            ci.publish(SimpleNamespace(artifact=artifact, target=target, revision='a' * 40,
                                       run_url='https://example.test/run'))
            self.assertEqual(previous.read_text(), 'last EPYC')
            for path in ('godot/github-intel/results.json', 'godot/history/' + 'a' * 40 + '/github-intel/results.json'):
                self.assertEqual((target / path).read_bytes(), (artifact / 'results.json').read_bytes())
            shared = json.loads((target / 'godot/github-intel/standard/records.json').read_text())
            self.assertEqual(len(shared['records']), 28)
            self.assertTrue((target / 'godot/github-intel/standard/godot.md').is_file())

    def test_dedicated_intel_preserves_hosted_results_and_labels_environment(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            artifact = root / 'artifact'
            artifact.mkdir()
            document = self.document()
            document['metadata']['cpu'] = 'Intel Xeon E5-2696 v4'
            ci.save(artifact / 'results.json', document)
            ci.save(artifact / 'metadata.json', document['metadata'])
            (artifact / 'compile-observations.jsonl').write_text('')
            (artifact / 'runtime-observations.jsonl').write_text(''.join(json.dumps(r) + '\n' for r in self.observations()))
            target = root / 'published'
            for host in ('epyc', 'github-intel'):
                path = target / 'godot' / host / 'report.md'
                path.parent.mkdir(parents=True)
                path.write_text('last ' + host)
            ci.publish(SimpleNamespace(artifact=artifact, target=target, revision='a' * 40,
                                       run_url='https://example.test/run', platform='intel'))
            for host in ('epyc', 'github-intel'):
                self.assertEqual((target / 'godot' / host / 'report.md').read_text(), 'last ' + host)
            shared = json.loads((target / 'godot/intel/standard/records.json').read_text())
            self.assertTrue(all('Dedicated Intel' in r['host']['environment'] for r in shared['records']))
            self.assertIn('1.00×', (target / 'godot/intel-build-gcc.svg').read_text())

    def test_shared_records_preserve_phase_metric_method_and_exact_host(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            ci.save(root / 'results.json', self.document())
            rows = self.observations()
            (root / 'runtime-observations.jsonl').write_text(''.join(json.dumps(r) + '\n' for r in rows))
            def normalized(environment='test host'):
                return godot_results.normalize(root, environment, *(['https://example.test/evidence'] * 3))
            doc = normalized()
            catalog = json.loads((ci.ROOT / 'benchmarks/catalog.json').read_text())
            benchmark_report.validate(catalog, doc)
            self.assertEqual(len(doc['records']), 28)
            builds = [r for r in doc['records'] if r['phase'] == 'build']
            self.assertEqual(len(builds), 4)
            self.assertTrue(all(r['test'] == 'full-build' and r['context']['workers'] == 2 for r in builds))
            run = next(r for r in doc['records'] if r['test'] == 'scene' and r['baseline'] == 'gcc'
                       and r['metric'] == 'cpu_seconds')
            self.assertEqual((run['speedup'], run['candidate_seconds'], run['reference_seconds']), (2, 1, 2))
            aggregate = next(r for r in doc['records'] if r['test'] == 'checked-workloads')
            self.assertEqual(aggregate['coverage']['selected'], 5)
            self.assertIsNone(aggregate['candidate_seconds'])
            self.assertIn('Equal-weight geometric mean', aggregate['method'])
            self.assertIn('system', aggregate['provenance']['historical_unknowns'])
            self.assertNotEqual(doc['records'][0]['host']['id'], normalized('another host')['records'][0]['host']['id'])
            changed = self.document()
            changed['metadata']['cpu'] = 'another EPYC model'
            ci.save(root / 'results.json', changed)
            self.assertNotEqual(doc['records'][0]['host']['id'], normalized()['records'][0]['host']['id'])
            changed['correctness']['scene']['cppgm'] += 1
            ci.save(root / 'results.json', changed)
            with self.assertRaises(ValueError):
                normalized()
            ci.save(root / 'results.json', self.document())
            (root / 'runtime-observations.jsonl').write_text('')
            with self.assertRaisesRegex(ValueError, 'missing raw runtime'):
                normalized()


if __name__ == '__main__':
    unittest.main()
