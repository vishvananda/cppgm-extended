#!/usr/bin/env python3
"""The public benchmarks must fail closed on bad pins and report what they measured."""
import contextlib
import io
import json
from pathlib import Path
import subprocess
import sys
import tempfile
from types import SimpleNamespace
import unittest
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import public_benchmarks as pb


def outcome(name, gcc, cppgm, status='ok', **extra):
    return dict(name=name, status=status, gcc=gcc, cppgm=cppgm, **extra)


class ManifestTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)

    def manifest(self):
        return dict(
            sources=dict(one=dict(commit='a' * 40, license='MIT')),
            programs=[dict(name='p', source='one', srcs=['p.cpp'], count=['run'])])

    def test_shipped_manifest_is_valid_and_counts_both_tiers(self):
        manifest = pb.load_manifest()
        tiers = [tier for p in manifest['programs'] for tier in p.get('count', [])]
        self.assertEqual(set(tiers), pb.TIERS)

    def test_accepts_a_pinned_program(self):
        pb.validate(self.manifest(), self.root)

    def test_rejects_an_abbreviated_commit(self):
        manifest = self.manifest()
        manifest['sources']['one']['commit'] = 'abc123'
        with self.assertRaisesRegex(ValueError, 'full hash'):
            pb.validate(manifest, self.root)

    def test_rejects_a_source_without_a_license(self):
        manifest = self.manifest()
        del manifest['sources']['one']['license']
        with self.assertRaisesRegex(ValueError, 'license'):
            pb.validate(manifest, self.root)

    def test_rejects_a_repeated_program(self):
        manifest = self.manifest()
        manifest['programs'].append(dict(manifest['programs'][0]))
        with self.assertRaisesRegex(ValueError, 'listed twice'):
            pb.validate(manifest, self.root)

    def test_rejects_a_missing_driver_and_patch(self):
        manifest = self.manifest()
        manifest['programs'][0]['srcs'] = ['@drivers/absent.cpp']
        with self.assertRaisesRegex(ValueError, 'absent.cpp'):
            pb.validate(manifest, self.root)
        manifest = self.manifest()
        manifest['sources']['one']['patch'] = 'patches/absent.patch'
        with self.assertRaisesRegex(ValueError, 'absent.patch'):
            pb.validate(manifest, self.root)

    def test_rejects_an_unknown_tier(self):
        manifest = self.manifest()
        manifest['programs'][0]['count'] = ['link']
        with self.assertRaisesRegex(ValueError, 'tier'):
            pb.validate(manifest, self.root)


class FetchTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.upstream = self.root / 'upstream'
        self.upstream.mkdir()
        self.git('init', '-q')
        self.git('config', 'user.email', 'test@example.com')
        self.git('config', 'user.name', 'Test')
        self.git('config', 'uploadpack.allowFilter', 'true')
        self.git('config', 'uploadpack.allowReachableSHA1InWant', 'true')
        (self.upstream / 'kept').mkdir()
        (self.upstream / 'skipped').mkdir()
        (self.upstream / 'kept/a.txt').write_bytes(b'one\r\ntwo\r\n')
        (self.upstream / 'skipped/b.txt').write_text('skipped\n')
        self.git('add', '.')
        self.git('commit', '-qm', 'first')
        self.commit = subprocess.check_output(
            ['git', '-C', str(self.upstream), 'rev-parse', 'HEAD'], text=True).strip()
        self.cache = self.root / 'cache'
        self.cache.mkdir()
        self.source = dict(url=f'file://{self.upstream}', commit=self.commit,
                           license='MIT', paths=['kept'])

    def git(self, *arguments):
        subprocess.run(['git', '-C', str(self.upstream), *arguments], check=True)

    def test_checks_out_only_the_listed_paths_and_reuses_them(self):
        self.assertTrue(pb.fetch_source('one', self.source, self.root, self.cache))
        self.assertTrue((self.cache / 'one/kept/a.txt').is_file())
        self.assertFalse((self.cache / 'one/skipped').exists())
        self.assertFalse(pb.fetch_source('one', self.source, self.root, self.cache))

    def test_applies_the_recorded_changes(self):
        (self.root / 'a.patch').write_text(
            '--- a/kept/a.txt\n+++ b/kept/a.txt\n@@ -1,2 +1,2 @@\n one\n-two\n+three\n')
        self.source.update(strip_cr=['kept/a.txt'], patch='a.patch')
        pb.fetch_source('one', self.source, self.root, self.cache)
        self.assertEqual((self.cache / 'one/kept/a.txt').read_bytes(), b'one\nthree\n')

    def test_a_changed_pin_fetches_again(self):
        pb.fetch_source('one', self.source, self.root, self.cache)
        (self.cache / 'one/kept/a.txt').write_text('stale\n')
        self.source['strip_cr'] = ['kept/a.txt']
        self.assertTrue(pb.fetch_source('one', self.source, self.root, self.cache))
        self.assertEqual((self.cache / 'one/kept/a.txt').read_bytes(), b'one\ntwo\n')

    def test_an_unknown_commit_fails_and_leaves_no_pin(self):
        self.source['commit'] = 'b' * 40
        with self.assertRaises(subprocess.CalledProcessError):
            pb.fetch_source('one', self.source, self.root, self.cache, retries=1)
        self.assertFalse((self.cache / 'one.pin').exists())


class MeasureTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)

    def test_reports_the_exit_code_and_feeds_stdin(self):
        (self.root / 'in').write_text('hello\n')
        with (self.root / 'out').open('wb') as stream:
            result = pb.measure(['sh', '-c', 'cat; exit 3'], self.root, 30,
                                self.root / 'log', self.root / 'in', stream)
        self.assertEqual(result['code'], 3)
        self.assertFalse(result['timed_out'])
        self.assertEqual((self.root / 'out').read_text(), 'hello\n')

    def test_a_timeout_stops_the_whole_group(self):
        result = pb.measure(['sh', '-c', 'sleep 60 & wait'], self.root, 0.2,
                            self.root / 'log')
        self.assertTrue(result['timed_out'])

    def test_reads_a_cachegrind_log(self):
        self.assertEqual(pb.instruction_count('==1== I   refs:      1,234,567\n'), 1234567)
        self.assertIsNone(pb.instruction_count('==1== error\n'))


class CountingTests(unittest.TestCase):
    def side(self, label):
        context = SimpleNamespace(manifest=dict(levels=['-O0', '-O3']), count=True)
        program = dict(name='p', count=['compile', 'run'])
        side = pb.Side.__new__(pb.Side)
        side.context, side.program, side.label = context, program, label
        return side

    def counted(self, label):
        return {tier for tier in pb.TIERS if self.side(label).counting(tier, '-O3')}

    def test_only_cppgm_compiles_are_counted(self):
        self.assertEqual(self.counted('cppgm'), {'compile', 'run'})
        self.assertEqual(self.counted('gcc'), {'run'})
        self.assertEqual(self.counted('clang'), set())

    def test_only_the_last_level_is_counted(self):
        self.assertFalse(self.side('cppgm').counting('run', '-O0'))


class ReportTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)

    def changes(self, gcc_after=100, cppgm_after=100):
        results = [outcome('p', dict(run_instructions=gcc_after),
                           dict(run_instructions=cppgm_after))]
        baseline = dict(programs=dict(p=dict(gcc=dict(run_instructions=100),
                                             cppgm=dict(run_instructions=100))))
        return pb.compare_baseline(results, baseline)

    def test_ignores_a_change_within_tolerance(self):
        self.assertEqual(self.changes(cppgm_after=100.5), [])

    def test_reports_a_regression(self):
        changes = self.changes(cppgm_after=110)
        self.assertEqual(len(changes), 1)
        self.assertAlmostEqual(changes[0]['change'], 0.10)
        self.assertFalse(changes[0]['drift'])

    def test_marks_a_change_g_plus_plus_shares(self):
        self.assertTrue(self.changes(gcc_after=110, cppgm_after=110)[0]['drift'])

    def test_a_program_new_to_the_baseline_is_no_change(self):
        results = [outcome('new', {}, dict(run_instructions=5))]
        self.assertEqual(pb.compare_baseline(results, dict(programs={})), [])

    def test_ratio_skips_times_below_the_clock(self):
        result = outcome('p', dict(run_cpu_O3=0.001), dict(run_cpu_O3=1.0))
        self.assertIsNone(pb.ratio(result, 'run_cpu_O3'))

    def test_ratio_uses_the_named_reference(self):
        result = outcome('p', dict(size=4), dict(size=2), clang=dict(size=1))
        self.assertEqual(pb.ratio(result, 'size'), 0.5)
        self.assertEqual(pb.ratio(result, 'size', 'clang'), 2)

    def test_baseline_records_only_programs_that_passed(self):
        results = [outcome('good', dict(run_instructions=1), dict(run_instructions=2),
                           clang=dict(run_instructions=3)),
                   outcome('bad', dict(run_instructions=1), {}, status='compile-failed')]
        baseline = pb.baseline_of(results)
        self.assertEqual(list(baseline['programs']), ['good'])
        self.assertEqual(baseline['programs']['good']['clang']['run_instructions'], 3)

    def test_badge_gives_one_speed_against_gcc(self):
        results = [outcome('p', dict(compile_cpu_O3=5.0, run_cpu_O3=1.0),
                           dict(compile_cpu_O3=1.0, run_cpu_O3=1.25))]
        badge = pb.badge(results, '-O3 generated speed vs gcc', 'run_cpu_O3', 2)
        self.assertEqual(badge['schemaVersion'], 1)
        self.assertEqual(badge['label'], '-O3 generated speed vs gcc')
        self.assertEqual(badge['message'], '0.80×')
        self.assertEqual(badge['color'], 'green')
        badge = pb.badge(results, '-O3 compile speed vs gcc', 'compile_cpu_O3', 1)
        self.assertEqual(badge['message'], '5.0×')
        self.assertEqual(badge['color'], 'brightgreen')

    def test_badge_color_follows_the_speed(self):
        self.assertEqual(pb.badge_color(1.0), 'brightgreen')
        self.assertEqual(pb.badge_color(0.7), 'yellow')
        self.assertEqual(pb.badge_color(0.5), 'orange')

    def test_badge_turns_red_on_a_failure(self):
        results = [outcome('p', dict(compile_cpu_O0=1.0, run_cpu_O0=1.0),
                           dict(compile_cpu_O0=1.0, run_cpu_O0=1.0)),
                   outcome('bad', {}, {}, status='output-mismatch')]
        badge = pb.badge(results, 'label', 'run_cpu_O0', 2)
        self.assertEqual(badge['color'], 'red')
        self.assertEqual(badge['message'], '1 failing')

    def test_badge_without_numbers_says_so(self):
        badge = pb.badge([], 'label', 'run_cpu_O3', 2)
        self.assertEqual((badge['message'], badge['color']), ('no data', 'lightgrey'))

    def test_build_badge_gives_the_speed_of_a_self_hosted_build(self):
        builds = dict(status='ok', gcc=500.0, self=125.0, inception=200.0)
        badge = pb.build_badge(builds, 'self', 'gcc', 'self compile speed vs gcc', 1)
        self.assertEqual((badge['message'], badge['color']), ('4.0×', 'brightgreen'))

    def test_build_badge_compares_the_self_built_compiler_with_the_gcc_built_one(self):
        builds = dict(status='ok', gcc=500.0, self=100.0, inception=102.0)
        badge = pb.build_badge(builds, 'inception', 'self', 'self-built speed vs gcc', 2)
        self.assertEqual((badge['message'], badge['color']), ('0.98×', 'green'))

    def test_build_badge_without_a_run_says_so(self):
        self.assertEqual(pb.build_badge(None, 'self', 'gcc', 'label', 1)['message'], 'no data')

    def test_build_badge_turns_red_on_a_failed_build(self):
        badge = pb.build_badge(dict(status='self build failed', gcc=1.0), 'self', 'gcc', 'label', 1)
        self.assertEqual((badge['message'], badge['color']), ('1 failing', 'red'))

    def test_report_writes_every_badge(self):
        path = self.root / 'results.json'
        facts = dict(compile_cpu_O0=1.0, run_cpu_O0=1.0, compile_cpu_O3=1.0, run_cpu_O3=1.0)
        path.write_text(json.dumps(dict(programs=[outcome('p', facts, facts)])))
        builds = self.root / 'selfhost.json'
        builds.write_text(json.dumps(dict(status='ok', gcc=4.0, self=1.0, inception=2.0)))
        with contextlib.redirect_stdout(io.StringIO()):
            pb.main_report(path, badges=self.root / 'badges', selfhost_path=builds)
        names = sorted(item.stem for item in (self.root / 'badges').iterdir())
        self.assertEqual(names, ['o0-compile', 'o0-generated', 'o3-compile',
                                 'o3-generated', 'self-built', 'self-compile'])
        written = json.loads((self.root / 'badges/self-built.json').read_text())
        self.assertEqual((written['label'], written['message']),
                         ('self-built speed vs gcc', '0.50×'))

    def test_report_fails_when_a_self_hosted_build_fails(self):
        path = self.root / 'results.json'
        path.write_text(json.dumps(dict(programs=[])))
        builds = self.root / 'selfhost.json'
        builds.write_text(json.dumps(dict(status='self build failed', gcc=4.0, self=None)))
        with contextlib.redirect_stdout(io.StringIO()) as output:
            code = pb.main_report(path, selfhost_path=builds)
        self.assertEqual(code, 1)
        self.assertIn('The self-hosted builds failed: self build failed.', output.getvalue())

    def test_summary_gives_speeds_like_the_badges(self):
        facts = dict(compile_cpu_O3=4.0, run_cpu_O3=2.0)
        reference = dict(compile_cpu_O3=1.0, run_cpu_O3=1.5)
        document = dict(programs=[outcome('p', reference, facts)])
        text = pb.summary(document, [], dict(status='ok', gcc=4.0, self=1.0, inception=2.0))
        self.assertIn('| Compile CPU, `-O3` | 0.25× | 1 |', text)
        self.assertIn('| Run CPU, `-O3` | 0.75× | 1 |', text)
        self.assertIn('| Self compile CPU | 4.00× |', text)
        self.assertIn('| Self-built compiler CPU | 0.50× |', text)

    def test_selfhost_builds_use_one_job_count(self):
        commands = dict(pb.selfhost_commands('/work', 3))
        self.assertEqual(sorted(commands), ['gcc', 'inception', 'self'])
        for command in commands.values():
            self.assertIn('-j3', command)
        self.assertIn('INCEPTION_OBJECT_BUILD_JOBS=3', commands['inception'])
        self.assertEqual(commands['self'][-1], 'cppgm++-self')

    def test_selfhost_records_the_failed_build(self):
        output = self.root / 'selfhost.json'
        arguments = SimpleNamespace(work=self.root, output=output, jobs=1)
        with mock.patch.object(pb, 'build_cpu', side_effect=[3.0, None]):
            self.assertEqual(pb.selfhost(arguments), 0)
        written = json.loads(output.read_text())
        self.assertEqual(written['status'], 'self build failed')

    def report(self, results):
        path = self.root / 'results.json'
        path.write_text(json.dumps(dict(programs=results)))
        summary = self.root / 'summary.md'
        with contextlib.redirect_stdout(io.StringIO()):
            code = pb.main_report(path, summary)
        return code, summary.read_text()

    def test_a_failure_fails_the_report_and_names_the_program(self):
        code, text = self.report([outcome('bad', {}, {}, status='output-mismatch', error='x')])
        self.assertEqual(code, 1)
        self.assertIn('| bad | output-mismatch |', text)

    def test_clang_declining_a_program_fails_nothing(self):
        code, text = self.report([outcome('p', dict(size_O3=2), dict(size_O3=1), clang={},
                                          clang_error='compile: no')])
        self.assertEqual(code, 0)
        self.assertIn('| p | `compile: no` |', text)
        self.assertIn('| Executable size, `-O3` | 2.00× | 1 | - | 0 |', text)


if __name__ == '__main__':
    unittest.main()
