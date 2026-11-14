import importlib.util
import json
from pathlib import Path
import subprocess
import tarfile
import tempfile
import unittest

SPEC = importlib.util.spec_from_file_location('release_source', Path(__file__).resolve().parents[1] / 'export_release_source.py')
release = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(release)


class SourceRelease(unittest.TestCase):
    def test_tagged_tree_is_reproducible_and_excludes_local_files(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            def git(*args):
                return subprocess.check_output(['git', *args], cwd=root, stderr=subprocess.DEVNULL)
            git('init', '-q')
            for name in ['LICENSE', 'NOTICE', 'source.cpp']:
                (root / name).write_text('committed ' + name)
            git('add', '.')
            git('-c', 'user.name=Test', '-c', 'user.email=test@example.invalid', 'commit', '-qm', 'Source')
            git('tag', 'v8.1')
            (root / 'source.cpp').write_text('local edits')
            (root / 'untracked.secret').write_text('must not ship')
            first = release.export(root, 'v8.1', root / 'a')
            second = release.export(root, 'v8.1', root / 'b')
            self.assertEqual(first.read_bytes(), second.read_bytes())
            with tarfile.open(first) as archive:
                self.assertEqual(set(archive.getnames()), {'cppgm-v8.1', 'cppgm-v8.1/LICENSE',
                                                        'cppgm-v8.1/NOTICE', 'cppgm-v8.1/source.cpp'})
                self.assertEqual(archive.extractfile('cppgm-v8.1/source.cpp').read(), b'committed source.cpp')
            record = json.loads((first.parent / 'source-provenance.json').read_text())
            self.assertEqual(record['source_sha'], git('rev-parse', 'HEAD').decode().strip())
            with self.assertRaises(FileExistsError):
                release.export(root, 'v8.1', root / 'a')
            git('add', 'source.cpp')
            git('-c', 'user.name=Test', '-c', 'user.email=test@example.invalid', 'commit', '-qm', 'Later')
            with self.assertRaisesRegex(ValueError, 'tagged commit'):
                release.export(root, 'v8.1', root / 'c')

    def test_only_version_tags(self):
        for tag in ['main', 'reference-binaries', '../v8', 'v8;command', 'v8/foo']:
            with self.assertRaises(ValueError):
                release.export(Path('/nonexistent'), tag, Path('/unused'))


if __name__ == '__main__':
    unittest.main()
