from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from build_gcc_cxx11_ci_manifest import Excluded, project, reviewed_projection


def directive(command, *arguments):
    return dict(command=command, arguments=list(arguments))


class GCCManifest(unittest.TestCase):
    def test_reviewed_extension_requires_diagnostic_without_severity_parity(self):
        review = dict(status='required-diagnostic', sha256='pinned', reason='C++17 syntax in C++11')
        result = reviewed_projection([directive('dg-do', 'compile'),
                                      directive('dg-warning', 'extension warning')],
                                     'int x [[using gnu: aligned(4)]];', 'pinned', review)
        self.assertEqual(result[:3], ('compile', 'diagnostic', []))
        self.assertIn('Source-pinned language review: C++17 syntax in C++11', result[3])

    def test_reviewed_diagnostic_options_are_retained_for_both_compilers(self):
        review = dict(status='required-diagnostic', sha256='pinned',
                      reason='Adopted DR rejects the temporary binding',
                      diagnostic_options=['-Wextra'])
        result = reviewed_projection([directive('dg-do', 'compile')],
                                     'int& r = 1;', 'pinned', review)
        self.assertEqual(result[:3], ('compile', 'diagnostic', ['-Wextra']))
        self.assertIn('Both compilers use reviewed diagnostic qualification options: -Wextra',
                      result[3])

    def test_diagnostic_review_cannot_add_semantic_or_escalating_options(self):
        for options in ['-Wextra', ['-fno-exceptions'], ['-Wextra', '-Werror']]:
            with self.subTest(options=options), self.assertRaises(ValueError):
                reviewed_projection([directive('dg-do', 'compile')], 'int& r = 1;',
                                    'pinned', dict(status='required-diagnostic', sha256='pinned',
                                                   reason='review', diagnostic_options=options))

    def test_stale_review_fails_instead_of_changing_coverage(self):
        with self.assertRaises(ValueError):
            reviewed_projection([], '', 'changed',
                                dict(status='required-diagnostic', sha256='pinned', reason='review'))

    def test_diagnostic_review_does_not_waive_unknown_metadata_or_runtime(self):
        review = dict(status='required-diagnostic', sha256='pinned', reason='review')
        for directives in [[directive('dg-do', 'run')], [directive('dg-additional-sources', 'other.C')]]:
            with self.subTest(directives=directives), self.assertRaises(Excluded):
                reviewed_projection(directives, 'int main() {}', 'pinned', review)

    def test_native_active_negative_does_not_require_diagnostic_parity(self):
        action, expect, flags, _ = project([
            directive('dg-do', 'compile', ' target c++11 '),
            directive('dg-error', 'exact GCC message', '', ' target c++11 ')], 'int x;')
        self.assertEqual((action, expect, flags), ('compile', 'diagnostic', []))

    def test_later_dialect_error_is_inactive(self):
        self.assertEqual(project([directive('dg-do', 'compile'),
                                  directive('dg-error', 'error', '', ' target c++14 ')], 'int x;')[1], 'accept')

    def test_unknown_selector_dependency_and_runtime_checker_are_exclusions(self):
        for directives, source in [
                ([directive('dg-do', 'compile', ' target int32plus ')], 'int x;'),
                ([directive('dg-additional-sources', 'other.C')], 'int x;'),
                ([directive('dg-output', 'expected text')], 'int x;'),
                ([directive('dg-do', 'run')], '#include <vector>'),
                ([directive('dg-do', 'compile')], '#if __GNUC__\nint x;\n#endif'),
                ([directive('dg-do', 'compile')], '%:include <vector>'),
                ([directive('dg-do', 'compile')], '\ufeff#include <vector>'),
                ([directive('dg-do', 'compile')], '#i\\\nnclude <vector>'),
                ([directive('dg-do', 'compile')], '__complex__ float x;'),
                ([directive('dg-excess-errors', '')], 'int x;')]:
            with self.subTest(directives=directives, source=source), self.assertRaises(Excluded):
                project(directives, source)

    def test_message_only_crash_regression_is_not_positive_validity(self):
        with self.assertRaises(Excluded):
            project([directive('dg-do', 'compile'), directive('dg-message', 'required from here')], 'bad code')

    def test_runtime_and_optimization_are_retained(self):
        self.assertEqual(project([directive('dg-do', 'run'), directive('dg-options', '-O2')],
                                 'int main() {}')[:3], ('run', 'accept', ['-O2']))

    def test_semantic_and_escalating_flags_never_become_optional(self):
        for flag in ['-Werror', '-Wno-narrowing', '-fno-exceptions', '-std=c++17']:
            with self.subTest(flag=flag), self.assertRaises(Excluded):
                project([directive('dg-options', flag)], 'int x;')


if __name__ == '__main__':
    unittest.main()
