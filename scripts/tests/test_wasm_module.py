#!/usr/bin/env python3
"""WASI assertion hosts must preserve the guest's code and public interfaces."""

from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]


def section(kind, payload):
    assert len(payload) < 128
    return bytes([kind, len(payload)]) + payload


class WasmExitHost(unittest.TestCase):
    def redirect(self, imports):
        # One imported function and a defined function calling index zero.
        prefix = b'\0asm\1\0\0\0' + section(1, bytes.fromhex('0260017f00600000'))
        suffix = section(3, b'\1\1')
        suffix += section(7, b'\1\6_start\0\1')
        suffix += section(10, bytes.fromhex('01070041c80110000b'))
        suffix += section(0, b'\4note' + b'wasi_snapshot_preview1/proc_exit')
        with tempfile.TemporaryDirectory() as directory:
            source, output = [Path(directory) / name for name in ('in.wasm', 'out.wasm')]
            source.write_bytes(prefix + section(2, imports) + suffix)
            result = subprocess.run(
                ['perl', '-I', str(ROOT / 'scripts'), '-MCppgmWasmModule', '-e',
                 'CppgmWasmModule::redirect_wasi_exit(@ARGV);', str(source), str(output)],
                capture_output=True, text=True, check=False)
            return result, output.read_bytes() if output.exists() else None, prefix, suffix

    def test_redirects_only_the_exit_namespace_preserving_code_and_indices(self):
        original = b'\x16wasi_snapshot_preview1\x09proc_exit\0\0'
        result, data, prefix, suffix = self.redirect(b'\1' + original)
        self.assertEqual(result.returncode, 0, result.stderr)
        expected = b'\1\x0fcppgm_test_exit\x09proc_exit\0\0'
        self.assertEqual(data, prefix + section(2, expected) + suffix)

    def test_missing_or_ambiguous_exit_import_fails(self):
        imported = b'\x16wasi_snapshot_preview1\x09proc_exit\0\0'
        for imports in (b'\2' + imported * 2,
                        b'\1\x03env\x09proc_exit\0\0'):
            with self.subTest(imports=imports):
                result, output, _, _ = self.redirect(imports)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn('Expected exactly one WASI proc_exit import', result.stderr)
                self.assertIsNone(output)


if __name__ == '__main__':
    unittest.main()
