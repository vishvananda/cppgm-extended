#!/usr/bin/env python3
"""Build the accepted/rejected class materialization ownership audit."""

from __future__ import annotations

import argparse
import hashlib
import json
import sys
from pathlib import Path
from typing import Any


def read_json(path: Path) -> dict[str, Any]:
    with path.open(encoding="utf-8") as stream:
        return json.load(stream)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def canonical_location(location: str) -> str:
    normalized = location.replace("\\", "/")
    for marker in ("/pa", "pa"):
        offset = normalized.find(marker)
        if offset >= 0:
            candidate = normalized[offset + (1 if marker.startswith("/") else 0) :]
            if candidate.startswith("pa"):
                return candidate
    return normalized


def event_key(event: dict[str, Any]) -> tuple[str, str]:
    return (
        canonical_location(str(event.get("location", ""))),
        str(event.get("template_name", event.get("template", ""))),
    )


def load_patched_clang_events(directory: Path) -> list[dict[str, Any]]:
    events: list[dict[str, Any]] = []
    for path in sorted(directory.glob("*.json")):
        payload = read_json(path)
        for event in payload.get("events", []):
            if event.get("kind") != "class_use":
                continue
            copied = dict(event)
            copied["artifact"] = path.name
            copied["location"] = canonical_location(str(event.get("location", "")))
            events.append(copied)
    return events


def prior_late_removed_rows(prior: dict[str, Any]) -> list[dict[str, Any]]:
    rows = prior.get("late_removed_rows")
    if isinstance(rows, list):
        return rows
    return [
        dict(item.get("prior_late_decision", item))
        for item in prior.get("rejected", [])
    ]


def prior_accepted_rows(prior: dict[str, Any]) -> list[dict[str, Any]]:
    rows = prior.get("accepted")
    return rows if isinstance(rows, list) else []


def prior_patched_clang_evidence(prior: dict[str, Any]) -> dict[str, Any]:
    evidence = prior.get("patched_clang")
    if isinstance(evidence, dict):
        return evidence

    results = prior.get("patched_clang_rejected_results", {})
    inputs = prior.get("inputs", {})
    compile_failures = sum(
        int(result.get("returncode", -1) != 0)
        for result in results.values()
    )
    class_rows = sum(
        len(result.get("class_rows_at_removed_locations", []))
        for result in results.values()
    )
    return {
        "binary": inputs.get("patched_clang_binary", ""),
        "checkout": inputs.get("patched_clang_checkout", ""),
        "class_rows_at_removed_locations": class_rows,
        "compile_failures": compile_failures,
        "results": results,
    }


def build_audit(
    prior_audit_path: Path,
    provenance_report_path: Path,
    patched_clang_positive_dir: Path,
) -> dict[str, Any]:
    prior = read_json(prior_audit_path)
    provenance = read_json(provenance_report_path)
    clang_events = load_patched_clang_events(patched_clang_positive_dir)

    accepted_keys = sorted(
        {event_key(item) for item in prior_accepted_rows(prior)}
    )
    final_class_rows = [
        item
        for item in provenance.get("public_source_ownership", [])
        if item.get("kind") == "class_use"
    ]
    final_by_key: dict[tuple[str, str], list[dict[str, Any]]] = {}
    for item in final_class_rows:
        copied = dict(item)
        copied["location"] = canonical_location(str(item.get("location", "")))
        final_by_key.setdefault(event_key(copied), []).append(copied)
    clang_by_key: dict[tuple[str, str], list[dict[str, Any]]] = {}
    for item in clang_events:
        clang_by_key.setdefault(event_key(item), []).append(item)

    accepted: list[dict[str, Any]] = []
    for key in accepted_keys:
        accepted.append(
            {
                "location": key[0],
                "template_name": key[1],
                "cppgm_final_rows": final_by_key.get(key, []),
                "patched_clang_rows": clang_by_key.get(key, []),
            }
        )

    rejected: list[dict[str, Any]] = []
    for prior_row in prior_late_removed_rows(prior):
        key = event_key(prior_row)
        rejected.append(
            {
                "location": key[0],
                "template_name": key[1],
                "prior_late_decision": prior_row,
                "cppgm_final_rows": final_by_key.get(key, []),
            }
        )

    class_route = provenance.get("site_coverage", {}).get(
        "class.class_template_reference.02", {}
    )
    prior_clang = prior_patched_clang_evidence(prior)
    failures: list[str] = []
    if len(accepted) != 5:
        failures.append(f"expected 5 accepted source occurrences, found {len(accepted)}")
    for item in accepted:
        if len(item["cppgm_final_rows"]) != 1:
            failures.append(
                f"accepted {item['location']} {item['template_name']} has "
                f"{len(item['cppgm_final_rows'])} CPPGM public rows"
            )
        if len(item["patched_clang_rows"]) != 1:
            failures.append(
                f"accepted {item['location']} {item['template_name']} has "
                f"{len(item['patched_clang_rows'])} patched-Clang rows"
            )
    if len(rejected) != 55:
        failures.append(f"expected 55 rejected audit rows, found {len(rejected)}")
    for item in rejected:
        if item["cppgm_final_rows"]:
            failures.append(
                f"rejected {item['location']} {item['template_name']} reached CPPGM output"
            )
    if int(prior_clang.get("class_rows_at_removed_locations", -1)) != 0:
        failures.append("patched Clang has a row at a rejected location")
    if int(prior_clang.get("compile_failures", -1)) != 0:
        failures.append("patched-Clang rejected-location corpus did not compile cleanly")
    class_publications = int(class_route.get("publications", -1))
    if class_publications != len(final_class_rows):
        failures.append("class publication coverage does not equal public rows")

    return {
        "schema_version": 2,
        "inputs": {
            "prior_boundary_audit": str(prior_audit_path),
            "prior_boundary_audit_sha256": sha256(prior_audit_path),
            "provenance_report": str(provenance_report_path),
            "provenance_report_sha256": sha256(provenance_report_path),
            "patched_clang_positive_dir": str(patched_clang_positive_dir),
            "patched_clang_binary": prior_clang.get("binary", ""),
            "patched_clang_checkout": prior_clang.get("checkout", ""),
        },
        "summary": {
            "accepted_source_occurrences": len(accepted),
            "rejected_audit_rows": len(rejected),
            "rejected_distinct_source_occurrences": len(
                {(item["location"], item["template_name"]) for item in rejected}
            ),
            "patched_clang_rows_at_rejected_locations": prior_clang.get(
                "class_rows_at_removed_locations", -1
            ),
            "class_source_publications": class_publications,
            "class_public_rows": len(final_class_rows),
        },
        "accepted": accepted,
        "rejected": rejected,
        "patched_clang_rejected_results": prior_clang.get("results", {}),
        "failures": failures,
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--prior-audit", type=Path, required=True)
    parser.add_argument("--provenance-report", type=Path, required=True)
    parser.add_argument("--patched-clang-positive-dir", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    audit = build_audit(
        args.prior_audit.resolve(),
        args.provenance_report.resolve(),
        args.patched_clang_positive_dir.resolve(),
    )
    args.output.write_text(json.dumps(audit, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    if audit["failures"]:
        for failure in audit["failures"]:
            print(f"ERROR: {failure}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
