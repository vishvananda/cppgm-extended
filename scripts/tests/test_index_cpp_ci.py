#!/usr/bin/env python3
"""Index-cpp CI must retain failures and publish only complete comparisons."""
import copy
import json
import os
import subprocess
from pathlib import Path
import sys
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import index_cpp_benchmarks as bench
import index_cpp_ci as ci


class IndexCppCITests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.artifact = self.root / 'artifact'
        (self.artifact / 'runtime').mkdir(parents=True)
        self.revision = 'a' * 40
        names = [f'work-{i:02}' for i in range(50)]
        self.document = dict(status='complete', source_commit=bench.UPSTREAM_COMMIT, source_sha256=bench.SOURCE_SHA256,
                             metadata=dict(revision=self.revision, dirty=False, cpu='AMD EPYC example',
                                           counters_available=False, counter_unavailable_reason='not permitted',
                                           toolchains={label: 'example 1.0' for label in ci.LABELS}),
                             compilation={ref: dict(cpu=dict(speed=2), calibration=dict(cpu=dict(speed=1.01)))
                                          for ref in ('gcc', 'clang')},
                             sizes={label: dict(executable_bytes=1234, text_bytes=1024) for label in ci.LABELS},
                             runtime=dict(status='complete', executables={label: {} for label in ci.LABELS}, workloads=names,
                                          correctness={label: dict(status=0, workloads=names) for label in ci.LABELS},
                                          timing_warnings={'gcc:all': [names[0]]},
                                          pairs={ref + ':all': dict(geometric_speed=2, loops={name: dict(speed=2) for name in names})
                                                 for ref in ('gcc', 'clang')}))
        self.write_document()
        ci.save(self.artifact / 'metadata.json', self.document['metadata'])
        (self.artifact / 'compile-observations.jsonl').write_text(''.join(json.dumps(dict(label=label, max_rss_kib=4096)) + '\n' for label in ci.LABELS))
        (self.artifact / 'runtime/observations.jsonl').write_text(''.join(json.dumps(dict(label=label, phase='AB', cases={name: value for name in names})) + '\n'
                                                                       for label, value in [('cppgm', 1), ('gcc', 2), ('clang', 2)]))

    def write_document(self):
        ci.save(self.artifact / 'results.json', self.document)

    def test_container_trust_is_scoped_to_the_mounted_workspace(self):
        workspace = self.root / 'mounted'
        sibling = self.root / 'untrusted'
        for path in (workspace, sibling):
            subprocess.run(['git', 'init', '-q', str(path)], check=True)
        command = ['git', '-C', str(workspace), 'rev-parse', '--show-toplevel']
        workflow = ci.ROOT / '.github/workflows/index-cpp.yml'
        trust = next(line.strip().removeprefix('run: ') for line in workflow.read_text().splitlines()
                     if 'run: git config --global --add safe.directory' in line)
        system = self.root / 'system.gitconfig'
        system.write_text('[safe]\n\tdirectory = *\n')
        configurations = [
            {},
            dict(GIT_CONFIG_SYSTEM=str(system)),
            dict(GIT_CONFIG_COUNT='1', GIT_CONFIG_KEY_0='safe.directory', GIT_CONFIG_VALUE_0='*'),
            dict(GIT_CONFIG_PARAMETERS="'safe.directory=*'"),
        ]
        for number, configuration in enumerate(configurations):
            with self.subTest(configuration=configuration), patch.dict(os.environ, configuration):
                # Test this workflow's exception independently of runner-wide trust.
                env = dict(os.environ, GIT_TEST_ASSUME_DIFFERENT_OWNER='1',
                           GIT_CONFIG_NOSYSTEM='1', GIT_CONFIG_COUNT='0',
                           GIT_CONFIG_GLOBAL=str(self.root / f'gitconfig-{number}'),
                           GITHUB_WORKSPACE=str(workspace))
                env.pop('GIT_CONFIG_PARAMETERS', None)
                rejected = subprocess.run(command, env=env, capture_output=True)
                self.assertNotEqual(rejected.returncode, 0)
                self.assertIn(b'dubious ownership', rejected.stderr)
                subprocess.run(['bash', '-euc', trust], env=env, check=True)
                self.assertEqual(subprocess.check_output(command, env=env, text=True).strip(), str(workspace))
                rejected = subprocess.run(['git', '-C', str(sibling), 'rev-parse', '--show-toplevel'], env=env, capture_output=True)
                self.assertNotEqual(rejected.returncode, 0)

    def test_compiler_symlink_keeps_driver_name_and_installed_configuration(self):
        binary = self.root / 'clang'
        binary.write_text('compiler binary')
        alias = self.root / 'clang++'
        alias.symlink_to(binary)
        self.assertEqual(ci.compiler_path(str(alias)), alias)
        self.assertEqual(ci.timing.sha(alias), ci.timing.sha(binary))

    def test_table_covers_every_workload_and_keeps_noise_visible(self):
        text = ci.report(self.document, self.artifact)
        self.assertIn('| Generated code | 2.000× | 2.000× |', text)
        self.assertEqual(text.count('| work-'), 50)
        self.assertIn('| work-00 | 1.000 | 2.000 | 2.000 | 2.000× | 2.000× | gcc |', text)
        self.assertIn('unavailable: not permitted', text)

    def test_available_counters_are_distinguished_from_timed_loop_speeds(self):
        self.document['metadata']['counters_available'] = True
        for ref in ('gcc', 'clang'):
            self.document['compilation'][ref]['instructions'] = dict(speed=3)
            self.document['runtime']['pairs'][ref + ':all']['process_instructions'] = dict(speed=4)
        text = ci.report(self.document, self.artifact)
        self.assertIn('| Compile instructions | 3.000× | 3.000× |', text)
        self.assertIn('| Process instructions, including setup | 4.000× | 4.000× |', text)
        self.assertIn('| Generated code | 2.000× | 2.000× |', text)

    def test_publication_persists_raw_results_and_retains_other_benchmarks(self):
        target = self.root / 'published'
        (target / 'epyc').mkdir(parents=True)
        (target / 'epyc/results.json').write_text('public suite')
        ci.publish(self.artifact, target, self.revision, 'https://example.test/run')
        self.assertEqual((target / 'epyc/results.json').read_text(), 'public suite')
        for path in ['index-cpp/epyc', 'index-cpp/history/' + self.revision + '/epyc']:
            published = target / path
            self.assertEqual((published / 'results.json').read_bytes(), (self.artifact / 'results.json').read_bytes())
            self.assertFalse((published / 'runtime-observations.jsonl').exists())
            self.assertFalse((published / 'compile-observations.jsonl').exists())
            self.assertIn('https://example.test/run', (published / 'report.md').read_text())

    def test_failed_incomplete_or_wrong_revision_does_not_replace_results(self):
        original = copy.deepcopy(self.document)
        mutations = [lambda d: d.update(status='failed'),
                     lambda d: d['runtime']['workloads'].pop(),
                     lambda d: d['runtime']['pairs']['clang:all']['loops'].pop('work-00'),
                     lambda d: d['runtime']['correctness']['gcc'].update(status=1),
                     lambda d: d['runtime']['correctness'].pop('clang'),
                     lambda d: d.update(source_sha256='0' * 64),
                     lambda d: d['metadata'].update(revision='b' * 40),
                     lambda d: d['metadata'].update(dirty=True),
                     lambda d: d['metadata'].update(cpu='unrecognized CPU')]
        target = self.root / 'published'
        target.mkdir()
        for mutate in mutations:
            self.document = copy.deepcopy(original)
            mutate(self.document)
            self.write_document()
            with self.subTest(change=mutate), self.assertRaises((ValueError, KeyError)):
                ci.publish(self.artifact, target, self.revision, 'https://example.test/run')
            self.assertEqual(list(target.iterdir()), [])

    def test_hosted_intel_preserves_epyc_and_dedicated_intel(self):
        self.document['metadata']['cpu'] = 'Intel Xeon Platinum 8370C'
        self.write_document()
        ci.save(self.artifact / 'metadata.json', self.document['metadata'])
        target = self.root / 'published'
        for platform in ('epyc', 'intel'):
            directory = target / 'index-cpp' / platform
            directory.mkdir(parents=True)
            (directory / 'report.md').write_text(platform + ' previous run')
        ci.publish(self.artifact, target, self.revision, 'https://example.test/run')
        for platform in ('epyc', 'intel'):
            self.assertEqual((target / 'index-cpp' / platform / 'report.md').read_text(), platform + ' previous run')
        for path in ('index-cpp/github-intel', 'index-cpp/history/' + self.revision + '/github-intel'):
            self.assertEqual((target / path / 'results.json').read_bytes(), (self.artifact / 'results.json').read_bytes())
        self.assertIn('Intel Xeon Platinum 8370C', (target / 'index-cpp/report.md').read_text())

    def test_dedicated_intel_publication_keeps_hosted_measurements(self):
        target = self.root / 'published'
        for cpu in ('AMD EPYC example', 'Intel Xeon Platinum 8370C'):
            self.document['metadata']['cpu'] = cpu
            self.write_document()
            ci.save(self.artifact / 'metadata.json', self.document['metadata'])
            ci.publish(self.artifact, target, self.revision, 'https://example.test/run')
        previous = {host: (target / 'index-cpp' / host / 'results.json').read_bytes()
                    for host in ('epyc', 'github-intel')}
        self.document['metadata']['cpu'] = 'Intel Xeon E5-2696 v4'
        self.write_document()
        ci.save(self.artifact / 'metadata.json', self.document['metadata'])
        ci.publish(self.artifact, target, self.revision, 'https://example.test/intel', 'intel')
        self.assertEqual((target / 'index-cpp/intel/results.json').read_bytes(), (self.artifact / 'results.json').read_bytes())
        for host, payload in previous.items():
            self.assertEqual((target / 'index-cpp' / host / 'results.json').read_bytes(), payload)
        self.assertIn('2.00×', (target / 'index-cpp/intel-runtime-gcc.svg').read_text())

    def test_mismatched_provenance_is_rejected_before_publication(self):
        ci.save(self.artifact / 'metadata.json', dict(self.document['metadata'], cpu='Intel Xeon'))
        target = self.root / 'published'
        with self.assertRaisesRegex(ValueError, 'provenance'):
            ci.publish(self.artifact, target, self.revision, 'https://example.test/run')
        self.assertFalse(target.exists())

    def test_missing_raw_observations_does_not_replace_existing_measurement(self):
        (self.artifact / 'runtime/observations.jsonl').unlink()
        target = self.root / 'published'
        with self.assertRaises(FileNotFoundError):
            ci.publish(self.artifact, target, self.revision, 'https://example.test/run')
        self.assertFalse(target.exists())

    def test_source_mismatch_leaves_a_failed_receipt(self):
        output = self.root / 'failed-run'
        args = SimpleNamespace(output=output, source=self.root)
        with patch.object(ci, 'verify_source', side_effect=ValueError('wrong pinned source')):
            with self.assertRaisesRegex(ValueError, 'wrong pinned source'):
                ci.run(args)
        document = json.loads((output / 'results.json').read_text())
        self.assertEqual(document['status'], 'failed')
        self.assertIn('wrong pinned source', (output / 'details.md').read_text())

    def test_runtime_checksum_failure_is_not_published_as_a_timing(self):
        source = self.root / 'source'
        source.mkdir()
        (source / 'index.cpp').write_text('pinned source fixture')
        program = self.root / 'program'
        program.write_text('immutable executable fixture')
        stdout = self.root / 'bad.stdout'
        stdout.write_text('work: ERR[actual=1, expected=2] in 0.1s\nSummary: 0.1s, 1, 0, 1\n')
        out = self.root / 'runtime-failed'
        args = SimpleNamespace(source_root=source, out=out, binary=['cppgm=' + str(program), 'gcc=' + str(program)],
                               candidate='cppgm', reference=['gcc'], cpus='0', blocks=1, aa_blocks=1,
                               metadata=None, timeout=1, no_counters=True, only=None, drift_warning=.02)
        original_sha = bench.timing.sha
        with patch.object(bench.subprocess, 'check_output', return_value=bench.UPSTREAM_COMMIT + '\n'), \
             patch.object(bench.timing, 'sha', side_effect=lambda path: bench.SOURCE_SHA256 if Path(path) == source / 'index.cpp' else original_sha(path)), \
             patch.object(bench, 'run', return_value=dict(status=0, cpu=.1, stdout=str(stdout))):
            with self.assertRaisesRegex(ValueError, 'failed or duplicate'):
                bench.measure(args)
        receipt = json.loads((out / 'report.json').read_text())
        self.assertEqual(receipt['status'], 'failed')
        self.assertFalse(receipt['pairs'])
        self.assertIn('ERR[actual=1', stdout.read_text())
        self.assertTrue((out / 'observations.jsonl').is_file())


if __name__ == '__main__':
    unittest.main()
