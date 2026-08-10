#!/usr/bin/env python3

import importlib.util
from pathlib import Path
import unittest


REPO_ROOT = Path(__file__).resolve().parents[2]
RENDERER_PATH = (
    REPO_ROOT / "validation" / "templates" / "emit_templates_to_witness.py"
)


def load_renderer():
    spec = importlib.util.spec_from_file_location(
        "emit_templates_to_witness", RENDERER_PATH
    )
    if spec is None or spec.loader is None:
        raise RuntimeError(f"unable to load witness renderer from {RENDERER_PATH}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


RENDERER = load_renderer()


def witness_document(assignment: str):
    test_path = f"/checkout/{assignment}/tests/spec/movable.t"
    return {
        "events": [
            {
                "kind": "class_use",
                "location": f"{test_path}:7:3",
                "selected_decl_location": f"{test_path}:2:8",
                "template": "box",
                "resolved": "box<int>",
                "bindings": [
                    {"param": "T", "arg": "int", "source": "explicit"}
                ],
                "drops": [
                    {
                        "candidate": "box<long>",
                        "candidate_decl_location": f"{test_path}:3:8",
                        "reason": "deduction_failure",
                    }
                ],
            }
        ],
        "closure_events": [
            {
                "kind": "class-finalization",
                "location": f"{test_path}:7:3",
                "decl_location": f"{test_path}:2:8",
                "trigger_decl": f"{test_path}:6:1",
                "entity": "box<int>",
            }
        ],
    }


class WitnessPathNormalizationTests(unittest.TestCase):
    def test_public_witness_coalesces_exact_source_and_compact_closure_rows(self):
        document = witness_document("pa22")
        document["events"].append(dict(document["events"][0]))
        repeated_closure = dict(document["closure_events"][0])
        repeated_closure["trigger_decl"] = (
            "/checkout/pa22/tests/spec/movable.t:9:1"
        )
        document["closure_events"].append(repeated_closure)

        output = RENDERER.render_emit_templates_text(document)

        self.assertEqual(output.count("  class-use at "), 1)
        self.assertEqual(output.count("  class-finalization\n"), 1)

    def test_public_witness_does_not_merge_distinct_raw_source_events(self):
        document = witness_document("pa22")
        distinct = dict(document["events"][0])
        distinct["selected_decl_location"] = (
            "/checkout/pa22/tests/spec/movable.t:3:8"
        )
        document["events"].append(distinct)

        output = RENDERER.render_emit_templates_text(document)

        self.assertEqual(output.count("  class-use at "), 2)

    def test_const_normalization_does_not_rewrite_identifier_suffixes(self):
        self.assertEqual(
            RENDERER.normalize_public_type_spellings(
                "typename remove_const<T>::type"
            ),
            "typename remove_const<T>::type",
        )
        self.assertEqual(
            RENDERER.normalize_template_log_type_spellings(
                "meta::is_const<T>::value"
            ),
            "meta::is_const<T>::value",
        )
        self.assertEqual(
            RENDERER.normalize_public_type_spellings("value_type const *"),
            "const value_type *",
        )

    def test_public_normalization_preserves_qualified_member_names(self):
        self.assertEqual(
            RENDERER.normalize_template_log_type_spellings(
                "segment::const_iterator"
            ),
            "segment::const_iterator",
        )
        self.assertEqual(
            RENDERER.normalize_binding_arg(
                "traits::value_type", [], None
            ),
            "traits::value_type",
        )

    def test_public_witness_ignores_assignment_directory(self):
        pa18 = RENDERER.render_emit_templates_text(witness_document("pa18"))
        pa23 = RENDERER.render_emit_templates_text(witness_document("pa23"))

        self.assertEqual(pa18, pa23)
        self.assertIn("class-use at tests/spec/movable.t:7:3", pa18)
        self.assertNotIn("pa18/", pa18)

    def test_debug_witness_normalizes_each_rendered_location(self):
        output = RENDERER.render_emit_templates_debug_text(
            witness_document("pa21")
        )

        self.assertIn("class-use at tests/spec/movable.t:7:3", output)
        self.assertIn("decl tests/spec/movable.t:2:8", output)
        self.assertIn("drop box<long> at tests/spec/movable.t:3:8", output)
        self.assertIn("trigger_decl tests/spec/movable.t:6:1", output)
        self.assertNotIn("/checkout/", output)
        self.assertNotIn("pa21/", output)

    def test_path_normalization_preserves_nonassignment_sources(self):
        self.assertEqual(
            RENDERER.normalize_witness_location(
                "/usr/local/opt/llvm/include/c++/v1/vector:10:4"
            ),
            "libc++/vector:10:4",
        )
        self.assertEqual(
            RENDERER.normalize_witness_location("/tmp/project/source.cpp:5:2"),
            "/tmp/project/source.cpp:5:2",
        )
        self.assertEqual(
            RENDERER.normalize_witness_location(
                r"C:\repo\pa22\tests\general\movable.t:8:9"
            ),
            "tests/general/movable.t:8:9",
        )
        self.assertEqual(
            RENDERER.normalize_witness_location(
                "/checkout/pa19/course/templates/movable.t:11:6"
            ),
            "course/templates/movable.t:11:6",
        )
        self.assertEqual(
            RENDERER.normalize_witness_location("tests/spec/movable.t:3:1"),
            "tests/spec/movable.t:3:1",
        )


if __name__ == "__main__":
    unittest.main()
