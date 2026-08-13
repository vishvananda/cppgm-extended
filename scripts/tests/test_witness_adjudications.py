#!/usr/bin/env python3

import hashlib
import importlib.util
import json
import pathlib
import tempfile
import unittest


REPO_ROOT = pathlib.Path(__file__).resolve().parents[2]
MODULE_PATH = REPO_ROOT / "validation" / "templates" / "witness_adjudications.py"
SPEC = importlib.util.spec_from_file_location("witness_adjudications", MODULE_PATH)
assert SPEC is not None and SPEC.loader is not None
ADJUDICATIONS = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(ADJUDICATIONS)


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


class WitnessAdjudicationsTests(unittest.TestCase):
    def write_fixture(self, root: pathlib.Path):
        test = root / "pa22" / "tests" / "sample.t"
        test.parent.mkdir(parents=True)
        test.write_text("template<class> struct sample;\n", encoding="utf-8")
        raw = "translation-unit\n  specialize #3 = <>\n  specialize #4 = void\n"
        corrected = "translation-unit\n  specialize #3 = void\n"
        manifest_path = root / "adjudications.json"
        manifest = {
            "schema_version": 1,
            "adjudications": [
                {
                    "id": "unit-pack-order",
                    "test": "pa22/tests/sample.t",
                    "source_sha256": sha256(test.read_bytes()),
                    "raw_clang_witness_sha256": sha256(raw.encode()),
                    "adjudicated_witness_sha256": sha256(corrected.encode()),
                    "reason": "unit test",
                    "rewrites": [
                        {
                            "before": "  specialize #3 = <>\n  specialize #4 = void\n",
                            "after": "  specialize #3 = void\n",
                        }
                    ],
                }
            ],
        }
        manifest_path.write_text(json.dumps(manifest), encoding="utf-8")
        return test, raw, corrected, manifest_path

    def test_applies_hash_pinned_rewrite_during_generation(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = pathlib.Path(tmp)
            test, raw, corrected, manifest_path = self.write_fixture(root)
            entries = ADJUDICATIONS.load_witness_adjudications(manifest_path)
            actual, entry_id = ADJUDICATIONS.apply_witness_adjudication(
                root, test, raw, entries)
            self.assertEqual(actual, corrected)
            self.assertEqual(entry_id, "unit-pack-order")

    def test_rejects_changed_raw_clang_witness(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = pathlib.Path(tmp)
            test, raw, _corrected, manifest_path = self.write_fixture(root)
            entries = ADJUDICATIONS.load_witness_adjudications(manifest_path)
            with self.assertRaisesRegex(ValueError, "raw_clang_witness_sha256 changed"):
                ADJUDICATIONS.apply_witness_adjudication(
                    root, test, raw + "drift\n", entries)

    def test_rejects_changed_source(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = pathlib.Path(tmp)
            test, raw, _corrected, manifest_path = self.write_fixture(root)
            entries = ADJUDICATIONS.load_witness_adjudications(manifest_path)
            test.write_text("changed\n", encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "source_sha256 changed"):
                ADJUDICATIONS.apply_witness_adjudication(root, test, raw, entries)

    def test_leaves_unlisted_witness_unchanged(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = pathlib.Path(tmp)
            test = root / "pa22" / "tests" / "other.t"
            test.parent.mkdir(parents=True)
            test.write_text("int main() {}\n", encoding="utf-8")
            actual, entry_id = ADJUDICATIONS.apply_witness_adjudication(
                root, test, "translation-unit\n", {})
            self.assertEqual(actual, "translation-unit\n")
            self.assertIsNone(entry_id)


if __name__ == "__main__":
    unittest.main()
