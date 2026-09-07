#!/usr/bin/env python3
"""Retirement preserves provenance without excusing missing live owners."""

from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[2]
OLD = "dev/src/pa6_recognizer.cpp"
FORMER_OWNER = "dev/src/recognition/recognizer.cpp"


class CompilerRenameManifestTests(unittest.TestCase):
    def audit(self, old=OLD, owner=FORMER_OWNER, disposition="retired",
              reason="Recognizer retired after AST intake", owner_exists=False):
        with tempfile.TemporaryDirectory(prefix="rename-manifest.") as temp:
            root = Path(temp)
            (root / "scripts").mkdir()
            (root / "doc").mkdir()
            shutil.copy2(ROOT / "scripts/audit_compiler_rename_manifest.pl",
                         root / "scripts/audit_compiler_rename_manifest.pl")
            subprocess.run(["git", "init", "-q", str(root)], check=True)
            (root / "doc/compiler-rename-path-manifest.tsv").write_text(
                "old_path\tcurrent_owner\tdisposition\treason\n"
                f"{old}\t{owner}\t{disposition}\t{reason}\n"
            )
            if owner_exists:
                path = root / owner
                path.parent.mkdir(parents=True)
                path.write_text("// live source\n")
            return subprocess.run(
                ["perl", str(root / "scripts/audit_compiler_rename_manifest.pl")],
                text=True, capture_output=True,
            )

    def test_retired_client_preserves_absent_owner_and_reason(self):
        result = self.audit()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("1 retired", result.stdout)

    def test_retirement_requires_reason_and_absent_owner(self):
        self.assertIn("retirement needs a reason", self.audit(reason="").stderr)
        self.assertIn("retired owner still exists", self.audit(owner_exists=True).stderr)

    def test_retirement_cannot_hide_an_unrelated_missing_owner(self):
        result = self.audit(old="dev/src/pa10_parser.cpp", owner="dev/src/syntax/parser.cpp")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("retirement is not approved", result.stderr)

    def test_live_owners_must_still_exist(self):
        result = self.audit(disposition="renamed", reason="")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("current owner is missing", result.stderr)


if __name__ == "__main__":
    unittest.main()
