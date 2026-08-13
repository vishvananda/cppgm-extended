#!/usr/bin/env python3
"""Reject duplicate AST semantic analysis on witness-only paths.

The template source-context index is a deliberately retained structural walk:
normal declaration collection misses explicit source ranges that witness
classification needs.  It may record ranges, but it may not perform semantic
analysis.  All template-parameter analysis must remain at the primary
declaration or lambda semantic boundaries.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from dataclasses import dataclass
from pathlib import Path


STRUCTURAL_WALK = "record_template_source_contexts_for_witness_node"
RENDERER_PATHS = (
    "dev/src/template_witness_renderer.cpp",
    "dev/src/template_text_output.cpp",
    "dev/src/witness_api.cpp",
    "dev/src/witness_text.cpp",
)
SEMANTIC_CALLS_FORBIDDEN_IN_STRUCTURAL_WALK = (
    "parse_template_parameters(",
    "collect_template_declaration(",
    "collect_template_declaration_impl(",
    "resolve_template_arguments(",
    "sem_declaration(",
)
ALLOWED_TEMPLATE_PARAMETER_ANALYSIS_OWNERS = {
    "dev/src/callsemantic.cpp": {
        "parse_template_parameters",
        "synthesize_lambda_closure_class",
    },
    "dev/src/callsemantic/template_declaration_collector.cpp": {
        "collect_template_declaration_impl",
        "parse_template_parameters",
    },
}
CONTROL_NAMES = {"if", "for", "while", "switch", "catch"}


@dataclass(frozen=True)
class FunctionSpan:
    name: str
    signature_start: int
    body_start: int
    body_end: int


def mask_cpp(text: str) -> str:
    """Blank comments and literals while preserving offsets and newlines."""
    out = list(text)
    index = 0
    state = "code"
    while index < len(text):
        char = text[index]
        nxt = text[index + 1] if index + 1 < len(text) else ""
        if state == "code":
            if char == "/" and nxt == "/":
                out[index] = out[index + 1] = " "
                index += 2
                state = "line_comment"
                continue
            if char == "/" and nxt == "*":
                out[index] = out[index + 1] = " "
                index += 2
                state = "block_comment"
                continue
            if char in {'"', "'"}:
                out[index] = " "
                state = "string" if char == '"' else "char"
        elif state == "line_comment":
            if char == "\n":
                state = "code"
            else:
                out[index] = " "
        elif state == "block_comment":
            if char == "*" and nxt == "/":
                out[index] = out[index + 1] = " "
                index += 2
                state = "code"
                continue
            if char != "\n":
                out[index] = " "
        else:
            if char == "\\" and index + 1 < len(text):
                out[index] = " "
                if text[index + 1] != "\n":
                    out[index + 1] = " "
                index += 2
                continue
            if (state == "string" and char == '"') or (
                state == "char" and char == "'"
            ):
                state = "code"
            if char != "\n":
                out[index] = " "
        index += 1
    return "".join(out)


def matching_brace(masked: str, opening: int) -> int | None:
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                return index
    return None


def function_spans(text: str) -> list[FunctionSpan]:
    masked = mask_cpp(text)
    spans: list[FunctionSpan] = []
    signature = re.compile(
        r"([A-Za-z_~][A-Za-z0-9_]*)\s*\([^;{}]*\)\s*"
        r"(?:const\s*)?(?:override\s*)?(?:final\s*)?$",
        re.DOTALL,
    )
    for opening, char in enumerate(masked):
        if char != "{":
            continue
        boundary = max(masked.rfind(";", 0, opening),
                       masked.rfind("}", 0, opening),
                       masked.rfind("{", 0, opening))
        candidate = masked[boundary + 1:opening]
        match = signature.search(candidate)
        if match is None or match.group(1) in CONTROL_NAMES:
            continue
        closing = matching_brace(masked, opening)
        if closing is None:
            continue
        spans.append(FunctionSpan(match.group(1),
                                  boundary + 1 + match.start(),
                                  opening,
                                  closing))
    return spans


def line_for_offset(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def owner_for_offset(spans: list[FunctionSpan], offset: int) -> str | None:
    owners = [span for span in spans if span.body_start < offset < span.body_end]
    if not owners:
        return None
    return min(owners, key=lambda span: span.body_end - span.body_start).name


def finding(path: str, text: str, offset: int, term: str, reason: str) -> dict[str, object]:
    return {
        "path": path,
        "line": line_for_offset(text, offset),
        "term": term,
        "reason": reason,
    }


def audit(root: Path) -> dict[str, object]:
    findings: list[dict[str, object]] = []
    callsemantic_path = root / "dev/src/callsemantic.cpp"
    callsemantic = callsemantic_path.read_text(encoding="utf-8")
    structural_spans = [
        span for span in function_spans(callsemantic)
        if span.name == STRUCTURAL_WALK
    ]
    if len(structural_spans) != 1:
        findings.append({
            "path": "dev/src/callsemantic.cpp",
            "line": 0,
            "term": STRUCTURAL_WALK,
            "reason": "expected exactly one structural source-context index walk",
        })
    else:
        span = structural_spans[0]
        body = callsemantic[span.body_start:span.body_end]
        if ".children" not in body or body.count(STRUCTURAL_WALK + "(") != 1:
            findings.append(finding(
                "dev/src/callsemantic.cpp",
                callsemantic,
                span.signature_start,
                STRUCTURAL_WALK,
                "source-context index is no longer one recursive structural walk",
            ))
        for term in SEMANTIC_CALLS_FORBIDDEN_IN_STRUCTURAL_WALK:
            offset = body.find(term)
            if offset >= 0:
                findings.append(finding(
                    "dev/src/callsemantic.cpp",
                    callsemantic,
                    span.body_start + offset,
                    term,
                    "structural source-context walk performs semantic analysis",
                ))

    for relative, allowed_owners in ALLOWED_TEMPLATE_PARAMETER_ANALYSIS_OWNERS.items():
        path = root / relative
        text = path.read_text(encoding="utf-8")
        masked = mask_cpp(text)
        spans = function_spans(text)
        for match in re.finditer(r"\bparse_template_parameters\s*\(", masked):
            line_start = masked.rfind("\n", 0, match.start()) + 1
            if masked[line_start:match.start()].strip() == "bool":
                continue
            owner = owner_for_offset(spans, match.start())
            if owner not in allowed_owners:
                findings.append(finding(
                    relative,
                    text,
                    match.start(),
                    "parse_template_parameters(",
                    f"template parameters are reanalyzed from unapproved owner {owner!r}",
                ))

    for relative in RENDERER_PATHS:
        path = root / relative
        if not path.exists():
            continue
        text = path.read_text(encoding="utf-8")
        for span in function_spans(text):
            signature = text[span.signature_start:span.body_start]
            body = text[span.body_start:span.body_end]
            if ("CppAstNode" in signature and
                    ".children" in body and
                    span.name + "(" in body):
                findings.append(finding(
                    relative,
                    text,
                    span.signature_start,
                    span.name,
                    "witness renderer contains a recursive AST walk",
                ))

    return {
        "schema_version": 1,
        "obligations": {
            "template_source_context_index_walk": len(structural_spans),
        },
        "findings": findings,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
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
