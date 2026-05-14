#!/usr/bin/env python3
"""Audit template-side semantic/template boundary debt.

The audit is intentionally a ratchet. Existing debt is recorded in a checked-in
baseline; new matches should either be removed or deliberately moved into the
baseline with the related consolidation stage.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from dataclasses import dataclass
from pathlib import Path


DEFAULT_ROOT = Path(".")
DEFAULT_BASELINE = Path("docs/template-side-boundary-audit-baseline.json")
AUDIT_MARKER_PATTERN = re.compile(
    r"template-boundary-audit:\s*(begin|end)\s+([A-Za-z0-9_, -]+)"
)


@dataclass(frozen=True)
class Category:
    name: str
    description: str
    globs: tuple[str, ...]
    pattern: re.Pattern[str]


TEMPLATE_IMPLEMENTATION_GLOBS = (
    "dev/src/template_*.cpp",
    "dev/src/template_*.h",
    "dev/src/template_api.cpp",
    "dev/src/template_api.h",
    "dev/src/template_api_internal.h",
    "dev/src/callsemantic_templates.cpp",
)


CATEGORIES = [
    Category(
        "service_adapter_construction",
        "SemanticContextTemplateServices construction or exposure in template-owned files.",
        TEMPLATE_IMPLEMENTATION_GLOBS,
        re.compile(r"\bSemanticContextTemplateServices\b"),
    ),
    Category(
        "service_bundle_construction",
        "TemplateServices bundle construction in template-owned files.",
        TEMPLATE_IMPLEMENTATION_GLOBS,
        re.compile(r"\bTemplateServices\s+[A-Za-z_][A-Za-z0-9_]*\s*="),
    ),
    Category(
        "semantic_service_access",
        "Direct TemplateServices type-system or recursive-semantic callback access.",
        TEMPLATE_IMPLEMENTATION_GLOBS,
        re.compile(
            r"\b[A-Za-z_][A-Za-z0-9_]*\.(?:type_system|recursive_semantic)\b|"
            r"\.(?:type_system|recursive_semantic)\."
        ),
    ),
    Category(
        "text_recovery_bridge",
        "Template-owned structured-to-text recovery or rewrite bridge use.",
        TEMPLATE_IMPLEMENTATION_GLOBS + ("dev/src/callsemantic.cpp",),
        re.compile(
            r"\b(?:resolve_type_argument_text|"
            r"resolve_instantiated_dependent_type|lookup_text_for_type_argument|"
            r"rewrite_bound_type[A-Za-z0-9_]*|rewrite_bound_template[A-Za-z0-9_]*|"
            r"expand_bound_[A-Za-z0-9_]*_pack_texts)\b"
        ),
    ),
    Category(
        "canonical_key_metadata",
        "Canonical template argument/key metadata reads, writes, or formatting.",
        TEMPLATE_IMPLEMENTATION_GLOBS + ("dev/src/callsemantic.cpp",),
        re.compile(
            r"\b(?:canonical_instantiation|instantiation_key|"
            r"template_instantiation_key|instantiation_arg_texts|"
            r"template_argument_key)\b"
        ),
    ),
    Category(
        "witness_source_location",
        "Template witness and source-location plumbing touchpoints.",
        TEMPLATE_IMPLEMENTATION_GLOBS + ("dev/src/callsemantic.cpp",),
        re.compile(
            r"\b(?:TemplateWitnessContext|ScopedTemplateArgumentSourceLocations|"
            r"template_argument_source|normalize_template_witness_source_location|"
            r"source_location_for_)\b"
        ),
    ),
    Category(
        "callsemantic_template_metadata_exception",
        "Template metadata use inside the mixed callsemantic.cpp exception.",
        ("dev/src/callsemantic.cpp",),
        re.compile(
            r"\b(?:source_template|template_instantiation_key|"
            r"instantiation_arguments|instantiation_arg_texts|"
            r"suppress_implicit_instantiation|is_explicit_specialization|"
            r"note_[A-Za-z0-9_]*closure_event|"
            r"maybe_enter_[A-Za-z0-9_]*closure_context)\b"
        ),
    ),
]


def candidate_files(root: Path, category: Category) -> list[Path]:
    paths: set[Path] = set()
    for pattern in category.globs:
        paths.update(path for path in root.glob(pattern) if path.is_file())
    return sorted(paths)


def count_sites(root: Path) -> dict[str, list[str]]:
    sites: dict[str, list[str]] = {category.name: [] for category in CATEGORIES}
    for category in CATEGORIES:
        for path in candidate_files(root, category):
            try:
                lines = path.read_text(encoding="utf-8").splitlines()
            except UnicodeDecodeError:
                lines = path.read_text(encoding="latin-1").splitlines()
            rel = path.relative_to(root).as_posix()
            ignored_categories: set[str] = set()
            for lineno, line in enumerate(lines, 1):
                marker = AUDIT_MARKER_PATTERN.search(line)
                if marker:
                    names = {
                        name.strip()
                        for name in re.split(r"[, ]+", marker.group(2))
                        if name.strip()
                    }
                    if marker.group(1) == "begin":
                        ignored_categories.update(names)
                    else:
                        ignored_categories.difference_update(names)
                    continue
                if category.name in ignored_categories:
                    continue
                if category.pattern.search(line):
                    sites[category.name].append(f"{rel}:{lineno}:{line.strip()}")
    return sites


def load_baseline(path: Path) -> dict[str, int]:
    with path.open("r", encoding="utf-8") as handle:
        data = json.load(handle)
    limits = data.get("limits")
    if not isinstance(limits, dict):
        raise ValueError(f"{path} does not contain a 'limits' object")
    return {str(name): int(limit) for name, limit in limits.items()}


def write_baseline(path: Path, counts: dict[str, int]) -> None:
    payload = {
        "description": (
            "Template-side semantic/template boundary audit limits. "
            "Counts are line-based matches from scripts/audit_template_boundary.py."
        ),
        "limits": {category.name: counts[category.name] for category in CATEGORIES},
    }
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2, sort_keys=False) + "\n",
                    encoding="utf-8")


def print_summary(counts: dict[str, int],
                  baseline: dict[str, int] | None) -> None:
    print("Template boundary audit")
    print(f"{'category':42} {'count':>7} {'limit':>7} status")
    for category in CATEGORIES:
        count = counts[category.name]
        limit = baseline.get(category.name, count) if baseline is not None else count
        if count > limit:
            status = "FAIL"
        elif count < limit:
            status = "reduced"
        else:
            status = "ok"
        print(f"{category.name:42} {count:7d} {limit:7d} {status}")


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--root",
        type=Path,
        default=DEFAULT_ROOT,
        help=f"repository root to scan, default: {DEFAULT_ROOT}",
    )
    parser.add_argument(
        "--baseline",
        type=Path,
        default=DEFAULT_BASELINE,
        help=f"baseline JSON path, default: {DEFAULT_BASELINE}",
    )
    parser.add_argument(
        "--no-baseline",
        action="store_true",
        help="print current counts without enforcing a baseline",
    )
    parser.add_argument(
        "--write-baseline",
        action="store_true",
        help="write the current counts to --baseline and exit",
    )
    parser.add_argument(
        "--list-sites",
        action="store_true",
        help="list matching source locations after the summary",
    )
    args = parser.parse_args(argv)

    root = args.root.resolve()
    if not root.exists():
        print(f"error: scan root does not exist: {root}", file=sys.stderr)
        return 2

    sites = count_sites(root)
    counts = {name: len(matches) for name, matches in sites.items()}

    baseline: dict[str, int] | None = None
    if args.write_baseline:
        write_baseline(args.baseline, counts)
        baseline = counts
    elif not args.no_baseline:
        if not args.baseline.exists():
            print(f"error: baseline does not exist: {args.baseline}", file=sys.stderr)
            return 2
        try:
            baseline = load_baseline(args.baseline)
        except (OSError, ValueError, json.JSONDecodeError) as exc:
            print(f"error: failed to read baseline: {exc}", file=sys.stderr)
            return 2

    print_summary(counts, baseline)

    if args.list_sites:
        for category in CATEGORIES:
            print()
            print(f"[{category.name}] {category.description}")
            for site in sites[category.name]:
                print(site)

    if baseline is None:
        return 0

    failed_categories = [
        category.name
        for category in CATEGORIES
        if counts[category.name] > baseline.get(category.name, 0)
    ]
    if failed_categories:
        print()
        print("New template boundary debt detected:")
        for name in failed_categories:
            print(f"- {name}")
        print("Run with --list-sites to inspect matching locations.")
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
