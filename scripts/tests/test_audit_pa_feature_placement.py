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
    def test_pre_lowir_semantic_surface_does_not_claim_later_runtime_owner(self) -> None:
        class_feature = audit.FeatureMeta(
            "class.basic", "pa16", 100, "", ""
        )
        exception_feature = audit.FeatureMeta(
            "exception.try_catch", "pa26", 100, "", ""
        )

        self.assertEqual(
            audit.placement_for(class_feature, "pa11", 200)[0],
            "semantic-surface",
        )
        self.assertEqual(
            audit.placement_for(class_feature, "pa15", 200)[0],
            "violation",
        )
        self.assertEqual(
            audit.placement_for(exception_feature, "pa11", 200)[0],
            "violation",
        )

    def test_pa12_semantic_output_does_not_claim_lowir_body_ownership(self) -> None:
        procedural = audit.FeatureMeta(
            "lowir.procedural", "pa15", 100, "", ""
        )
        condition = audit.FeatureMeta(
            "stmt.condition_declaration", "pa15", 100, "", ""
        )

        self.assertEqual(
            audit.placement_for(procedural, "pa12", 300)[0],
            "semantic-surface",
        )
        self.assertEqual(
            audit.placement_for(condition, "pa12", 300)[0],
            "semantic-surface",
        )
        self.assertEqual(
            audit.placement_for(condition, "pa11", 300)[0],
            "violation",
        )

    def test_pa22_template_review_retains_prerequisites_for_integration_audit(self) -> None:
        features = {
            "template.deduction_full": audit.FeatureMeta(
                "template.deduction_full", "pa23", 100, "", ""
            ),
            "sfinae": audit.FeatureMeta("sfinae", "pa23", 300, "", ""),
            "template.member_template": audit.FeatureMeta(
                "template.member_template", "pa22", 300, "", ""
            ),
            "template.pack": audit.FeatureMeta(
                "template.pack", "pa20", 200, "", ""
            ),
        }

        review = audit.template_review_for(
            list(features), [], [], 300, "pa23", features
        )

        self.assertEqual(
            review["review_template_concepts"],
            ["function-deduction", "sfinae"],
        )
        self.assertEqual(
            review["integration_template_concepts"],
            [
                "function-deduction",
                "member-template",
                "pack-expansion",
                "sfinae",
            ],
        )
        self.assertEqual(review["integration_template_concept_arity"], 4)

    def test_single_pa22_feature_does_not_gain_integration_concepts(self) -> None:
        features = {
            "sfinae": audit.FeatureMeta("sfinae", "pa23", 300, "", ""),
            "template.substitution": audit.FeatureMeta(
                "template.substitution", "pa23", 300, "", ""
            ),
        }

        review = audit.template_review_for(
            list(features), [], [], 300, "pa23", features
        )

        self.assertEqual(review["integration_template_concepts"], ["sfinae"])
        self.assertEqual(review["integration_template_concept_arity"], 1)

    def test_default_argument_detector_ignores_condition_declarations(self) -> None:
        declarations = "int f(int value = 1);\nvoid g(int = 2);\n"
        conditions = textwrap.dedent(
            """\
            int f() {
              if (int value = get(1)) return value;
              for (int index = 0; index != 2; ++index) {}
              return 0;
            }
            """
        )

        self.assertIn("function.default_argument", audit.detect_features(declarations))
        self.assertNotIn("function.default_argument", audit.detect_features(conditions))

    def test_bodyless_placeholder_return_is_hosted_compatibility(self) -> None:
        deleted = "inline constexpr auto blocked(long double) = delete;\n"
        declaration = "auto inspect(int);\n"
        definition = "auto inspect(int value) { return value; }\n"
        trailing = "auto inspect(int value) -> int;\n"
        nested_trailing = (
            "auto sequence_begin(T & value) -> decltype(value.begin());\n"
        )

        self.assertIn(
            "support.bodyless_placeholder_return",
            audit.detect_features(deleted),
        )
        self.assertIn(
            "support.bodyless_placeholder_return",
            audit.detect_features(declaration),
        )
        self.assertNotIn(
            "support.bodyless_placeholder_return",
            audit.detect_features(definition),
        )
        self.assertNotIn(
            "support.bodyless_placeholder_return",
            audit.detect_features(trailing),
        )
        self.assertNotIn(
            "support.bodyless_placeholder_return",
            audit.detect_features(nested_trailing),
        )

    def test_operator_detector_requires_an_operator_function_id(self) -> None:
        identifier = "int operator_arrow_dispatch_(int value);\n"
        overload = "struct value { value &operator+=(int); };\n"

        self.assertNotIn("operator.overload", audit.detect_features(identifier))
        self.assertIn("operator.overload", audit.detect_features(overload))

    def test_bitfield_detector_does_not_match_short_identifier_in_conditional(self) -> None:
        conditional = textwrap.dedent(
            """\
            char short_array[3];
            char first_array[4];
            char *select_array(int index) {
              return index == 0 ? short_array
                                : index == 1 ? first_array : short_array;
            }
            """
        )
        bitfield = "struct flags { unsigned bits : 3; };\n"

        self.assertNotIn("class.layout.bitfield", audit.detect_features(conditional))
        self.assertIn("class.layout.bitfield", audit.detect_features(bitfield))

    def test_vmi_rtti_requires_cast_or_typeid_source_for_pa27_detection(self) -> None:
        ordinary_polymorphic_rtti = textwrap.dedent(
            """\
            struct base {};
            struct derived : base { virtual int value(); };
            """
        )
        vmi_ref = (
            "declare global @__external_rtti_vtable____vmi_class_type_info "
            "[object=_ZTVN10__cxxabiv121__vmi_class_type_infoE]\n"
        )

        self.assertNotIn(
            "rtti.dynamic_cast.multi_vptr",
            audit.detect_features(ordinary_polymorphic_rtti, vmi_ref),
        )
        self.assertIn(
            "rtti.dynamic_cast.multi_vptr",
            audit.detect_features(
                ordinary_polymorphic_rtti
                + "base *convert(derived *p) { return dynamic_cast<base *>(p); }\n",
                vmi_ref,
            ),
        )

    def test_multiple_inheritance_requires_materialized_object_behavior(self) -> None:
        type_lookup_only = textwrap.dedent(
            """\
            struct left { typedef int marker; };
            struct right { typedef long marker; };
            struct derived : left, right {};
            template<class T> struct probe;
            typedef probe<derived> result;
            """
        )
        materialized = type_lookup_only + "derived value;\n"

        self.assertNotIn(
            "class.inheritance.multiple",
            audit.detect_features(type_lookup_only),
        )
        self.assertIn(
            "class.inheritance.multiple",
            audit.detect_features(materialized),
        )

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

    def test_hygiene_allows_family_owned_angle_header_override(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cppgm-placement-audit.") as temp_dir:
            root = Path(temp_dir)
            driver = root / "pa30" / "tests" / "general" / "300-include-order.t"
            source = driver.with_name("300-include-order.t.1")
            flags = driver.with_suffix(".flags")
            override = (
                root / "pa30" / "tests" / "general" /
                "300-include-order.inc" / "exception"
            )
            write(driver, "user include precedence\n")
            write(source, "#include <exception>\nint main() { return selected(); }\n")
            write(flags, "-I tests/general/300-include-order.inc\n")
            write(override, "int selected() { return 0; }\n")

            findings = audit.scan_test_hygiene(root, ["pa30"])
            self.assertNotIn(
                "early-hosted-eh-rtti-header",
                [finding.kind for finding in findings],
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
                root / "pa13" / "tests" / "general" /
                "100-dependent-mangling-case.t"
            )
            write(early_path, "int main() { return 0; }\n")

            early_content = (
                root / "pa13" / "tests" / "general" /
                "100-dependent-value-case.t"
            )
            write(early_content, "// ABI mangling should not be named here.\n")

            owner_stage = (
                root / "pa14" / "tests" / "abi" /
                "100-dependent-mangling-case.t"
            )
            write(owner_stage, "target function\n")

            findings = audit.scan_test_hygiene(root, ["pa13", "pa14"])
            self.assertEqual(
                [(finding.path, finding.kind, finding.evidence) for finding in findings],
                [
                    (
                        "pa13/tests/general/100-dependent-mangling-case.t",
                        "early-abi-naming-wording",
                        "path:mangl",
                    ),
                    (
                        "pa13/tests/general/100-dependent-value-case.t",
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

    def test_delegating_constructor_ignores_conditional_prvalues(self) -> None:
        conditional = (
            "int result = value() + (choose ? value() : value());"
        )
        delegating = textwrap.dedent(
            """\
            struct value {
              value(int);
              value() : value(0) {}
            };
            """
        )

        self.assertNotIn(
            "value.delegating_ctor",
            audit.detect_features(conditional),
        )
        self.assertIn(
            "value.delegating_ctor",
            audit.detect_features(delegating),
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

    def test_local_static_ref_detector_ignores_guard_class_symbols(self) -> None:
        source = textwrap.dedent(
            """\
            struct guard { guard(); ~guard(); };
            void build() { guard cleanup; }
            """
        )
        ref_text = textwrap.dedent(
            """\
            declare function @guard__guard(%arg0 : ptr) -> void
            declare function @guard___guard(%arg0 : ptr) -> void
            """
        )

        self.assertNotIn(
            "lowir.procedural.local_static",
            audit.detect_features(source, ref_text),
        )
        self.assertIn(
            "lowir.procedural.local_static",
            audit.detect_features(
                "int read();",
                "global @__local_static__read__value__guard : i64 = zero\n",
            ),
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

    def test_class_template_conversion_member_is_not_conversion_template(self) -> None:
        class_template_member = textwrap.dedent(
            """\
            template<class T>
            struct box {
              operator bool() const;
            };
            template<class T>
            box<T>::operator bool() const { return true; }
            int main() {
              box<int> source;
              return static_cast<bool>(source) ? 0 : 1;
            }
            """
        )

        self.assertNotIn(
            "template.conversion_deduction",
            audit.detect_features(class_template_member),
        )

    def test_lowir_eh_review_reports_hidden_source_to_lowir_output(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cppgm-placement-audit.") as temp_dir:
            root = Path(temp_dir)
            hidden = root / "pa16" / "tests" / "general" / "100-hidden-cleanup.t"
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

            runtime_only = root / "pa17" / "tests" / "general" / "100-runtime-only.t"
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

            explicit = root / "pa16" / "tests" / "general" / "100-explicit-throw.t"
            write(explicit, "int main() { throw 1; }\n")
            write(explicit.with_suffix(".ref"), "function @main() -> i32 {\n  block ^entry:\n    eh_try ^cleanup\n}\n")

            pa13 = root / "pa13" / "tests" / "spec" / "100-lowir-input.t"
            write(pa13, "function @main() -> i32 { block ^entry: eh_try ^cleanup }\n")
            write(pa13.with_suffix(".ref"), "ok\n")

            pa26 = root / "pa26" / "tests" / "general" / "100-owned-hidden-eh.t"
            write(pa26, "struct X { ~X(); }; int main() { X x; return 0; }\n")
            write(pa26.with_suffix(".ref"), "function @main() -> i32 {\n  block ^entry:\n    eh_try ^cleanup\n}\n")

            findings = audit.scan_lowir_eh_review(root, ["pa13", "pa16", "pa17", "pa26"])
            self.assertEqual(
                [(finding.path, finding.kind) for finding in findings],
                [
                    ("pa16/tests/general/100-hidden-cleanup.t", "eh-control"),
                    ("pa17/tests/general/100-runtime-only.t", "eh-runtime-declaration-only"),
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

            generated = {}
            findings = audit.scan_generated_lowir_eh_review(
                root,
                compiler,
                [selected],
                generated,
            )
            self.assertEqual(len(findings), 1)
            self.assertEqual(findings[0].path, "pa31/tests/general/200-selected.t")
            self.assertEqual(findings[0].kind, "generated-eh-control")
            self.assertIn("200-selected.t.1:eh_try", findings[0].evidence)
            self.assertIn("function @main", generated["pa31/tests/general/200-selected.t"])

    def test_generated_lowir_uses_checked_reference_feature_detection(self) -> None:
        source = textwrap.dedent(
            """\
            struct guard { ~guard(); };
            void run() {
              guard value;
              try { throw 1; }
              catch (int) { if (true) throw 2; }
            }
            """
        )
        hits = audit.generated_lowir_feature_hits(
            source,
            "function @run() {\n  eh_try ^catch\n  eh_cleanup ^outer\n  eh_end\n}\n",
            "pa31/tests/general/200-handler-branch.t",
        )
        self.assertIn("exception.handler_branch_cleanup", hits)
        self.assertTrue(any(
            evidence.startswith("generated:")
            for evidence in hits["exception.handler_branch_cleanup"].evidence
        ))

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
            self.assertEqual(audit.host_eh_object_evidence(anchor, "pa26", source), "")

    def test_pa32_host_object_attributes_do_not_require_pa34_attribute_support(self) -> None:
        hits = audit.detect_features(
            "__attribute__((weak)) int value;\n"
            "__attribute__((section(\"data\"))) int placed;\n"
            "__attribute__((visibility(\"default\"))) int exported;\n"
            "__attribute__((noinline)) int function();\n",
            test_path="pa32/tests/general/200-object-attributes.t",
        )
        self.assertIn("host.object_attribute", hits)
        self.assertNotIn("support.attribute", hits)

    def test_visibility_attribute_outside_pa32_retains_pa34_owner(self) -> None:
        hits = audit.detect_features(
            "__attribute__((visibility(\"default\"))) int exported;\n",
            test_path="pa22/tests/general/300-attribute.t",
        )
        self.assertNotIn("host.object_attribute", hits)
        self.assertIn("support.attribute", hits)

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

    def test_pa33_abi_tag_inspection_uses_host_abi_attribute_owner(self) -> None:
        hits = audit.detect_features(
            'struct __attribute__((abi_tag("tag"))) Tagged { ~Tagged(); };',
            test_path="pa33/tests/general/200-host-abi-tag-dtor.t",
        )
        self.assertIn("host.abi_name_attribute", hits)
        self.assertNotIn("support.attribute", hits)

    def test_pa14_abi_facts_are_not_classified_as_later_source_features(self) -> None:
        hits = audit.detect_features(
            "typeinfo named:C\nvtable named:C\noperator-terminal plus\n",
            ref_text="_ZTI1C\n_ZTV1C\n",
            test_path="pa14/tests/abi/200-future-vocabulary.t",
        )
        self.assertEqual(hits, {})

    def test_pa33_builtin_transform_mangling_uses_host_abi_owner(self) -> None:
        hits = audit.detect_features(
            "template<class T> using decay_alias = __decay(T);",
            test_path="pa33/tests/general/200-host-builtin-transform-mangling.t",
        )
        self.assertIn("host.abi_builtin_type", hits)
        self.assertNotIn("template.builtin_traits", hits)

    def test_pa34_run_has_hosted_runtime_owner(self) -> None:
        hits = audit.detect_features(
            "int main() { return 0; }",
            test_path="pa34/tests/run/800-hosted-runtime.t",
        )
        self.assertIn("hosted.runtime_compat", hits)

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
            "pa26/tests/general/200-guarded-static.t",
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
            "pa26/tests/general/200-aggregate-cleanup.t",
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
            "pa26/tests/general/200-class-value-argument-cleanup.t",
        )
        self.assertIn("exception.class_value_argument_cleanup", hits)

    def test_class_throw_operand_with_eh_has_operand_cleanup_feature(self) -> None:
        source = textwrap.dedent(
            """\
            struct text { ~text(); };
            struct error { error(text); };
            void raise(bool take) {
              if (take) throw error(text());
              throw 1;
            }
            """
        )
        hits = audit.detect_features(
            source,
            "declare function @__cxa_throw() -> void [role=eh_throw]\n"
            "function @raise() {\n  eh_try ^cleanup\n  eh_end\n}\n",
            "pa26/tests/general/200-throw-operand-cleanup.t",
        )
        self.assertIn("exception.throw_operand_cleanup", hits)


if __name__ == "__main__":
    unittest.main()
