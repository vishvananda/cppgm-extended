#!/usr/bin/env python3
"""Audit semantic text reparse debt.

The audit is intentionally a ratchet: current debt can stay temporarily, but
new debt should not be introduced while structured carriers replace it.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable


DEFAULT_ROOT = Path("dev/src")
DEFAULT_BASELINE = Path("docs/text-reparse-audit-baseline.json")


@dataclass(frozen=True)
class Category:
    name: str
    description: str
    pattern: re.Pattern[str]


CATEGORIES = [
    Category(
        "qualified_name_string_parse",
        "Qualified-name structure recovered from strings.",
        re.compile(r"\bparse_qualified_name_string\s*\("),
    ),
    Category(
        "template_id_string_parse",
        "Template-id structure recovered from strings.",
        re.compile(r"\bparse_template_id_string(?:_in_scope|_scoped)?\s*\("),
    ),
    Category(
        "expression_fragment_parse",
        "Expression AST recovered from compiler-produced fragments.",
        re.compile(r"\bparse_expression_fragment\s*\("),
    ),
    Category(
        "type_fragment_parse",
        "Type-id AST recovered from compiler-produced fragments.",
        re.compile(r"\bparse_type_fragment\s*\("),
    ),
    Category(
        "semantic_type_text_bridge",
        "Semantic type text routed back through type parsing bridges.",
        re.compile(
            r"\bctx\.parse_type_text\s*\(|"
            r"\btemplate_decl_ast::parse_type_text(?:_scoped)?\s*\(|"
            r"\b(?:template_argument_semantics|template_api)::"
            r"parse_type_text_for_(?:templates|deduction|lookup)\s*\("
        ),
    ),
    Category(
        "translation_unit_fragment_parse",
        "Translation-unit AST recovered from rebuilt fragments.",
        re.compile(r"\bparse_translation_unit_fragment\s*\("),
    ),
    Category(
        "ast_text_rebuild",
        "AST serialized to text; allowed only for formatting unless reparsed.",
        re.compile(r"\b(?:rebuild_node_text|fully_spaced_node_text)\s*\("),
    ),
    Category(
        "source_line_recovery",
        "Source-use or witness facts recovered by scanning original lines.",
        re.compile(
            r"\b(?:source_lines_for_file|source_location_for_identifier"
            r"(?:_on_or_after|_on_same_line_at_or_after)?|"
            r"nested_source_template_id_occurrences(?:_at_location)?)\s*\("
        ),
    ),
    Category(
        "template_argument_text_shape_deduction",
        "Template-argument deduction facts recovered from argument spelling shape.",
        re.compile(
            r"\bdeduce_template_argument_text_shape\b|"
            r"\bdeduce(?:-[A-Za-z]+)*-text-shape(?:-[A-Za-z]+)*\b"
        ),
    ),
    Category(
        "semantic_template_fragment_reparse",
        "Template argument syntax recovered by tokenizing semantic text fragments.",
        re.compile(
            r"\bparse_(?:type_argument_text_syntax|"
            r"non_type_argument_text_expression_syntax|"
            r"repaired_template_argument_syntax)\s*\("
        ),
    ),
    Category(
        "semantic_text_tokenizer_reparse",
        "Semantic text fragments routed back through the C++ tokenizer/parser stack.",
        re.compile(
            r"\bPPTokenizer\s+pp_tokens\s*\(|"
            r"\bPostTokenizer\s+post_tokens\s*\(|"
            r"\bRecogTokenizer\s+recog_tokens\s*\("
        ),
    ),
    Category(
        "manual_template_argument_text_parse",
        "Template argument semantics recovered by manually parsing source text.",
        re.compile(
            r"\bparse_simple_(?:fundamental_type|integral_value)_text\s*\(|"
            r"\b(?:split|try_evaluate_target)_cstyle_cast_integral_text\s*\(|"
            r"\bsplit_top_level_function_type_argument_text\s*\("
        ),
    ),
    Category(
        "semantic_nttp_text_rebind",
        "Non-type template argument semantics recovered by re-looking up saved text.",
        re.compile(
            r"\btry_analyze_non_type_template_member_pointer_text\b|"
            r"\bnon_type_template_argument_text\b.*\blookup_(?:value|functions)\b|"
            r"\blookup_(?:value|functions)\b.*\bnon_type_template_argument_text\b|"
            r"\brebound_text\b"
        ),
    ),
]


SOURCE_SUFFIXES = {".cpp", ".h", ".hpp", ".inc"}


def iter_source_files(root: Path) -> Iterable[Path]:
    for path in sorted(root.rglob("*")):
        if path.is_file() and path.suffix in SOURCE_SUFFIXES:
            yield path


def count_sites(root: Path) -> dict[str, list[str]]:
    sites = {category.name: [] for category in CATEGORIES}
    for path in iter_source_files(root):
        try:
            lines = path.read_text(encoding="utf-8").splitlines()
        except UnicodeDecodeError:
            lines = path.read_text(encoding="latin-1").splitlines()
        rel = path.as_posix()
        for lineno, line in enumerate(lines, 1):
            for category in CATEGORIES:
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
    print("Text reparse audit")
    print(f"{'category':36} {'count':>7} {'limit':>7} status")
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
        print(f"{category.name:36} {count:7d} {limit:7d} {status}")


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--root",
        type=Path,
        default=DEFAULT_ROOT,
        help=f"source root to scan, default: {DEFAULT_ROOT}",
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
        print(f"error: scan root does not exist: {args.root}", file=sys.stderr)
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

    failed_categories = []
    for category in CATEGORIES:
        count = counts[category.name]
        if args.strict:
            limit = 0
        elif baseline is not None:
            limit = baseline.get(category.name, 0)
        else:
            limit = count
        if count > limit:
            failed_categories.append(category.name)

    if args.list_sites:
        for category in CATEGORIES:
            print()
            print(f"[{category.name}] {category.description}")
            for site in sites[category.name]:
                print(site)

    if failed_categories:
        print()
        print("New or unclosed text reparse debt detected:")
        for name in failed_categories:
            print(f"- {name}")
        print("Run with --list-sites to inspect matching locations.")
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
