#!/usr/bin/env python3
"""Check short-input batching preserves arguments, failures and every output."""
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile
import unittest

from upstream_cxx11_counter_run import BATCH_RUNNER, validate_batch


class BatchTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory(prefix="cppgm perf batch ")
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        self.batch = self.root / "batch runner"
        self.batch.write_text(BATCH_RUNNER)
        self.batch.chmod(0o755)
        self.compiler = self.root / "compiler stub"
        self.compiler.write_text('''#!/bin/sh
previous=
output=
payload=
for argument do
  if [ "$previous" = "-o" ]; then output="$argument"; fi
  if [ "$previous" = "--payload" ]; then payload="$argument"; fi
  if [ "$argument" = "--fail" ]; then exit 70; fi
  previous="$argument"
done
printf '%s' "$payload" > "$output"
''')
        self.compiler.chmod(0o755)

    def test_every_child_output_and_argument_fidelity(self):
        output = self.root / "result object"
        command = [str(self.batch), str(self.compiler), "3", "--payload", "literal $value; spaces",
                   "-o", str(output)]
        subprocess.run(command, check=True)
        self.assertEqual(output.read_text(), "literal $value; spaces")
        report = self.root / "report.json"
        report.write_text(json.dumps(dict(runs=[dict(command=command,
            output_sha256=hashlib.sha256(output.read_bytes()).hexdigest())])))
        children = validate_batch(report, 3)
        self.assertEqual(len(children), 3)
        Path(children[0]["path"]).write_text("changed intermediate output")
        with self.assertRaisesRegex(ValueError, "batch child differs"):
            validate_batch(report, 3)

    def test_abnormal_child_failure_is_not_hidden(self):
        output = self.root / "failed object"
        result = subprocess.run([str(self.batch), str(self.compiler), "3", "--fail", "-o", str(output)])
        self.assertEqual(result.returncode, 70)
        self.assertFalse(output.exists())


if __name__ == "__main__":
    unittest.main()
