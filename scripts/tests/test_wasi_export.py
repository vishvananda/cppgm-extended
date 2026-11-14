import hashlib
import json
from pathlib import Path
import sys
import tempfile
import unittest

SCRIPTS = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(SCRIPTS))
from experimental.wasi_inception import source_sets
from unpack_wasi_inputs import unpack


def digest(data):
    return hashlib.sha256(data).hexdigest()


class WasiExport(unittest.TestCase):
    def test_all_lowir_tools_include_host_independent_floating_conversion(self):
        for name, sources in source_sets(['lowir', 'lowiropt', 'lowir2native', 'cppgm++']).items():
            with self.subTest(tool=name):
                self.assertIn('dev/src/lex/interchange_float.cpp', sources)
                self.assertIn('dev/src/preprocess/lexer/char_classes.cpp', sources)
                self.assertEqual(len(sources), len(set(sources)))
                self.assertEqual(sources[-1], 'dev/' + name + '.cpp')

    def test_unpack_checks_every_input_before_creating_a_fresh_root(self):
        with tempfile.TemporaryDirectory() as directory:
            package = Path(directory)
            data = b'firstsecond'
            rows = [dict(path='source/a.h', offset=0, size=5, sha256=digest(b'first')),
                    dict(path='sdk/b.h', offset=5, size=6, sha256=digest(b'second'))]
            def manifest(entries):
                raw = json.dumps(dict(schema=1, files=entries, bundle_sha256=digest(data))).encode()
                (package / 'manifest.json').write_bytes(raw)
                (package / 'manifest.sha256').write_text(digest(raw))
            (package / 'bundle.bin').write_bytes(data)
            manifest(rows)
            output = package / 'inputs'
            self.assertEqual(unpack(package, output), 2)
            self.assertEqual((output / 'sdk/b.h').read_bytes(), b'second')
            with self.assertRaises(FileExistsError):
                unpack(package, output)
            for invalid in [dict(rows[1], path='../outside'), dict(rows[1], path='.'),
                            dict(rows[1], path='/absolute'), dict(rows[1], path=rows[0]['path']),
                            dict(rows[1], sha256='0'*64), dict(rows[1], offset=-1),
                            dict(rows[1], size=999999)]:
                manifest([rows[0], invalid])
                with self.assertRaises(ValueError):
                    unpack(package, package / 'bad')
                self.assertFalse((package / 'bad').exists())
            manifest(rows)
            (package / 'bundle.bin').write_bytes(b'changed')
            with self.assertRaisesRegex(ValueError, 'bundle checksum'):
                unpack(package, package / 'corrupt')


if __name__ == '__main__':
    unittest.main()
