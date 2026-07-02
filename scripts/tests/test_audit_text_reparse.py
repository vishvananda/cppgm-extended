#!/usr/bin/env python3

from __future__ import annotations

import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPT = REPO_ROOT / "scripts" / "audit_text_reparse.py"


class AuditTextReparseTests(unittest.TestCase):
    def run_script(self, root: Path, *args: str) -> subprocess.CompletedProcess[str]:
        return subprocess.run(
            [sys.executable, str(SCRIPT), "--root", str(root), *args],
            cwd=REPO_ROOT,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=False,
        )

    def test_template_argument_text_shape_deduction_is_counted(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cppgm-text-reparse-audit.") as temp_dir:
            root = Path(temp_dir)
            src = root / "dev" / "src"
            src.mkdir(parents=True)
            (src / "template_resolution.cpp").write_text(
                "auto deduce_template_argument_text_shape = [] { return false; };\n"
                "trace << \"deduce-text-shape-template-mismatch\";\n"
                "trace << \"deduce-template-arg-text-shape-fail\";\n"
                "trace << \"template argument spelling diagnostic only\";\n",
                encoding="utf-8",
            )
            baseline = root / "baseline.json"
            baseline.write_text(
                json.dumps({
                    "limits": {
                        "qualified_name_string_parse": 0,
                        "template_id_string_parse": 0,
                        "expression_fragment_parse": 0,
                        "type_fragment_parse": 0,
                        "semantic_type_text_bridge": 0,
                        "translation_unit_fragment_parse": 0,
                        "ast_text_rebuild": 0,
                        "source_line_recovery": 0,
                        "template_argument_text_shape_deduction": 0,
                    }
                }),
                encoding="utf-8",
            )

            result = self.run_script(src, "--baseline", str(baseline), "--list-sites")

            self.assertEqual(result.returncode, 1)
            self.assertIn("template_argument_text_shape_deduction", result.stdout)
            self.assertIn("deduce_template_argument_text_shape", result.stdout)
            self.assertIn("deduce-text-shape-template-mismatch", result.stdout)
            self.assertIn("deduce-template-arg-text-shape-fail", result.stdout)
            self.assertNotIn("template argument spelling diagnostic only", result.stdout)


if __name__ == "__main__":
    unittest.main()
