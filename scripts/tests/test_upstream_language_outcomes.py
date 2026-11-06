import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from upstream_language_outcomes import classify


SOURCE = '/frozen/test.C'


def receipt(negative=False, accepted=True, diagnostic='', native='native-failed', flags=()):
    return dict(status=native, assertions=[dict(status='PASS', text='test for errors, line 1')]
                if negative else [], calls=[dict(role='test', cwd='/frozen',
                incoming=[SOURCE, '-c', *flags], classification='success' if accepted else 'rejection',
                stderr=diagnostic, stdout='', steps=[dict(command=['compiler', '-c', SOURCE],
                classification='success' if accepted else 'rejection', stderr=diagnostic)])])


class LanguageOutcomes(unittest.TestCase):
    def test_equivalent_diagnostic_and_first_error_only_pass(self):
        host = receipt(True, False, 'GCC exact phrase')
        host['assertions'] *= 4
        candidate = receipt(False, False, 'one differently worded diagnostic')
        candidate['assertions'] = [dict(status='FAIL', text='test for errors'),
                                   dict(status='FAIL', text='test for excess errors')]
        self.assertEqual(classify(host, candidate, SOURCE)['status'], 'outcome-match')

    def test_missing_negative_diagnostic_fails(self):
        self.assertEqual(classify(receipt(True, False, 'error'), receipt(), SOURCE)['status'], 'outcome-mismatch')

    def test_silent_negative_rejection_fails(self):
        self.assertEqual(classify(receipt(True, False, 'error'), receipt(accepted=False), SOURCE)['status'], 'outcome-mismatch')

    def test_diagnosed_extension_can_be_accepted(self):
        self.assertEqual(classify(receipt(True, False, 'error'), receipt(diagnostic='warning'), SOURCE)['status'], 'outcome-match')

    def test_positive_rejection_fails(self):
        self.assertEqual(classify(receipt(), receipt(accepted=False, diagnostic='error'), SOURCE)['status'], 'outcome-mismatch')

    def test_optional_warning_parity_is_not_required(self):
        host = receipt(diagnostic='optional warning')
        host['assertions'] = [dict(status='PASS', text='test for warnings')]
        candidate = receipt()
        candidate['assertions'] = [dict(status='FAIL', text='test for warnings')]
        self.assertEqual(classify(host, candidate, SOURCE)['status'], 'outcome-match')

    def test_abnormal_compiler_does_not_satisfy_negative(self):
        candidate = receipt(accepted=False, diagnostic='internal error')
        candidate['calls'][0]['classification'] = 'abnormal'
        self.assertEqual(classify(receipt(True, False, 'error'), candidate, SOURCE)['status'], 'compiler-abnormal')

    def test_probe_is_not_primary_translation(self):
        candidate = receipt()
        candidate['calls'][0]['incoming'] = ['/frozen/probe.C']
        self.assertEqual(classify(receipt(), candidate, SOURCE)['status'], 'harness-blocked')

    def test_warning_escalation_requires_language_projection(self):
        host = receipt(True, False, 'warning treated as error', flags=['-Werror'])
        self.assertEqual(classify(host, receipt(), SOURCE)['status'], 'diagnostic-policy-review')

    def test_ifndr_disposition_is_not_a_failure_or_pass(self):
        disposition = dict(status='not-required', reason='No diagnostic required')
        self.assertEqual(classify(receipt(), receipt(), SOURCE, disposition)['status'], 'not-required')

    def test_runtime_and_output_are_still_required(self):
        host, candidate = receipt(), receipt()
        for data in [host, candidate]:
            data['calls'].append(dict(role='runtime', classification='success', exitcode=0))
            data['assertions'] = [dict(status='PASS', text='execution test'),
                                  dict(status='PASS', text='output pattern')]
        self.assertEqual(classify(host, candidate, SOURCE)['status'], 'outcome-match')
        candidate['assertions'][1]['status'] = 'FAIL'
        self.assertEqual(classify(host, candidate, SOURCE)['status'], 'runtime-mismatch')

    def test_missing_runtime_is_not_compile_only_success(self):
        host, candidate = receipt(), receipt()
        host['calls'].append(dict(role='runtime', classification='success', exitcode=0))
        host['assertions'] = [dict(status='PASS', text='execution test')]
        self.assertEqual(classify(host, candidate, SOURCE)['status'], 'harness-blocked')

    def test_final_interface_blocker_stays_visible(self):
        candidate = receipt(native='harness-blocked')
        self.assertEqual(classify(receipt(), candidate, SOURCE)['status'], 'harness-blocked')

    def test_linker_error_is_not_a_translation_diagnostic(self):
        candidate = receipt(accepted=False)
        candidate['calls'][0]['stderr'] = 'undefined reference'
        candidate['calls'][0]['steps'].append(dict(command=['host', 'test.o'],
            classification='rejection', stderr='undefined reference'))
        self.assertEqual(classify(receipt(True, False, 'error'), candidate, SOURCE)['status'], 'outcome-mismatch')

    def test_negative_combined_link_needs_translation_oracle(self):
        host = receipt(True, False, 'undefined reference')
        host['calls'][0]['incoming'].remove('-c')
        self.assertEqual(classify(host, receipt(), SOURCE)['status'], 'baseline-unresolved')

    def test_missing_compiler_step_is_not_positive_success(self):
        candidate = receipt()
        candidate['calls'][0]['steps'] = []
        self.assertEqual(classify(receipt(), candidate, SOURCE)['status'], 'harness-blocked')

    def test_unknown_option_blocker_is_not_a_compiler_crash(self):
        candidate = receipt(native='harness-blocked')
        candidate['calls'][0]['classification'] = 'blocked'
        candidate['calls'][0]['steps'] = []
        self.assertEqual(classify(receipt(), candidate, SOURCE)['status'], 'harness-blocked')

    def test_preprocessor_output_is_not_compile_only_success(self):
        host = receipt()
        host['calls'][0]['incoming'].append('-E')
        self.assertEqual(classify(host, receipt(), SOURCE)['status'], 'baseline-unresolved')

    def test_unprojected_codegen_check_is_not_discarded(self):
        host = receipt()
        host['assertions'] = [dict(status='PASS', text='scan-assembler required instruction')]
        self.assertEqual(classify(host, receipt(), SOURCE)['status'], 'baseline-unresolved')

    def test_one_runtime_check_cannot_stand_in_for_another(self):
        host, candidate = receipt(), receipt()
        for data in [host, candidate]:
            data['calls'].append(dict(role='runtime', classification='success'))
            data['assertions'] = [dict(status='PASS', text='execution test'),
                                  dict(status='PASS', text='output pattern')]
        candidate['assertions'][1]['text'] = 'execution test'
        self.assertEqual(classify(host, candidate, SOURCE)['status'], 'harness-blocked')
        candidate['assertions'][1]['text'] = 'output pattern'
        host['calls'].append(dict(role='runtime', classification='success'))
        self.assertEqual(classify(host, candidate, SOURCE)['status'], 'harness-blocked')


if __name__ == '__main__':
    unittest.main()
