#!/usr/bin/env python3
"""Classify strict witness mismatches and correlate semantic provenance."""

from __future__ import annotations

import argparse
import collections
import json
import pathlib
import re
import sys
from dataclasses import dataclass
from typing import Any, Iterable


STRICT_PAS = ("pa19", "pa20", "pa22", "pa23", "pa24")
SOURCE_HEADERS = {
    "alias-use": "alias_use",
    "class-use": "class_use",
    "function-call": "function_call",
    "variable-use": "variable_use",
}
LIFECYCLE_HEADERS = {
    "require-definition",
    "ensure-definition",
    "function-instantiation",
    "class-instantiation",
    "variable-instantiation",
    "class-finalization",
}
LOCATION_RE = re.compile(r"^  ([a-z-]+) at (.+)$")


@dataclass(frozen=True)
class WitnessEvent:
    family: str
    kind: str
    location: str
    template_name: str
    entity: str
    lines: tuple[str, ...]

    @property
    def block(self) -> str:
        return "\n".join(self.lines)


def parse_witness(text: str) -> list[WitnessEvent]:
    lines = text.splitlines()
    events: list[WitnessEvent] = []
    in_lifecycle = False
    index = 0
    while index < len(lines):
        line = lines[index]
        if line == "template-closure-events":
            in_lifecycle = True
            index += 1
            continue

        source_match = LOCATION_RE.match(line)
        source_kind = source_match.group(1) if source_match else ""
        lifecycle_kind = (
            line[2:] if in_lifecycle and line.startswith("  ") else ""
        )
        if source_kind in SOURCE_HEADERS:
            start = index
            index += 1
            while index < len(lines) and lines[index].startswith("    "):
                index += 1
            block_lines = tuple(lines[start:index])
            template_name = _detail_value(block_lines, "template ")
            if source_kind == "function-call" and not template_name:
                template_name = _detail_value(block_lines, "callee ")
            events.append(
                WitnessEvent(
                    family=SOURCE_HEADERS[source_kind],
                    kind=source_kind,
                    location=source_match.group(2),
                    template_name=template_name,
                    entity="",
                    lines=block_lines,
                )
            )
            continue
        if lifecycle_kind in LIFECYCLE_HEADERS:
            start = index
            index += 1
            while index < len(lines) and lines[index].startswith("    "):
                index += 1
            block_lines = tuple(lines[start:index])
            events.append(
                WitnessEvent(
                    family="lifecycle",
                    kind=lifecycle_kind.replace("-", "_"),
                    location=_detail_value(block_lines, "at "),
                    template_name="",
                    entity=_detail_value(block_lines, "entity "),
                    lines=block_lines,
                )
            )
            continue
        index += 1
    return events


def _detail_value(lines: Iterable[str], prefix: str) -> str:
    marker = "    " + prefix
    for line in lines:
        if line.startswith(marker):
            return line[len(marker) :]
    return ""


def _subtract_exact(
    expected: list[WitnessEvent], actual: list[WitnessEvent]
) -> tuple[list[WitnessEvent], list[WitnessEvent]]:
    actual_counts = collections.Counter(event.block for event in actual)
    remaining_expected: list[WitnessEvent] = []
    for event in expected:
        if actual_counts[event.block]:
            actual_counts[event.block] -= 1
        else:
            remaining_expected.append(event)

    expected_counts = collections.Counter(event.block for event in expected)
    remaining_actual: list[WitnessEvent] = []
    for event in actual:
        if expected_counts[event.block]:
            expected_counts[event.block] -= 1
        else:
            remaining_actual.append(event)
    return remaining_expected, remaining_actual


def _pair_score(expected: WitnessEvent, actual: WitnessEvent) -> int:
    if expected.family != actual.family:
        return 0
    if expected.family == "lifecycle":
        if expected.kind == actual.kind and expected.entity == actual.entity:
            return 5
        if (
            expected.kind == actual.kind
            and expected.location
            and expected.location == actual.location
        ):
            return 4
        return 0
    if expected.location != actual.location:
        return 0
    if expected.template_name == actual.template_name:
        return 5
    if expected.kind == actual.kind:
        return 4
    return 0


def classify_events(
    expected: list[WitnessEvent], actual: list[WitnessEvent]
) -> list[dict[str, Any]]:
    missing, extra = _subtract_exact(expected, actual)
    occurrences: list[dict[str, Any]] = []
    paired_actual: set[int] = set()
    paired_expected: set[int] = set()
    candidates: list[tuple[int, int, int]] = []
    for expected_index, expected_event in enumerate(missing):
        for actual_index, actual_event in enumerate(extra):
            score = _pair_score(expected_event, actual_event)
            if score:
                candidates.append((-score, expected_index, actual_index))
    for _, expected_index, actual_index in sorted(candidates):
        if expected_index in paired_expected or actual_index in paired_actual:
            continue
        paired_expected.add(expected_index)
        paired_actual.add(actual_index)
        occurrences.append(
            _occurrence("changed", missing[expected_index], extra[actual_index])
        )
    for index, event in enumerate(missing):
        if index not in paired_expected:
            occurrences.append(_occurrence("missing_expected", event, None))
    for index, event in enumerate(extra):
        if index not in paired_actual:
            occurrences.append(_occurrence("unexpected_actual", None, event))
    return sorted(
        occurrences,
        key=lambda item: (
            item["family"],
            item["location"],
            item["template_name"],
            item["classification"],
        ),
    )


def _occurrence(
    classification: str,
    expected: WitnessEvent | None,
    actual: WitnessEvent | None,
) -> dict[str, Any]:
    event = expected or actual
    assert event is not None
    expectation = {
        "missing_expected": "presence",
        "unexpected_actual": "absence",
        "changed": "content",
        "ordering_only": "ordering",
    }[classification]
    return {
        "classification": classification,
        "expectation": expectation,
        "family": event.family,
        "kind": event.kind,
        "location": event.location,
        "template_name": event.template_name,
        "entity": event.entity,
        "expected": expected.block if expected else "",
        "actual": actual.block if actual else "",
    }


def _test_path_from_reference(reference: pathlib.Path) -> pathlib.Path:
    suffix = ".ref.witness"
    return reference.with_name(reference.name[: -len(suffix)] + ".t")


def _actual_path(reference: pathlib.Path) -> pathlib.Path:
    suffix = ".ref.witness"
    return reference.with_name(reference.name[: -len(suffix)] + ".my.witness")


def _strict_references(
    repo_root: pathlib.Path, strict_pas: Iterable[str]
) -> list[pathlib.Path]:
    references: list[pathlib.Path] = []
    for assignment in strict_pas:
        references.extend((repo_root / assignment / "tests").rglob("*.ref.witness"))
    return sorted(references)


def _same_location(lhs: str, rhs: str) -> bool:
    return bool(lhs and rhs) and (lhs == rhs or lhs.endswith(rhs) or rhs.endswith(lhs))


def _same_template(lhs: str, rhs: str) -> bool:
    if not lhs or not rhs:
        return True
    return lhs == rhs or lhs.split("::")[-1] == rhs.split("::")[-1]


def _record_belongs_to_test(record: dict[str, Any], test: str) -> bool:
    source = str(record.get("source", ""))
    assignment = test.split("/", 1)[0]
    test_name = pathlib.Path(test).name
    if f"/{assignment}/" in source and test_name in source:
        return True
    location = str(record.get("location", ""))
    location_assignments = {
        candidate
        for candidate in STRICT_PAS
        if f"/{candidate}/" in location
    }
    if location_assignments:
        return assignment in location_assignments and test_name in location
    return test_name in location and not any(
        f"/{other}/" in source for other in STRICT_PAS if other != assignment
    )


def correlate_provenance(
    occurrence: dict[str, Any],
    test: str,
    provenance: dict[str, Any],
) -> dict[str, Any]:
    family = occurrence["family"]
    location = occurrence["location"]
    template_name = occurrence["template_name"]
    source_attempts: list[dict[str, Any]] = []
    for decision in provenance.get("source_attempt_decisions", []):
        if decision.get("kind") != family:
            continue
        if not _record_belongs_to_test(decision, test):
            continue
        if location and not _same_location(location, str(decision.get("location", ""))):
            continue
        if not _same_template(template_name, str(decision.get("template_name", ""))):
            continue
        source_attempts.append(decision)

    alias_completions: list[dict[str, Any]] = []
    if family == "alias_use":
        for decision in provenance.get("alias_completion_decisions", []):
            if not _record_belongs_to_test(decision, test):
                continue
            if location and not _same_location(
                location, str(decision.get("location", ""))
            ):
                continue
            if not _same_template(
                template_name, str(decision.get("source_template_name", ""))
            ):
                continue
            alias_completions.append(decision)

    class_materialization: list[dict[str, Any]] = []
    if family == "class_use":
        for decision in provenance.get("class_materialization_decisions", []):
            if not _record_belongs_to_test(decision, test):
                continue
            if location and not _same_location(
                location, str(decision.get("location", ""))
            ):
                continue
            if not _same_template(
                template_name, str(decision.get("template_name", ""))
            ):
                continue
            class_materialization.append(decision)

    lifecycle_attempts: list[dict[str, Any]] = []
    if family == "lifecycle":
        for decision in provenance.get("lifecycle_attempt_decisions", []):
            if not _record_belongs_to_test(decision, test):
                continue
            if decision.get("kind") != occurrence["kind"]:
                continue
            entity = occurrence["entity"]
            if entity and entity != decision.get("entity"):
                continue
            lifecycle_attempts.append(decision)

    routes = sorted(
        {
            str(item.get("upstream_route", "unknown"))
            for item in source_attempts
        }
    )
    producers = sorted(
        {str(item.get("producer", "unknown")) for item in source_attempts}
    )
    operations = sorted(
        {str(item.get("operation", "unknown")) for item in alias_completions}
    )
    typed_owners = sorted(
        {str(item.get("typed_owner", "none")) for item in class_materialization}
    )
    if not routes and alias_completions:
        routes = sorted(
            {
                f"alias.operation:{item.get('operation', 'unknown')}"
                for item in alias_completions
            }
        )
    if not routes and class_materialization:
        routes = sorted(
            {
                "class.materialization:"
                f"{item.get('typed_owner', 'none')}:"
                f"{'admitted' if item.get('typed_materialization') else 'rejected'}"
                for item in class_materialization
            }
        )
    if not routes and lifecycle_attempts:
        routes = sorted(
            {
                "lifecycle:"
                f"entry_origin={item.get('entry_origin', 0)}:"
                f"cause={item.get('cause', 0)}:"
                f"closure_reason={item.get('closure_reason', 0)}"
                for item in lifecycle_attempts
            }
        )
        producers = sorted(
            {str(item.get("producer", "unknown")) for item in lifecycle_attempts}
        )
    return {
        "semantic_routes": routes,
        "producers": producers,
        "alias_operations": operations,
        "class_materialization_owners": typed_owners,
        "source_attempts": source_attempts,
        "alias_completions": alias_completions,
        "class_materialization": class_materialization,
        "lifecycle_attempts": lifecycle_attempts,
    }


def audit_class_materialization_candidates(
    expected_by_test: dict[str, list[WitnessEvent]],
    provenance: dict[str, Any],
) -> tuple[dict[str, int], list[dict[str, Any]]]:
    tests_by_name: dict[str, list[str]] = collections.defaultdict(list)
    for test in expected_by_test:
        tests_by_name[pathlib.Path(test).name].append(test)

    grouped: dict[tuple[str, str, str, bool], dict[str, Any]] = {}
    candidate_decisions = 0
    for decision in provenance.get("class_materialization_decisions", []):
        operation = str(decision.get("active_operation", "none"))
        semantic_owner_kind = str(
            decision.get("semantic_owner_state", {}).get(
                "semantic_owner_kind", "none"
            )
        )
        materialization_operation = operation in {
            "source_type_node",
            "static_member_initializer",
            "variable_template_initializer",
        }
        owner_matches_operation = (
            operation == "source_type_node"
            and semantic_owner_kind in {"declaration_type", "function_body"}
        ) or operation == semantic_owner_kind
        candidate = (
            int(decision.get("source_dependency", -1)) > 0
            and bool(decision.get("semantic_owner_committed", False))
            and materialization_operation
            and owner_matches_operation
            and bool(decision.get("exact_source_node", False))
        )
        if not candidate:
            continue
        candidate_decisions += 1
        location = str(decision.get("location", ""))
        template_name = str(decision.get("template_name", ""))
        source_name = pathlib.Path(location.rsplit(":", 2)[0]).name
        matching_tests = [
            test
            for test in tests_by_name.get(source_name, [])
            if _record_belongs_to_test(decision, test)
        ]
        test = matching_tests[0] if len(matching_tests) == 1 else ""
        expected_event = next(
            (
                event
                for event in expected_by_test.get(test, [])
                if event.family == "class_use"
                and _same_location(event.location, location)
                and _same_template(event.template_name, template_name)
            ),
            None,
        )
        expected = expected_event is not None
        display_location = expected_event.location if expected_event else location
        display_template = (
            expected_event.template_name if expected_event else template_name
        )
        key = (test, display_location, display_template, expected)
        item = grouped.setdefault(
            key,
            {
                "test": test,
                "location": display_location,
                "template_name": display_template,
                "patched_clang_presence": expected,
                "decision_count": 0,
                "typed_materialization_count": 0,
                "owners": set(),
                "operations": set(),
            },
        )
        item["decision_count"] += 1
        item["typed_materialization_count"] += int(
            bool(decision.get("typed_materialization", False))
        )
        item["owners"].add(str(decision.get("active_owner", "none")))
        item["operations"].add(operation)

    candidates: list[dict[str, Any]] = []
    for item in grouped.values():
        copied = dict(item)
        copied["owners"] = sorted(item["owners"])
        copied["operations"] = sorted(item["operations"])
        candidates.append(copied)
    candidates.sort(
        key=lambda item: (
            item["test"],
            item["location"],
            item["template_name"],
        )
    )
    return (
        {
            "candidate_decisions": candidate_decisions,
            "candidate_occurrences": len(candidates),
            "patched_clang_present_occurrences": sum(
                int(item["patched_clang_presence"]) for item in candidates
            ),
            "patched_clang_absent_occurrences": sum(
                int(not item["patched_clang_presence"]) for item in candidates
            ),
        },
        candidates,
    )


def build_report(
    repo_root: pathlib.Path,
    strict_pas: Iterable[str] = STRICT_PAS,
    provenance: dict[str, Any] | None = None,
) -> dict[str, Any]:
    provenance = provenance or {}
    references = _strict_references(repo_root, strict_pas)
    tests: list[dict[str, Any]] = []
    family_counts: dict[str, collections.Counter[str]] = collections.defaultdict(
        collections.Counter
    )
    owner_counts: collections.Counter[tuple[str, str]] = collections.Counter()
    matching = 0
    missing_actual_files = 0
    expected_by_test: dict[str, list[WitnessEvent]] = {}

    for reference in references:
        actual = _actual_path(reference)
        expected_text = reference.read_text(encoding="utf-8")
        actual_text = actual.read_text(encoding="utf-8") if actual.exists() else ""
        relative_test = str(_test_path_from_reference(reference).relative_to(repo_root))
        expected_events = parse_witness(expected_text)
        expected_by_test[relative_test] = expected_events
        if expected_text == actual_text:
            matching += 1
            continue
        if not actual.exists():
            missing_actual_files += 1
        actual_events = parse_witness(actual_text)
        occurrences = classify_events(expected_events, actual_events)
        ordering_only = not occurrences and expected_text != actual_text
        if ordering_only:
            actual_blocks = [event.block for event in actual_events]
            for index, expected_event in enumerate(expected_events):
                if index < len(actual_blocks) and (
                    expected_event.block == actual_blocks[index]
                ):
                    continue
                actual_event = next(
                    (
                        event
                        for event in actual_events
                        if event.block == expected_event.block
                    ),
                    expected_event,
                )
                occurrences.append(
                    _occurrence("ordering_only", expected_event, actual_event)
                )
                break
        test_families: set[str] = set()
        for occurrence in occurrences:
            occurrence["provenance"] = correlate_provenance(
                occurrence, relative_test, provenance
            )
            family = occurrence["family"]
            test_families.add(family)
            family_counts[family][occurrence["classification"]] += 1
            routes = occurrence["provenance"]["semantic_routes"] or [
                "no_matching_attempt"
            ]
            for route in routes:
                owner_counts[(family, route)] += 1
        for family in test_families:
            family_counts[family]["tests"] += 1
        tests.append(
            {
                "test": relative_test,
                "reference": str(reference.relative_to(repo_root)),
                "actual": str(actual.relative_to(repo_root)),
                "families": sorted(test_families),
                "ordering_only": ordering_only,
                "occurrences": occurrences,
            }
        )

    family_summary = {
        family: dict(sorted(counts.items()))
        for family, counts in sorted(family_counts.items())
    }
    ownership_summary = [
        {"family": key[0], "route": key[1], "occurrences": count}
        for key, count in sorted(owner_counts.items())
    ]
    materialization_summary, materialization_candidates = (
        audit_class_materialization_candidates(expected_by_test, provenance)
    )
    return {
        "schema_version": 1,
        "references": len(references),
        "matching_outputs": matching,
        "mismatching_outputs": len(tests),
        "missing_actual_files": missing_actual_files,
        "family_summary": family_summary,
        "ownership_summary": ownership_summary,
        "class_materialization_candidate_summary": materialization_summary,
        "class_materialization_candidates": materialization_candidates,
        "tests": tests,
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo-root", type=pathlib.Path, default=pathlib.Path.cwd())
    parser.add_argument("--strict-pas", nargs="+", default=list(STRICT_PAS))
    parser.add_argument("--provenance-report", type=pathlib.Path)
    parser.add_argument("--output", type=pathlib.Path)
    args = parser.parse_args()
    provenance: dict[str, Any] = {}
    if args.provenance_report:
        provenance = json.loads(args.provenance_report.read_text(encoding="utf-8"))
    report = build_report(args.repo_root.resolve(), args.strict_pas, provenance)
    text = json.dumps(report, indent=2, sort_keys=True) + "\n"
    if args.output:
        args.output.write_text(text, encoding="utf-8")
    else:
        sys.stdout.write(text)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
