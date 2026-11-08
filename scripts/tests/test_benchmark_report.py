import copy
import json
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import benchmark_report as report


class Results(unittest.TestCase):
    def setUp(self):
        self.catalog = json.loads((Path(__file__).resolve().parents[2]/'benchmarks/catalog.json').read_text())
        self.row = dict(workload='simdjson', configuration='scalar', test='twitter.json', scope='summary',
                        host=dict(id='epyc-7763-hosted', cpu='AMD EPYC 7763', architecture='x86_64', environment='GitHub'),
                        optimization='O3', baseline='gcc', revision='a'*40,
                        context=dict(language='c++17', target_isa='baseline', standard_library='libstdc++15',
                                     workers=1, affinity=[0], flags=dict(cppgm=['-O3'], gcc=['-O3'])),
                        phase='runtime', metric='cpu_seconds', method='median of paired block ratios',
                        status='passed', speedup=.9, candidate_seconds=1., reference_seconds=.91,
                        coverage=dict(selected=2, candidate_executed=2, candidate_passed=2,
                                      reference_executed=2, reference_passed=2, excluded=0,
                                      unit='inputs', case_set_sha256='b'*64),
                        details_url='https://example.org/details', raw_url='https://example.org/raw',
                        run_url='https://example.org/run', provenance=dict(warnings=[]))

    def document(self, rows=None):
        return dict(schema=1, records=rows if rows is not None else [self.row])

    def test_godot_configuration_needs_no_renderer_changes(self):
        godot = next(x for x in self.catalog['workloads'] if x['id']=='godot')
        godot['status']='active'
        godot['configurations']=[dict(id='test-preset', name='Test preset', parameters=dict(renderer='test'))]
        self.row.update(workload='godot', configuration='test-preset', phase='build', test='editor')
        with tempfile.TemporaryDirectory() as directory:
            target=Path(directory)/'report'
            report.render(self.catalog, self.document(), target)
            self.assertTrue((target/'godot.md').is_file())
            self.assertIn('Godot build and benchmark', (target/'summary.md').read_text())

    def test_axes_are_kept_separate(self):
        rows=[self.row]
        for key, value in [('configuration','sse'),('baseline','clang'),('optimization','O0'),('phase','compile')]:
            row=copy.deepcopy(self.row);row[key]=value;rows.append(row)
        row=copy.deepcopy(self.row);row['host'].update(id='intel-dedicated',cpu='Intel Xeon',environment='dedicated');rows.append(row)
        with tempfile.TemporaryDirectory() as directory:
            target=Path(directory)/'report';report.render(self.catalog,self.document(rows),target)
            text=(target/'summary.md').read_text()
            for value in ['scalar','sse','clang','O0','compile','Intel Xeon','AMD EPYC 7763']:
                self.assertIn(value.lower(),text.lower())
            self.assertEqual(len(json.loads((target/'records.json').read_text())['records']),6)

    def test_failed_partial_or_incomplete_work_cannot_publish_speedup(self):
        for status in ['failed','partial','not-measured','not-applicable']:
            row=copy.deepcopy(self.row);row['status']=status
            with self.subTest(status=status),self.assertRaisesRegex(ValueError,'Ineligible speedup'):
                report.validate(self.catalog,self.document([row]))
        self.row['coverage']['reference_passed']=1
        with self.assertRaisesRegex(ValueError,'complete matching'):
            report.validate(self.catalog,self.document())

    def test_missing_results_remain_visible_and_never_become_zero(self):
        with tempfile.TemporaryDirectory() as directory:
            target=Path(directory)/'report';report.render(self.catalog,self.document([]),target)
            text=(target/'summary.md').read_text();self.assertIn('planned',text);self.assertIn('not measured',text)
            self.assertNotIn('0.000×',text)

    def test_host_id_cannot_merge_different_epyc_models(self):
        row=copy.deepcopy(self.row);row['host']['cpu']='AMD EPYC 9V74';row['baseline']='clang'
        with self.assertRaisesRegex(ValueError,'Host ID merges'):
            report.validate(self.catalog,self.document([self.row,row]))

    def test_paired_ratio_is_not_replaced_by_quotient_of_absolute_medians(self):
        report.validate(self.catalog,self.document())
        self.assertNotEqual(self.row['speedup'],self.row['reference_seconds']/self.row['candidate_seconds'])

    def test_child_cases_do_not_expand_the_readme_summary(self):
        child=copy.deepcopy(self.row);child.update(test='another-input',scope='test')
        with tempfile.TemporaryDirectory() as directory:
            target=Path(directory)/'report';report.render(self.catalog,self.document([self.row,child]),target)
            self.assertNotIn('another-input',(target/'summary.md').read_text())
            self.assertIn('another-input',(target/'simdjson.md').read_text())

    def test_missing_clang_is_labeled_without_inventing_measurement(self):
        with tempfile.TemporaryDirectory() as directory:
            target=Path(directory)/'report';report.render(self.catalog,self.document(),target)
            text=(target/'summary.md').read_text()
            self.assertIn('vs Clang',text);self.assertIn('not-measured',text)
            self.assertEqual(len(json.loads((target/'records.json').read_text())['records']),1)

    def test_malformed_measurements_and_duplicate_identity_fail(self):
        for value in [0,-1,float('nan'),float('inf'),True]:
            row=copy.deepcopy(self.row);row['speedup']=value
            with self.subTest(value=value),self.assertRaises(ValueError):
                report.validate(self.catalog,self.document([row]))
        with self.assertRaisesRegex(ValueError,'Duplicate comparison'):
            report.validate(self.catalog,self.document([self.row,self.row]))
        self.row['coverage']['reference_executed']=1
        with self.assertRaisesRegex(ValueError,'Impossible coverage'):
            report.validate(self.catalog,self.document())

    def test_candidate_flags_cannot_be_silently_merged_between_baselines(self):
        clang=copy.deepcopy(self.row);clang['baseline']='clang'
        clang['context']['flags']={'cppgm':['-O3'], 'clang':['-O3','-fno-vectorize']}
        self.assertEqual(len(report.missing_baselines([self.row,clang])),2)
        clang['context']['flags']['cppgm'].append('-fno-inline')
        self.assertEqual(len(report.missing_baselines([self.row,clang])),4)
        with tempfile.TemporaryDirectory() as directory:
            target=Path(directory)/'report';report.render(self.catalog,self.document([self.row,clang]),target)
            rows=[line for line in (target/'summary.md').read_text().splitlines()
                  if line.startswith('| [simdjson/scalar/twitter.json]')]
            self.assertEqual(len(rows),2)
            self.assertTrue(all('not-measured' in line for line in rows))


if __name__=='__main__':
    unittest.main()
