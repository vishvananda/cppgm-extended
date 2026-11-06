#!/usr/bin/env python3
"""Protect pinned input identity and native lit directive/feature fidelity."""
import csv
import json
from pathlib import Path
from types import SimpleNamespace
import sys
import tempfile
import unittest

REPO = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO / 'scripts'))
import upstream_clang_lit_metadata as metadata


class MetadataTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name).resolve()
        (self.root / 'case.cpp').write_text('int value;\n')
        self.row = dict(repository='llvm-project', role='candidate', path='case.cpp',
                        revision='pinned', sha256=metadata.digest(self.root / 'case.cpp'))

    def parse(self, source, require_script):
        self.assertEqual(Path(source), self.root / 'case.cpp')
        self.assertFalse(require_script)
        directives = [SimpleNamespace(keyword='DEFINE:', value='-std=c++11', start_line_number=1),
                      SimpleNamespace(keyword='RUN:', command='%clang_cc1 %{flags} %s', start_line_number=2),
                      SimpleNamespace(keyword='REDEFINE:', value='-std=c++20', start_line_number=3),
                      SimpleNamespace(keyword='RUN:', command='%clang_cc1 %{flags} %s', start_line_number=4)]
        return {'RUN:': directives, 'DEFINE:': directives, 'REDEFINE:': directives,
                'REQUIRES:': ['x86 && (linux || darwin)'], 'UNSUPPORTED:': ['c++98'],
                'XFAIL:': ['ppc || (arm && darwin)'], 'ALLOW_RETRIES:': [2]}

    def test_order_and_feature_expressions_are_preserved_without_evaluation(self):
        row = metadata.extract_case(self.row, self.root, 'pinned', None, self.parse)
        self.assertEqual([d['keyword'] for d in row['directives']],
                         ['DEFINE:', 'RUN:', 'REDEFINE:', 'RUN:'])
        self.assertEqual(len(row['commands']), 2)
        self.assertEqual(row['metadata']['REQUIRES:'], ['x86 && (linux || darwin)'])
        self.assertEqual(row['metadata']['XFAIL:'], ['ppc || (arm && darwin)'])
        self.assertEqual(row['metadata']['UNSUPPORTED:'], ['c++98'])
        self.assertEqual(row['metadata']['ALLOW_RETRIES:'], [2])
        self.assertEqual(row['applicability'], 'unresolved')
        self.assertEqual(row['execution'], 'not-executed')

    def test_path_traversal_absolute_paths_and_external_symlinks_are_rejected(self):
        for path in ['../case.cpp', str(self.root / 'case.cpp')]:
            with self.subTest(path=path), self.assertRaisesRegex(ValueError, 'escapes'):
                metadata.extract_case(dict(self.row, path=path), self.root, 'pinned', None, self.parse)
        with tempfile.TemporaryDirectory() as other:
            external = Path(other) / 'outside.cpp'
            external.write_text('int outside;\n')
            (self.root / 'link.cpp').symlink_to(external)
            with self.assertRaisesRegex(ValueError, 'resolves outside'):
                metadata.extract_case(dict(self.row, path='link.cpp'), self.root, 'pinned', None, self.parse)

    def test_stale_revision_and_source_are_not_counted_as_parsed(self):
        with self.assertRaisesRegex(ValueError, 'revision differs'):
            metadata.extract_case(self.row, self.root, 'different', None, self.parse)
        (self.root / 'case.cpp').write_text('int changed;\n')
        with self.assertRaisesRegex(ValueError, 'differs from pinned'):
            metadata.extract_case(self.row, self.root, 'pinned', None, self.parse)

    def test_noncanonical_aliases_cannot_inflate_inventory_counts(self):
        for path in ['./case.cpp', 'dir//case.cpp', 'dir/./case.cpp']:
            with self.subTest(path=path), self.assertRaisesRegex(ValueError, 'not canonical'):
                metadata.extract_case(dict(self.row, path=path), self.root, 'pinned', None, self.parse)

    def test_source_drift_during_native_parsing_is_detected(self):
        def changed(source, require_script):
            result = self.parse(source, require_script)
            Path(source).write_text('int changed;\n')
            return result
        with self.assertRaisesRegex(ValueError, 'changed during'):
            metadata.extract_case(self.row, self.root, 'pinned', None, changed)

    def test_native_parse_error_is_an_explicit_blocker(self):
        def malformed(source, require_script):
            raise ValueError('native unterminated RUN directive')
        row = metadata.extract_case(self.row, self.root, 'pinned', None, malformed)
        self.assertEqual(row['status'], 'metadata-parse-blocked')
        self.assertIn('unterminated', row['reason'])
        self.assertEqual(row['execution'], 'not-executed')

    def test_no_run_file_keeps_unresolved_driver_ownership(self):
        row = metadata.extract_case(self.row, self.root, 'pinned', None,
                                    lambda *args, **kw: {'RUN:': [], 'REQUIRES:': None})
        self.assertEqual(row['status'], 'no-run-driver-ownership-unresolved')
        self.assertEqual(row['directives'], [])

    def inventory(self, rows):
        path = self.root / 'inventory.jsonl'
        path.write_text(''.join(json.dumps(row) + '\n' for row in rows))
        return path

    def test_all_candidates_remain_accounted_for_when_selection_is_narrow(self):
        counts = metadata.extract_inventory(self.inventory([self.row]), self.root, 'pinned',
                                             set(), self.parse, self.root / 'output.jsonl')
        row = json.loads((self.root / 'output.jsonl').read_text())
        self.assertFalse(row['selected'])
        self.assertEqual(counts['files'], 1)
        self.assertEqual(counts['command-directives'], 2)

    def test_duplicate_and_unmatched_selection_are_rejected(self):
        with self.assertRaisesRegex(ValueError, 'duplicate inventory'):
            metadata.extract_inventory(self.inventory([self.row, self.row]), self.root, 'pinned',
                                       None, self.parse, self.root / 'duplicate.jsonl')
        with self.assertRaisesRegex(ValueError, 'absent from inventory'):
            metadata.extract_inventory(self.inventory([self.row]), self.root, 'pinned',
                                       {'llvm-project/missing.cpp'}, self.parse, self.root / 'unmatched.jsonl')

    def test_existing_inventory_output_is_not_overwritten(self):
        path = self.root / 'output.jsonl'
        path.write_text('old evidence\n')
        with self.assertRaises(FileExistsError):
            metadata.extract_inventory(self.inventory([self.row]), self.root, 'pinned',
                                       None, self.parse, path)
        self.assertEqual(path.read_text(), 'old evidence\n')

    def test_selection_deduplicates_only_llvm_identities(self):
        path = self.root / 'selection.csv'
        with path.open('w', newline='') as stream:
            writer = csv.DictWriter(stream, ['source'])
            writer.writeheader()
            for source in ['llvm-project/case.cpp', 'gcc/case.C', 'llvm-project/case.cpp']:
                writer.writerow(dict(source=source))
        self.assertEqual(metadata.selected_sources(path), {'llvm-project/case.cpp'})


if __name__ == '__main__':
    unittest.main()
