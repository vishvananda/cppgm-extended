#!/usr/bin/env python3
"""Protect language projections from vendor diagnostics and false coverage."""
from pathlib import Path
import hashlib
import struct
import sys
import tempfile
from types import SimpleNamespace
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from upstream_clang_language import ProjectionBlocked, assess, elf_object, project_arguments
from run_upstream_clang_language import compiler_arguments, feature_value, freeze_file, metadata_disposition


SOURCE = '/pinned/source.cpp'


def project(*arguments):
    return project_arguments(['%clang_cc1', *arguments, SOURCE], SOURCE,
                             'c++11', 'x86_64-pc-linux-gnu', '/resource/include')


class Projections(unittest.TestCase):
    def test_positive_syntax_becomes_object_and_verifier_is_metadata(self):
        result = project('-std=c++11', '-fsyntax-only', '-verify=cxx11')
        self.assertEqual(result['verification'], ['-verify=cxx11'])
        self.assertNotIn('-verify=cxx11', result['candidate_args'])
        self.assertNotIn('-fsyntax-only', result['candidate_args'])
        self.assertIn('-fno-exceptions', result['candidate_args'])
        self.assertIn('object', result['adaptations'][0])

    def test_optional_diagnostic_flags_do_not_change_candidate_oracle(self):
        result = project('-std=c++11', '-Wunused', '-pedantic', '-w')
        for value in ['-Wunused', '-pedantic', '-w']:
            self.assertIn(value, result['native_args'])
            self.assertNotIn(value, result['candidate_args'])

    def test_warning_escalation_and_narrowing_suppression_need_review(self):
        for value in ['-Werror', '-Werror=c++11-compat', '-pedantic-errors', '-Wno-narrowing']:
            with self.subTest(value=value), self.assertRaisesRegex(ProjectionBlocked, 'policy'):
                project(value)

    def test_unproven_disable_flags_are_not_language_rejections(self):
        for value in ['-fno-exceptions', '-fno-rtti']:
            with self.subTest(value=value), self.assertRaisesRegex(ProjectionBlocked, 'capability'):
                project(value)

    def test_unknown_semantic_options_fail_closed(self):
        for value in ['-fms-extensions', '-fdelayed-template-parsing', '-mms-bitfields']:
            with self.subTest(value=value), self.assertRaisesRegex(ProjectionBlocked, 'Unreviewed'):
                project(value)

    def test_language_target_and_input_must_be_explicitly_supported(self):
        for args in [['-std=c++17'], ['-triple', 'i386-pc-linux'], ['other.cpp'], ['-include']]:
            with self.subTest(args=args), self.assertRaises(ProjectionBlocked):
                project(*args)
        with self.assertRaisesRegex(ProjectionBlocked, 'Compiler position'):
            project_arguments(['%clang', SOURCE], SOURCE, 'c++11', 'x86_64-pc-linux-gnu', '/resource')

    def test_no_standard_is_an_explicit_cxx11_projection(self):
        result = project('-verify')
        self.assertIn('-std=c++11', result['native_args'])
        self.assertTrue(any('cc1 default' in text for text in result['adaptations']))

    def test_llvm_and_ast_interfaces_remain_visible(self):
        result = project('-emit-llvm', '-ast-dump', '-o', '-')
        self.assertEqual(result['unprojected_interfaces'], ['-emit-llvm', '-ast-dump'])
        self.assertNotIn('-emit-llvm', result['candidate_args'])
        self.assertIn('no full upstream', result['coverage'])

    def test_preprocessing_and_pch_are_not_syntax_only_passes(self):
        for flag in ['-E', '-emit-pch', '-include-pch']:
            with self.subTest(flag=flag), self.assertRaisesRegex(ProjectionBlocked, 'own reviewed'):
                project(flag)

    def test_native_exception_enable_is_projected_as_supported_candidate_mode(self):
        result = project('-fcxx-exceptions', '-fexceptions')
        self.assertTrue(result['native_cxx_exceptions'])
        self.assertIn('-fexceptions', result['candidate_args'])
        self.assertNotIn('-fcxx-exceptions', result['candidate_args'])

    def test_unresolved_substitution_cannot_disappear_in_an_unused_option(self):
        with self.assertRaisesRegex(ProjectionBlocked, 'Unresolved native substitution'):
            project('-I%unknown_include')


class FeatureStates(unittest.TestCase):
    class Boolean:
        @staticmethod
        def tokenize(expression):
            yield expression

        @staticmethod
        def isMatchExpression(token):
            return True

        @staticmethod
        def evaluate(expression, variables):
            return expression in variables

    def test_unknown_feature_is_a_blocker_and_known_false_is_explicit(self):
        with self.assertRaisesRegex(ProjectionBlocked, 'Unresolved native feature'):
            feature_value('unprobed-capability', {'asserts': False}, self.Boolean)
        self.assertFalse(feature_value('asserts', {'asserts': False}, self.Boolean))
        self.assertEqual(metadata_disposition({'REQUIRES:': ['asserts']},
                         {'asserts': False}, self.Boolean)[0], 'configured-unsupported')

    def test_active_xfail_requires_language_review_instead_of_becoming_a_pass(self):
        result = metadata_disposition({'XFAIL:': ['asserts']}, {'asserts': True}, self.Boolean)
        self.assertEqual(result[0], 'expected-failure-review')


class ShellBoundaries(unittest.TestCase):
    class Command:
        def __init__(self, args, redirects=()):
            self.args, self.redirects = args, redirects

    class Pipeline:
        def __init__(self, commands, negate=False):
            self.commands, self.negate = commands, negate

    def parse(self, tree):
        native = SimpleNamespace(
            TestRunner=SimpleNamespace(kPdbgRegex=r'^%dbg\((.*?)\) (.*)$'),
            ShCommands=SimpleNamespace(Pipeline=self.Pipeline, Command=self.Command),
            ShUtil=SimpleNamespace(ShParser=lambda *args, **kw: SimpleNamespace(parse=lambda: tree)))
        return compiler_arguments('original native command', native)

    def test_filecheck_pipeline_retains_output_obligation(self):
        args, interfaces = self.parse(self.Pipeline([
            self.Command(['%clang_cc1', '-emit-llvm', SOURCE]),
            self.Command(['FileCheck', SOURCE])]))
        self.assertEqual(args[0], '%clang_cc1')
        self.assertIn('native FileCheck output obligation', interfaces)

    def test_runtime_or_setup_pipeline_cannot_be_discarded(self):
        for command in [['runtime.exe'], ['split-file', SOURCE], ['rm', '-rf', '/tmp/unused']]:
            with self.subTest(command=command), self.assertRaises(ProjectionBlocked):
                self.parse(self.Pipeline([self.Command(['%clang_cc1', SOURCE]), self.Command(command)]))

    def test_sequence_or_negated_pipeline_is_not_a_positive_compile(self):
        for tree in [object(), self.Pipeline([self.Command(['%clang_cc1', SOURCE])], negate=True)]:
            with self.subTest(tree=tree), self.assertRaises(ProjectionBlocked):
                self.parse(tree)

    def test_output_redirection_remains_a_named_obligation(self):
        _, interfaces = self.parse(self.Pipeline([self.Command(['%clang_cc1', SOURCE], [object()])]))
        self.assertIn('native shell redirection/output obligation', interfaces)

    def test_glob_does_not_masquerade_as_the_declared_source(self):
        with self.assertRaisesRegex(ProjectionBlocked, 'glob'):
            self.parse(self.Pipeline([self.Command(['%clang_cc1', object()])]))


class InputSnapshots(unittest.TestCase):
    def test_snapshots_preserve_old_input_and_record_live_drift(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / 'source'
            source.write_text('original\n')
            inputs, origins, guard = {}, {}, {}
            saved = freeze_file(source, root / 'frozen/source', inputs, origins, guard)
            source.write_text('changed\n')
            self.assertEqual(saved.read_text(), 'original\n')
            self.assertEqual(origins[str(saved)], str(source.resolve()))
            self.assertEqual(inputs[str(saved)], guard[str(source.resolve())])
            self.assertNotEqual(hashlib.sha256(source.read_bytes()).hexdigest(), guard[str(source.resolve())])

    def test_existing_snapshot_cannot_be_overwritten(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / 'source'
            source.write_text('original\n')
            freeze_file(source, root / 'saved', {}, {}, {})
            with self.assertRaises(FileExistsError):
                freeze_file(source, root / 'saved', {}, {}, {})


class Outcomes(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        self.projection = project('-verify')
        self.discovery = dict(status='expectations-discovered', expected_error=False,
                              same_preprocessed_tokens=True, native_cxx_exceptions=False,
                              native_rtti=True, language_policy_tokens=[], active_pragma=False)

    def receipt(self, label, classification='success', diagnostic=''):
        path = self.root / (label + '.stderr')
        path.write_text(diagnostic)
        return dict(classification=classification, stderr_path=str(path))

    def check(self, negative=False, candidate='success', diagnostic='', adjudicated=True):
        discovery = dict(self.discovery, expected_error=negative)
        host = self.receipt('host', 'rejection' if negative else 'success', 'native error' if negative else '')
        candidate = self.receipt('candidate', candidate, diagnostic)
        adjudication = dict(expected='diagnose-invalid' if negative else 'accept-positive',
                            reason='Reviewed language rule') if adjudicated else None
        return assess(discovery, host, candidate, self.projection, adjudication)

    def test_one_differently_worded_diagnostic_satisfies_reviewed_negative(self):
        result = self.check(True, 'rejection', 'one entirely different diagnostic')
        self.assertEqual(result['status'], 'language-obligation-match')
        self.assertFalse(result['diagnostic_parity_required'])

    def test_diagnosed_extension_can_be_accepted(self):
        self.assertEqual(self.check(True, diagnostic='warning')['status'], 'language-obligation-match')

    def test_no_diagnostic_and_silent_rejection_fail_reviewed_negative(self):
        for status in ['success', 'rejection']:
            self.assertEqual(self.check(True, status)['status'], 'language-obligation-mismatch')

    def test_native_negative_without_language_adjudication_cannot_pass(self):
        result = self.check(True, 'rejection', 'error', adjudicated=False)
        self.assertEqual(result['status'], 'negative-needs-adjudication')
        self.assertTrue(result['candidate_diagnosed'])

    def test_crash_or_timeout_never_satisfies_negative(self):
        for status in ['abnormal', 'timeout']:
            self.assertEqual(self.check(True, status, 'error')['status'], 'compiler-abnormal')

    def test_positive_rejection_remains_a_language_difference(self):
        self.assertEqual(self.check(candidate='rejection', diagnostic='error')['status'], 'language-obligation-mismatch')

    def test_positive_object_result_does_not_claim_codegen_or_runtime_coverage(self):
        self.projection['unprojected_interfaces'] = ['-emit-llvm', 'runtime']
        result = self.check()
        self.assertEqual(result['status'], 'language-obligation-match')
        self.assertEqual(result['unprojected_interfaces'], ['-emit-llvm', 'runtime'])
        self.assertFalse(result['full_upstream_pass'])

    def test_compiler_specific_branch_difference_blocks_a_score(self):
        self.discovery['same_preprocessed_tokens'] = False
        self.assertEqual(self.check()['status'], 'preprocessing-context-review')

    def test_disabled_exception_diagnostic_is_not_a_language_negative(self):
        self.discovery['language_policy_tokens'] = ['throw']
        self.assertEqual(self.check(True, 'success')['status'], 'configuration-blocked')

    def test_disabled_rtti_and_active_pragmas_need_policy_equivalence(self):
        self.discovery.update(native_rtti=False, language_policy_tokens=['dynamic_cast'])
        self.assertEqual(self.check()['status'], 'configuration-blocked')
        self.discovery.update(native_rtti=True, language_policy_tokens=[], active_pragma=True)
        self.assertEqual(self.check()['status'], 'configuration-blocked')

    def test_ifndr_is_neither_required_rejection_nor_positive_pass(self):
        host, candidate = self.receipt('host'), self.receipt('candidate')
        result = assess(self.discovery, host, candidate, self.projection,
                        dict(expected='not-required', reason='Reviewed IFNDR clause'))
        self.assertEqual(result['status'], 'not-required')

    def test_missing_or_abnormal_host_baseline_is_separate(self):
        result = assess(self.discovery, self.receipt('host', 'timeout'),
                        self.receipt('candidate'), self.projection)
        self.assertEqual(result['status'], 'baseline-unresolved')

    def test_executable_and_absent_elf_are_not_object_generation(self):
        path = self.root / 'object.o'
        header = bytearray(128)
        header[:7] = b'\x7fELF\x02\x01\x01'
        header[16:18] = (1).to_bytes(2, 'little')
        header[18:20] = (62).to_bytes(2, 'little')
        struct.pack_into('<I', header, 20, 1)
        struct.pack_into('<Q', header, 40, 64)
        struct.pack_into('<HHHHHH', header, 52, 64, 0, 0, 64, 1, 0)
        path.write_bytes(header)
        self.assertTrue(elf_object(path))
        header[16:18] = (2).to_bytes(2, 'little')
        path.write_bytes(header)
        self.assertFalse(elf_object(path))
        path.write_bytes(b'')
        self.assertFalse(elf_object(path))

    def test_magic_prefix_with_truncated_sections_is_not_an_object(self):
        path = self.root / 'truncated.o'
        header = bytearray(64)
        header[:7] = b'\x7fELF\x02\x01\x01'
        struct.pack_into('<HHI', header, 16, 1, 62, 1)
        struct.pack_into('<Q', header, 40, 64)
        struct.pack_into('<HHHHHH', header, 52, 64, 0, 0, 64, 1, 0)
        path.write_bytes(header)
        self.assertFalse(elf_object(path))


if __name__ == '__main__':
    unittest.main()
