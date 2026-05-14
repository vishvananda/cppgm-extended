#!/usr/bin/env python3

from __future__ import annotations

import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPT = REPO_ROOT / "scripts" / "audit_template_boundary.py"


class AuditTemplateBoundaryTests(unittest.TestCase):
    def run_script(self, root: Path, *args: str) -> subprocess.CompletedProcess[str]:
        return subprocess.run(
            [sys.executable, str(SCRIPT), "--root", str(root), *args],
            cwd=REPO_ROOT,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=False,
        )

    def test_write_baseline_records_current_counts(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cppgm-template-audit.") as temp_dir:
            root = Path(temp_dir)
            src = root / "dev" / "src"
            src.mkdir(parents=True)
            (src / "template_resolution.cpp").write_text(
                "template_api::SemanticContextTemplateServices storage(ctx);\n"
                "template_api::TemplateServices services = storage.bundle();\n"
                "return services.type_system.resolve_direct_type_lookup(request, out);\n",
                encoding="utf-8",
            )
            baseline = root / "baseline.json"

            result = self.run_script(root, "--baseline", str(baseline), "--write-baseline")

            self.assertEqual(result.returncode, 0, result.stderr)
            data = json.loads(baseline.read_text(encoding="utf-8"))
            self.assertEqual(data["limits"]["service_adapter_construction"], 1)
            self.assertEqual(data["limits"]["service_bundle_construction"], 1)
            self.assertEqual(data["limits"]["semantic_service_access"], 1)

    def test_baseline_failure_reports_expanded_debt(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cppgm-template-audit.") as temp_dir:
            root = Path(temp_dir)
            src = root / "dev" / "src"
            src.mkdir(parents=True)
            (src / "callsemantic.cpp").write_text(
                "binding.source_template = source_template;\n",
                encoding="utf-8",
            )
            baseline = root / "baseline.json"
            baseline.write_text(
                json.dumps({
                    "limits": {
                        "service_adapter_construction": 0,
                        "service_bundle_construction": 0,
                        "semantic_service_access": 0,
                        "text_recovery_bridge": 0,
                        "canonical_key_metadata": 0,
                        "witness_source_location": 0,
                        "callsemantic_template_metadata_exception": 0,
                    }
                }),
                encoding="utf-8",
            )

            result = self.run_script(root, "--baseline", str(baseline))

            self.assertEqual(result.returncode, 1)
            self.assertIn("canonical_key_metadata", result.stdout)
            self.assertIn("callsemantic_template_metadata_exception", result.stdout)

    def test_marked_owner_block_is_not_counted_as_debt(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cppgm-template-audit.") as temp_dir:
            root = Path(temp_dir)
            src = root / "dev" / "src"
            src.mkdir(parents=True)
            (src / "template_instantiation.cpp").write_text(
                "// template-boundary-audit: begin canonical_key_metadata\n"
                "binding.template_instantiation_key = instantiation_key;\n"
                "// template-boundary-audit: end canonical_key_metadata\n"
                "return binding.template_instantiation_key;\n",
                encoding="utf-8",
            )
            baseline = root / "baseline.json"

            result = self.run_script(root, "--baseline", str(baseline), "--write-baseline")

            self.assertEqual(result.returncode, 0, result.stderr)
            data = json.loads(baseline.read_text(encoding="utf-8"))
            self.assertEqual(data["limits"]["canonical_key_metadata"], 1)


if __name__ == "__main__":
    unittest.main()
