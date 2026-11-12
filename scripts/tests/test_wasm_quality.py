#!/usr/bin/env python3
"""Maintainer checks for the shipped Perl Wasm measurement helper."""

import json
from pathlib import Path
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[2]


def perl(code, *args):
    return subprocess.run(["perl", "-I", str(ROOT / "scripts"),
                           "-MCppgmWasmQuality", "-MJSON::PP", "-e", code, *args],
                          capture_output=True, text=True, check=False)


def leb(value):
    result = bytearray()
    while True:
        part = value & 127
        value >>= 7
        result.append(part | (128 if value else 0))
        if not value:
            return bytes(result)


def section(kind, data):
    return bytes([kind]) + leb(len(data)) + data


class WasmQuality(unittest.TestCase):
    def metrics(self, body):
        result = perl("print encode_json(CppgmWasmQuality::function_metrics(pack('H*', $ARGV[0])));",
                      body.hex())
        self.assertEqual(result.returncode, 0, result.stderr)
        return json.loads(result.stdout)

    def test_local_declarations_and_instructions_are_separate(self):
        self.assertEqual(self.metrics(bytes.fromhex("01027f200041016a0b")),
                         dict(code_bytes=9, locals=2, instructions=4,
                              memory_ops=0, control_depth=0))

    def test_simd_immediates_are_not_counted_as_instructions(self):
        body = b"\0\xfd\x0c" + bytes(range(16)) + b"\xfd\x1b\0\x0b"
        metrics = self.metrics(body)
        self.assertEqual(metrics["instructions"], 3)
        self.assertEqual(metrics["memory_ops"], 0)

    def test_memory_lane_and_bulk_immediates(self):
        metrics = self.metrics(bytes.fromhex("00fd54000003fc0a00000b"))
        self.assertEqual(metrics["instructions"], 3)
        self.assertEqual(metrics["memory_ops"], 2)

    def test_nested_control_and_branch_table(self):
        metrics = self.metrics(bytes.fromhex("00024003400e0100010b0b0b"))
        self.assertEqual(metrics["instructions"], 6)
        self.assertEqual(metrics["control_depth"], 2)

    def check_module(self, name, bounds, level=2, module_bounds=None):
        # The export is an alias of function 1, after one imported function.
        data = b"\0asm\1\0\0\0" + section(1, bytes.fromhex("0160017f017f"))
        data += section(2, b"\1\3env\6callee\0\0")
        data += section(3, b"\1\0")
        data += section(7, b"\2\6answer\0\1\x08imported\0\0")
        data += section(10, b"\1\4\0\x20\0\x0b")
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / "module.wasm"
            path.write_bytes(data)
            quality = {"functions": {name: bounds} if name else {}}
            if module_bounds is not None:
                quality["module"] = module_bounds
                quality["levels"] = [0, 1, 2, 3]
            quality = json.dumps(quality)
            return perl("CppgmWasmQuality::check_quality($ARGV[0], decode_json($ARGV[1]), $ARGV[2]);",
                        str(path), quality, str(level))

    def test_export_alias_after_import_uses_defined_body(self):
        result = self.check_module("answer", {"locals": 0, "code_bytes": 4,
                                              "instructions": 2, "memory_ops": 0})
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_exceeded_and_unknown_limits_fail(self):
        for bounds, message in [({"instructions": 1}, "exceeds 1"),
                                ({"instruction": 5}, "Unknown Wasm quality metric")]:
            with self.subTest(bounds=bounds):
                result = self.check_module("answer", bounds)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn(message, result.stderr)

    def test_missing_and_imported_exports_fail(self):
        for name, message in [("missing", "Missing quality export"),
                              ("imported", "has no body")]:
            with self.subTest(name=name):
                result = self.check_module(name, {"instructions": 8})
                self.assertNotEqual(result.returncode, 0)
                self.assertIn(message, result.stderr)

    def test_o0_has_correctness_without_optimizing_envelope(self):
        result = self.check_module("answer", {"instructions": 0}, level=0)
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_module_envelopes_check_encoded_sections_without_function_limits(self):
        result = self.check_module(None, {}, level=0,
                                   module_bounds={"module": 1000, "code": 6, "data": 0})
        self.assertEqual(result.returncode, 0, result.stderr)
        result = self.check_module(None, {}, module_bounds={"code": 5})
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("code 6 exceeds 5", result.stderr)

    def test_module_envelopes_reject_unknown_and_invalid_bounds(self):
        for bounds in ({"unknown": 1}, {"module": -1}):
            with self.subTest(bounds=bounds):
                result = self.check_module(None, {}, module_bounds=bounds)
                self.assertNotEqual(result.returncode, 0)


if __name__ == "__main__":
    unittest.main()
