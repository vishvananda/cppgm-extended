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


if __name__ == "__main__":
    unittest.main()
