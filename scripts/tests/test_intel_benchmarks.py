#!/usr/bin/env python3
"""Dedicated runs must preserve compiler provenance and avoid launching during setup."""
import contextlib
import io
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import benchmark_hosts
import intel_library_benchmarks as library
import run_intel_benchmarks_once as launcher


class IntelBenchmarkTests(unittest.TestCase):
    def test_plan_pins_all_suites_without_registering_or_dispatching(self):
        with patch.object(sys, 'argv', ['runner', '--plan', '--ref', 'feature', '--revision', 'main']), \
                patch.object(launcher, 'github', return_value={'sha': 'a' * 40}) as api, \
                patch.object(launcher.subprocess, 'run') as run, contextlib.redirect_stdout(io.StringIO()) as output:
            launcher.main()
        api.assert_called_once_with('repos/vishvananda/cppgm-extended/commits/main')
        run.assert_not_called()
        plan = json.loads(output.getvalue())
        self.assertEqual(plan['revision'], 'a' * 40)
        self.assertEqual(plan['workflow_ref'], 'feature')
        self.assertEqual(list(plan['suites']), list(launcher.SUITES))

    def test_library_commands_measure_source_instead_of_workflow_checkout(self):
        source = Path('/measured/compiler')
        for suite in library.SCRIPTS:
            command = library.run_command(suite, source, 16)
            with self.subTest(suite=suite):
                self.assertEqual(command[command.index('--repo-root') + 1], str(source))
                self.assertEqual(command[command.index('--compiler') + 1], str(source / 'dev/cppgm++'))
                self.assertEqual(command[command.index('--output') + 1], str(source / 'obj' / suite))
                self.assertEqual('--jobs' in command, suite in ('godot', 'boost'))
                self.assertEqual(Path(command[1]).name, library.SCRIPTS[suite])

    def test_source_metadata_reads_the_selected_clean_checkout(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            subprocess.run(['git', 'init', '-q', str(root)], check=True)
            (root / 'source.cpp').write_text('original\n')
            subprocess.run(['git', '-C', str(root), 'add', '.'], check=True)
            subprocess.run(['git', '-C', str(root), '-c', 'user.name=Test',
                            '-c', 'user.email=test@example.org', 'commit', '-qm', 'source'], check=True)
            expected = subprocess.check_output(['git', '-C', str(root), 'rev-parse', 'HEAD'], text=True).strip()
            self.assertEqual(benchmark_hosts.source_metadata(root), dict(revision=expected, dirty=False))
            (root / 'source.cpp').write_text('changed\n')
            self.assertEqual(benchmark_hosts.source_metadata(root), dict(revision=expected, dirty=True))

    def test_dedicated_hardware_cannot_be_inferred_from_hosted_intel(self):
        self.assertEqual(benchmark_hosts.publication_platform('Intel Xeon'), 'github-intel')
        self.assertEqual(benchmark_hosts.publication_platform('Intel Xeon', 'intel'), 'intel')
        self.assertEqual(benchmark_hosts.publication_platform('AMD EPYC'), 'epyc')
        with self.assertRaisesRegex(ValueError, 'Intel hardware'):
            benchmark_hosts.publication_platform('AMD EPYC', 'intel')
        self.assertIn('Dedicated Intel', benchmark_hosts.environment('intel'))

    def test_collection_retains_nested_observations_without_build_trees(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            source, target = root / 'run', root / 'artifact'
            keep = ['results.json', 'compile.stdout', 'compile.stderr', 'counter-probe',
                    'runtime/observations.jsonl', 'runtime/000/stdout', 'runtime/000/stderr',
                    'cppgm/libs-json-test/invocations.jsonl', 'cppgm/libs-json-test/b2.log']
            discard = ['compiler', 'output.o', 'source/input.json', 'source/tests/result.stdout',
                       'cppgm/build/large.json', 'binary']
            for name in keep + discard:
                path = source / name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text(name)
            library.collect(source, target)
            self.assertEqual({str(p.relative_to(target)) for p in target.rglob('*') if p.is_file()}, set(keep))


if __name__ == '__main__':
    unittest.main()
