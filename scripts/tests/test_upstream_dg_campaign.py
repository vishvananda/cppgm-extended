#!/usr/bin/env python3
"""Protect corpus accounting and resume from silent skips or repeated workers."""
import csv
import fcntl
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import time
import unittest
from unittest import mock

REPO = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO / 'scripts'))
import run_upstream_dg_campaign as campaign
import summarize_upstream_dg_campaign as summary
from upstream_dg_proxy import sha256, write_json


class CampaignTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.sources = self.root / 'sources'
        self.gcc = self.sources / 'gcc/gcc/testsuite/g++.dg'
        self.gcc.mkdir(parents=True)
        (self.gcc / 'dg.exp').write_text('# native owner\n')
        (self.gcc / 'case.C').write_text('int value;\n')
        self.row = dict(repository='gcc', path='gcc/testsuite/g++.dg/case.C', role='candidate',
                        family='g++.dg', revision='pinned', sha256=sha256(self.gcc / 'case.C'))

    def inventory(self, rows):
        path = self.root / 'inventory.jsonl'
        path.write_text(''.join(json.dumps(row) + '\n' for row in rows))
        return path

    def test_per_file_accounting_includes_unselected_and_unavailable_drivers(self):
        (self.gcc / 'torture').mkdir()
        (self.gcc / 'torture/dg-torture.exp').write_text('# different native driver\n')
        other = dict(self.row, path='gcc/testsuite/g++.dg/torture/other.C')
        clang = dict(self.row, repository='llvm-project', path='clang/test/SemaCXX/other.cpp')
        unselected = dict(self.row, path='gcc/testsuite/g++.dg/unselected.C')
        rows = [self.row, other, clang, unselected]
        chosen = {row['repository'] + '/' + row['path'] for row in rows[:3]}
        accounting, tasks = campaign.make_plan(self.inventory(rows), self.sources, chosen,
                                                ['c++11', 'gnu++11'], ['host', 'candidate'])
        self.assertEqual(len(accounting), 4)
        self.assertEqual([row['status'] for row in accounting],
                         ['scheduled-native-variants', 'harness-blocked', 'harness-blocked',
                          'not-scheduled-in-this-cohort'])
        self.assertEqual(len(tasks), 4)
        self.assertTrue(all(task['identity'].endswith('/case.C') for task in tasks))
        self.assertTrue(all(row['applicability'] == 'unresolved' for row in accounting))

    def test_driver_family_and_discovery_suffix_are_preserved(self):
        cases = [('gcc/testsuite/g++.dg/file.cc', False),
                 ('gcc/testsuite/c-c++-common/ordinary.c', True),
                 ('gcc/testsuite/c-c++-common/cpp/preprocessor.c', True),
                 ('gcc/testsuite/c-c++-common/torture/opt.c', False),
                 ('gcc/testsuite/g++.old-deja/old.C', False)]
        for path, supported in cases:
            row = dict(self.row, path=path)
            _, blocker = campaign.driver_for(row, self.sources)
            self.assertEqual(blocker is None, supported, path)

    def test_source_drift_cannot_become_a_scheduled_pinned_case(self):
        (self.gcc / 'case.C').write_text('int changed;\n')
        with self.assertRaisesRegex(ValueError, 'differs from pinned'):
            campaign.make_plan(self.inventory([self.row]), self.sources, None, ['c++11'], ['candidate'])

    def test_missing_selected_identity_is_explicit(self):
        accounting, tasks = campaign.make_plan(self.inventory([self.row]), self.sources,
                                               {'gcc/missing.C'}, ['c++11'], ['candidate'])
        self.assertEqual(len(accounting), 2)
        self.assertEqual(accounting[-1]['status'], 'selection-unmatched')
        self.assertEqual(tasks, [])

    def test_duplicate_inventory_identity_is_rejected(self):
        with self.assertRaisesRegex(ValueError, 'duplicate inventory'):
            campaign.make_plan(self.inventory([self.row, self.row]), self.sources,
                               None, ['c++11'], ['candidate'])

    def test_selection_deduplicates_sources_and_preserves_observation_boundary(self):
        path = self.root / 'observations.csv'
        with path.open('w', newline='') as stream:
            writer = csv.DictWriter(stream, ['observation_id', 'source']); writer.writeheader()
            for ident, source in [('OBS-0001', 'gcc/one.C'), ('OBS-0002', 'gcc/one.C'),
                                  ('OBS-0003', 'gcc/two.C')]:
                writer.writerow(dict(observation_id=ident, source=source))
        self.assertEqual(campaign.selection(path, 2), {'gcc/one.C'})

    def test_reused_pid_does_not_count_as_original_live_worker(self):
        with mock.patch.object(campaign, 'process_stamp', return_value='new-process'):
            self.assertFalse(campaign.worker_alive(dict(pid=123, process_start_ticks='original-process')))

    def task_and_config(self):
        task = dict(id='test-cell', identity='gcc/' + self.row['path'], revision='pinned',
                    sha256=self.row['sha256'], dialect='c++11', mode='candidate',
                    source=str(self.gcc / 'case.C'))
        config = dict(defaults='adapted', hashes=dict(candidate='candidate', host='host', runtest='runtest'),
                      scripts={'run_upstream_dg.py': 'runner', 'upstream_dg_proxy.py': 'proxy'},
                      native_helpers={}, dejagnu_helpers={})
        output = self.root / 'output'
        (output / 'cells' / task['id'] / 'native').mkdir(parents=True)
        directory = output / 'cells' / task['id']
        write_json(directory / 'native/manifest.json', dict(source=dict(path=task['source'], sha256=task['sha256']),
                   binaries={name: dict(sha256=digest) for name, digest in config['hashes'].items()},
                   adapters={'runner': 'runner', 'proxy': 'proxy'}, native_helpers={}, dejagnu_helpers={}))
        write_json(directory / 'native/config.json', dict(dialect='c++11', mode='candidate', defaults='adapted'))
        return task, config, output, directory

    def native_result(self, directory):
        write_json(directory / 'native/result.json', dict(status='native-assertions-passed',
                   reasons=[], native_counts=dict(PASS=1)))

    def test_terminal_result_is_verified_without_starting_worker_again(self):
        task, config, output, directory = self.task_and_config()
        self.native_result(directory)
        state = dict(task=task, state='terminal', **campaign.result_for(task, directory, config, 0))
        write_json(directory / 'state.json', state)
        before = (directory / 'state.json').read_bytes()
        with mock.patch.object(campaign.subprocess, 'Popen', side_effect=AssertionError('must not rerun')):
            self.assertEqual(campaign.run_task(task, output, config), state)
        self.assertEqual((directory / 'state.json').read_bytes(), before)
        (directory / 'native/result.json').write_text('{}\n')
        with self.assertRaisesRegex(ValueError, 'recorded result changed'):
            campaign.run_task(task, output, config)

    def test_dead_incomplete_worker_is_not_restarted_or_counted_as_compiler_failure(self):
        task, config, output, directory = self.task_and_config()
        state = dict(task=task, state='running', pid=999999999, process_start_ticks='missing')
        write_json(directory / 'state.json', state)
        with mock.patch.object(campaign.subprocess, 'Popen', side_effect=AssertionError('must not rerun')):
            result = campaign.run_task(task, output, config)
        self.assertEqual(result['status'], 'harness-incomplete')
        self.assertEqual(result['state'], 'terminal')
        self.assertTrue(result['recovered'])

    @unittest.skipUnless(Path('/proc/self/stat').exists(), 'Linux process identity required')
    def test_resume_attaches_to_exact_live_worker_and_waits_for_its_result(self):
        task, config, output, directory = self.task_and_config()
        result = dict(status='native-assertions-passed', reasons=[], native_counts=dict(PASS=1))
        code = ('import time; from pathlib import Path; time.sleep(.4); Path(' +
                repr(str(directory / 'native/result.json')) + ').write_text(' + repr(json.dumps(result)) + ')')
        worker = subprocess.Popen([sys.executable, '-c', code])
        self.addCleanup(lambda: worker.poll() is None and worker.kill())
        stamp = campaign.process_stamp(worker.pid)
        self.assertIsNotNone(stamp)
        write_json(directory / 'state.json', dict(task=task, state='running', pid=worker.pid,
                                                 process_start_ticks=stamp))
        with mock.patch.object(campaign.subprocess, 'Popen', side_effect=AssertionError('must not rerun')):
            state = campaign.run_task(task, output, config)
        worker.wait(timeout=2)
        self.assertEqual(state['status'], 'native-assertions-passed')
        self.assertTrue(state['recovered'])

    def test_native_result_with_wrong_compiler_is_invalid_evidence(self):
        task, config, output, directory = self.task_and_config()
        self.native_result(directory)
        manifest_path = directory / 'native/manifest.json'
        data = json.loads(manifest_path.read_text()); data['binaries']['candidate']['sha256'] = 'wrong'
        write_json(manifest_path, data)
        self.assertEqual(campaign.result_for(task, directory, config)['status'], 'harness-invalid-evidence')

    def test_scheduler_lock_prevents_overlapping_resume(self):
        output = self.root / 'locked'; output.mkdir()
        (output / 'manifest.json').write_text('{}\n')
        with (output / 'scheduler.lock').open('w') as lock:
            fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
            command = [sys.executable, '-B', str(REPO / 'scripts/run_upstream_dg_campaign.py'),
                       '--inventory', str(self.root / 'inventory'), '--sources', str(self.sources),
                       '--dejagnu-library', str(self.root), '--runtest', sys.executable,
                       '--candidate', sys.executable, '--output', str(output), '--resume']
            result = subprocess.run(command, capture_output=True, text=True, timeout=5)
            self.assertEqual(result.returncode, 2, result.stderr)
            self.assertIn('another scheduler', result.stderr)

    def test_host_failure_prevents_attributing_a_differential_candidate_bug(self):
        self.assertEqual(summary.pair_status('native-failed', 'compiler-abnormal'),
                         'host-baseline-not-passing')
        self.assertEqual(summary.pair_status('native-assertions-passed', 'compiler-abnormal'),
                         'candidate-abnormal')
        self.assertEqual(summary.pair_status(None, 'native-assertions-passed'), 'incomplete-pair')

    def test_missing_warnings_are_reported_separately_from_error_patterns(self):
        data = dict(assertions=[dict(status='FAIL', text='test for warnings, line 1'),
                                dict(status='FAIL', text='test for errors, line 2'),
                                dict(status='PASS', text='test for warnings, line 3')])
        self.assertEqual(summary.failed_assertions(data), {'warning': 1, 'required-error-pattern': 1})


NATIVE = all(os.environ.get(key) for key in ['CPPGM_DG_RUNTEST', 'CPPGM_DG_TESTSUITE', 'CPPGM_DG_LIBRARY'])


@unittest.skipUnless(NATIVE and shutil.which('g++'), 'external native helpers required')
class NativeCampaignTests(unittest.TestCase):
    def test_prepare_execute_resume_and_reject_source_drift(self):
        retained = os.environ.get('CPPGM_DG_TEST_OUTPUT')
        if retained:
            Path(retained).mkdir(parents=True, exist_ok=True)
            root = Path(tempfile.mkdtemp(prefix='native-campaign-', dir=retained))
        else:
            temporary = tempfile.TemporaryDirectory(); self.addCleanup(temporary.cleanup)
            root = Path(temporary.name)
        sources = root / 'sources'
        testsuite = sources / 'gcc/gcc/testsuite'
        cases = testsuite / 'g++.dg'
        cases.mkdir(parents=True)
        helpers = Path(os.environ['CPPGM_DG_TESTSUITE'])
        (testsuite / 'lib').symlink_to(helpers / 'lib', target_is_directory=True)
        (cases / 'dg.exp').symlink_to(helpers / 'g++.dg/dg.exp')
        source = cases / 'positive.C'; source.write_text('int main() { return 0; }\n')
        inventory = root / 'inventory.jsonl'
        inventory.write_text(json.dumps(dict(repository='gcc', path='gcc/testsuite/g++.dg/positive.C',
                              role='candidate', family='g++.dg', revision='synthetic', sha256=sha256(source))) + '\n')
        candidate = root / 'candidate'
        candidate.write_text('#!/bin/sh\nexec ' + shutil.which('g++') + ' -fdiagnostics-plain-output "$@"\n')
        candidate.chmod(0o755)
        output = root / 'campaign'
        command = [sys.executable, '-B', str(REPO / 'scripts/run_upstream_dg_campaign.py'),
                   '--sources', str(sources), '--inventory', str(inventory), '--defaults', 'adapted',
                   '--dejagnu-library', os.environ['CPPGM_DG_LIBRARY'], '--runtest', os.environ['CPPGM_DG_RUNTEST'],
                   '--candidate', str(candidate), '--output', str(output), '--jobs', '2']
        for flags in [['--prepare-only'], ['--resume']]:
            result = subprocess.run(command + flags, capture_output=True, text=True, timeout=40)
            self.assertEqual(result.returncode, 0, result.stderr)
        progress = json.loads((output / 'progress.json').read_text())
        self.assertEqual(progress['completed'], 4)
        self.assertEqual(progress['counts'], {'native-assertions-passed': 4}, progress)
        states = {str(p): p.read_bytes() for p in (output / 'cells').glob('*/state.json')}
        result = subprocess.run(command + ['--resume'], capture_output=True, text=True, timeout=15)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(states, {str(p): p.read_bytes() for p in (output / 'cells').glob('*/state.json')})
        source.write_text('int changed;\n')
        result = subprocess.run(command + ['--resume'], capture_output=True, text=True, timeout=15)
        self.assertEqual(result.returncode, 2)
        self.assertIn('pinned source changed', result.stderr)


if __name__ == '__main__':
    unittest.main()
