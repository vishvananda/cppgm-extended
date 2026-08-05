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
    *(f"class.callsemantic.{index:02d}" for index in range(6, 12) if index != 9),
    "class.callsemantic.13",
    "class.class_template_reference.01",
    "class.class_template_reference.02",
    *(f"class.constant_value_lookup.{index:02d}" for index in range(2, 6)),
    "class.template_instantiation",
    "alias.template_argument_semantics.02",
    "alias.template_specialization.01",
    "alias.callsemantic.02",
    "alias.callsemantic.03",
    "function.semantic_template_function",
    "function.semantic_overload.declval",
    "function.constant_value_lookup.constexpr",
    "variable.template_instantiation",
]

LIFECYCLE_PRODUCERS = [
    *(f"lifecycle.template_api.{index:02d}" for index in range(1, 10) if index != 8),
    "lifecycle.semantic_class_model",
    "lifecycle.template_argument_semantics.01",
    "lifecycle.template_argument_semantics.02",
    "lifecycle.callsemantic.01",
    "lifecycle.callsemantic.02",
    "lifecycle.constant_value_lookup.01",
    "lifecycle.constant_value_lookup.02",
    "lifecycle.constant_value_lookup.03",
]

UPSTREAM_ROUTES = [
    "nested_class_use.ast_node",
    "nested_class_use.template_arguments",
    "class_use.static_member_definition_ast_node",
    "class_use.resolved_alias_type",
    "class_use.resolved_type_node",
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


def build_report(records: list[dict[str, Any]]) -> dict[str, Any]:
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
    upstream = collections.Counter({route: 0 for route in UPSTREAM_ROUTES})
    unique_output: list[dict[str, Any]] = []
    lifecycle_output: list[dict[str, Any]] = []
    unknown_producers: collections.Counter[str] = collections.Counter()

    for record in records:
        record_kind = record.get("record")
        if record_kind in {"source_attempt", "lifecycle_attempt"}:
            producer = str(record.get("producer", "unknown"))
            coverage = site_coverage.setdefault(producer, collections.Counter())
            coverage["attempts"] += 1
            coverage[str(record.get("action", "unknown"))] += 1
            if producer == "unknown":
                unknown_producers[record_kind] += 1

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
            continue

        if record_kind == "final_visible":
            producers = sorted(set(record.get("producers", [])))
            for producer in producers:
                site_coverage.setdefault(producer, collections.Counter())["final_visible_rows"] += 1
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

        if record_kind == "upstream_route":
            upstream[str(record.get("route", "unknown"))] += 1

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

    return {
        "schema_version": 1,
        "trace_files": len({record["_trace_file"] for record in records}),
        "records": len(records),
        "site_coverage": coverage_output,
        "unexercised_sites": unexercised,
        "collision_matrix": collision_output(collision_pairs),
        "source_collision_matrix": collision_output(source_collision_pairs),
        "lifecycle_collision_matrix": collision_output(lifecycle_collision_pairs),
        "replacement_matrix": replacement_output,
        "renderer_ownership": renderer_output,
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
        "upstream_route_count": dict(sorted(upstream.items())),
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
    report = build_report(load_records(paths))
    text = json.dumps(report, indent=2, sort_keys=True) + "\n"
    if args.output:
        args.output.write_text(text, encoding="utf-8")
    else:
        sys.stdout.write(text)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
