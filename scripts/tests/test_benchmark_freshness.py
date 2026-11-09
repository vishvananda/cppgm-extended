#!/usr/bin/env python3
"""Old revisions remain visible without changing the measured numbers."""
import json
from pathlib import Path
import re
import sys
import tempfile
import unittest
import xml.etree.ElementTree as ET

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import benchmark_freshness as freshness
from publish_benchmark_badges import badge_svg


class FreshnessTests(unittest.TestCase):
    def make_group(self, root, revision='a' * 40, value='4.41×', host='intel'):
        directory = root / host
        directory.mkdir(exist_ok=True)
        metadata = directory / 'provenance.json'
        metadata.write_text(json.dumps(dict(cpu='Intel test CPU', revision=revision)))
        path = directory / 'o3-compile.svg'
        path.write_text(badge_svg(dict(label='O3 compile vs GCC', message=value,
                                      color='14532d', left_label='E5 v4')))
        return path, metadata

    def age(self, count):
        return dict(commits_behind=count, status='ahead' if count else 'identical', measured_at='2026-11-08T00:00:00Z')

    def test_old_number_is_marked_and_linked_to_its_exact_revision(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            path, metadata = self.make_group(root)
            original = metadata.read_bytes()
            freshness.refresh(root, 'b' * 40, 'owner/repo', lambda *args: self.age(40))
            text = path.read_text()
            self.assertIn('4.41×*', text)
            self.assertIn('40 commits behind main', text)
            self.assertIn('>Intel</text>', text)
            self.assertNotIn('>E5 v4</text>', text)
            self.assertIn('fill="#14532d"', text)
            self.assertEqual(metadata.read_bytes(), original)
            self.assertIn('owner/repo/commit/' + 'a' * 40, (root / 'freshness.md').read_text())

    def test_threshold_and_unmeasured_placeholders(self):
        for count in (0, 9, 10):
            with self.subTest(count=count), tempfile.TemporaryDirectory() as tmp:
                root = Path(tmp)
                path, _ = self.make_group(root)
                freshness.refresh(root, 'b' * 40, 'owner/repo', lambda *args: self.age(count))
                self.assertEqual('4.41×*' in path.read_text(), count >= 10)
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            path, _ = self.make_group(root, value='n/m', host='github-intel')
            freshness.refresh(root, 'b' * 40, 'owner/repo', lambda *args: self.age(100))
            self.assertNotIn('n/m*', path.read_text())
            self.assertIn('>CI Intel</text>', path.read_text())

    def test_repeated_refresh_is_idempotent_and_reuses_comparisons(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            path, _ = self.make_group(root)
            other, _ = self.make_group(root, host='epyc')
            calls = []
            def compare(*args):
                calls.append(args)
                return self.age(40)
            freshness.refresh(root, 'b' * 40, 'owner/repo', compare)
            first = {p: p.read_bytes() for p in (path, other, root / 'freshness.json', root / 'freshness.md')}
            freshness.refresh(root, 'b' * 40, 'owner/repo', compare)
            self.assertEqual(len(calls), 1)
            self.assertEqual(first, {p: p.read_bytes() for p in first})
            self.assertNotIn('×**', path.read_text())

    def test_age_updates_when_main_moves_without_rerunning_measurement(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            path, _ = self.make_group(root)
            freshness.refresh(root, 'b' * 40, 'owner/repo', lambda *args: self.age(9))
            self.assertNotIn('4.41×*', path.read_text())
            freshness.refresh(root, 'c' * 40, 'owner/repo', lambda *args: self.age(1))
            self.assertIn('4.41×*', path.read_text())
            self.assertIn('10 commits behind', path.read_text())

    def test_fast_forward_uses_one_lookup_to_advance_all_known_ancestors(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            a, _ = self.make_group(root)
            c, _ = self.make_group(root, revision='c' * 40, host='epyc')
            freshness.refresh(root, 'b' * 40, 'owner/repo', lambda *args: self.age(8))
            calls = []
            def compare(repo, measured, main):
                calls.append(measured)
                return self.age(3)
            freshness.refresh(root, 'd' * 40, 'owner/repo', compare)
            self.assertEqual(calls, ['b' * 40])
            self.assertIn('11 commits behind main', a.read_text())
            self.assertIn('11 commits behind main', c.read_text())

    def test_diverged_main_rechecks_measurements_instead_of_incrementing(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            path, _ = self.make_group(root)
            freshness.refresh(root, 'b' * 40, 'owner/repo', lambda *args: self.age(8))
            calls = []
            def compare(repo, measured, main):
                calls.append(measured)
                return self.age(None if measured == 'b' * 40 else 2)
            freshness.refresh(root, 'd' * 40, 'owner/repo', compare)
            self.assertEqual(calls, ['b' * 40, 'a' * 40])
            self.assertNotIn('4.41×*', path.read_text())

    def test_unmerged_source_is_explicit_instead_of_claiming_current(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            path, _ = self.make_group(root)
            freshness.refresh(root, 'b' * 40, 'owner/repo', lambda *args: self.age(None))
            self.assertIn('outside main history', path.read_text())
            self.assertIn('4.41×*', path.read_text())

    def test_failed_lookup_cannot_partly_rewrite_retained_badges(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            path, _ = self.make_group(root)
            other, _ = self.make_group(root, revision='c' * 40, host='epyc')
            original = {p: p.read_bytes() for p in (path, other)}
            def compare(repo, revision, main):
                if revision == 'a' * 40:
                    raise ValueError('comparison unavailable')
                return self.age(40)
            with self.assertRaisesRegex(ValueError, 'unavailable'):
                freshness.refresh(root, 'b' * 40, 'owner/repo', compare)
            self.assertEqual(original, {p: p.read_bytes() for p in original})

    def test_library_and_boost_badges_share_the_same_labels_and_policy(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            for workload in ('index-cpp', 'simdjson', 'godot', 'boost/o3', 'internal'):
                directory = root / workload
                (directory / 'github-intel').mkdir(parents=True)
                (directory / 'github-intel/metadata.json').write_text(json.dumps(
                    dict(revision='a' * 40, cpu='Intel hosted CPU')))
                (directory / 'github-intel-compile-gcc.svg').write_text(badge_svg(
                    dict(label='compile & verify', message='3.00×', color='14532d', left_label='8370C')))
            freshness.refresh(root, 'b' * 40, 'owner/repo', lambda *args: self.age(40))
            for path in root.rglob('*.svg'):
                text = path.read_text()
                ET.fromstring(text)
                self.assertIn('>CI Intel</text>', text)
                self.assertIn('3.00×*', text)
                self.assertIn('compile &amp; verify', text)

    def test_publication_refreshes_in_existing_write_jobs_without_completion_events(self):
        workflows = Path(__file__).resolve().parents[2] / '.github/workflows'
        standalone = (workflows / 'benchmark-freshness.yml').read_text()
        self.assertNotIn('workflow_run:', standalone)
        self.assertIn('branches: [main]', standalone)
        count = 0
        for path in workflows.glob('*.yml'):
            for job in re.split(r'\n  [a-zA-Z0-9_-]+:\n', path.read_text()):
                if 'cd badge-branch' not in job:
                    continue
                with self.subTest(workflow=path.name):
                    self.assertIn('GH_TOKEN: ${{ github.token }}', job)
                    self.assertLess(job.index('scripts/benchmark_freshness.py'), job.index('cd badge-branch'))
                    self.assertIn('git add .', job)
                count += 1
        self.assertGreaterEqual(count, 10)


if __name__ == '__main__':
    unittest.main()
