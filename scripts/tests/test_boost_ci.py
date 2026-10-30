#!/usr/bin/env python3
"""The advisory review must fail closed on incomplete or misleading B2 runs."""
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
import xml.etree.ElementTree as ET

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import boost_ci
import boost_ci_toolchain


class ReviewTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.project = 'libs/type_erasure/test'

    def fixture(self, kind='COMPILE', returncode=0, route='cppgm', name='example.test'):
        build = self.root / 'build/boost/bin.v2' / self.project / name / 'gcc-cppgm/debug'
        build.mkdir(parents=True, exist_ok=True)
        marker = build / name
        marker.write_text('passed\n')
        tree = ET.Element('build')
        ET.SubElement(tree, 'directory').text = str(self.root)
        test = ET.SubElement(tree, 'test', type=kind, name='example')
        ET.SubElement(test, 'target').text = self.project + '//' + name
        ET.SubElement(test, 'source').text = 'test.cpp'
        action = ET.SubElement(tree, 'action', status='0')
        ET.SubElement(action, 'name').text = 'testing%testing.expect-' + ('failure' if kind.endswith('_FAIL') else 'success')
        ET.SubElement(action, 'path').text = str(marker)
        call = dict(cwd=str(self.root), route=route, compiler='/compiler',
                    args=['-c', 'test.cpp', '-o', str(build / 'test.o')], returncode=returncode)
        self.calls = [call]
        self.tree, self.marker = tree, marker
        self.log = self.root / 'b2.log'
        self.log.write_text('')

    def check(self):
        xml = self.root / 'b2.xml'
        ET.ElementTree(self.tree).write(xml)
        calls = self.root / 'invocations.jsonl'
        calls.write_text(''.join(json.dumps(c) + '\n' for c in self.calls))
        return boost_ci.review(xml, self.log, calls, self.project, 1)

    def test_positive_and_actual_negative(self):
        for kind, code in [('COMPILE', 0), ('COMPILE_FAIL', 1)]:
            with self.subTest(kind=kind):
                self.fixture(kind, code)
                self.assertEqual(self.check()['passed'], 1)

    def test_negative_that_compiles_is_failure(self):
        self.fixture('COMPILE_FAIL', 0)
        with self.assertRaisesRegex(ValueError, 'incorrect compilation'):
            self.check()

    def test_compiler_crash_is_not_an_expected_rejection(self):
        for code in [-11, 70, 139]:
            with self.subTest(code=code):
                self.fixture('COMPILE_FAIL', code)
                with self.assertRaisesRegex(ValueError, 'compiler crash'):
                    self.check()

    def test_host_cpp_cannot_validate_cppgm(self):
        self.fixture(route='host-c')
        with self.assertRaisesRegex(ValueError, 'no cppgm compilation'):
            self.check()

    def test_missing_declaration(self):
        self.fixture()
        self.tree.remove(self.tree.find('test'))
        with self.assertRaisesRegex(ValueError, 'expected 1 declarations'):
            self.check()

    def test_missing_or_duplicate_expectation(self):
        self.fixture()
        self.tree.remove(self.tree.find('action'))
        with self.assertRaisesRegex(ValueError, 'missing or duplicate'):
            self.check()
        self.fixture()
        self.tree.append(self.tree.find('action'))
        with self.assertRaisesRegex(ValueError, 'missing or duplicate'):
            self.check()

    def test_b2_zero_exit_with_failed_build_summary(self):
        self.fixture()
        self.log.write_text('...failed updating 1 target...\n')
        with self.assertRaisesRegex(ValueError, 'B2 reported failed'):
            self.check()

    def test_native_requires_exit_status_and_capture(self):
        self.fixture('RUN')
        self.marker.with_suffix('.output').write_text('EXIT STATUS: 1\n')
        with self.assertRaisesRegex(ValueError, 'incorrect native result'):
            self.check()
        self.marker.with_suffix('.output').write_text('EXIT STATUS: 0\n')
        with self.assertRaisesRegex(ValueError, 'no actual execution'):
            self.check()
        action = ET.SubElement(self.tree, 'action', status='0')
        ET.SubElement(action, 'name').text = 'testing%testing.capture-output'
        ET.SubElement(action, 'path').text = str(self.marker.with_suffix('.run'))
        self.assertEqual(self.check()['counts'], {'RUN': 1})

    def test_json_requires_debugger_execution(self):
        self.project = 'libs/json/test'
        self.fixture('RUN', name='_gdb-printers.test')
        self.marker.with_suffix('.output').write_text('EXIT STATUS: 0\n')
        action = ET.SubElement(self.tree, 'action', status='0')
        ET.SubElement(action, 'name').text = 'testing%testing.capture-output'
        ET.SubElement(action, 'path').text = str(self.marker.with_suffix('.run'))
        command = ET.SubElement(action, 'command')
        command.text = './ordinary-native-program'
        with self.assertRaisesRegex(ValueError, 'did not execute GDB'):
            self.check()
        command.text = '/generated/boost-ci-gdb --batch-silent -x _test.py ./program'
        self.assertEqual(self.check()['passed'], 1)

    def test_cpp_response_files_are_expanded(self):
        response = self.root / 'args.rsp'
        response.write_text('-std=c++11 -c "file with spaces.cpp" -o output.o')
        self.assertEqual(boost_ci_toolchain.expand(['@' + str(response)]),
                         ['-std=c++11', '-c', 'file with spaces.cpp', '-o', 'output.o'])

    def test_response_file_reaches_cpp_compiler_as_arguments(self):
        received = self.root / 'received.json'
        compiler = self.root / 'compiler'
        compiler.write_text('#!/usr/bin/python3\nimport json,os,sys\n'
                            'open(os.environ["RECEIVED_ARGS"], "w").write(json.dumps(sys.argv[1:]))\n')
        compiler.chmod(0o755)
        response = self.root / 'args.rsp'
        response.write_text('-std=c++11 -c "file with spaces.cpp" -o output.o')
        log = self.root / 'invocations.jsonl'
        env = dict(os.environ, BOOST_CI_COMPILER=str(compiler), BOOST_CI_HOST_CC=str(compiler),
                   BOOST_CI_HOST_CXX=str(compiler), BOOST_CI_INVOCATIONS=str(log), RECEIVED_ARGS=str(received))
        result = subprocess.run([sys.executable, boost_ci_toolchain.__file__, '@' + str(response)], env=env)
        self.assertEqual(result.returncode, 0)
        self.assertEqual(json.loads(received.read_text()),
                         ['-std=c++11', '-c', 'file with spaces.cpp', '-o', 'output.o'])
        self.assertEqual(json.loads(log.read_text())['route'], 'cppgm')


if __name__ == '__main__':
    unittest.main()
