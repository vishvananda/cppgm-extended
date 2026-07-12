#!/usr/bin/env python3
"""Audit semantic text reparse debt.

The default audit is a ratchet: current legacy debt can stay temporarily, but
new debt must not be introduced while structured carriers replace it. Strict
mode requires every category to reach zero.
"""

from __future__ import annotations

import argparse
import json
import re
import subprocess
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
    diff_only: bool = False
    whole_file: bool = False


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
            r"repaired_template_argument_syntax|"
            r"dependent_type_expr_text)\s*\("
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
        "semantic_expression_text_reparse",
        "Expression and pack semantics recovered by manually decomposing text.",
        re.compile(
            r"\btry_evaluate_integral_text_with_pack_scope\s*\(|"
            r"\bfind_top_level_binary_operator(?:_token)?_text\s*\(|"
            r"\b(?:parse_sizeof_pack_count|lookup_integral_constant_count|"
            r"evaluate_integer_pack_count)_text\s*\(|"
            r"\binteger_pack_prefix\b|"
            r"\brewrite_decltype_expression_pack_texts(?:_impl)?\s*\(|"
            r"\bsplit_top_level_call_expression_text\s*\("
        ),
    ),
    Category(
        "semantic_template_id_text_decomposition",
        "Template-id structure or dependencies recovered from semantic text.",
        re.compile(
            r"\bsemantic_utils::split_top_level_template_id_text\s*\(|"
            r"\bdeduce_from_named_template_id_text\s*\(|"
            r"\btemplate_id_syntax_from_component_text\s*\(|"
            r"\bqualifier_template_id_syntaxes_from_text\s*\(|"
            r"\bcollect_enable_if_condition_dependency_from_type_text\s*\(|"
            r"\bparse_external_named_text_candidate\s*\(|"
            r"\bparse_unary_builtin_type_transform_syntax_text\s*\(|"
            r"\btemplate_lookup_fragment_(?:text|identifier)\s*\(|"
            r"\bsplit_unqualified_template_head_text\s*\(|"
            r"\bparse_(?:unary|angle)_type_transform_text\s*\("
        ),
    ),
    Category(
        "semantic_qualified_name_text_reparse",
        "Qualified-name AST state refreshed from substituted semantic text.",
        re.compile(
            r"\brefresh_substituted_member_value_expression\s*\(|"
            r"\bparse_out_of_class_member_qualified_name\s*\(|"
            r"\brefresh_qualified_name_qualifier_template_id_texts\s*\(|"
            r"\b(?:semantic_utils::)?split_qualified_name_text\s*\(|"
            r"\b(?:reparse|recover|refresh)_[A-Za-z0-9_]*qualified"
            r"[A-Za-z0-9_]*(?:from|with)_text\s*\("
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
    Category(
        "function_result_argument_text_reparse",
        "Function-template result arguments recovered from saved argument text.",
        re.compile(
            r"\binstantiated_result_[A-Za-z0-9_]*argument_text\s*\(|"
            r"\bresult_[A-Za-z0-9_]*argument_text_reparse\b"
        ),
    ),
    Category(
        "owner_member_text_reparse",
        "Owner/member references recovered from saved source text in semantic code.",
        re.compile(
            r"\bpattern_mentions_bound_type_pack_value_member\b|"
            r"\bsplit_top_level_member_expression_text\s*\(|"
            r"\bcollect_non_type_parameter_pack_references_from_text\s*\(|"
            r"\b[A-Za-z0-9_]*(?:owner|member|qualified_member)"
            r"[A-Za-z0-9_]*_text_reparse\b|"
            r"\b(?:resolve|lookup|evaluate)_[A-Za-z0-9_]*"
            r"(?:owner|member)[A-Za-z0-9_]*_from_text\s*\("
        ),
    ),
    Category(
        "abi_template_component_text_reparse",
        "ABI template components recovered by splitting rendered name text.",
        re.compile(
            r"\bparse_template_component\s*\(|"
            r"\bsplit_template_arguments\s*\("
        ),
    ),
    Category(
        "semantic_argument_spelling_recovery",
        "Template and non-type argument semantics recovered from saved spelling.",
        re.compile(
            r"\bevaluate_non_type_argument_text\s*\(|"
            r"\btry_parse_builtin_type_trait_text\s*\(|"
            r"\bresolve_template_template_argument_text\s*\(|"
            r"\bresolve_member_template_template_argument_text\s*\(|"
            r"\bresolve_member_template_owner_type_text\s*\(|"
            r"\blookup_rewritten_bound_type_argument\s*\(|"
            r"\btemplate_argument_text_matches_type_binding\s*\(|"
            r"\bannotate_template_id_type_arguments_"
            r"from_matching_scope_bindings\s*\("
        ),
    ),
    Category(
        "template_parameter_display_lookup",
        "Template parameters recovered by looking up rendered type display text.",
        re.compile(
            r"\bfind_(?:template_parameter(?:_by_name)?|parameter_for_type_text)"
            r"\s*\([\s\S]{0,320}?\bnamed_display\b"
        ),
        whole_file=True,
    ),
    Category(
        "added_semantic_text_reparse",
        "Newly added semantic code that resolves or parses template/type facts from text.",
        re.compile(
            r"\b(?:template_api|template_argument_semantics)::"
            r"(?:resolve_template_arguments|"
            r"resolve_template_template_argument_text|"
            r"resolve_type_argument_text|parse_type_argument_text)\s*\(|"
            r"\b(?:resolve|lookup|parse|evaluate)_[A-Za-z0-9_]*"
            r"(?:template|type|argument|owner|member)[A-Za-z0-9_]*_text\s*\(|"
            r"\bsemantic_utils::split_top_level_template_id_text\s*\("
        ),
        diff_only=True,
    ),
]


SOURCE_SUFFIXES = {".cpp", ".h", ".hpp", ".inc"}


def iter_source_files(root: Path) -> Iterable[Path]:
    for path in sorted(root.rglob("*")):
        if path.is_file() and path.suffix in SOURCE_SUFFIXES:
            yield path


def git_added_lines(root: Path) -> Iterable[tuple[str, int | None, str]]:
    try:
        top_result = subprocess.run(
            ["git", "rev-parse", "--show-toplevel"],
            cwd=Path.cwd(),
            check=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            text=True,
        )
    except (OSError, subprocess.CalledProcessError):
        return

    repo_root = Path(top_result.stdout.strip()).resolve()
    scan_root = root
    if not scan_root.is_absolute():
        scan_root = (Path.cwd() / scan_root).resolve()
    try:
        root_rel = scan_root.relative_to(repo_root).as_posix()
    except ValueError:
        root_rel = root.as_posix()

    hunk_re = re.compile(r"@@ -\d+(?:,\d+)? \+(\d+)(?:,\d+)? @@")
    commands = [
        ("worktree", ["git", "diff", "--unified=0", "--no-ext-diff", "--", root_rel]),
        ("index", ["git", "diff", "--cached", "--unified=0", "--no-ext-diff", "--", root_rel]),
    ]
    for label, command in commands:
        try:
            result = subprocess.run(
                command,
                cwd=repo_root,
                check=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.DEVNULL,
                text=True,
            )
        except (OSError, subprocess.CalledProcessError):
            continue

        current_path: str | None = None
        new_lineno: int | None = None
        for raw_line in result.stdout.splitlines():
            if raw_line.startswith("+++ b/"):
                current_path = raw_line[len("+++ b/"):]
                new_lineno = None
                continue
            match = hunk_re.match(raw_line)
            if match:
                new_lineno = int(match.group(1))
                continue
            if raw_line.startswith("+") and not raw_line.startswith("+++"):
                if current_path is not None:
                    yield (f"{label}:{current_path}", new_lineno, raw_line[1:])
                if new_lineno is not None:
                    new_lineno += 1
                continue
            if raw_line.startswith("-"):
                continue
            if new_lineno is not None:
                new_lineno += 1


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
                if category.diff_only or category.whole_file:
                    continue
                if category.pattern.search(line):
                    sites[category.name].append(f"{rel}:{lineno}:{line.strip()}")
        text = "\n".join(lines)
        for category in CATEGORIES:
            if category.diff_only or not category.whole_file:
                continue
            for match in category.pattern.finditer(text):
                lineno = text.count("\n", 0, match.start()) + 1
                snippet = " ".join(match.group(0).split())
                sites[category.name].append(f"{rel}:{lineno}:{snippet}")
    diff_categories = [category for category in CATEGORIES if category.diff_only]
    if diff_categories:
        for rel, lineno, line in git_added_lines(root):
            location = f"{rel}:{lineno if lineno is not None else '?'}:{line.strip()}"
            for category in diff_categories:
                if category.pattern.search(line):
                    sites[category.name].append(location)
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
