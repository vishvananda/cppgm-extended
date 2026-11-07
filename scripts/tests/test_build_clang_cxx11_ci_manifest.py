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
    def test_llvm_object_projection_preserves_semantic_flags_and_original_obligations(self):
        args = ['%clang_cc1', '-std=c++11', '-emit-llvm', '-disable-llvm-passes',
                '-O2', '-triple', 'x86_64-linux-gnu', '%s', '-o', '-']
        check = ['FileCheck', '--check-prefixes=CHECK,O2', '%s',
                 '--implicit-check-not=call void @llvm.lifetime']
        row = manifest.project_llvm_arguments(args, check, set())
        self.assertEqual(row['candidate_flags'], ['-O2', '--target=x86_64-linux-gnu'])
        self.assertEqual(row['upstream_flags'], ['-cc1', '-O2', '-triple', 'x86_64-linux-gnu'])
        self.assertEqual(row['qualification']['kind'], 'llvm-object-cxx11')
        self.assertEqual(row['qualification']['additional_upstream_flags'], ['-pedantic-errors'])
        self.assertEqual(row['unmet_native_oracles']['filecheck_arguments'], check)
        self.assertEqual(row['unmet_native_oracles']['status'], 'not-executed')
        self.assertEqual(row['unmet_native_oracles']['original_action_output_options'],
                         ['-emit-llvm', '-disable-llvm-passes', '-o', '-'])
        implicit = manifest.project_llvm_arguments([v for v in args if v != '-std=c++11'],
                                                   check, set(), implicit_cxx11=True)
        self.assertEqual(implicit['dialect'], 'c++11')
        self.assertEqual(implicit['qualification']['kind'], 'llvm-object-cxx11')

    def test_llvm_projection_blocks_unmapped_policies_inputs_actions_and_verifiers(self):
        base = ['%clang_cc1', '-std=c++11', '-emit-llvm', '%s', '-o', '-']
        for flag in ['-fno-rtti', '-fno-exceptions', '-fms-extensions', '-Werror',
                     '-verify', '-verify=expected', '-std=c++17', '-fsyntax-only']:
            with self.subTest(flag=flag), self.assertRaises(manifest.Excluded):
                manifest.project_llvm_arguments([*base, flag], ['FileCheck', '%s'], set())
        for args, check in [([*base[:-1], '%t'], ['FileCheck', '%s']),
                            (base, ['FileCheck', 'other.cpp']),
                            (base, ['FileCheck', '%s', '--input-file=%t']),
                            (base, ['FileCheck', '%s', '%s']),
                            (base, ['other-helper', '%s']),
                            ([v for v in base if v != '-emit-llvm'], ['FileCheck', '%s'])]:
            with self.subTest(args=args, check=check), self.assertRaises(manifest.Excluded):
                manifest.project_llvm_arguments(args, check, set())
        with self.assertRaises(manifest.Excluded):
            manifest.project_llvm_arguments(base, ['FileCheck', '%s'], {'throw'})

    def test_native_pipeline_boundary_requires_opt_in_and_literal_two_stage_commands(self):
        class Command:
            def __init__(self, args, redirects=()):
                self.args, self.redirects = args, redirects
        class Pipeline:
            def __init__(self, commands, negate=False):
                self.commands, self.negate = commands, negate
        first = Command(['%clang_cc1', '-std=c++11', '-emit-llvm', '%s', '-o', '-'])
        last = Command(['FileCheck', '%s'])
        tree = Pipeline([first, last])
        native = SimpleNamespace(TestRunner=SimpleNamespace(kPdbgRegex=r'a^'),
            ShCommands=SimpleNamespace(Command=Command, Pipeline=Pipeline),
            ShUtil=SimpleNamespace(ShParser=lambda *a, **k: SimpleNamespace(parse=lambda: tree)))
        with self.assertRaises(manifest.Excluded):
            manifest.project_run('native', native, set())
        self.assertEqual(manifest.project_run('native', native, set(), llvm_objects=True)
                         ['projection_kind'], 'llvm-object-acceptance')
        for invalid in [Pipeline([first, last], negate=True), Pipeline([first, last, last]),
                        Pipeline([Command(first.args, redirects=['output']), last]),
                        Pipeline([Command([object()]), last])]:
            tree = invalid
            with self.subTest(tree=tree), self.assertRaises(manifest.Excluded):
                manifest.project_run('native', native, set(), llvm_objects=True)

    def test_implicit_mode_requires_separate_qualification_without_changing_normal_flags(self):
        args = ['%clang_cc1', '-fsyntax-only', '-verify=expected,cxx11', '%s']
        with self.assertRaises(manifest.Excluded):
            manifest.project_arguments(args, set())
        row = manifest.project_arguments(args, set(), implicit_cxx11=True)
        self.assertEqual(row['dialect'], 'c++11')
        self.assertEqual(row['upstream_flags'], ['-cc1'])
        self.assertEqual(row['candidate_flags'], [])
        self.assertEqual(row['verification'], ['-verify=expected,cxx11'])
        self.assertEqual(row['qualification']['additional_upstream_flags'], ['-pedantic-errors'])
        explicit = ['%clang_cc1', '-std=c++11', '-fsyntax-only', '%s']
        self.assertEqual(manifest.project_arguments(explicit, set()),
                         manifest.project_arguments(explicit, set(), implicit_cxx11=True))
        with self.assertRaises(manifest.Excluded):
            manifest.project_arguments([*args, '-std=c++17'], set(), implicit_cxx11=True)

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
    def test_source_pinned_vendor_tokens_keep_attribute_effects_unmet(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            name = 'clang/test/attribute.cpp'
            path = root/name
            path.parent.mkdir(parents=True)
            path.write_text('void f(){ [[clang::annotate(_Generic(0,int:0,default:1))]] int n; }\n')
            policy = {'llvm-project/'+name: dict(status='vendor-attribute-positive',
                sha256=hashlib.sha256(path.read_bytes()).hexdigest(), issue='CXX11-067')}
            projection = manifest.project_llvm_arguments(
                ['%clang_cc1', '-emit-llvm', '%s', '-o', '-'], ['FileCheck', '%s'], set(),
                implicit_cxx11=True)
            parsed = {'RUN:': [SimpleNamespace(keyword='RUN:', command='native LLVM pipeline')]}
            with patch.object(manifest.subprocess, 'check_output', return_value=(name+'\0').encode()), \
                    patch.object(manifest, 'project_run', return_value=projection):
                row = manifest.collect(root, lambda *a, **k: parsed, None, dispositions=policy,
                                       implicit_cxx11=True, llvm_objects=True)
                case, = row['cases']
                self.assertEqual(case['expect'], 'accept')
                self.assertEqual(case['dialect'], 'c++11')
                self.assertEqual(case['qualification']['kind'], 'vendor-attribute-cxx11')
                self.assertEqual(case['qualification']['additional_upstream_flags'], [])
                self.assertEqual(case['unmet_native_oracles']['status'], 'not-executed')
                path.write_text(path.read_text()+'int x = _Generic(0,int:0,default:1);')
                with self.assertRaisesRegex(ValueError, 'Stale'):
                    manifest.collect(root, lambda *a, **k: parsed, None, dispositions=policy,
                                     implicit_cxx11=True, llvm_objects=True)

    def test_llvm_case_keeps_all_native_runs_unmet_and_cannot_become_a_negative(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            name = 'clang/test/ir.cpp'
            path = root/name
            path.parent.mkdir(parents=True)
            path.write_text('// RUN: -std=c++11\nint x;\n')
            projection = manifest.project_llvm_arguments(
                ['%clang_cc1', '-std=c++11', '-emit-llvm', '%s', '-o', '-'], ['FileCheck', '%s'], set())
            parsed = {'RUN:': [SimpleNamespace(keyword='RUN:', command='LLVM pipeline'),
                              SimpleNamespace(keyword='RUN:', command='native runtime')]}
            def project(command, *a, **k):
                if command == 'native runtime':
                    raise manifest.Excluded('Unmapped runtime')
                return projection
            with patch.object(manifest.subprocess, 'check_output', return_value=(name+'\0').encode()), \
                    patch.object(manifest, 'project_run', side_effect=project):
                row = manifest.collect(root, lambda *a, **k: parsed, None, llvm_objects=True)
                case, = row['cases']
                self.assertEqual(case['expect'], 'accept')
                self.assertEqual(case['unmet_native_oracles']['all_original_RUNs'],
                                 ['LLVM pipeline', 'native runtime'])
                self.assertEqual(len(row['exclusions']), 1)
                path.write_text(path.read_text()+'// expected-error {{bad}}\n')
                negative = manifest.collect(root, lambda *a, **k: parsed, None,
                                            llvm_objects=True, verify_negatives=True)
                self.assertEqual(negative['cases'], [])
                self.assertIn('active-oracle review', negative['exclusions'][0]['reason'])

    def test_documented_gnu_extension_uses_source_pinned_separate_mode(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            name = 'clang/test/anonymous.cpp'
            path = root/name
            path.parent.mkdir(parents=True)
            path.write_text('// RUN: %clang_cc1 -fsyntax-only %s\nstruct S { struct { int x; }; };\n')
            policy = {'llvm-project/'+name: dict(status='documented-gnu-extension-positive',
                        sha256=hashlib.sha256(path.read_bytes()).hexdigest(), issue='CXX11-065')}
            projection = manifest.project_arguments(['%clang_cc1', '-fsyntax-only', '%s'],
                                                    set(), implicit_cxx11=True)
            parsed = {'RUN:': [SimpleNamespace(keyword='RUN:', command='same'),
                              SimpleNamespace(keyword='RUN:', command='same')]}
            with patch.object(manifest.subprocess, 'check_output', return_value=(name+'\0').encode()), \
                    patch.object(manifest, 'project_run', return_value=projection):
                row = manifest.collect(root, lambda *a, **k: parsed, None, dispositions=policy,
                                       implicit_cxx11=True)
                self.assertEqual(len(row['cases']), 1)
                case = row['cases'][0]
                self.assertEqual(case['dialect'], 'gnu++11')
                self.assertEqual(case['expect'], 'accept')
                self.assertEqual(case['qualification']['additional_upstream_flags'], [])
                self.assertEqual(case['qualification']['kind'], 'documented-gnu-cxx11')
                self.assertEqual(projection['dialect'], 'c++11')
                self.assertEqual(len(case['original']), 2)
                policy['llvm-project/'+name]['sha256'] = 'stale'
                with self.assertRaisesRegex(ValueError, 'Stale'):
                    manifest.collect(root, lambda *a, **k: parsed, None, dispositions=policy,
                                     implicit_cxx11=True)

    def test_no_standard_sources_are_inventoried_only_with_explicit_projection_option(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            name = 'clang/test/implicit.cpp'
            path = root/name
            path.parent.mkdir(parents=True)
            path.write_text('// RUN: %clang_cc1 -fsyntax-only %s\nint x;\n')
            parsed = {'RUN:': [SimpleNamespace(keyword='RUN:', command='native command')]}
            projection = manifest.project_arguments(['%clang_cc1', '-fsyntax-only', '%s'],
                                                    set(), implicit_cxx11=True)
            with patch.object(manifest.subprocess, 'check_output', return_value=(name+'\0').encode()), \
                    patch.object(manifest, 'project_run', return_value=projection), \
                    patch.object(manifest.subprocess, 'run', side_effect=AssertionError('compiler forbidden')):
                self.assertEqual(manifest.collect(root, lambda *a, **k: parsed, None)['cases'], [])
                row = manifest.collect(root, lambda *a, **k: parsed, None, implicit_cxx11=True)
            self.assertEqual(len(row['cases']), 1)
            self.assertEqual(row['cases'][0]['expect'], 'accept')
            self.assertIn('qualification', row['cases'][0])

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
            # Each unsupported negative RUN retains its own position; the
            # positive duplicate still deduplicates to one language case.
            self.assertEqual(len(row['exclusions']), 5)
            case = row['cases'][0]
            self.assertEqual(case['path'], names[0])
            self.assertEqual(case['sha256'], hashlib.sha256((root/names[0]).read_bytes()).hexdigest())
            self.assertEqual(len(case['original']), 2)
            self.assertEqual(case['expect'], 'accept')
            self.assertEqual(case['action'], 'compile')
            self.assertTrue(any('configuration' in v['reason'] for v in row['exclusions']))


class SourceDispositions(unittest.TestCase):
    def test_review_is_bound_to_exact_source(self):
        name = 'clang/test/ifndr.cpp'
        dispositions = {'llvm-project/' + name: dict(sha256='known', status='not-required', reason='IFNDR')}
        self.assertEqual(manifest.disposition_reason(name, 'known', dispositions), 'not-required: IFNDR')
        with self.assertRaisesRegex(ValueError, 'Stale'):
            manifest.disposition_reason(name, 'changed', dispositions)

    def test_unreviewed_sources_stay_in_selection(self):
        self.assertIsNone(manifest.disposition_reason('clang/test/valid.cpp', 'known', {}))


class RequiredDiagnosticDiscovery(unittest.TestCase):
    def test_active_prefixes_use_complete_names(self):
        source = '// expected-cxx11-error {{one}}\n// expected-note {{not an error}}'
        with self.assertRaises(manifest.Excluded):
            manifest.required_errors(source, ['-verify'])
        row = manifest.required_errors(source, ['-verify=expected-cxx11,extra'])
        self.assertTrue(row['active_mandatory_error'])
        self.assertEqual(row['prefixes'], ['expected-cxx11', 'extra'])

    def test_literals_and_raw_literals_cannot_create_expectations(self):
        for source in ['const char*s="// expected-error {{x}}";',
                       'const char*s=R"tag(/* expected-error {{x}} */)tag";',
                       '// other-error {{x}}\n// expected-warning {{x}}']:
            with self.subTest(source=source), self.assertRaises(manifest.Excluded):
                manifest.required_errors(source, ['-verify'])

    def test_only_positive_minimum_errors_are_mandatory(self):
        for count in ['', '+', '1', '2+', '1-3']:
            with self.subTest(count=count):
                self.assertTrue(manifest.required_errors(
                    '// expected-error@+1 '+count+' {{ignored text}}', ['-verify']))
        for count in ['0', '0+', '0-1']:
            with self.subTest(count=count), self.assertRaises(manifest.Excluded):
                manifest.required_errors('// expected-error '+count+' {{optional}}', ['-verify'])

    def test_unknown_verifier_forms_remain_blocked(self):
        for flags in [[], ['-verify', '-verify'], ['-verify='], ['-verify=expected,*']]:
            with self.subTest(flags=flags), self.assertRaises(manifest.Excluded):
                manifest.required_errors('// expected-error {{bad}}', flags)
        for text in ['// expected-error@#marker {{bad}}',
                     '// expected-error@header.h:3 {{bad}}',
                     '// expected-error@0 {{bad}}', '// expected-error 3-1 {{bad}}',
                     '// expected-error-re {{regex {{.*}}}}',
                     '// expected-error 1- {{bad}}', '// expected-error missing-body',
                     '// expected-no-diagnostics\n// expected-error {{bad}}',
                     'const char*s="unterminated; // expected-error {{bad}}']:
            with self.subTest(text=text), self.assertRaises(manifest.Excluded):
                manifest.required_errors(text, ['-verify'])

    def test_line_splicing_precedes_comment_discovery(self):
        source = '// expected-err\\\nor {{bad}}'
        self.assertTrue(manifest.required_errors(source, ['-verify']))

    def test_negative_sources_keep_preprocessing_and_exception_guards(self):
        with self.assertRaises(manifest.Excluded):
            manifest.source_policy('#if 0\n// expected-error {{hidden}}\n#endif',
                                   allow_diagnostics=True)
        tokens = manifest.source_policy('void f(){throw 1;} // expected-error {{bad}}',
                                        allow_diagnostics=True)
        with self.assertRaises(manifest.Excluded):
            manifest.project_arguments(['%clang_cc1', '-std=c++11', '-fsyntax-only',
                                        '-verify', '%s'], tokens)

    def test_selection_requires_exact_source_language_adjudication(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp).resolve()
            name = 'clang/test/negative.cpp'
            path = root/name
            path.parent.mkdir(parents=True)
            path.write_text('// RUN: -std=c++11\nint bad; // expected-error {{bad}}\n')
            digest = hashlib.sha256(path.read_bytes()).hexdigest()
            adjudication = {'expected': 'diagnose-invalid', 'source_sha256': digest,
                            'clause': 'required language rule'}
            parsed = {'RUN:': [SimpleNamespace(keyword='RUN:', command='native command')]}
            with patch.object(manifest.subprocess, 'check_output', return_value=(name+'\0').encode()), \
                    patch.object(manifest, 'project_run', return_value=arguments('-verify')):
                unreviewed = manifest.collect(root, lambda *a, **k: parsed, None)
                self.assertEqual(unreviewed['cases'], [])
                self.assertIn('language-rule triage', unreviewed['exclusions'][0]['reason'])
                broader = manifest.collect(root, lambda *a, **k: parsed, None, verify_negatives=True)
                broad_case, = broader['cases']
                self.assertEqual(broad_case['expect'], 'diagnostic')
                self.assertIsNone(broad_case['language_oracle']['adjudication'])
                self.assertIn('failure-led triage', broad_case['language_oracle']['standards_status'])
                key = 'llvm-project/'+name+'#0#c++11'
                reviewed = manifest.collect(root, lambda *a, **k: parsed, None,
                                             adjudications={key: adjudication})
                case, = reviewed['cases']
                self.assertEqual(case['expect'], 'diagnostic')
                self.assertTrue(case['language_oracle']['active_mandatory_error'])
                self.assertNotIn('verification', case)
                self.assertNotIn('-verify', case['upstream_flags'])
                blocked = manifest.collect(root, lambda *a, **k: parsed, None,
                                           adjudications={key: dict(adjudication, expected='not-required')},
                                           verify_negatives=True)
                self.assertEqual(blocked['cases'], [])
                common = manifest.collect(root, lambda *a, **k: parsed, None,
                                          dispositions={'llvm-project/'+name: dict(
                                              sha256=digest, status='not-required', reason='IFNDR')},
                                          verify_negatives=True)
                self.assertEqual(common['cases'], [])
                self.assertIn('IFNDR', common['exclusions'][0]['reason'])
                with self.assertRaisesRegex(RuntimeError, 'Stale'):
                    manifest.collect(root, lambda *a, **k: parsed, None,
                                     adjudications={key: dict(adjudication, source_sha256='stale')})


if __name__ == '__main__':
    unittest.main()
