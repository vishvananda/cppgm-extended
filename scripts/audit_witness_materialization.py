#!/usr/bin/env python3
"""Reject semantic recovery in the class materialization admission boundary."""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path


FORBIDDEN_SYMBOLS = (
    "source_arguments_are_fixed_class_constants",
    "source_arguments_are_fixed_class_aliases",
    "fixed_class_constant_source",
    "fixed_conversion_alias_source",
    "produce_resolved_source_type_materialization",
    "class_source_dependencies",
)

FORBIDDEN_DECISION_TERMS = (
    "node_text(",
    "spaced_node_text(",
    "contains_identifier_token(",
    "source_argument_texts",
    "template_id_argument_witness_source_texts",
    "current_template_witness_entry_context",
    "class_template_use_matches_current_conversion_result",
)


def source_files(root: Path) -> list[Path]:
    return sorted(path for path in (root / "dev" / "src").rglob("*") if path.suffix in {".h", ".cpp"})


def line_for_offset(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def find_section(text: str, start: str, end: str) -> tuple[str, int] | None:
    start_offset = text.find(start)
    if start_offset < 0:
        return None
    end_offset = text.find(end, start_offset)
    if end_offset < 0:
        return None
    return text[start_offset:end_offset], start_offset


def audit(root: Path) -> dict[str, object]:
    findings: list[dict[str, object]] = []
    for path in source_files(root):
        text = path.read_text(encoding="utf-8")
        for symbol in FORBIDDEN_SYMBOLS:
            offset = text.find(symbol)
            if offset >= 0:
                findings.append(
                    {
                        "path": path.relative_to(root).as_posix(),
                        "line": line_for_offset(text, offset),
                        "term": symbol,
                        "reason": "deleted materialization recovery returned",
                    }
                )

    boundaries = (
        (
            root / "dev/src/callsemantic/class_template_reference.cpp",
            "bool complete_source_type_materialization(",
            "void record_class_template_binding_state(",
            "typed materialization producer",
        ),
        (
            root / "dev/src/callsemantic.cpp",
            "cpp_decl::TemplateIdSourceDependency source_dependency =",
            "const auto claim_nondependent_source_observation =",
            "concrete dependent-source admission",
        ),
    )
    for path, start, end, label in boundaries:
        text = path.read_text(encoding="utf-8")
        section = find_section(text, start, end)
        if section is None:
            findings.append(
                {
                    "path": path.relative_to(root).as_posix(),
                    "line": 0,
                    "term": start,
                    "reason": f"cannot locate {label}",
                }
            )
            continue
        body, body_offset = section
        for term in FORBIDDEN_DECISION_TERMS:
            offset = body.find(term)
            if offset >= 0:
                findings.append(
                    {
                        "path": path.relative_to(root).as_posix(),
                        "line": line_for_offset(text, body_offset + offset),
                        "term": term,
                        "reason": f"{label} uses forbidden recovery input",
                    }
                )

    class_reference = (root / "dev/src/callsemantic/class_template_reference.cpp").read_text(encoding="utf-8")
    callsemantic = (root / "dev/src/callsemantic.cpp").read_text(encoding="utf-8")
    required = (
        (
            "dev/src/callsemantic/class_template_reference.cpp",
            class_reference,
            "current_source_type_materialization_operation()",
        ),
        (
            "dev/src/callsemantic.cpp",
            callsemantic,
            "!typed_materialization_admitted",
        ),
        (
            "dev/src/callsemantic.cpp",
            callsemantic,
            "resolved.source_dependency",
        ),
    )
    for path, text, term in required:
        if term not in text:
            findings.append(
                {
                    "path": path,
                    "line": 0,
                    "term": term,
                    "reason": "required typed materialization boundary is absent",
                }
            )

    return {
        "schema_version": 1,
        "forbidden_symbol_count": len(FORBIDDEN_SYMBOLS),
        "decision_boundary_count": len(boundaries),
        "findings": findings,
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, default=Path("."))
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    report = audit(args.root.resolve())
    rendered = json.dumps(report, indent=2, sort_keys=True) + "\n"
    if args.output:
        args.output.write_text(rendered, encoding="utf-8")
    else:
        sys.stdout.write(rendered)
    return 1 if report["findings"] else 0


if __name__ == "__main__":
    raise SystemExit(main())
