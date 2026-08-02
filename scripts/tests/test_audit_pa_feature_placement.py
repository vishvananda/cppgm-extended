#!/usr/bin/env python3

import importlib.util
from pathlib import Path
import sys
import tempfile
import textwrap
import unittest


REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPT_PATH = REPO_ROOT / "scripts" / "audit_pa_feature_placement.py"


def load_module():
    spec = importlib.util.spec_from_file_location("audit_pa_feature_placement", SCRIPT_PATH)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


audit = load_module()


def write(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text)


class AuditPAFeaturePlacementTests(unittest.TestCase):
    def test_hygiene_reports_compile_flags_sidecar(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cppgm-placement-audit.") as temp_dir:
            root = Path(temp_dir)
            test = root / "pa36" / "tests" / "link" / "600-hosted-smoke.t"
            write(test, "int main() { return 0; }\n")
            write(test.with_suffix(".compile.flags"), "-O1\n")

            findings = audit.scan_test_hygiene(root, ["pa36"])
            self.assertEqual(
                [(finding.path, finding.kind) for finding in findings],
                [
                    (
                        "pa36/tests/link/600-hosted-smoke.compile.flags",
                        "compile-flags-sidecar",
                    ),
                ],
            )

    def test_hygiene_reports_early_hosted_eh_rtti_headers(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cppgm-placement-audit.") as temp_dir:
            root = Path(temp_dir)
            pa33_typeinfo = root / "pa33" / "tests" / "general" / "200-typeid-header.t"
            write(pa33_typeinfo, "#include <typeinfo>\nint main() { return 0; }\n")

            pa34_exception = root / "pa34" / "tests" / "compile" / "600-exception-header.t"
            write(pa34_exception, "#include <exception>\nint main() { return 0; }\n")

            pa35_typeinfo = root / "pa35" / "tests" / "compile" / "700-typeinfo-header.t"
            write(pa35_typeinfo, "#include <typeinfo>\nint main() { return 0; }\n")

            findings = audit.scan_test_hygiene(root, ["pa33", "pa34", "pa35"])
            self.assertEqual(
                [(finding.path, finding.kind, finding.evidence) for finding in findings],
                [
                    (
                        "pa33/tests/general/200-typeid-header.t",
                        "early-hosted-eh-rtti-header",
                        "#include <typeinfo>",
                    ),
                    (
                        "pa34/tests/compile/600-exception-header.t",
                        "early-hosted-eh-rtti-header",
                        "#include <exception>",
                    ),
                ],
            )

    def test_hygiene_reports_early_exception_ptr_runtime(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cppgm-placement-audit.") as temp_dir:
            root = Path(temp_dir)
            pa32_exception_ptr = (
                root / "pa32" / "tests" / "general" /
                "200-exception-ptr-runtime.t"
            )
            write(
                pa32_exception_ptr,
                textwrap.dedent(
                    """\
                    namespace std {
                    class exception_ptr;
                    exception_ptr make_exception_ptr(int);
                    void rethrow_exception(exception_ptr);
                    }
                    int main() {
                      std::exception_ptr p = std::make_exception_ptr(1);
                      std::rethrow_exception(p);
                    }
                    """
                ),
            )

            pa36_exception_ptr = (
                root / "pa36" / "tests" / "link" /
                "600-exception-ptr-runtime.t"
            )
            write(
                pa36_exception_ptr,
                "int main() { return 0; }\n",
            )

            findings = audit.scan_test_hygiene(root, ["pa32", "pa36"])
            self.assertEqual(
                [(finding.path, finding.kind, finding.evidence) for finding in findings],
                [
                    (
                        "pa32/tests/general/200-exception-ptr-runtime.t",
                        "early-hosted-exception-runtime",
                        "std::exception_ptr",
                    ),
                ],
            )

    def test_hygiene_reports_early_abi_naming_wording(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cppgm-placement-audit.") as temp_dir:
            root = Path(temp_dir)
            early_path = (
                root / "pa23" / "tests" / "general" /
                "100-dependent-mangling-case.t"
            )
            write(early_path, "int main() { return 0; }\n")

            early_content = (
                root / "pa23" / "tests" / "general" /
                "100-dependent-value-case.t"
            )
            write(early_content, "// ABI mangling should not be named here.\n")

            owner_stage = (
                root / "pa30" / "tests" / "abi" /
                "100-dependent-mangling-case.t"
            )
            write(owner_stage, "target function\n")

            findings = audit.scan_test_hygiene(root, ["pa23", "pa30"])
            self.assertEqual(
                [(finding.path, finding.kind, finding.evidence) for finding in findings],
                [
                    (
                        "pa23/tests/general/100-dependent-mangling-case.t",
                        "early-abi-naming-wording",
                        "path:mangl",
                    ),
                    (
                        "pa23/tests/general/100-dependent-value-case.t",
                        "early-abi-naming-wording",
                        "source:mangl",
                    ),
                ],
            )
    def test_range_for_ignores_scope_qualified_ordinary_for_loop(self) -> None:
        ordinary = "for (std::size_t i = 0u; i < size; ++i) {}"
        range_for = "for (std::size_t value : values) {}"

        self.assertNotIn("support.range_for", audit.detect_features(ordinary))
        self.assertIn("support.range_for", audit.detect_features(range_for))

    def test_lambda_ignores_array_allocation_operators(self) -> None:
        allocation_operators = (
            "static void *operator new[](unsigned long); "
            "static void operator delete[](void *p) noexcept { free(p); }"
        )
        lambda_expression = "[](void *p) noexcept { free(p); }"

        self.assertNotIn(
            "support.lambda",
            audit.detect_features(allocation_operators),
        )
        self.assertIn(
            "support.lambda",
            audit.detect_features(lambda_expression),
        )

    def test_nttp_default_reference_expression_is_not_pointer_nttp(self) -> None:
        default_expression = (
            "template<class T, bool Value = noexcept(declval<T&>())> "
            "struct trait;"
        )
        pointer_parameter = "template<class T, T *Value> struct pointer_trait;"

        self.assertNotIn(
            "template.nttp.pointer_member",
            audit.detect_features(default_expression),
        )
        self.assertIn(
            "template.nttp.pointer_member",
            audit.detect_features(pointer_parameter),
        )

    def test_dynamic_local_static_ignores_class_static_member_array(self) -> None:
        class_static = textwrap.dedent(
            """\
            struct Entry { constexpr Entry(int) {} };
            template<class T> struct Tables {
              static constexpr Entry rows[1] = {Entry(1)};
            };
            """
        )
        function_local = textwrap.dedent(
            """\
            struct Entry { Entry(int) {} };
            int read() {
              static Entry rows[1] = {Entry(1)};
              return 0;
            }
            """
        )

        self.assertNotIn(
            "lowir.procedural.local_static.dynamic_class",
            audit.detect_features(class_static),
        )
        self.assertIn(
            "lowir.procedural.local_static.dynamic_class",
            audit.detect_features(function_local),
        )

    def test_conversion_template_deduction_requires_a_conversion_use(self) -> None:
        declaration_only = textwrap.dedent(
            """\
            struct box {
              template<class T> operator T() const;
            };
            template<class T> box::operator T() const { return T(); }
            int main() { return 0; }
            """
        )
        copy_initialization = textwrap.dedent(
            """\
            struct box {
              template<class T> operator T() const { return T(); }
            };
            int main() {
              box source;
              int result = source;
              return result;
            }
            """
        )

        self.assertNotIn(
            "template.conversion_deduction",
            audit.detect_features(declaration_only),
        )
        self.assertIn(
            "template.conversion_deduction",
            audit.detect_features(copy_initialization),
        )

    def test_lowir_eh_review_reports_hidden_source_to_lowir_output(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cppgm-placement-audit.") as temp_dir:
            root = Path(temp_dir)
            hidden = root / "pa15" / "tests" / "general" / "100-hidden-cleanup.t"
            write(hidden, "struct X { ~X(); }; int main() { X x; return 0; }\n")
            write(hidden.with_suffix(".ref"), textwrap.dedent(
                """\
                function @main() -> i32 {
                  block ^entry:
                    eh_try ^cleanup
                    eh_end
                    return i32 0

                  block ^cleanup:
                    resume
                }
                """
            ))

            runtime_only = root / "pa16" / "tests" / "general" / "100-runtime-only.t"
            write(runtime_only, "struct X { ~X(); }; int main() { X x; return 0; }\n")
            write(runtime_only.with_suffix(".ref"), textwrap.dedent(
                """\
                declare function @__external_runtime___Unwind_Resume() -> void [role=eh_resume]

                function @main() -> i32 {
                  block ^entry:
                    return i32 0
                }
                """
            ))

            explicit = root / "pa15" / "tests" / "general" / "100-explicit-throw.t"
            write(explicit, "int main() { throw 1; }\n")
            write(explicit.with_suffix(".ref"), "function @main() -> i32 {\n  block ^entry:\n    eh_try ^cleanup\n}\n")

            pa13 = root / "pa13" / "tests" / "spec" / "100-lowir-input.t"
            write(pa13, "function @main() -> i32 { block ^entry: eh_try ^cleanup }\n")
            write(pa13.with_suffix(".ref"), "ok\n")

            pa25 = root / "pa25" / "tests" / "general" / "100-owned-hidden-eh.t"
            write(pa25, "struct X { ~X(); }; int main() { X x; return 0; }\n")
            write(pa25.with_suffix(".ref"), "function @main() -> i32 {\n  block ^entry:\n    eh_try ^cleanup\n}\n")

            findings = audit.scan_lowir_eh_review(root, ["pa13", "pa15", "pa16", "pa25"])
            self.assertEqual(
                [(finding.path, finding.kind) for finding in findings],
                [
                    ("pa15/tests/general/100-hidden-cleanup.t", "eh-control"),
                    ("pa16/tests/general/100-runtime-only.t", "eh-runtime-declaration-only"),
                ],
            )

    def test_companion_source_includes_numbered_host_translation_units(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cppgm-placement-audit.") as temp_dir:
            root = Path(temp_dir)
            anchor = root / "pa31" / "tests" / "general" / "200-numbered-source.t"
            write(anchor, "")
            write(anchor.parent / f"{anchor.name}.1", "int main() { throw 1; }\n")
            write(anchor.parent / f"{anchor.name}.2", "int helper() { return 2; }\n")
            write(anchor.with_suffix(".ref.stdout"), "ignored\n")

            source = audit.companion_source_text_for(anchor)
            self.assertIn("throw 1", source)
            self.assertIn("int helper", source)
            self.assertNotIn("ignored", source)

    def test_generated_lowir_review_probes_only_selected_host_tests(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cppgm-placement-audit.") as temp_dir:
            root = Path(temp_dir)
            selected = root / "pa31" / "tests" / "general" / "200-selected.t"
            unselected = root / "pa31" / "tests" / "general" / "200-unselected.t"
            write(selected, "")
            write(selected.parent / f"{selected.name}.1", "// hidden-eh\nint main() { return 0; }\n")
            write(unselected, "")
            write(unselected.parent / f"{unselected.name}.1", "// hidden-eh\nint main() { return 0; }\n")

            compiler = root / "fake-cppgm++"
            write(compiler, textwrap.dedent(
                """\
                #!/usr/bin/env python3
                from pathlib import Path
                import sys

                output = Path(sys.argv[sys.argv.index("-o") + 1])
                output.write_text("function @main() {\\n  eh_try ^cleanup\\n  eh_end\\n}\\n")
                """
            ))
            compiler.chmod(0o755)

            findings = audit.scan_generated_lowir_eh_review(
                root,
                compiler,
                [selected],
            )
            self.assertEqual(len(findings), 1)
            self.assertEqual(findings[0].path, "pa31/tests/general/200-selected.t")
            self.assertEqual(findings[0].kind, "generated-eh-control")
            self.assertIn("200-selected.t.1:eh_try", findings[0].evidence)

    def test_pa31_explicit_exception_source_has_host_object_layer(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cppgm-placement-audit.") as temp_dir:
            root = Path(temp_dir)
            anchor = root / "pa31" / "tests" / "general" / "100-host-object.t"
            write(anchor, "")
            write(anchor.parent / f"{anchor.name}.1", "int main() { throw 1; }\n")
            source = audit.companion_source_text_for(anchor)

            self.assertEqual(
                audit.host_eh_object_evidence(anchor, "pa31", source),
                "harness:cppgm++ -c, source:throw",
            )
            self.assertEqual(audit.host_eh_object_evidence(anchor, "pa25", source), "")

    def test_pa32_host_object_attributes_do_not_require_pa34_attribute_support(self) -> None:
        hits = audit.detect_features(
            "__attribute__((weak)) int value;\n"
            "__attribute__((section(\"data\"))) int placed;\n"
            "__attribute__((noinline)) int function();\n",
            test_path="pa32/tests/general/200-object-attributes.t",
        )
        self.assertIn("host.object_attribute", hits)
        self.assertNotIn("support.attribute", hits)

    def test_pa32_host_object_anchor_is_an_explicit_layer_assertion(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cppgm-placement-audit.") as temp_dir:
            root = Path(temp_dir)
            anchor = root / "pa32" / "tests" / "general" / "200-host-object.t"
            write(anchor, "# The host-object path must preserve member moves.\n")
            write(anchor.parent / f"{anchor.name}.1", "int main() { return 0; }\n")

            self.assertEqual(
                audit.host_object_interop_evidence(anchor, "pa32"),
                "anchor-contract:host-object",
            )
            self.assertEqual(audit.host_object_interop_evidence(anchor, "pa31"), "")

    def test_dynamic_local_static_with_eh_has_guarded_cleanup_feature(self) -> None:
        source = textwrap.dedent(
            """\
            struct value { value(int); };
            int read() {
              static value item(value(1));
              return 0;
            }
            """
        )
        hits = audit.detect_features(
            source,
            "global @__local_static__read__item = zero\n"
            "function @read() {\n  eh_try ^cleanup\n  eh_end\n}\n",
            "pa25/tests/general/200-guarded-static.t",
        )
        self.assertIn("exception.guarded_static_cleanup", hits)

    def test_aggregate_return_with_eh_has_aggregate_cleanup_feature(self) -> None:
        source = textwrap.dedent(
            """\
            struct member { member(int); ~member(); };
            struct aggregate { member value; };
            aggregate make() { return { member(1) }; }
            """
        )
        hits = audit.detect_features(
            source,
            "function @make() {\n  eh_try ^cleanup\n  eh_end\n}\n",
            "pa25/tests/general/200-aggregate-cleanup.t",
        )
        self.assertIn("exception.aggregate_cleanup", hits)

    def test_class_value_argument_with_eh_has_transfer_cleanup_feature(self) -> None:
        source = textwrap.dedent(
            """\
            struct value {
              value();
              value(const value &);
              ~value();
            };
            int consume(value argument) { return 0; }
            int main() { value source; return consume(source); }
            """
        )
        hits = audit.detect_features(
            source,
            "function @consume(%argument : ptr [pass=by_address]) -> i32 {\n"
            "  call void @value___value(%argument)\n}\n"
            "function @main() {\n  eh_try ^cleanup\n  eh_end\n}\n",
            "pa25/tests/general/200-class-value-argument-cleanup.t",
        )
        self.assertIn("exception.class_value_argument_cleanup", hits)


if __name__ == "__main__":
    unittest.main()
