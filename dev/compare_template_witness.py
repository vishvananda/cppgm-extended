#!/usr/bin/env python3

import argparse
import json
import os
import re
import sys
from typing import Dict, List, Optional, Pattern, Tuple


def load_json(path: str) -> Dict:
    with open(path, "r", encoding="utf-8") as handle:
        return json.load(handle)


def short_location(location: Optional[str]) -> Optional[str]:
    if not location:
        return None
    if ":" not in location:
        return os.path.basename(location)
    parts = location.rsplit(":", 2)
    if len(parts) == 3:
        path, line, column = parts
        return os.path.basename(path) + ":" + line + ":" + column
    return os.path.basename(location)


def canonical_arg(arg: str) -> str:
    value = canonical_text(arg)
    value = re.sub(r"((?:^|::)plus)<void>$", r"\1<>", value)
    literal_match = re.match(r"^(\d+)[uUlL]*$", value)
    if literal_match:
        return literal_match.group(1)
    typed_literal_match = re.match(r"^(?:int|unsigned|signed|long|longlong|unsignedlong|unsignedlonglong):(\d+)$",
                                   value)
    if typed_literal_match:
        return typed_literal_match.group(1)
    if value in ("[]", "<>"):
        return "<>"
    if value.startswith("<") and value.endswith(">") and len(value) > 2:
        return canonical_text(value[1:-1])
    if value.startswith("[") and value.endswith("]") and len(value) > 2:
        return canonical_text(value[1:-1])
    if re.match(r"^&[A-Za-z_][A-Za-z0-9_:]*$", value):
        return value[1:]
    return value


def canonical_drop_candidate(text: Optional[str]) -> str:
    value = canonical_text(text)
    if "::" in value:
        value = value.split("::")[-1]
    return value


def canonical_drop_reason(text: Optional[str]) -> str:
    value = canonical_text(text)
    if value in ("deduce-finalize-failed", "deduction-failed"):
        return "substitution_failure"
    return value


def canonical_text(text: Optional[str]) -> str:
    if not text:
        return ""
    value = text
    value = re.sub(r"((?:/[A-Za-z0-9_+.\-]+)+:\d+:\d+)",
                   lambda match: normalize_path_fragment(match.group(1)),
                   value)
    value = value.replace("std::__1::", "std::")
    value = re.sub(r"\b(struct|class|typename)\s+", "", value)
    value = re.sub(r"__local_\d+", "", value)
    value = re.sub(r"::+", "::", value)
    if "__local_" in text and not value.startswith("std::") and "::" in value:
        value = value.split("::")[-1]
    value = value.replace(" ", "")
    value = value.replace("longint", "long")
    value = value.replace("unsignedint", "unsigned")
    value = value.replace("signedint", "signed")
    value = value.replace("longlongint", "longlong")
    value = re.sub(r"([A-Za-z_][A-Za-z0-9_:]*)const([*&])", r"const\1\2", value)
    value = re.sub(r"\b([A-Za-z_][A-Za-z0-9_]*)::(value_type|iterator|const_iterator|reference|const_reference)\b",
                   r"\2",
                   value)
    return value


def normalize_path_fragment(path: str) -> str:
    value = path.replace("\\", "/")
    libcxx_marker = "/include/c++/v1/"
    libcxx_pos = value.find(libcxx_marker)
    if libcxx_pos != -1:
        return "libc++/" + value[libcxx_pos + len(libcxx_marker):]
    match = re.search(r"(pa\d+/(?:tests|course)/.*)$", value)
    if match:
        return match.group(1)
    return value


def bindings_map(event: Dict) -> Dict[str, str]:
    out: Dict[str, str] = {}
    for binding in event.get("bindings", []):
        param = binding.get("param")
        arg = binding.get("arg")
        if param and arg is not None:
            out[param] = canonical_arg(arg)
    return out


def specialization_bindings_map(event: Dict) -> Dict[str, str]:
    out: Dict[str, str] = {}
    for binding in event.get("specialization_bindings", []):
        param = binding.get("param")
        arg = binding.get("arg")
        if param and arg is not None:
            out[param] = canonical_arg(arg)
    return out


def normalized_drops(event: Dict) -> List[Tuple[str, str]]:
    drops: List[Tuple[str, str]] = []
    for drop in event.get("drops", []):
        candidate = canonical_drop_candidate(drop.get("candidate"))
        reason = canonical_drop_reason(drop.get("reason"))
        if candidate or reason:
            drops.append((candidate, reason))
    drops.sort()
    return drops


def comparable_drops(event: Dict) -> List[Tuple[str, str]]:
    drops = event.get("drops", [])
    selected = event.get("selected") or ""
    if selected.endswith("operator>>") or selected.endswith("operator<<"):
        filtered = [drop for drop in drops if drop[1] == "substitution_failure"]
        if filtered:
            return sorted(set(filtered))
    return drops


def compiled_pattern(text: Optional[str]) -> Optional[Pattern[str]]:
    if not text:
        return None
    return re.compile(text)


def event_matches(event: Dict,
                  kind_filters: Optional[set],
                  symbol_pattern: Optional[Pattern[str]]) -> bool:
    if kind_filters and event.get("kind") not in kind_filters:
        return False
    if symbol_pattern is None:
        return True
    haystacks = [
        event.get("selected") or "",
        event.get("template") or "",
        event.get("location") or "",
        event.get("selected_decl_location") or ""
    ]
    return any(symbol_pattern.search(text) for text in haystacks if text)


def normalized_events(document: Dict,
                      kind_filters: Optional[set] = None,
                      symbol_pattern: Optional[Pattern[str]] = None) -> List[Dict]:
    out = []
    for event in document.get("events", []):
        if not event_matches(event, kind_filters, symbol_pattern):
            continue
        binding_map = bindings_map(event)
        out.append({
            "kind": event.get("kind"),
            "template": canonical_text(event.get("template")),
            "selected": canonical_text(event.get("selected")),
            "selection": event.get("selection"),
            "location": short_location(event.get("location")),
            "selected_decl_location": short_location(event.get("selected_decl_location")),
            "bindings": binding_map,
            "binding_signature": tuple(sorted(binding_map.items())),
            "specialization_bindings": specialization_bindings_map(event),
            "expanded_to": canonical_text(event.get("expanded_to")),
            "value": canonical_text(str(event.get("value"))) if event.get("value") is not None else "",
            "drops": normalized_drops(event)
        })
    return out


def normalized_closure_events(document: Dict) -> List[Dict]:
    out = []
    for event in document.get("closure_events", []):
        out.append({
            "kind": event.get("kind"),
            "entity": canonical_text(event.get("entity")),
            "location": short_location(event.get("location")),
            "decl_location": short_location(event.get("decl_location")),
            "reason": canonical_text(event.get("reason")),
            "trigger": canonical_text(event.get("trigger")),
            "trigger_decl": short_location(event.get("trigger_decl")),
            "detail": canonical_text(event.get("detail")),
        })
    return out


def event_identity(event: Dict) -> Tuple[str, str, str]:
    kind = event.get("kind") or ""
    if kind in ("class_use", "alias_use", "variable_use"):
        return (
            kind,
            event.get("selected") or "",
            event.get("template") or "",
            repr(event.get("binding_signature") or ())
        )
    return (
        kind,
        event.get("selected") or "",
        event.get("template") or ""
    )


def closure_event_identity(event: Dict) -> Tuple[str, str, str, str]:
    return (
        event.get("kind") or "",
        event.get("entity") or "",
        event.get("trigger") or "",
        event.get("reason") or "",
    )


def compare_documents(lhs: Dict,
                      rhs: Dict,
                      kind_filters: Optional[set] = None,
                      symbol_pattern: Optional[Pattern[str]] = None,
                      compare_decls: bool = False) -> Tuple[List[str], List[str]]:
    issues: List[str] = []
    notes: List[str] = []
    left_events = normalized_events(lhs, kind_filters, symbol_pattern)
    right_events = normalized_events(rhs, kind_filters, symbol_pattern)

    notes.append("left events=%d right events=%d" % (len(left_events), len(right_events)))

    unmatched_right = list(range(len(right_events)))
    remaining_left = {}
    for left_event in left_events:
        left_id = event_identity(left_event)
        remaining_left[left_id] = remaining_left.get(left_id, 0) + 1

    for left_event in left_events:
        left_id = event_identity(left_event)
        left_remaining_for_id = remaining_left.get(left_id, 0)
        candidate_indices = [
            index for index in unmatched_right
            if event_identity(right_events[index]) == left_id
        ]
        match_index = None
        if left_event.get("location"):
            for index in candidate_indices:
                if right_events[index].get("location") == left_event.get("location"):
                    match_index = index
                    break
        if match_index is None and left_event.get("selection"):
            for index in candidate_indices:
                if right_events[index].get("selection") == left_event.get("selection"):
                    match_index = index
                    break
        if match_index is None and compare_decls and left_event.get("selected_decl_location"):
            for index in candidate_indices:
                if right_events[index].get("selected_decl_location") == \
                        left_event.get("selected_decl_location"):
                    match_index = index
                    break
        if match_index is None and candidate_indices:
            match_index = candidate_indices[0]
        if match_index is not None and len(candidate_indices) < left_remaining_for_id:
            exact_location_match = left_event.get("location") and \
                right_events[match_index].get("location") == left_event.get("location")
            if not exact_location_match:
                match_index = None
        if match_index is None:
            notes.append("unmatched on right: " + " ".join(left_id))
            remaining_left[left_id] = max(0, left_remaining_for_id - 1)
            continue

        right_event = right_events[match_index]
        unmatched_right.remove(match_index)
        remaining_left[left_id] = max(0, left_remaining_for_id - 1)

        if left_event.get("selection") and right_event.get("selection") and \
                left_event["selection"] != right_event["selection"]:
            issues.append("selection mismatch for %s: left=%s right=%s" % (
                left_id, left_event["selection"], right_event["selection"]))

        if left_event.get("location") and right_event.get("location") and \
                left_event["location"] != right_event["location"]:
            issues.append("location mismatch for %s: left=%s right=%s" % (
                left_id, left_event["location"], right_event["location"]))

        if compare_decls and left_event.get("selected_decl_location") and \
                right_event.get("selected_decl_location") and \
                left_event["selected_decl_location"] != right_event["selected_decl_location"]:
            issues.append("selected_decl_location mismatch for %s: left=%s right=%s" % (
                left_id,
                left_event["selected_decl_location"],
                right_event["selected_decl_location"]))

        left_bindings = left_event.get("bindings", {})
        right_bindings = right_event.get("bindings", {})
        common_params = sorted(set(left_bindings) & set(right_bindings))
        for param in common_params:
            if left_bindings[param] != right_bindings[param]:
                issues.append("binding mismatch for %s param=%s: left=%s right=%s" % (
                    left_id, param, left_bindings[param], right_bindings[param]))

        if not common_params and left_bindings and right_bindings:
            issues.append("no comparable bindings for %s" % (left_id,))

        left_specialization_bindings = left_event.get("specialization_bindings", {})
        right_specialization_bindings = right_event.get("specialization_bindings", {})
        common_specialization_params = sorted(
            set(left_specialization_bindings) & set(right_specialization_bindings))
        for param in common_specialization_params:
            if left_specialization_bindings[param] != right_specialization_bindings[param]:
                issues.append(
                    "specialization binding mismatch for %s param=%s: left=%s right=%s" % (
                        left_id,
                        param,
                        left_specialization_bindings[param],
                        right_specialization_bindings[param]))

        if (not common_specialization_params and left_specialization_bindings and
                right_specialization_bindings):
            issues.append("no comparable specialization bindings for %s" % (left_id,))

        if left_event.get("expanded_to") and right_event.get("expanded_to") and \
                left_event["expanded_to"] != right_event["expanded_to"]:
            issues.append("expanded_to mismatch for %s: left=%s right=%s" % (
                left_id, left_event["expanded_to"], right_event["expanded_to"]))

        if left_event.get("value") and right_event.get("value") and \
                left_event["value"] != right_event["value"]:
            issues.append("value mismatch for %s: left=%s right=%s" % (
                left_id, left_event["value"], right_event["value"]))

        left_drops = comparable_drops(left_event)
        right_drops = comparable_drops(right_event)
        if left_drops and right_drops and left_drops != right_drops:
            issues.append("drop mismatch for %s: left=%s right=%s" % (
                left_id, left_drops, right_drops))

    for index in unmatched_right:
        right_event = right_events[index]
        notes.append("unmatched on left: " + " ".join(event_identity(right_event)))

    left_closure_events = normalized_closure_events(lhs)
    right_closure_events = normalized_closure_events(rhs)
    notes.append("left closure_events=%d right closure_events=%d" %
                 (len(left_closure_events), len(right_closure_events)))

    unmatched_right = list(range(len(right_closure_events)))
    for left_event in left_closure_events:
        left_id = closure_event_identity(left_event)
        candidate_indices = [
            index for index in unmatched_right
            if closure_event_identity(right_closure_events[index]) == left_id
        ]
        match_index = None
        if left_event.get("location"):
            for index in candidate_indices:
                if right_closure_events[index].get("location") == left_event.get("location"):
                    match_index = index
                    break
        if match_index is None and left_event.get("decl_location"):
            for index in candidate_indices:
                if right_closure_events[index].get("decl_location") == \
                        left_event.get("decl_location"):
                    match_index = index
                    break
        if match_index is None and candidate_indices:
            match_index = candidate_indices[0]
        if match_index is None:
            issues.append("missing closure event on right: %s" % (left_id,))
            continue

        right_event = right_closure_events[match_index]
        unmatched_right.remove(match_index)

        if left_event.get("location") and right_event.get("location") and \
                left_event["location"] != right_event["location"]:
            issues.append("closure location mismatch for %s: left=%s right=%s" % (
                left_id, left_event["location"], right_event["location"]))

        if left_event.get("decl_location") and right_event.get("decl_location") and \
                left_event["decl_location"] != right_event["decl_location"]:
            issues.append("closure decl_location mismatch for %s: left=%s right=%s" % (
                left_id, left_event["decl_location"], right_event["decl_location"]))

        if left_event.get("trigger_decl") and right_event.get("trigger_decl") and \
                left_event["trigger_decl"] != right_event["trigger_decl"]:
            issues.append("closure trigger_decl mismatch for %s: left=%s right=%s" % (
                left_id, left_event["trigger_decl"], right_event["trigger_decl"]))

        if left_event.get("detail") and right_event.get("detail") and \
                left_event["detail"] != right_event["detail"]:
            issues.append("closure detail mismatch for %s: left=%s right=%s" % (
                left_id, left_event["detail"], right_event["detail"]))

    for index in unmatched_right:
        right_event = right_closure_events[index]
        issues.append("extra closure event on right: %s" %
                      (closure_event_identity(right_event),))

    return issues, notes


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Compare two template witness JSON documents on their common student-facing fields.")
    parser.add_argument("--left", required=True, help="Left witness JSON path")
    parser.add_argument("--right", required=True, help="Right witness JSON path")
    parser.add_argument("--kind",
                        action="append",
                        default=[],
                        help="Only compare events of this kind; may be repeated")
    parser.add_argument("--symbol-filter",
                        help="Regex applied symmetrically to selected/template/location fields on both sides")
    parser.add_argument("--compare-selected-decl-location",
                        action="store_true",
                        help="Also compare selected_decl_location after path normalization")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    left = load_json(args.left)
    right = load_json(args.right)
    kind_filters = set(args.kind) if args.kind else None
    symbol_pattern = compiled_pattern(args.symbol_filter)
    issues, notes = compare_documents(left,
                                      right,
                                      kind_filters=kind_filters,
                                      symbol_pattern=symbol_pattern,
                                      compare_decls=args.compare_selected_decl_location)
    for note in notes:
        print("note:", note)
    if not issues:
        print("match: no comparable-field mismatches")
        return 0
    for issue in issues:
        print("mismatch:", issue)
    return 1


if __name__ == "__main__":
    sys.exit(main())
