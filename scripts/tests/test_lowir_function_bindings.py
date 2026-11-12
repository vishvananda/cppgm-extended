#!/usr/bin/env python3
"""Host binding identities survive relaxed LowIR comparison."""
import unittest
import test_compare_lowir_results as comparison


class FunctionBindingTest(unittest.TestCase):
    compare = comparison.LowIrComparisonTest.compare
    PROGRAM = '''target wasm32 1
declare function @receive(%x : i32) -> i32
function @wrapper(%x : i32) -> i32 {
  block ^entry:
    %result = call i32 @receive(%x)
    return i32 %result
}
import @receive "host @receive" "a\\\"b"
export @wrapper "answer"
'''

    def check(self, candidate, expected):
        result = self.compare(self.PROGRAM, candidate)
        self.assertEqual(result.returncode == 0, expected, result.stdout + result.stderr)

    def test_internal_names_can_change_without_renaming_host_strings(self):
        candidate = self.PROGRAM.replace('@receive(', '@other(').replace(
            'import @receive ', 'import @other ')
        self.check(candidate, True)

    def test_binding_order_does_not_matter(self):
        lines = self.PROGRAM.splitlines(True)
        self.check(''.join(lines[:-2] + list(reversed(lines[-2:]))), True)

    def test_changed_module_field_or_export_fails(self):
        for old, new in [('host @receive', 'other'), ('a\\\"b', 'other'),
                         ('"answer"', '"other"')]:
            with self.subTest(old=old):
                self.check(self.PROGRAM.replace(old, new), False)

    def test_missing_or_duplicate_binding_fails(self):
        self.check(self.PROGRAM.replace('export @wrapper "answer"\n', ''), False)
        self.check(self.PROGRAM + 'export @receive "answer"\n', False)
        self.check(self.PROGRAM + 'import @receive "other" "field"\n', False)

    def test_native_defined_and_address_only_imports_fail(self):
        for candidate in [self.PROGRAM.replace('target wasm32 1', 'target linux 1'),
                          self.PROGRAM.replace('import @receive ', 'import @wrapper '),
                          self.PROGRAM.replace('declare function @receive(%x : i32) -> i32',
                                               'declare function @receive')]:
            with self.subTest(candidate=candidate):
                self.check(candidate, False)


if __name__ == '__main__':
    unittest.main()
