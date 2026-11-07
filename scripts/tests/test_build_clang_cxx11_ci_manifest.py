#!/usr/bin/env python3
"""Keep initial CI selection conservative, reproducible and compiler-independent."""
import hashlib
from pathlib import Path
import sys
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import build_clang_cxx11_ci_manifest as manifest


def arguments(*flags):
    return manifest.project_arguments(['%clang_cc1', '-std=c++11', '-fsyntax-only',
                                       *flags, '%s'], set())


class PolicyTests(unittest.TestCase):
    def test_comments_strings_and_raw_strings_are_not_active_policy(self):
        text = '// #include <missing>\n/* __clang__ try */\n' \
               'const char *s = R"tag(#if __clang__ throw)tag";\n'
        self.assertEqual(manifest.source_policy(text), set())

    def test_preprocessing_and_compiler_branches_stay_visible(self):
        for text in ['#if __clang__\nint x;\n#endif', '#include "local.h"', '\ufeff#if 1',
                     '%:define VALUE 1', '/* before */ #define X 1',
                     '//continued\\\n#include "hidden.h"\n#if 1']:
            with self.subTest(text=text), self.assertRaises(manifest.Excluded):
                manifest.source_policy(text)

    def test_trigraphs_and_predefined_tokens_need_review(self):
        for text in ['??=if 1', 'int x=__cplusplus;', 'int x=__clang__;',
                     'int f(){return __builtin_clz(1);}']:
            with self.subTest(text=text), self.assertRaises(manifest.Excluded):
                manifest.source_policy(text)

    def test_mixed_diagnostics_are_not_positive_or_adjudicated_negatives(self):
        for text in ['// expected-error {{bad}}', '// cxx11-error@+1 {{bad}}',
                     '// expected-warning-re {{extension}}', '// ref-note {{note}}']:
            with self.subTest(text=text), self.assertRaises(manifest.Excluded):
                manifest.source_policy(text)
        self.assertEqual(manifest.source_policy('// expected-no-diagnostics\nint x;'), set())

    def test_cc1_driver_policy_and_action_are_owned_separately(self):
        row = arguments('-verify', '-Wall', '-O2')
        self.assertEqual(row['upstream_flags'], ['-cc1', '-Wall', '-O2'])
        self.assertEqual(row['candidate_flags'], ['-O2'])
        self.assertEqual(row['dialect'], 'c++11')
        self.assertTrue(any('object generation' in v for v in row['adaptations']))
        for key in ['upstream_flags', 'candidate_flags']:
            self.assertNotIn('-fsyntax-only', row[key])
            self.assertNotIn('-pedantic-errors', row[key])

    def test_unknown_policies_other_actions_and_inputs_are_exclusions(self):
        for value in ['-fno-rtti', '-fno-exceptions', '-Werror', '-Wno-narrowing',
                      '-Wno-c++11-narrowing', '-Wno-c++11-extensions',
                      '-emit-llvm', '-ast-dump', '-DVALUE=1', '-include', 'other.cpp']:
            with self.subTest(value=value), self.assertRaises(manifest.Excluded):
                arguments(value)

    def test_language_aliases_and_conflicts(self):
        for spelling, dialect in manifest.DIALECTS.items():
            row = manifest.project_arguments(['%clang_cc1', '-std='+spelling,
                                               '-fsyntax-only', '%s'], set())
            self.assertEqual(row['dialect'], dialect)
        for flags in [[], ['-std=c++17'], ['-std=c++11', '-std=gnu++11']]:
            with self.subTest(flags=flags), self.assertRaises(manifest.Excluded):
                manifest.project_arguments(['%clang_cc1', *flags, '-fsyntax-only', '%s'], set())

    def test_disabled_exception_errors_cannot_become_language_checks(self):
        args = ['%clang_cc1', '-std=c++11', '-fsyntax-only', '%s']
        with self.assertRaises(manifest.Excluded):
            manifest.project_arguments(args, {'throw'})
        with self.assertRaises(manifest.Excluded):
            manifest.project_arguments([*args, '-fexceptions'], {'throw'})
        row = manifest.project_arguments([*args, '-fexceptions', '-fcxx-exceptions'], {'throw'})
        self.assertIn('-fcxx-exceptions', row['upstream_flags'])

    def test_target_capability_is_not_inferred_from_syntax(self):
        row = arguments('-triple', 'x86_64-pc-linux-gnu')
        self.assertEqual(row['candidate_flags'], ['--target=x86_64-pc-linux-gnu'])
        with self.assertRaises(manifest.Excluded):
            arguments('-triple', 'i386-pc-linux-gnu')


class CatalogueTests(unittest.TestCase):
    def test_selection_hash_dedup_and_unresolved_cases_do_not_depend_on_compiler(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp).resolve()
            names = ['clang/test/good.cpp', 'clang/test/bad.cpp', 'clang/test/Inputs/part.cpp',
                     'clang/test/config/case.cpp', 'clang/test/feature.cpp']
            for name in names:
                path = root/name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text('// RUN: %clang_cc1 -std=c++11 -fsyntax-only %s\nint x;\n')
            (root/names[1]).write_text('// RUN: -std=c++11\n// expected-error {{bad}}\n')
            (root/'clang/test/config/lit.local.cfg').write_text('config.unsupported=True\n')

            def parse(path, require_script):
                command = SimpleNamespace(keyword='RUN:', command='same native command')
                return {'RUN:': [command, command],
                        'REQUIRES:': ['unknown-feature'] if path.endswith('feature.cpp') else None}

            projection = arguments()
            with patch.object(manifest.subprocess, 'check_output', return_value=
                              ('\0'.join(names)+'\0').encode()), \
                    patch.object(manifest, 'project_run', return_value=projection), \
                    patch.object(manifest.subprocess, 'run', side_effect=AssertionError('compiler forbidden')):
                row = manifest.collect(root, parse, None)
            self.assertEqual(len(row['cases']), 1)
            self.assertEqual(len(row['exclusions']), 4)
            case = row['cases'][0]
            self.assertEqual(case['path'], names[0])
            self.assertEqual(case['sha256'], hashlib.sha256((root/names[0]).read_bytes()).hexdigest())
            self.assertEqual(len(case['original']), 2)
            self.assertEqual(case['expect'], 'accept')
            self.assertEqual(case['action'], 'compile')
            self.assertTrue(any('configuration' in v['reason'] for v in row['exclusions']))


if __name__ == '__main__':
    unittest.main()
