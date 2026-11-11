#!/usr/bin/env python3
"""Publication bounds protect Git without discarding displayed measurements."""
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import benchmark_publication as publication


class PublicationTests(unittest.TestCase):
    def test_only_compact_report_material_is_retained(self):
        for name in ('boost/o3/epyc/report.md', 'boost/o3/intel/metadata.json',
                     'boost/o3/history/' + 'a' * 40 + '/epyc/standard/records.json',
                     'simdjson/epyc/results.json', 'epyc/o3-generated.svg'):
            self.assertTrue(publication.retained(name), name)
        for name in ('boost/o3/epyc/cppgm/results.json', 'boost/o3/epyc/cppgm/b2.xml',
                     'boost/o3/epyc/runtime/manifest.json', 'godot/epyc/commands-cppgm.json',
                     'internal/epyc/gcc-observations.jsonl.gz', 'runtime-observations.jsonl'):
            self.assertFalse(publication.retained(name), name)

    def test_prepare_preserves_all_values_and_rejects_raw_or_oversized_files(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            data = dict(speedup=1.25, metadata=dict(cpu='Intel Xeon', revision='a' * 40))
            path = root / 'results.json'
            path.write_text(json.dumps(data, indent=2))
            publication.prepare(root)
            self.assertEqual(json.loads(path.read_text()), data)
            self.assertEqual(path.read_text(), json.dumps(data, separators=(',', ':')) + '\n')
            bad = root / 'b2.log'
            bad.write_text('a raw log')
            with self.assertRaisesRegex(ValueError, 'CI artifacts'):
                publication.check(root)
            bad.unlink()
            with path.open('wb') as stream:
                stream.truncate(publication.MAX_FILE_BYTES + 1)
            with self.assertRaisesRegex(ValueError, 'CI artifacts'):
                publication.check(root)

    def test_migration_preserves_badges_reports_and_provenance_without_raw_checkout(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            repo, output = root / 'repo', root / 'compact'
            repo.mkdir()
            subprocess.run(['git', 'init', '-q', str(repo)], check=True)
            base = repo / 'boost/o3/epyc'
            (base / 'standard').mkdir(parents=True)
            (base / 'cppgm').mkdir()
            url = 'https://github.com/example/project/actions/runs/123'
            records = dict(schema=1, records=[dict(run_url=url, raw_url='old', speedup=1.25)])
            (base / 'standard/records.json').write_text(json.dumps(records))
            (base / 'results.json').write_text(json.dumps(dict(compilers='raw commands')))
            (base / 'metadata.json').write_text(json.dumps(dict(cpu='Intel', revision='a' * 40)))
            (base / 'cppgm/results.json').write_text('raw commands')
            (base / 'report.md').write_text('Measurement (O3) [log](cppgm/b2.log) [records](results.json)\n')
            (base / 'cppgm/b2.log').write_text('raw log')
            badge = repo / 'boost/o3/epyc-runtime-gcc.svg'
            badge.write_text('<svg>1.25×</svg>')
            subprocess.run(['git', '-C', str(repo), 'add', '.'], check=True)
            subprocess.run(['git', '-C', str(repo), '-c', 'user.name=Test', '-c', 'user.email=test@example.org',
                            'commit', '-qm', 'fixture'], check=True)
            before = subprocess.check_output(['git', '-C', str(repo), 'rev-parse', 'HEAD'])
            publication.compact_git(repo, 'HEAD', output)
            self.assertEqual(before, subprocess.check_output(['git', '-C', str(repo), 'rev-parse', 'HEAD']))
            self.assertEqual((output / 'boost/o3/epyc-runtime-gcc.svg').read_bytes(), badge.read_bytes())
            compact = output / 'boost/o3/epyc'
            self.assertFalse((compact / 'cppgm').exists())
            self.assertEqual(json.loads((compact / 'results.json').read_text())['records'][0]['speedup'], 1.25)
            self.assertEqual((compact / 'report.md').read_text(),
                             f'Measurement (O3) [log]({url}) [records](results.json)\n')


if __name__ == '__main__':
    unittest.main()
