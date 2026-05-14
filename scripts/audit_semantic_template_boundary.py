#!/usr/bin/env python3
"""Audit semantic-side template boundary consolidation debt.

The audit is a ratchet. Existing semantic-side decision distribution is allowed
temporarily, but new direct sites should not appear while semantic-owned facades
replace them.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Iterable


DEFAULT_BASELINE = Path("docs/semantic-template-boundary-audit-baseline.json")
DEFAULT_SOURCE_ROOT = Path("dev/src")

SEMANTIC_FILE_PATTERNS = (
    "semantic_*.cpp",
    "semantic_*.h",
    "output_requirement_engine.cpp",
    "output_requirement_engine.h",
    "callsemantic_phase_bridge.cpp",
    "callsemantic_phase_bridge.h",
)


@dataclass(frozen=True)
class Category:
    name: str
    description: str
    pattern: re.Pattern[str]
    allowed_paths: frozenset[str] = field(default_factory=frozenset)


CATEGORIES = [
    Category(
        "function_template_requests",
        "Semantic files directly construct/deduce/acquire function template requests.",
        re.compile(
            r"\bTemplateFunctionDeductionRequest\b|"
            r"\bTemplateFunctionInstantiationRequest\b|"
            r"\bTemplateVariableInstantiationRequest\b|"
            r"\btemplate_api::resolve_template_arguments\s*\(|"
            r"\btemplate_api::binding::overlay_instantiation_(?:"
            r"use_scope_bindings|local_named_types"
            r")\s*\(|"
            r"\btemplate_api::resolution::resolve_function_explicit_template_arguments\s*\(|"
            r"\btemplate_api::resolution::deduce_template_argument\s*\(|"
            r"\btemplate_type_ops::substitute_type\s*\(|"
            r"\bdeduce_function_template\s*\(|"
            r"\bacquire_.*instantiation\s*\("
        ),
        frozenset(
            {
                "dev/src/semantic_template_function.cpp",
                "dev/src/semantic_template_variable.cpp",
            }
        ),
    ),
    Category(
        "dependent_type_ops",
        "Semantic files directly call template type recovery operations.",
        re.compile(
            r"\btemplate_type_ops::(?:"
            r"resolve_type_argument_text|"
            r"resolve_instantiated_dependent_type|"
            r"lookup_text_for_type_argument"
            r")\s*\(|"
            r"\btemplate_api::type::(?:"
            r"lookup_text_for_type_argument"
            r")\s*\(|"
            r"\btemplate_api::resolve_type_text_without_fragment_fallback\s*\("
        ),
        frozenset({"dev/src/semantic_dependent_type.cpp"}),
    ),
    Category(
        "scope_template_mutation_ops",
        "Semantic files directly mutate template binding fingerprints or named-type bindings.",
        re.compile(
            r"\btemplate_api::bump_scope_template_binding_fingerprint_epoch\s*\(|"
            r"\btemplate_binding_ops::bind_named_type\s*\("
        ),
        frozenset({"dev/src/semantic_scope_mutation.cpp"}),
    ),
    Category(
        "output_readiness_queries",
        "Semantic output files directly compose low-level template output/readiness facts.",
        re.compile(
            r"\bcompute_instantiated_class_output_readiness\s*\(|"
            r"\bfunction_binding_instantiation_arguments_(?:complete|dependent)\s*\(|"
            r"\bfunction_binding_output_suppressed(?:_by_explicit_instantiation)?\s*\(|"
            r"\bclass_suppresses_implicit_instantiation_definition\s*\("
        ),
        frozenset(
            {
                "dev/src/output_requirement_engine.cpp",
                "dev/src/semantic_template_output_policy.cpp",
            }
        ),
    ),
    Category(
        "template_services_mentions",
        "Semantic files mention TemplateServices implementation glue directly.",
        re.compile(r"\b(?:SemanticContextTemplateServices|TemplateServices)\b"),
    ),
    Category(
        "template_internal_headers",
        "Semantic files include template implementation/internal headers directly.",
        re.compile(
            r'^\s*#\s*include\s+"(?:'
            r"template_api_internal|"
            r"template_[a-z0-9_]*(?:services|instantiation|deduction|scope)"
            r')\.h"'
        ),
    ),
]


def iter_semantic_files(root: Path) -> Iterable[Path]:
    seen: set[Path] = set()
    for pattern in SEMANTIC_FILE_PATTERNS:
        for path in root.glob(pattern):
            if path.is_file() and path not in seen:
                seen.add(path)
                yield path


def read_lines(path: Path) -> list[str]:
    try:
        return path.read_text(encoding="utf-8").splitlines()
    except UnicodeDecodeError:
        return path.read_text(encoding="latin-1").splitlines()


def count_sites(root: Path) -> dict[str, list[str]]:
    sites = {category.name: [] for category in CATEGORIES}
    for path in sorted(iter_semantic_files(root)):
        rel = path.as_posix()
        for lineno, line in enumerate(read_lines(path), 1):
            for category in CATEGORIES:
                if rel in category.allowed_paths:
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


def print_summary(counts: dict[str, int],
                  baseline: dict[str, int] | None,
                  strict: bool) -> None:
    print("Semantic/template boundary audit")
    print(f"{'category':34} {'count':>7} {'limit':>7} status")
    for category in CATEGORIES:
        count = counts[category.name]
        if strict:
            limit = 0
        elif baseline is not None:
            limit = baseline.get(category.name, 0)
        else:
            limit = count
        if count > limit:
            status = "FAIL"
        elif count < limit:
            status = "reduced"
        else:
            status = "ok"
        print(f"{category.name:34} {count:7d} {limit:7d} {status}")


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--root",
        type=Path,
        default=DEFAULT_SOURCE_ROOT,
        help=f"source root to scan, default: {DEFAULT_SOURCE_ROOT}",
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
        "--strict",
        action="store_true",
        help="require all tracked categories to be zero",
    )
    parser.add_argument(
        "--list-sites",
        action="store_true",
        help="list matching source locations after the summary",
    )
    args = parser.parse_args(argv)

    if not args.root.exists():
        print(f"error: source root does not exist: {args.root}", file=sys.stderr)
        return 2

    sites = count_sites(args.root)
    counts = {name: len(matches) for name, matches in sites.items()}

    baseline = None
    if not args.no_baseline and not args.strict:
        if not args.baseline.exists():
            print(f"error: baseline does not exist: {args.baseline}", file=sys.stderr)
            return 2
        try:
            baseline = load_baseline(args.baseline)
        except (OSError, ValueError, json.JSONDecodeError) as exc:
            print(f"error: failed to read baseline: {exc}", file=sys.stderr)
            return 2

    print_summary(counts, baseline, args.strict)

    failed = []
    for category in CATEGORIES:
        count = counts[category.name]
        if args.strict:
            limit = 0
        elif baseline is not None:
            limit = baseline.get(category.name, 0)
        else:
            limit = count
        if count > limit:
            failed.append(category.name)

    if args.list_sites:
        for category in CATEGORIES:
            print()
            print(f"[{category.name}] {category.description}")
            for site in sites[category.name]:
                print(site)

    if failed:
        print()
        print("New semantic/template boundary debt detected:")
        for name in failed:
            print(f"- {name}")
        print("Run with --list-sites to inspect matching locations.")
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
