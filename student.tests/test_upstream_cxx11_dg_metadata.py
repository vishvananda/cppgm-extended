#!/usr/bin/env python3
"""Explicit-run checks for the native metadata bridge; requires DejaGnu files."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


class NativeMetadataTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix="cppgm-dg-metadata-test-")
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.suite = self.root / "sources/gcc/gcc/testsuite/g++.dg"
        self.suite.mkdir(parents=True)

    def invoke(self):
        rows = [dict(repository="gcc", role="candidate", path=str(p.relative_to(self.root / "sources/gcc")),
                     sha256=hashlib.sha256(p.read_bytes()).hexdigest(), git_blob="unit-fixture",
                     revision="unit-fixture", directives=[]) for p in self.suite.iterdir()]
        inventory = self.root / "inventory.jsonl"
        inventory.write_text("".join(json.dumps(row) + "\n" for row in rows))
        output = self.root / "output"
        subprocess.run([sys.executable, "-B", str(Path(__file__).with_name("upstream_cxx11_dg_metadata.py")),
                        "--inventory", str(inventory), "--sources", str(self.root / "sources"),
                        "--dejagnu-library", str(LIBRARY), "--output", str(output)],
                       stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, check=True)
        with (output / "metadata.jsonl").open() as stream:
            return {Path(row["path"]).name: row for line in stream if (row := json.loads(line))}

    def test_native_words_and_unresolved_inputs(self):
        body = '''// { dg-do run { target { c++11 && { ! c++14 } } } }
// { dg-options "-O2 -std=c++11" }
// { dg-error "missing.*name" "" { target *-*-* } .-1 }
// { dg-additional-sources "helper.C" }
// { dg-xfail-if "known" { *-*-* } }
// { dg-unfamiliar "retained" }
'''
        (self.suite / "quoted.C").write_text(body)
        (self.suite / "alias.C").symlink_to("quoted.C")
        (self.suite / "plain.C").write_text("int main() {}\n")
        (self.suite / "malformed.C").write_text('// { dg-options "unterminated }\n')
        results = self.invoke()
        self.assertEqual(set(results), {"quoted.C", "alias.C", "plain.C", "malformed.C"})
        for name in ["quoted.C", "alias.C"]:
            directives = results[name]["directives"]
            self.assertEqual(len(directives), 6)
            self.assertEqual(directives[0]["arguments"], ["run", " target { c++11 && { ! c++14 } } "])
            self.assertEqual(directives[2]["arguments"], ["missing.*name", "", " target *-*-* ", ".-1"])
            self.assertEqual(directives[3]["command"], "dg-additional-sources")
            self.assertEqual(directives[4]["command"], "dg-xfail-if")
            self.assertEqual(directives[5]["command"], "dg-unfamiliar")
        self.assertTrue(results["plain.C"]["status"].startswith("native-parser-empty"))
        self.assertEqual(results["malformed.C"]["status"], "native-parser-error")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dejagnu-library", type=Path, required=True)
    args, remaining = parser.parse_known_args()
    LIBRARY = args.dejagnu_library
    unittest.main(argv=[sys.argv[0], *remaining])
