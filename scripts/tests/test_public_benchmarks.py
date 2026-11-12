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
import publish_benchmark_badges as publication
import run_intel_benchmarks_once as intel_runner


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


class TimingTests(unittest.TestCase):
    def setUp(self):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        root = Path(temp.name)
        self.context = SimpleNamespace(manifest=dict(levels=['-O0', '-O3']), work=root,
                                       cache=root, tools=dict(gcc='g++', cppgm='cppgm++'))
        self.program = dict(name='p', source='s')
        for label in self.context.tools:
            directory = pb.Side(self.context, self.program, label).work / '-O3'
            directory.mkdir(parents=True)
            (directory / 'a.out').write_bytes(label.encode())

    def time(self, result, times, rounds=2, output=b'ok', exit_code=0, timed_out=False):
        samples = iter(times)
        calls = []

        def fake(command, cwd, timeout, log, stdin, stdout):
            calls.append(Path(command[0]).parent.parent.name)
            stdout.write(output(len(calls)) if callable(output) else output)
            return dict(code=exit_code, timed_out=timed_out, cpu=next(samples))
        with mock.patch.object(pb, 'measure', fake):
            pb.time_runs(self.context, self.program, result, rounds)
        return calls

    def result(self, gcc_cpu):
        digest = pb.hashlib.sha256(b'ok').hexdigest()
        return dict(status='ok', gcc=dict(run_cpu_O3=gcc_cpu, exit_code_O3=0, output_O3=digest),
                    cppgm=dict(run_cpu_O3=9.0, exit_code_O3=0, output_O3=digest))

    def test_uses_paired_ratios_instead_of_independent_minima(self):
        result = self.result(0.5)
        calls = self.time(result, [1] * 4 + [1, 2, 2, 1, 3, 2, 2, 3])
        self.assertEqual(calls, ['gcc'] * 4 + ['gcc', 'cppgm', 'cppgm', 'gcc']
                         + ['cppgm', 'gcc', 'gcc', 'cppgm'])
        self.assertEqual(result['gcc']['run_cpu_O3'], 1.5)
        self.assertEqual(result['cppgm']['run_cpu_O3'], 2.5)
        self.assertEqual(pb.ratio(result, 'run_cpu_O3'), 1.75)
        self.assertEqual(len(result['timing']['samples']), 12)
        receipts = list(self.context.work.glob('timing/p/paired-*/observations.jsonl'))
        self.assertEqual(len(receipts[0].read_text().splitlines()), 12)

    def test_a_long_program_gets_calibration_and_one_block(self):
        result = self.result(2.0)
        self.assertEqual(len(self.time(result, [2] * 4 + [1.9, 2.1, 2.1, 1.9])), 8)
        self.assertEqual(result['cppgm']['run_cpu_O3'], 2.1)

    def test_a_failed_program_is_not_timed(self):
        result = dict(self.result(0.5), status='output-mismatch')
        self.assertEqual(self.time(result, []), [])

    def test_bad_output_or_exit_or_timeout_fails_and_retains_the_observation(self):
        for options in [dict(output=b'wrong'), dict(exit_code=1), dict(timed_out=True)]:
            result = self.result(0.5)
            self.time(result, [0.2], **options)
            self.assertEqual(result['status'], 'timing-failed')
            self.assertEqual(len(result['timing']['samples']), 1)
            self.assertIn('gcc: timed run', result['error'])

    def test_a_repeatable_nonzero_exit_is_allowed(self):
        result = self.result(0.5)
        result['gcc']['exit_code_O3'] = result['cppgm']['exit_code_O3'] = 7
        self.time(result, [0.5] * 12, exit_code=7)
        self.assertEqual(result['status'], 'ok')

    def test_clang_is_paired_directly_with_cppgm(self):
        self.context.tools['clang'] = 'clang++'
        directory = pb.Side(self.context, self.program, 'clang').work / '-O3'
        directory.mkdir(parents=True)
        (directory / 'a.out').write_bytes(b'clang')
        result = self.result(0.5)
        result['clang'] = dict(result['gcc'])
        calls = self.time(result, [1] * 12 + [4, 2, 2, 4, 2, 4, 4, 2])
        self.assertEqual(calls[-8:], ['clang', 'cppgm', 'cppgm', 'clang',
                                     'cppgm', 'clang', 'clang', 'cppgm'])
        self.assertEqual(pb.ratio(result, 'run_cpu_O3', 'clang'), 0.5)

    def test_clang_timing_failure_is_advisory_but_cppgm_failure_is_not(self):
        self.context.tools['clang'] = 'clang++'
        directory = pb.Side(self.context, self.program, 'clang').work / '-O3'
        directory.mkdir(parents=True)
        (directory / 'a.out').write_bytes(b'clang')
        for bad_sample, expected in [(13, 'ok'), (14, 'timing-failed')]:
            result = self.result(0.5)
            result['clang'] = dict(result['gcc'])
            self.time(result, [1] * 20,
                      output=lambda n: b'wrong' if n == bad_sample else b'ok')
            self.assertEqual(result['status'], expected)
            self.assertEqual(len(result['timing']['samples']), bad_sample)
            if expected == 'ok':
                self.assertEqual(result['clang'], {})
                self.assertIn('clang: timed run', result['clang_error'])
                self.assertEqual(pb.ratio(result, 'run_cpu_O3'), 1.0)


class RebuildTimingTests(unittest.TestCase):
    def setUp(self):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        self.root = Path(temp.name)
        source = self.root / 'compiler'
        source.write_bytes(b'original compiler')
        directory = self.root / 'measurement'
        directory.mkdir()
        self.object = b'object'
        self.code = 0

        def measure(command, cwd, timeout, log, stdout):
            self.command = command
            build = directory / 'pa35'
            objects = build / 'selfhost'
            objects.mkdir(parents=True)
            (objects / 'one.o').write_bytes(self.object)
            binary = build / 'bin/selfhost/cppgm++-self'
            binary.parent.mkdir(parents=True, exist_ok=True)
            binary.write_bytes(b'identical compiler')
            return dict(cpu=1.0, code=self.code, timed_out=False)

        self.directory = directory
        self.rebuild = pb.timing.Rebuild(self.root, directory, 3, source, source, measure)

    def test_reuses_frozen_compilers_and_requires_identical_outputs(self):
        (self.root / 'compiler').write_bytes(b'later change')
        first = self.rebuild.run('seed', 'AB', 0, 0)
        second = self.rebuild.run('self', 'AB', 0, 1)
        self.assertEqual(first['object_sha256'], second['object_sha256'])
        self.assertEqual(first['output_sha256'], second['output_sha256'])
        self.assertEqual((self.directory / 'seed').read_bytes(), b'original compiler')
        self.assertIn('INCEPTION_COMPILER_FLAVOR=selfhost', self.command)
        self.assertIn('INCEPTION_OBJECT_BUILD_JOBS=3', self.command)

    def test_changed_objects_fail_and_keep_the_mismatching_hashes(self):
        self.rebuild.run('seed', 'AB', 0, 0)
        self.object = b'changed object'
        with self.assertRaisesRegex(RuntimeError, 'changed compiler output'):
            self.rebuild.run('self', 'AB', 0, 1)
        rows = [json.loads(line) for line in (self.directory / 'outputs.jsonl').read_text().splitlines()]
        self.assertEqual(len(rows), 2)
        self.assertNotEqual(rows[0]['object_sha256'], rows[1]['object_sha256'])

    def test_failed_rebuild_keeps_its_observation(self):
        self.code = 2
        with self.assertRaisesRegex(RuntimeError, 'paired rebuild failed'):
            self.rebuild.run('seed', 'AA', 0, 0)
        rows = (self.directory / 'observations.jsonl').read_text().splitlines()
        self.assertEqual(json.loads(rows[0])['code'], 2)


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
        self.assertEqual(badge['color'], pb.badge_color(.80))
        badge = pb.badge(results, '-O3 compile speed vs gcc', 'compile_cpu_O3', 1)
        self.assertEqual(badge['message'], '5.0×')
        self.assertEqual(badge['color'], '14532d')

    def test_badge_color_follows_the_speed(self):
        for speed, color in [(1.2, '14532d'), (1.0, '14532d'),
                             (0.85, 'eab308'), (0.65, 'f97316'), (0.5, 'b91c1c'), (0.4, 'b91c1c')]:
            with self.subTest(speed=speed):
                self.assertEqual(pb.badge_color(speed), color)
        # Neighboring measurements vary smoothly, including around each anchor.
        colors = {pb.badge_color(n / 1000) for n in range(500, 1001)}
        self.assertGreater(len(colors), 200)
        for anchor in [.50, .65, .85, 1.0]:
            a, b = pb.badge_color(anchor - .0001), pb.badge_color(anchor + .0001)
            self.assertLessEqual(max(abs(int(a[n:n+2],16)-int(b[n:n+2],16)) for n in (0,2,4)), 2)

    def test_badges_keep_yellow_text_readable(self):
        yellow = publication.badge_svg(dict(label='speed', message='0.85×', color=pb.badge_color(.85)))
        green = publication.badge_svg(dict(label='speed', message='1.00×', color=pb.badge_color(1)))
        self.assertIn('fill="#000" text-anchor', yellow)
        self.assertIn('fill="#fff" text-anchor', green)

    def test_split_badge_keeps_processor_gray_and_value_colored(self):
        import xml.etree.ElementTree as ET
        svg = publication.badge_svg(dict(label='compile speed', left_label='E5 <model>',
                                        message='0.80×', color=pb.badge_color(.80)))
        root = ET.fromstring(svg)
        ns = '{http://www.w3.org/2000/svg}'
        text = root.findall(ns+'text')
        self.assertEqual([node.text for node in text], ['E5 <model>', '0.80×'])
        panels = root.find(ns+'g').findall(ns+'rect')
        self.assertEqual(panels[0].attrib['fill'], '#555')
        self.assertEqual(panels[1].attrib['fill'], '#'+pb.badge_color(.80))
        self.assertEqual(int(panels[1].attrib['x']), int(panels[0].attrib['width']))
        self.assertEqual(int(root.attrib['width']), sum(int(node.attrib['width']) for node in panels))
        for processor, value in [('9V45', '4.02×'), ('E5 v4', '0.98×'), ('8370C', 'n/m')]:
            with self.subTest(processor=processor, value=value):
                sized = ET.fromstring(publication.badge_svg(dict(label='speed', left_label=processor,
                                                                 message=value, color='475569')))
                self.assertEqual((sized.attrib['width'], sized.attrib['height']), ('120', '24'))
                parts = sized.find(ns+'g').findall(ns+'rect')
                self.assertEqual([node.attrib['width'] for node in parts], ['64', '56'])

    def test_badge_turns_red_on_a_failure(self):
        results = [outcome('p', dict(compile_cpu_O0=1.0, run_cpu_O0=1.0),
                           dict(compile_cpu_O0=1.0, run_cpu_O0=1.0)),
                   outcome('bad', {}, {}, status='output-mismatch')]
        badge = pb.badge(results, 'label', 'run_cpu_O0', 2)
        self.assertEqual(badge['color'], 'b91c1c')
        self.assertEqual(badge['message'], '1 failing')

    def test_badge_without_numbers_says_so(self):
        badge = pb.badge([], 'label', 'run_cpu_O3', 2)
        self.assertEqual((badge['message'], badge['color']), ('no data', '475569'))

    def test_build_badge_gives_the_speed_of_a_self_hosted_build(self):
        builds = dict(status='ok', gcc=500.0, self=125.0, inception=200.0)
        badge = pb.build_badge(builds, 'self', 'gcc', 'self compile speed vs gcc', 1)
        self.assertEqual((badge['message'], badge['color']), ('4.0×', '14532d'))

    def test_build_badge_compares_the_self_built_compiler_with_the_gcc_built_one(self):
        builds = dict(status='ok', gcc=500.0, self=100.0, inception=102.0)
        badge = pb.build_badge(builds, 'inception', 'self', 'self-built speed vs gcc', 2)
        self.assertEqual((badge['message'], badge['color']), ('0.98×', pb.badge_color(100/102)))

    def test_self_built_badge_uses_the_paired_result(self):
        builds = dict(status='ok', gcc=500.0, self=100.0, inception=102.0,
                      paired_self=dict(comparison=dict(ratio=1.0)))
        badge = pb.build_badge(builds, 'inception', 'self', 'label', 2)
        self.assertEqual(badge['message'], '1.00×')

    def test_clang_compilation_and_codegen_use_distinct_baselines(self):
        def paired(ratio):
            return dict(AA=dict(ratio=1.), comparison=dict(ratio=ratio, blocks=[
                dict(ratio=ratio, reference_cpu=100., candidate_cpu=100.*ratio)]))
        builds = dict(status='ok', gcc=500., clang=400., self=100., inception=102.,
                      paired_self=paired(1.02), paired_clang=paired(.80))
        self.assertEqual(pb.build_ratio(builds, 'self', 'clang'), .25)
        self.assertEqual(pb.build_ratio(builds, 'inception', 'clang-built'), .80)
        text = pb.summary(dict(programs=[]), [], builds)
        self.assertIn('| Self compile CPU vs Clang | - | - | 4.00× | - |', text)
        self.assertIn('| cppgm codegen vs Clang | - | - | 1.25× | - |', text)
        del builds['paired_clang']
        self.assertIsNone(pb.build_ratio(builds, 'inception', 'clang-built'))

    def test_build_badge_without_a_run_says_so(self):
        self.assertEqual(pb.build_badge(None, 'self', 'gcc', 'label', 1)['message'], 'no data')

    def test_build_badge_turns_red_on_a_failed_build(self):
        badge = pb.build_badge(dict(status='self build failed', gcc=1.0), 'self', 'gcc', 'label', 1)
        self.assertEqual((badge['message'], badge['color']), ('1 failing', 'b91c1c'))

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
                                 'o3-generated', 'self-built', 'self-built-clang',
                                 'self-compile', 'self-compile-clang'])
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

    def test_details_show_raw_compile_times_and_higher_is_better_ratios(self):
        own = dict(compile_cpu_O0=0.5, compile_cpu_O3=2.0, frontend_cpu=0.25)
        gcc = dict(compile_cpu_O0=2.0, compile_cpu_O3=6.0, frontend_cpu=1.0)
        clang = dict(compile_cpu_O0=1.5, compile_cpu_O3=4.0, frontend_cpu=0.5)
        document = dict(programs=[outcome('sample', gcc, own, clang=clang)])
        text = pb.summary(document, [], dict(status='ok', gcc=100, self=20, inception=22))
        self.assertIn('### O0 compilation', text)
        self.assertIn('| sample | 0.500 | 2.000 | 4.00× | 1.500 | 3.00× | ok |', text)
        self.assertIn('| sample | 2.000 | 6.000 | 3.00× | 4.000 | 2.00× | ok |', text)
        self.assertIn('| sample | 0.250 | 1.000 | 4.00× | 0.500 | 2.00× | ok |', text)
        self.assertIn('| GCC | 100.000 |', text)
        self.assertIn('| GCC-built cppgm++ | 20.000 |', text)

    def test_details_retain_tiny_missing_and_failing_measurements(self):
        records = [outcome('tiny', dict(compile_cpu_O3=1), dict(compile_cpu_O3=0.005)),
                   outcome('bad|name', dict(compile_cpu_O3=2), dict(compile_cpu_O3=1), status='output-mismatch')]
        text = '\n'.join(pb.timing_table(records, 'compile_cpu_O3'))
        self.assertIn('| tiny | 0.005 | 1.000 | — | — | — | ok |', text)
        self.assertIn(r'| bad\|name | 1.000 | 2.000 | — | — | — | output-mismatch |', text)

    def test_details_file_matches_the_workflow_summary(self):
        path = self.root / 'results.json'
        path.write_text(json.dumps(dict(programs=[outcome('sample', {}, {})])))
        summary, details = self.root / 'summary.md', self.root / 'details.md'
        with contextlib.redirect_stdout(io.StringIO()):
            pb.main_report(path, summary_path=summary, details_path=details)
        self.assertEqual(details.read_bytes(), summary.read_bytes())
        self.assertIn('### O3 compilation', details.read_text())

    def test_selfhost_builds_use_one_job_count(self):
        commands = dict(pb.selfhost_commands('/work', 3))
        self.assertEqual(sorted(commands), ['clang', 'gcc', 'inception', 'self'])
        for command in commands.values():
            self.assertIn('-j3', command)
        self.assertIn('INCEPTION_OBJECT_BUILD_JOBS=3', commands['inception'])
        self.assertEqual(commands['self'][-1], 'cppgm++-self')
        self.assertIn('CXX=clang++', commands['clang'])
        self.assertIn('CXX=g++', commands['gcc'])
        self.assertIn('CPPGM_HOST_CXX=g++', commands['clang'])
        self.assertIn('OBJ=/work/clang', commands['clang'])
        self.assertIn('OBJ=/work/gcc', commands['gcc'])

    def test_selfhost_records_the_failed_build(self):
        output = self.root / 'selfhost.json'
        arguments = SimpleNamespace(work=self.root, output=output, jobs=1)
        (self.root / 'dev').mkdir()
        (self.root / 'dev/cppgm++').write_bytes(b'seed')
        with mock.patch.object(pb, 'build_cpu', side_effect=[3.0, None]), \
                mock.patch.object(pb, 'ROOT', self.root):
            self.assertEqual(pb.selfhost(arguments), 0)
        written = json.loads(output.read_text())
        self.assertEqual(written['status'], 'self build failed')

    def test_selfhost_measures_the_selected_checkout(self):
        source = self.root / 'measured-source'
        (source / 'dev').mkdir(parents=True)
        (source / 'dev/cppgm++').write_bytes(b'compiler')
        (source / 'dev/example.cpp').write_text('int main() {}\n')
        output = self.root / 'selfhost.json'
        arguments = SimpleNamespace(work=self.root / 'work', output=output,
                                    repo_root=source, jobs=1)
        with mock.patch.object(pb, 'build_cpu', side_effect=[3., None]) as build, \
                mock.patch.object(pb, 'tool_version', return_value='a' * 40):
            pb.selfhost(arguments)
        self.assertTrue(all(call.args[2] == source for call in build.call_args_list))
        written = json.loads(output.read_text())
        self.assertEqual(written['metadata']['revision'], 'a' * 40)
        self.assertEqual(written['metadata']['source_sha256'],
                         {'dev/example.cpp': pb.timing.sha(source / 'dev/example.cpp')})

    def test_standalone_selfhost_report_has_paired_ratios_and_only_build_badges(self):
        paired = dict(AA=dict(ratio=1.001), comparison=dict(ratio=.8, blocks=[
            dict(reference_cpu=20., candidate_cpu=16., ratio=.8)]))
        builds = dict(status='ok', gcc=100., clang=80., self=20., inception=22.,
                      paired_self=paired, paired_clang=paired,
                      metadata=dict(cpu='Intel Xeon', revision='a' * 40))
        path = self.root / 'selfhost.json'
        path.write_text(json.dumps(builds))
        arguments = SimpleNamespace(results=path, summary=self.root / 'summary.md',
                                    details=self.root / 'details.md', badges=self.root / 'badges')
        with contextlib.redirect_stdout(io.StringIO()):
            code = pb.report_selfhost(arguments)
        self.assertEqual(code, 0)
        self.assertEqual(arguments.summary.read_bytes(), arguments.details.read_bytes())
        text = arguments.details.read_text()
        self.assertIn('| Building cppgm++ | 5.00× | 4.00× |', text)
        self.assertIn('| Self-built compiler speed | 1.25× | 1.25× |', text)
        self.assertIn('| 1 | 20.000 | 16.000 | 1.25× |', text)
        self.assertIn('A/A CPU ratio: 1.001', text)
        self.assertIn('CPU: Intel Xeon', text)
        self.assertNotIn('0 of 0 programs', text)
        self.assertEqual({p.stem for p in arguments.badges.iterdir()},
                         {row[2] for row in pb.BUILDS})

    def test_standalone_selfhost_report_fails_and_retains_failure_badges(self):
        path = self.root / 'selfhost.json'
        path.write_text(json.dumps(dict(status='gcc build failed')))
        arguments = SimpleNamespace(results=path, summary=None, details=None,
                                    badges=self.root / 'badges')
        with contextlib.redirect_stdout(io.StringIO()) as output:
            self.assertEqual(pb.report_selfhost(arguments), 1)
        self.assertIn('gcc build failed', output.getvalue())
        self.assertEqual(json.loads((arguments.badges / 'self-built.json').read_text())['message'],
                         '1 failing')

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


class BadgePublicationTests(unittest.TestCase):
    def setUp(self):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        self.root = Path(temp.name)
        self.artifact = self.root / 'artifact'
        self.target = self.root / 'published'
        (self.artifact / 'badges').mkdir(parents=True)
        self.target.mkdir()
        for name in publication.BADGES:
            (self.artifact / 'badges' / f'{name}.json').write_text(json.dumps(
                dict(schemaVersion=1, label=name, message='0.97×', color='166534')))

    def prepare(self, platform, cpu):
        (self.artifact / 'results.json').write_text(json.dumps(dict(environment=dict(cpu=cpu),
            programs=[outcome('p', dict(run_cpu_O3=1), dict(run_cpu_O3=2))])))
        publication.prepare(self.artifact, self.target, platform, 'a' * 40,
                            'https://github.com/owner/repo/actions/runs/1', legacy_epyc=True)

    def test_intel_publication_preserves_epyc_results(self):
        self.prepare('epyc', 'AMD EPYC 9V45 96-Core Processor')
        before = (self.target / 'epyc/o3-generated.json').read_bytes()
        legacy = (self.target / 'o3-generated.json').read_bytes()
        report = (self.target / 'epyc/report.md').read_bytes()
        raw = (self.target / 'epyc/results.json').read_bytes()
        self.prepare('intel', 'Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz')
        self.assertEqual((self.target / 'epyc/o3-generated.json').read_bytes(), before)
        self.assertEqual((self.target / 'o3-generated.json').read_bytes(), legacy)
        self.assertEqual((self.target / 'epyc/report.md').read_bytes(), report)
        self.assertEqual((self.target / 'epyc/results.json').read_bytes(), raw)
        provenance = json.loads((self.target / 'intel/provenance.json').read_text())
        self.assertEqual(provenance['revision'], 'a' * 40)
        self.assertIn('Intel', provenance['cpu'])

    def test_hosted_intel_preserves_both_established_hosts(self):
        self.prepare('epyc', 'AMD EPYC 7763 64-Core Processor')
        self.prepare('intel', 'Intel Xeon E5-2696 v4')
        paths = ['epyc/report.md', 'intel/report.md', 'o3-generated.svg']
        previous = {name: (self.target / name).read_bytes() for name in paths}
        self.prepare('github', 'INTEL(R) XEON(R) PLATINUM 8573C')
        for name, content in previous.items():
            self.assertEqual((self.target / name).read_bytes(), content)
        provenance = json.loads((self.target / 'github-intel/provenance.json').read_text())
        self.assertEqual(provenance['cpu'], 'INTEL(R) XEON(R) PLATINUM 8573C')
        self.assertIn('Intel (GitHub)', (self.target / 'github-intel/report.md').read_text())

    def test_hosted_epyc_updates_epyc_and_legacy_badges(self):
        self.prepare('github', 'AMD EPYC 7763 64-Core Processor')
        self.assertEqual((self.target / 'epyc/o3-generated.svg').read_bytes(),
                         (self.target / 'o3-generated.svg').read_bytes())

    def test_report_identifies_its_source_and_retains_exact_raw_measurements(self):
        builds = dict(status='ok', gcc=100, self=20, inception=21)
        (self.artifact / 'selfhost.json').write_text(json.dumps(builds))
        self.prepare('intel', 'Intel Xeon')
        text = (self.target / 'intel/report.md').read_text()
        self.assertIn('# Public benchmarks: Intel Xeon', text)
        self.assertIn('commit/' + 'a' * 40, text)
        self.assertIn('https://github.com/owner/repo/actions/runs/1', text)
        self.assertIn('[Raw program results](results.json)', text)
        self.assertIn('[Raw rebuild results](selfhost.json)', text)
        self.assertIn('### O3 compilation', text)
        self.assertEqual((self.target / 'intel/results.json').read_bytes(),
                         (self.artifact / 'results.json').read_bytes())
        self.assertEqual(json.loads((self.target / 'intel/selfhost.json').read_text()), builds)

    def test_epyc_publication_preserves_intel_results(self):
        self.prepare('intel', 'Intel Xeon')
        before = (self.target / 'intel/o3-generated.json').read_bytes()
        self.prepare('epyc', 'AMD EPYC 9V45')
        self.assertEqual((self.target / 'intel/o3-generated.json').read_bytes(), before)

    def test_cpu_column_can_be_seeded_without_replacing_main_measurements(self):
        self.prepare('epyc', 'AMD EPYC 9V45')
        old = (self.target / 'o3-generated.json').read_bytes()
        path = self.artifact / 'badges/o3-generated.json'
        data = json.loads(path.read_text())
        data['message'] = '1.01×'
        path.write_text(json.dumps(data))
        publication.prepare(self.artifact, self.target, 'epyc', 'b' * 40,
                            'https://github.com/owner/repo/actions/runs/2')
        self.assertEqual((self.target / 'o3-generated.json').read_bytes(), old)
        self.assertEqual(json.loads((self.target / 'epyc/o3-generated.json').read_text())['message'], '1.01×')

    def test_wrong_hardware_does_not_replace_a_measurement(self):
        with self.assertRaisesRegex(ValueError, 'Intel hardware'):
            self.prepare('intel', 'AMD EPYC 9V45')
        self.assertEqual(list(self.target.iterdir()), [])

    def test_program_count_uses_the_same_cutoff_as_the_speed_badge(self):
        self.prepare('intel', 'Intel Xeon')
        document = json.loads((self.artifact / 'results.json').read_text())
        document['programs'] += [outcome('tiny', dict(run_cpu_O3=0.1), dict(run_cpu_O3=0.006)),
                                 outcome('bad', dict(run_cpu_O3=1), dict(run_cpu_O3=2), status='output-mismatch')]
        (self.artifact / 'results.json').write_text(json.dumps(document))
        publication.prepare(self.artifact, self.target, 'intel', 'a' * 40,
                            'https://github.com/owner/repo/actions/runs/1')
        self.assertEqual(json.loads((self.target / 'intel/o3-programs.json').read_text())['message'], '1')

    def test_incomplete_artifact_does_not_replace_a_measurement(self):
        (self.artifact / 'badges/self-built.json').unlink()
        with self.assertRaises(FileNotFoundError):
            self.prepare('intel', 'Intel Xeon')
        self.assertEqual(list(self.target.iterdir()), [])

    def self_build(self, revision='b' * 40):
        builds = dict(status='ok', gcc=100, clang=80, self=20, inception=21,
                      metadata=dict(cpu='Intel Xeon', revision=revision))
        (self.artifact / 'selfhost.json').write_text(json.dumps(builds))
        publication.prepare(self.artifact, self.target, 'intel', revision,
                            'https://github.com/owner/repo/actions/runs/2', component='self-build')

    def test_public_update_preserves_every_self_build_file(self):
        self.self_build()
        self.assertIn('>Intel</text>', (self.target / 'intel/self-built.svg').read_text())
        previous = {p.name: p.read_bytes() for p in (self.target / 'intel').iterdir()
                    if p.name.startswith('self')}
        (self.artifact / 'selfhost.json').unlink()
        for name in publication.BADGES[4:] + publication.CLANG_BADGES:
            (self.artifact / 'badges' / f'{name}.json').unlink(missing_ok=True)
        (self.artifact / 'results.json').write_text(json.dumps(dict(environment=dict(cpu='Intel Xeon'),
            programs=[outcome('p', dict(run_cpu_O3=1), dict(run_cpu_O3=2))])))
        publication.prepare(self.artifact, self.target, 'intel', 'c' * 40,
                            'https://github.com/owner/repo/actions/runs/3', component='public')
        for name, content in previous.items():
            self.assertEqual((self.target / 'intel' / name).read_bytes(), content, name)
        text = (self.target / 'intel/report.md').read_text()
        self.assertIn('self-build.md', text)
        self.assertNotIn('Building the compiler', text)
        self.assertEqual(json.loads((self.target / 'intel/provenance.json').read_text())['revision'],
                         'c' * 40)

    def test_self_build_update_preserves_every_public_file(self):
        self.prepare('intel', 'Intel Xeon')
        paths = ['report.md', 'provenance.json', 'results.json', 'revision.svg',
                 'o3-programs.json'] + [name + '.svg' for name in publication.BADGES[:4]]
        previous = {name: (self.target / 'intel' / name).read_bytes() for name in paths}
        (self.artifact / 'results.json').unlink()
        for name in publication.BADGES[:4]:
            (self.artifact / 'badges' / f'{name}.json').unlink()
        self.self_build()
        for name, content in previous.items():
            self.assertEqual((self.target / 'intel' / name).read_bytes(), content, name)
        report = (self.target / 'intel/self-build.md').read_text()
        self.assertIn('commit/' + 'b' * 40, report)
        self.assertIn('[Raw rebuild results](selfhost.json)', report)
        self.assertEqual(json.loads((self.target / 'intel/self-build-provenance.json').read_text())['revision'],
                         'b' * 40)

    def test_self_build_revision_mismatch_fails_before_writing(self):
        self.self_build()
        previous = {p.name: p.read_bytes() for p in (self.target / 'intel').iterdir()}
        with self.assertRaisesRegex(ValueError, 'does not match'):
            publication.prepare(self.artifact, self.target, 'intel', 'c' * 40,
                                'https://github.com/owner/repo/actions/runs/3', component='self-build')
        self.assertEqual({p.name: p.read_bytes() for p in (self.target / 'intel').iterdir()}, previous)

    def test_invalid_badge_color_fails_before_any_replacement(self):
        self.prepare('intel', 'Intel Xeon')
        previous = {p.name: p.read_bytes() for p in (self.target / 'intel').iterdir()}
        path = self.artifact / 'badges/self-built.json'
        data = json.loads(path.read_text())
        data['color'] = 'invalid'
        path.write_text(json.dumps(data))
        with self.assertRaisesRegex(ValueError, 'color'):
            self.self_build()
        self.assertEqual({p.name: p.read_bytes() for p in (self.target / 'intel').iterdir()}, previous)


class IntelRunnerTests(unittest.TestCase):
    def test_suites_use_separate_sequential_runners_at_one_pinned_revision(self):
        args = SimpleNamespace(repo='owner/repo', ref='main', course_quality=True)
        calls = []

        def run(command, check):
            suite = command[command.index('--suite') + 1]
            calls.append(suite)
            self.assertIn('--revision', command)
            self.assertEqual(command[command.index('--revision') + 1], 'a' * 40)
            self.assertEqual(command[command.index('--directory') + 1], '/tmp/runners/' + suite)
            self.assertEqual('--course-quality' in command, suite == 'public')

        with mock.patch.object(intel_runner.subprocess, 'run', side_effect=run):
            intel_runner.run_suites(args, 'a' * 40, Path('/tmp/runners'))
        self.assertEqual(calls, ['public', 'self-build', 'index-cpp', 'simdjson', 'godot', 'boost', 'internal', 'wasm'])

    def test_suites_dispatch_distinct_workflows(self):
        args = SimpleNamespace(repo='owner/repo', ref='main', course_quality=True, suite='public')
        public = intel_runner.dispatch_command(args, 'a' * 40, 'unique-label')
        args.suite = 'self-build'
        build = intel_runner.dispatch_command(args, 'a' * 40, 'unique-label')
        self.assertEqual(public[3], 'public-benchmarks.yml')
        self.assertEqual(build[3], 'self-build-benchmarks.yml')
        self.assertIn('course_quality=true', public)
        self.assertNotIn('course_quality=true', build)
        self.assertIn('runner_label=unique-label', build)
        for suite, workflow in intel_runner.SUITES.items():
            args.suite = suite
            command = intel_runner.dispatch_command(args, 'a' * 40, 'unique-label')
            self.assertEqual(command[3], workflow)
            self.assertEqual(command[command.index('--ref') + 1], 'main')
            self.assertIn('revision=' + 'a' * 40, command)
            self.assertIn('runner_label=unique-label', command)


if __name__ == '__main__':
    unittest.main()
