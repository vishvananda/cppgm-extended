import json
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import internal_test_badges as badges


class InternalTestBadges(unittest.TestCase):
    def result(self, **changes):
        result = dict(passed=10603, total=10603, revision='a' * 40,
                      run_number=10, run_attempt=1,
                      run_url='https://github.com/example/project/actions/runs/100')
        return dict(result, **changes)

    def test_record_requires_a_complete_success_and_keeps_source(self):
        env = dict(GITHUB_SHA='a' * 40, GITHUB_RUN_NUMBER='10', GITHUB_RUN_ATTEMPT='1',
                   GITHUB_SERVER_URL='https://github.com', GITHUB_REPOSITORY='example/project',
                   GITHUB_RUN_ID='100')
        log = '===== ALL TESTS PASSED SUCCESSFULLY! (10603 / 10603) =====\n'
        self.assertEqual(badges.record(log, env), self.result())
        for changed in ['===== TEST SUMMARY: 10602 / 10603 TESTS PASSED =====',
                        log.replace('10603 /', '10602 /')]:
            with self.assertRaises(ValueError):
                badges.record(changed, env)

    def test_publication_uses_report_count_and_refuses_an_old_run(self):
        with tempfile.TemporaryDirectory() as tmp:
            self.assertTrue(badges.publish(self.result(), tmp, 'a' * 40))
            folder = Path(tmp) / 'internal-tests'
            self.assertIn('10,603 / 10,603', (folder / 'tests.svg').read_text())
            self.assertIn('/actions/runs/100', (folder / 'report.md').read_text())
            self.assertFalse(badges.publish(self.result(run_number=9), tmp, 'a' * 40))
            self.assertTrue(badges.publish(self.result(run_attempt=2), tmp, 'a' * 40))
            self.assertEqual(json.loads((folder / 'results.json').read_text())['run_attempt'], 2)
            with self.assertRaises(ValueError):
                badges.publish(self.result(), tmp, 'b' * 40)


if __name__ == '__main__':
    unittest.main()
