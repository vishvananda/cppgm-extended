#!/usr/bin/env python3
"""Keep witness session stores and destructive boundaries explicitly owned.

Phase 6 intentionally retains a small number of session-scoped semantic
stores.  This audit prevents that inventory from growing silently, keeps each
store within its measured producer/consumer boundary, and rejects generic
destructive arbitration in the append-only source ledger and renderers.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path


SESSION_FIELDS = {
    "primary_source_file": {
        "dev/src/template_witness.h",
        "dev/src/callsemantic.cpp",
        "dev/src/witness_api.h",
        "dev/src/witness_provenance.cpp",
    },
    "lifecycle_events": {
        "dev/src/template_witness.h",
        "dev/src/template_text_output.cpp",
        "dev/src/callsemantic.cpp",
    },
    "lifecycle_transition_states": {
        "dev/src/template_witness.h",
        "dev/src/template_api.cpp",
    },
    "public_source_definition_dependencies": {
        "dev/src/template_witness.h",
        "dev/src/template_api.cpp",
        "dev/src/semantic_conversion.cpp",
    },
    "source_use_table": {
        "dev/src/template_witness.h",
        "dev/src/witness_api.cpp",
        "dev/src/semantic_template_function.cpp",
        "dev/src/template_witness_renderer.cpp",
    },
    "variable_source_use_results": {
        "dev/src/template_witness.h",
        "dev/src/witness_api.cpp",
        "dev/src/template_instantiation.cpp",
    },
    "inline_namespace_names": {
        "dev/src/template_witness.h",
        "dev/src/template_text_output.cpp",
        "dev/src/template_witness_renderer.cpp",
    },
    "template_body_ranges": {
        "dev/src/template_witness.h",
        "dev/src/template_api.cpp",
    },
    "template_header_contexts": {
        "dev/src/template_witness.h",
        "dev/src/template_api.cpp",
    },
    "class_source_occurrences": {
        "dev/src/template_witness.h",
        "dev/src/callsemantic.cpp",
        "dev/src/template_resolution.cpp",
        "dev/src/template_argument_semantics.cpp",
        "dev/src/callsemantic/class_template_reference.cpp",
    },
    "retained_enum_value_bindings": {
        "dev/src/template_witness.h",
        "dev/src/template_api.cpp",
        "dev/src/template_argument_semantics.cpp",
    },
}

REMOVED_LIFECYCLE_ADAPTER = "template_witness_lifecycle_events_by_origin"


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


def session_member_names(text: str) -> set[str]:
    masked = mask_cpp(text)
    match = re.search(r"\bstruct\s+TemplateWitnessSession\s*\{", masked)
    if match is None:
        return set()
    opening = masked.find("{", match.start())
    closing = matching_brace(masked, opening)
    if closing is None:
        return set()
    body = masked[opening + 1:closing]
    members: set[str] = set()
    depth = 0
    start = 0
    for index, char in enumerate(body):
        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
        elif char == ";" and depth == 0:
            declaration = body[start:index].strip()
            start = index + 1
            if "{" in declaration:
                continue
            name = re.search(r"([A-Za-z_][A-Za-z0-9_]*)\s*$", declaration)
            if name:
                members.add(name.group(1))
    return members


def line_for_offset(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def add_finding(findings: list[dict[str, object]], path: str, text: str,
                offset: int, term: str, reason: str) -> None:
    findings.append({
        "path": path,
        "line": line_for_offset(text, offset),
        "term": term,
        "reason": reason,
    })


def read_source(root: Path, relative: str) -> str:
    path = root / relative
    return path.read_text(encoding="utf-8") if path.exists() else ""


def exact_count(text: str, term: str) -> int:
    return len(re.findall(re.escape(term), mask_cpp(text)))


def audit(root: Path) -> dict[str, object]:
    findings: list[dict[str, object]] = []
    header_path = "dev/src/template_witness.h"
    header = read_source(root, header_path)
    actual_fields = session_member_names(header)
    expected_fields = set(SESSION_FIELDS)
    for field in sorted(actual_fields - expected_fields):
        findings.append({
            "path": header_path,
            "line": 0,
            "term": field,
            "reason": "unclassified TemplateWitnessSession store",
        })
    for field in sorted(expected_fields - actual_fields):
        findings.append({
            "path": header_path,
            "line": 0,
            "term": field,
            "reason": "classified TemplateWitnessSession store is missing",
        })

    source_files = sorted(
        path for path in (root / "dev/src").rglob("*")
        if path.is_file() and path.suffix in {".h", ".cpp"}
    ) if (root / "dev/src").exists() else []
    observed_owners: dict[str, set[str]] = {
        field: ({header_path} if field in actual_fields else set())
        for field in SESSION_FIELDS
    }
    for path in source_files:
        relative = path.relative_to(root).as_posix()
        text = path.read_text(encoding="utf-8")
        masked = mask_cpp(text)
        for field, allowed_paths in SESSION_FIELDS.items():
            receiver = (
                r"\b(?:session|witness_session|template_witness_session_)"
                if field == "primary_source_file" else r""
            )
            match = re.search(
                rf"{receiver}(?:->|\.)\s*{re.escape(field)}\b",
                masked,
            )
            if match is None:
                continue
            observed_owners[field].add(relative)
            if relative not in allowed_paths:
                add_finding(
                    findings,
                    relative,
                    text,
                    match.start(),
                    field,
                    "session store escaped its classified owner boundary",
                )

    source_ledger_path = "dev/src/semantic_source_use.h"
    source_ledger = read_source(root, source_ledger_path)
    required_append = "table.uses.push_back(use);"
    if exact_count(source_ledger, required_append) != 1:
        findings.append({
            "path": source_ledger_path,
            "line": 0,
            "term": required_append,
            "reason": "source ledger is no longer one append-only publication boundary",
        })
    for match in re.finditer(r"\btable\.uses\.(?:erase|clear)\s*\(",
                             mask_cpp(source_ledger)):
        add_finding(
            findings,
            source_ledger_path,
            source_ledger,
            match.start(),
            match.group(0),
            "source ledger contains destructive arbitration",
        )

    text_output_path = "dev/src/template_text_output.cpp"
    text_output = read_source(root, text_output_path)
    masked_text_output = mask_cpp(text_output)
    lifecycle_size_scans = exact_count(
        text_output,
        "session.lifecycle_events.size()",
    )
    lifecycle_index_reads = exact_count(
        text_output,
        "session.lifecycle_events[i]",
    )
    if lifecycle_size_scans != 1 or lifecycle_index_reads != 1:
        findings.append({
            "path": text_output_path,
            "line": 0,
            "term": "session.lifecycle_events",
            "reason": "closure lifecycle analysis is no longer one aggregate session scan",
        })
    if REMOVED_LIFECYCLE_ADAPTER in masked_text_output or \
            REMOVED_LIFECYCLE_ADAPTER in mask_cpp(header):
        findings.append({
            "path": text_output_path,
            "line": 0,
            "term": REMOVED_LIFECYCLE_ADAPTER,
            "reason": "one-use lifecycle filtering adapter was restored",
        })

    renderer_path = "dev/src/template_witness_renderer.cpp"
    renderer = read_source(root, renderer_path)
    renderer_destructive = list(re.finditer(
        r"\b(?:aliases|events|closure_events|source_events|public_seen)"
        r"\.(erase|clear)\s*\(",
        mask_cpp(renderer),
    ))
    allowed_renderer_erase = "aliases.erase(found)"
    for match in renderer_destructive:
        statement = mask_cpp(renderer)[match.start():match.end() + 32]
        if allowed_renderer_erase not in statement:
            add_finding(
                findings,
                renderer_path,
                renderer,
                match.start(),
                match.group(0),
                "unnamed destructive renderer arbitration",
            )
    if exact_count(renderer, allowed_renderer_erase) != 1:
        findings.append({
            "path": renderer_path,
            "line": 0,
            "term": allowed_renderer_erase,
            "reason": "named ambiguous-qualified-alias obligation changed",
        })

    compact_projection = "public_seen.insert(public_key).second"
    compact_comment = "Compact closure rows intentionally omit provenance"
    if exact_count(text_output, compact_projection) != 1 or \
            compact_comment not in text_output:
        findings.append({
            "path": text_output_path,
            "line": 0,
            "term": compact_projection,
            "reason": "compact lifecycle coalescing lost its named presentation obligation",
        })

    transition_mutation = re.search(
        r"lifecycle_transition_states\s*\.\s*(?:erase|clear)\s*\(",
        "\n".join(mask_cpp(path.read_text(encoding="utf-8"))
                  for path in source_files),
    )
    if transition_mutation:
        findings.append({
            "path": "dev/src",
            "line": 0,
            "term": transition_mutation.group(0),
            "reason": "lifecycle transition ownership must remain monotonic",
        })

    callsemantic_path = "dev/src/callsemantic.cpp"
    callsemantic = read_source(root, callsemantic_path)
    witness_api_path = "dev/src/witness_api.cpp"
    witness_api = read_source(root, witness_api_path)
    destructive_obligations = {
        "alias_terminal_idempotence_release": exact_count(
            callsemantic, "completed_alias_source_occurrences_.clear()"
        ),
        "class_terminal_result_release": (
            exact_count(callsemantic, "pending_class_uses.clear()") +
            exact_count(callsemantic, "pending_class_use_indices.clear()")
        ),
        "variable_terminal_result_release": exact_count(
            witness_api, "variable_source_use_results.clear()"
        ),
        "ambiguous_qualified_alias_rejection": exact_count(
            renderer, allowed_renderer_erase
        ),
        "compact_lifecycle_projection_coalescing": exact_count(
            text_output, compact_projection
        ),
    }
    expected_destructive_counts = {
        "alias_terminal_idempotence_release": 1,
        "class_terminal_result_release": 2,
        "variable_terminal_result_release": 1,
        "ambiguous_qualified_alias_rejection": 1,
        "compact_lifecycle_projection_coalescing": 1,
    }
    if "completed_alias_source_occurrences_.erase(" in mask_cpp(callsemantic):
        findings.append({
            "path": callsemantic_path,
            "line": 0,
            "term": "completed_alias_source_occurrences_.erase(",
            "reason": "zero-obligation alias completion rollback was restored",
        })
    for name, expected in expected_destructive_counts.items():
        if destructive_obligations[name] != expected:
            findings.append({
                "path": "dev/src",
                "line": 0,
                "term": name,
                "reason": (
                    "named destructive ownership boundary count changed: "
                    f"expected {expected}, observed {destructive_obligations[name]}"
                ),
            })

    return {
        "schema_version": 1,
        "session_stores": {
            field: sorted(observed_owners[field])
            for field in sorted(observed_owners)
        },
        "obligations": {
            "renderer_lifecycle_session_scans": lifecycle_size_scans,
            **destructive_obligations,
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
