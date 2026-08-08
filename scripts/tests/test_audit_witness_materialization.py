import importlib.util
import tempfile
import unittest
from pathlib import Path


SCRIPT = Path(__file__).parents[1] / "audit_witness_materialization.py"
SPEC = importlib.util.spec_from_file_location("audit_witness_materialization", SCRIPT)
assert SPEC and SPEC.loader
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class WitnessMaterializationAuditTest(unittest.TestCase):
    def make_root(self, producer: str, observer: str) -> Path:
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        root = Path(temporary.name)
        source = root / "dev/src/callsemantic"
        source.mkdir(parents=True)
        (source / "class_template_reference.cpp").write_text(
            producer + "\nvoid record_class_template_binding_state() {}\n",
            encoding="utf-8",
        )
        (root / "dev/src/callsemantic.cpp").write_text(
            observer + "\nconst auto claim_nondependent_source_observation = 0;\n",
            encoding="utf-8",
        )
        return root

    def valid_sources(self) -> tuple[str, str]:
        producer = """bool complete_source_type_materialization() {
  current_source_type_materialization_operation();
  return true;
}"""
        observer = """const cpp_decl::TemplateIdSourceDependency source_dependency =
  resolved.source_dependency;
if (!typed_materialization_admitted) return;"""
        return producer, observer

    def test_clean_typed_boundary_passes(self):
        root = self.make_root(*self.valid_sources())
        self.assertEqual(MODULE.audit(root)["findings"], [])

    def test_text_recovery_in_admission_fails(self):
        producer, observer = self.valid_sources()
        observer += "\nspaced_node_text(source);"
        findings = MODULE.audit(self.make_root(producer, observer))["findings"]
        self.assertTrue(any(item["term"] == "spaced_node_text(" for item in findings))

    def test_deleted_helper_name_fails_anywhere(self):
        producer, observer = self.valid_sources()
        producer += "\nbool source_arguments_are_fixed_class_aliases();"
        findings = MODULE.audit(self.make_root(producer, observer))["findings"]
        self.assertTrue(
            any(item["term"] == "source_arguments_are_fixed_class_aliases" for item in findings)
        )


if __name__ == "__main__":
    unittest.main()
