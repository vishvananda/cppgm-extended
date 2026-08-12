#!/usr/bin/env python3
"""Aggregate witness semantic-path provenance JSONL files."""

from __future__ import annotations

import argparse
import collections
import json
import pathlib
import sys
from typing import Any, Iterable


SOURCE_PRODUCERS = [
    "class.class_template_reference.02",
    "alias.canonical_occurrence",
    "function.semantic_template_function",
    "variable.template_instantiation",
]

LIFECYCLE_PRODUCERS = [
    "lifecycle.transition_observer.01",
]

UPSTREAM_ROUTES = [
    "alias.canonical_occurrence",
    "class.resolved_template_id",
    "class.declaration_type_source",
    "class.explicit_specialization_source",
    "class.qualified_value_source",
    "class.nested_source_template_id",
    "function.constant_value_lookup",
    "function.conversion",
    "function.declval",
    "function.overload_resolution",
    "variable.direct_instantiation",
    "variable.initializer_replay",
]

ALIAS_UPSTREAM_ROUTES = [
    "alias.canonical_occurrence",
]


def load_records(paths: Iterable[pathlib.Path]) -> list[dict[str, Any]]:
    records: list[dict[str, Any]] = []
    for path in paths:
        with path.open(encoding="utf-8") as stream:
            for line_number, line in enumerate(stream, 1):
                if not line.strip():
                    continue
                try:
                    record = json.loads(line)
                except json.JSONDecodeError as exc:
                    raise SystemExit(f"{path}:{line_number}: {exc}") from exc
                record["_trace_file"] = str(path)
                records.append(record)
    return records


def sorted_counter(counter: collections.Counter[str]) -> dict[str, int]:
    return dict(sorted(counter.items()))


def build_report(
    records: list[dict[str, Any]], trace_file_count: int | None = None
) -> dict[str, Any]:
    site_coverage: dict[str, collections.Counter[str]] = {
        site: collections.Counter() for site in SOURCE_PRODUCERS + LIFECYCLE_PRODUCERS
    }
    upstream_routes: dict[str, collections.Counter[str]] = {
        route: collections.Counter() for route in UPSTREAM_ROUTES
    }
    alias_routes: dict[str, collections.Counter[str]] = {
        route: collections.Counter() for route in ALIAS_UPSTREAM_ROUTES
    }
    public_source_ownership: list[dict[str, Any]] = []
    lifecycle_public_ownership: list[dict[str, Any]] = []
    unknown_producers: collections.Counter[str] = collections.Counter()
    source_publications: list[dict[str, Any]] = []
    lifecycle_publications: list[dict[str, Any]] = []
    lifecycle_publication_context = collections.Counter()

    for record in records:
        record_kind = record.get("record")
        if record_kind in {"source_publication", "lifecycle_publication"}:
            producer = str(record.get("producer", "unknown"))
            coverage = site_coverage.setdefault(producer, collections.Counter())
            coverage["publications"] += 1
            if producer == "unknown":
                unknown_producers[record_kind] += 1

            if record_kind == "lifecycle_publication":
                lifecycle_publication_context[
                    f"entry_origin:{int(record.get('entry_origin', 0))}"
                ] += 1
                lifecycle_publication_context[
                    f"closure_reason:{int(record.get('closure_reason', 0))}"
                ] += 1
                lifecycle_publication_context[
                    f"cause:{int(record.get('cause', 0))}"
                ] += 1
                lifecycle_publication_context[
                    "public_source_required"
                    if bool(record.get("public_source_required", False))
                    else "not_public_source_required"
                ] += 1
                lifecycle_publications.append(
                    {
                        key: record.get(key)
                        for key in (
                            "producer",
                            "kind",
                            "location",
                            "entity",
                            "cause",
                            "entry_origin",
                            "closure_reason",
                            "trigger_entity",
                            "trigger_decl_location",
                            "detail",
                            "public_source_required",
                        )
                    }
                )
                lifecycle_publications[-1]["source"] = record.get(
                    "_trace_file", ""
                )
                lifecycle_public_ownership.append(
                    {
                        "producer": producer,
                        "kind": record.get("kind"),
                        "location": record.get("location"),
                        "entity": record.get("entity"),
                    }
                )

            if record_kind == "source_publication":
                route = str(record.get("upstream_route", "unknown"))
                route_counts = upstream_routes.setdefault(
                    route, collections.Counter()
                )
                route_counts["publications"] += 1
                route_counts[f"kind:{record.get('kind', 'unknown')}"] += 1
                source_publications.append(
                    {
                        key: record.get(key)
                        for key in (
                            "producer",
                            "upstream_route",
                            "kind",
                            "role",
                            "ownership",
                            "location",
                            "spelling_anchor",
                            "selected_entity",
                            "selected_decl",
                            "template_name",
                            "selection",
                            "binding_sources",
                            "binding_args",
                            "binding_type_like",
                            "specialization_binding_sources",
                            "occurrence_present",
                            "occurrence_argument_texts",
                            "occurrence_argument_semantic_texts",
                            "occurrence_argument_dependent",
                        )
                    }
                )
                source_publications[-1]["source"] = record.get(
                    "_trace_file", ""
                )
                public_source_ownership.append(
                    {
                        "producer": producer,
                        "kind": record.get("kind"),
                        "source": record.get("_trace_file", ""),
                        "location": record.get("location"),
                        "template_name": record.get("template_name"),
                    }
                )
                if record.get("kind") == "alias_use":
                    route_counts = alias_routes.setdefault(
                        route, collections.Counter()
                    )
                    route_counts["publications"] += 1
            continue

    coverage_output: dict[str, dict[str, int]] = {}
    for site in SOURCE_PRODUCERS + LIFECYCLE_PRODUCERS:
        counts = site_coverage.get(site, collections.Counter())
        coverage_output[site] = {
            "publications": counts["publications"],
        }
    unexercised = [
        site
        for site, counts in coverage_output.items()
        if counts["publications"] == 0
    ]
    alias_route_output: dict[str, dict[str, int]] = {}
    for route in ALIAS_UPSTREAM_ROUTES + ["unknown"]:
        counts = alias_routes.get(route, collections.Counter())
        alias_route_output[route] = {
            "publications": counts["publications"],
        }
    upstream_route_output: dict[str, dict[str, int]] = {}
    for route in UPSTREAM_ROUTES + ["unknown"]:
        counts = upstream_routes.get(route, collections.Counter())
        upstream_route_output[route] = dict(sorted(counts.items()))

    return {
        "schema_version": 6,
        "trace_files": trace_file_count if trace_file_count is not None else len(
            {record["_trace_file"] for record in records}
        ),
        "records": len(records),
        "site_coverage": coverage_output,
        "unexercised_sites": unexercised,
        "source_publications": sorted(
            source_publications,
            key=lambda item: (
                str(item.get("kind", "")),
                str(item.get("location", "")),
                str(item.get("template_name", "")),
                str(item.get("upstream_route", "")),
            ),
        ),
        "lifecycle_publication_context_summary": sorted_counter(
            lifecycle_publication_context
        ),
        "lifecycle_publications": sorted(
            lifecycle_publications,
            key=lambda item: (
                str(item.get("kind", "")),
                str(item.get("location", "")),
                str(item.get("entity", "")),
            ),
        ),
        "public_source_ownership": sorted(
            public_source_ownership,
            key=lambda item: (
                item["producer"],
                str(item["source"]),
                str(item["location"]),
                str(item["kind"]),
            ),
        ),
        "lifecycle_public_ownership": sorted(
            lifecycle_public_ownership,
            key=lambda item: (
                str(item["kind"]),
                str(item["location"]),
                str(item["entity"]),
                item["producer"],
            ),
        ),
        "upstream_route_coverage": upstream_route_output,
        "alias_upstream_route_coverage": alias_route_output,
        "unknown_producer_publications": sorted_counter(unknown_producers),
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--input-dir", type=pathlib.Path, required=True, help="trace JSONL directory"
    )
    parser.add_argument("--output", type=pathlib.Path)
    args = parser.parse_args()

    paths = sorted(args.input_dir.rglob("*.jsonl"))
    if not paths:
        parser.error(f"no JSONL traces found under {args.input_dir}")
    report = build_report(load_records(paths), len(paths))
    text = json.dumps(report, indent=2, sort_keys=True) + "\n"
    if args.output:
        args.output.write_text(text, encoding="utf-8")
    else:
        sys.stdout.write(text)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
