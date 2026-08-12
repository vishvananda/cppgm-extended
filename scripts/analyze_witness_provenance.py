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


def nested_counter() -> collections.defaultdict[str, collections.Counter[str]]:
    return collections.defaultdict(collections.Counter)


def sorted_counter(counter: collections.Counter[str]) -> dict[str, int]:
    return dict(sorted(counter.items()))


def build_report(
    records: list[dict[str, Any]], trace_file_count: int | None = None
) -> dict[str, Any]:
    site_coverage: dict[str, collections.Counter[str]] = {
        site: collections.Counter() for site in SOURCE_PRODUCERS + LIFECYCLE_PRODUCERS
    }
    collision_pairs: collections.Counter[tuple[str, str]] = collections.Counter()
    source_collision_pairs: collections.Counter[tuple[str, str]] = collections.Counter()
    lifecycle_collision_pairs: collections.Counter[tuple[str, str]] = (
        collections.Counter()
    )
    replacement: collections.Counter[tuple[str, str, str, str]] = (
        collections.Counter()
    )
    renderer = nested_counter()
    upstream_routes: dict[str, collections.Counter[str]] = {
        route: collections.Counter() for route in UPSTREAM_ROUTES
    }
    alias_routes: dict[str, collections.Counter[str]] = {
        route: collections.Counter() for route in ALIAS_UPSTREAM_ROUTES
    }
    alias_renderer_routes = nested_counter()
    unique_output: list[dict[str, Any]] = []
    lifecycle_output: list[dict[str, Any]] = []
    unknown_producers: collections.Counter[str] = collections.Counter()
    source_attempt_decisions: list[dict[str, Any]] = []
    lifecycle_attempt_decisions: list[dict[str, Any]] = []
    lifecycle_attempt_context = collections.Counter()

    for record in records:
        record_kind = record.get("record")
        if record_kind in {"source_attempt", "lifecycle_attempt"}:
            producer = str(record.get("producer", "unknown"))
            coverage = site_coverage.setdefault(producer, collections.Counter())
            coverage["attempts"] += 1
            coverage[str(record.get("action", "unknown"))] += 1
            if producer == "unknown":
                unknown_producers[record_kind] += 1

            if record_kind == "lifecycle_attempt":
                lifecycle_attempt_context[
                    f"entry_origin:{int(record.get('entry_origin', 0))}"
                ] += 1
                lifecycle_attempt_context[
                    f"closure_reason:{int(record.get('closure_reason', 0))}"
                ] += 1
                lifecycle_attempt_context[
                    f"cause:{int(record.get('cause', 0))}"
                ] += 1
                lifecycle_attempt_context[
                    "public_source_required"
                    if bool(record.get("public_source_required", False))
                    else "not_public_source_required"
                ] += 1
                lifecycle_attempt_decisions.append(
                    {
                        key: record.get(key)
                        for key in (
                            "producer",
                            "action",
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
                lifecycle_attempt_decisions[-1]["source"] = record.get(
                    "_trace_file", ""
                )

            collided = sorted(set(record.get("collided_producers", [])))
            for other in collided:
                if producer == other:
                    continue
                pair = tuple(sorted((producer, other)))
                collision_pairs[pair] += 1
                if record_kind == "source_attempt":
                    source_collision_pairs[pair] += 1
                else:
                    lifecycle_collision_pairs[pair] += 1

            if record_kind == "source_attempt":
                action = str(record.get("action", ""))
                route = str(record.get("upstream_route", "unknown"))
                route_counts = upstream_routes.setdefault(
                    route, collections.Counter()
                )
                route_counts["attempts"] += 1
                route_counts[action] += 1
                route_counts[f"kind:{record.get('kind', 'unknown')}"] += 1
                source_attempt_decisions.append(
                    {
                        key: record.get(key)
                        for key in (
                            "producer",
                            "upstream_route",
                            "action",
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
                            "changed_fields",
                            "collided_producers",
                        )
                    }
                )
                source_attempt_decisions[-1]["source"] = record.get(
                    "_trace_file", ""
                )
                if record.get("kind") == "alias_use":
                    route_counts = alias_routes.setdefault(
                        route, collections.Counter()
                    )
                    route_counts["attempts"] += 1
                    route_counts[action] += 1
                if action in {"replaced", "enriched"}:
                    fields = str(record.get("changed_fields", "")) or "row"
                    if not collided:
                        replacement[("none", producer, action, fields)] += 1
                    for other in collided:
                        replacement[(other, producer, action, fields)] += 1
            continue

        if record_kind == "final_table_row":
            for producer in set(record.get("producers", [])):
                site_coverage.setdefault(producer, collections.Counter())["surviving_rows"] += 1
            if record.get("kind") == "alias_use":
                for route in set(record.get("upstream_routes", [])):
                    alias_routes.setdefault(route, collections.Counter())[
                        "surviving_rows"
                    ] += 1
            for route in set(record.get("upstream_routes", [])):
                upstream_routes.setdefault(route, collections.Counter())[
                    "surviving_rows"
                ] += 1
            continue

        if record_kind == "final_lifecycle_event":
            producers = sorted(set(record.get("producers", [])))
            for producer in producers:
                site_coverage.setdefault(producer, collections.Counter())["surviving_rows"] += 1
            lifecycle_output.append(
                {
                    "producers": producers,
                    "kind": record.get("kind"),
                    "location": record.get("location"),
                    "entity": record.get("entity"),
                }
            )
            continue

        if record_kind == "renderer_action":
            key = f"{record.get('action', 'unknown')}"
            renderer[str(record.get("pass", "unknown"))][key] += 1
            for producer in set(record.get("producers", [])):
                renderer[str(record.get("pass", "unknown"))][f"producer:{producer}"] += 1
            for route in set(record.get("upstream_routes", [])):
                renderer[str(record.get("pass", "unknown"))][
                    f"route:{route}"
                ] += 1
            if record.get("kind") == "alias_use":
                for route in set(record.get("upstream_routes", [])):
                    route_key = f"{record.get('pass', 'unknown')}:{key}"
                    alias_renderer_routes[route][route_key] += 1
            continue

        if record_kind == "final_visible":
            producers = sorted(set(record.get("producers", [])))
            for producer in producers:
                site_coverage.setdefault(producer, collections.Counter())["final_visible_rows"] += 1
            if record.get("kind") == "alias_use":
                for route in set(record.get("upstream_routes", [])):
                    alias_routes.setdefault(route, collections.Counter())[
                        "final_visible_rows"
                    ] += 1
            for route in set(record.get("upstream_routes", [])):
                upstream_routes.setdefault(route, collections.Counter())[
                    "final_visible_rows"
                ] += 1
            if len(producers) == 1:
                unique_output.append(
                    {
                        "producer": producers[0],
                        "kind": record.get("kind"),
                        "source": record.get("source"),
                        "location": record.get("location"),
                        "template_name": record.get("template_name"),
                    }
                )
            continue

    coverage_output: dict[str, dict[str, int]] = {}
    for site in SOURCE_PRODUCERS + LIFECYCLE_PRODUCERS:
        counts = site_coverage.get(site, collections.Counter())
        coverage_output[site] = {
            "attempts": counts["attempts"],
            "inserted": counts["inserted"],
            "exact_duplicate": counts["exact_duplicate"],
            "rejected": counts["rejected"],
            "replaced": counts["replaced"],
            "enriched": counts["enriched"],
            "surviving_rows": counts["surviving_rows"],
            "final_visible_rows": counts["final_visible_rows"],
        }

    def collision_output(
        counter: collections.Counter[tuple[str, str]],
    ) -> list[dict[str, Any]]:
        return [
            {"producer_a": pair[0], "producer_b": pair[1], "count": count}
            for pair, count in sorted(counter.items())
        ]
    replacement_output = [
        {
            "previous_producer": key[0],
            "retained_producer": key[1],
            "action": key[2],
            "fields": key[3],
            "count": count,
        }
        for key, count in sorted(replacement.items())
    ]
    renderer_output = {
        name: sorted_counter(counts) for name, counts in sorted(renderer.items())
    }
    unexercised = [
        site for site, counts in coverage_output.items() if counts["attempts"] == 0
    ]
    alias_route_output: dict[str, dict[str, int]] = {}
    for route in ALIAS_UPSTREAM_ROUTES + ["unknown"]:
        counts = alias_routes.get(route, collections.Counter())
        alias_route_output[route] = {
            "attempts": counts["attempts"],
            "inserted": counts["inserted"],
            "exact_duplicate": counts["exact_duplicate"],
            "rejected": counts["rejected"],
            "replaced": counts["replaced"],
            "enriched": counts["enriched"],
            "surviving_rows": counts["surviving_rows"],
            "final_visible_rows": counts["final_visible_rows"],
        }
    upstream_route_output: dict[str, dict[str, int]] = {}
    for route in UPSTREAM_ROUTES + ["unknown"]:
        counts = upstream_routes.get(route, collections.Counter())
        upstream_route_output[route] = dict(sorted(counts.items()))

    return {
        "schema_version": 3,
        "trace_files": trace_file_count if trace_file_count is not None else len(
            {record["_trace_file"] for record in records}
        ),
        "records": len(records),
        "site_coverage": coverage_output,
        "unexercised_sites": unexercised,
        "collision_matrix": collision_output(collision_pairs),
        "source_collision_matrix": collision_output(source_collision_pairs),
        "lifecycle_collision_matrix": collision_output(lifecycle_collision_pairs),
        "replacement_matrix": replacement_output,
        "renderer_ownership": renderer_output,
        "source_attempt_decisions": sorted(
            source_attempt_decisions,
            key=lambda item: (
                str(item.get("kind", "")),
                str(item.get("location", "")),
                str(item.get("template_name", "")),
                str(item.get("upstream_route", "")),
                str(item.get("action", "")),
            ),
        ),
        "lifecycle_attempt_context_summary": sorted_counter(
            lifecycle_attempt_context
        ),
        "lifecycle_attempt_decisions": sorted(
            lifecycle_attempt_decisions,
            key=lambda item: (
                str(item.get("kind", "")),
                str(item.get("location", "")),
                str(item.get("entity", "")),
                str(item.get("action", "")),
            ),
        ),
        "unique_output_ownership": sorted(
            unique_output,
            key=lambda item: (
                item["producer"],
                str(item["source"]),
                str(item["location"]),
                str(item["kind"]),
            ),
        ),
        "lifecycle_output_ownership": sorted(
            lifecycle_output,
            key=lambda item: (
                str(item["kind"]),
                str(item["location"]),
                str(item["entity"]),
                item["producers"],
            ),
        ),
        "upstream_route_coverage": upstream_route_output,
        "alias_upstream_route_coverage": alias_route_output,
        "alias_renderer_ownership_by_route": {
            route: sorted_counter(counts)
            for route, counts in sorted(alias_renderer_routes.items())
        },
        "unknown_producer_attempts": sorted_counter(unknown_producers),
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
