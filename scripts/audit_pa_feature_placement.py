#!/usr/bin/env python3
"""Detect PA feature usage and report placement risks.

This is a source/ref audit helper. It is intentionally conservative: a match
means "review this test", not "move this test without reading it".
"""

from __future__ import annotations

import argparse
import csv
import json
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Iterable


DEFAULT_TRACKER = Path("docs/pa14-pa22-contract-test-audit-tracker.md")
DEFAULT_PAS = tuple(f"pa{i}" for i in range(14, 23))
SEMANTIC_ONLY_PA_MAX = 12
LOWIR_SOURCE_PAS = set(range(14, 23)) | set(range(26, 30))
BACKEND_ONLY_PAS = {23}


@dataclass(frozen=True)
class FeatureMeta:
    feature_id: str
    owner_pa: str
    owner_cluster: int
    n3485_ref: str
    required_detections: str


@dataclass(frozen=True)
class FeatureRule:
    feature_id: str
    patterns: tuple[re.Pattern[str], ...]
    all_patterns: bool = False
    use_raw: bool = False
    ref_patterns: tuple[re.Pattern[str], ...] = ()


@dataclass
class FeatureHit:
    feature_id: str
    evidence: list[str] = field(default_factory=list)


def rx(pattern: str) -> re.Pattern[str]:
    return re.compile(pattern, re.MULTILINE | re.DOTALL)


RULES: tuple[FeatureRule, ...] = (
    FeatureRule("lowir.procedural", (rx(r"\bint\s+main\s*\("),),
                ref_patterns=(rx(r"^function\s+@",),)),
    FeatureRule("lowir.procedural.local_static",
                (rx(r"\b[A-Za-z_][A-Za-z0-9_:<>*&\s]*\s+[A-Za-z_][A-Za-z0-9_:<>]*\s*\([^;{}]*\)\s*\{[^{}]*\bstatic\b[^{};]*[=;]"),),
                use_raw=True,
                ref_patterns=(rx(r"__local_static__|local_static_(?:init|ready)|__guard"),)),
    FeatureRule("lowir.procedural.local_static.dynamic_class",
                (rx(r"\bstatic\b[^;{}]*[A-Z][A-Za-z0-9_:<>]*[^;{}]*=\s*\{[^{}]*[A-Z][A-Za-z0-9_:<>]*\s*\("),),
                use_raw=True,
                ref_patterns=(rx(r"__local_static__.*(?:C[12]E|copyobj)|(?:C[12]E|copyobj).*__local_static__"),)),
    FeatureRule("lowir.procedural.float_conversion",
                (rx(r"\b(?:float|double|long\s+double)\b|"
                    r"(?<![A-Za-z0-9_])(?:[0-9]+\.[0-9]*|[0-9]*\.[0-9]+|[0-9]+[eE][+-]?[0-9]+)[fFlL]?"),),
                use_raw=True,
                ref_patterns=(rx(r"\b(?:f32|f64|f80)\b|convert\s+(?:fpto|[su]itofp|fpext|fptrunc)"),)),
    FeatureRule("lang.extended_integer", (rx(r"\b__int128\b"),),
                ref_patterns=(rx(r"\b[ui]128\b"),)),
    FeatureRule("stmt.condition_declaration",
                (rx(r"\b(?:if|switch)\s*\(\s*(?:const\s+|volatile\s+|unsigned\s+|signed\s+|long\s+|short\s+|int\b|bool\b|char\b|[A-Za-z_][A-Za-z0-9_:<>]*\s+[&*]?\s*[A-Za-z_])"),)),
    FeatureRule("expr.reference", (rx(r"(?:^|[^\w])(?:const\s+)?[A-Za-z_][A-Za-z0-9_:<>]*\s*&\s*[A-Za-z_]|\bstatic_cast\s*<[^>]*&"),)),
    FeatureRule("expr.array_pointer", (rx(r"\[[^]]*\]|\bnullptr\b|->|\b[A-Za-z_][A-Za-z0-9_]*\s*\+"),)),
    FeatureRule("lang.enum", (rx(r"\benum\b"),)),
    FeatureRule("expr.cast.builtin", (rx(r"\b(?:static_cast|const_cast|reinterpret_cast|dynamic_cast)\s*<|\([A-Za-z_][A-Za-z0-9_:<>\s*&]*\)\s*[A-Za-z_(0-9]"),)),
    FeatureRule("call.variadic_promotions",
                (rx(r"\b(?:int|void|char|short|long|float|double|signed|unsigned|[A-Za-z_][A-Za-z0-9_:<>*&\s]+)\s+"
                    r"[A-Za-z_][A-Za-z0-9_:]*\s*\([^)]*(?:,\s*)?\.\.\.\s*\)"),)),
    FeatureRule("class.basic", (rx(r"(?<!enum )\b(?:class|struct)\s+[A-Za-z_]"),)),
    FeatureRule("class.access_control", (rx(r"\b(?:public|private|protected)\s*:"),)),
    FeatureRule("class.nested_type",
                (rx(r"\b(?:typedef|using)\b[^;]*(?:::|class|struct)|\btypename\s+[A-Za-z_][A-Za-z0-9_:<>]*::"),)),
    FeatureRule("class.static_member",
                (rx(r"\b(?:class|struct)\s+\w+(?:\s*:[^{;]+)?\s*\{[^{}]*\bstatic\b[^;{}]*;"),)),
    FeatureRule("class.default_member_initializer",
                (rx(r"\b(?:class|struct)\s+\w+(?:\s*:[^{;]+)?\s*\{[^{}]*\b[A-Za-z_][A-Za-z0-9_:<>*&\s]*\s+[A-Za-z_][A-Za-z0-9_]*\s*=\s*[^;{}]+;"),)),
    FeatureRule("class.aggregate_init",
                (rx(r"(?:=\s*|return\s+)[A-Z][A-Za-z0-9_:<>]*\s*\{[^{};]*\}|"
                    r"\b[A-Z][A-Za-z0-9_:<>]*(?:\s*<[^;{}]*>)?\s+[a-z_][A-Za-z0-9_]*\s*\{[^{};]*\}"),)),
    FeatureRule("class.anonymous_member", (rx(r"\b(?:struct|union)\s*\{"),)),
    FeatureRule("class.friend", (rx(r"\bfriend\b"),)),
    FeatureRule("class.layout.bitfield", (rx(r"\b(?:int|bool|char|unsigned|signed|long|short)\s*(?:[A-Za-z_][A-Za-z0-9_]*)?\s*(?<!:):(?!:)\s*[^;{}]+;"),)),
    FeatureRule("class.bitfield.access_or_init",
                (rx(r"\b(?:int|bool|char|unsigned|signed|long|short)\s*(?:[A-Za-z_][A-Za-z0-9_]*)?\s*(?<!:):(?!:)\s*[^;{}]+;"),
                 rx(r"\.[A-Za-z_][A-Za-z0-9_]*|:[^;,(]+[,)]")),
                all_patterns=True),
    FeatureRule("class.inheritance.single", (rx(r"\b(?:class|struct)\s+\w+\s*:\s*(?:public|private|protected)?\s*\w+\s*\{"),)),
    FeatureRule("class.attribute.no_unique_address", (rx(r"\[\[\s*no_unique_address\s*\]\]"),)),
    FeatureRule("expr.new_delete", (rx(r"(?<!operator )\bnew\s+(?!;)|(?<!operator )\bdelete\s+(?:\[\]\s*)?[A-Za-z_(]"),),
                ref_patterns=(rx(r"operator_(?:new|delete)|cppgm_builtin_operator_(?:new|delete)|delete_nonnull"),)),
    FeatureRule("expr.pseudo_destructor", (rx(r"\.\s*~|->\s*~"),)),
    FeatureRule("function.default_argument", (rx(r"\([^)]*(?<!!)(?<![<>=])=(?!=)[^)]*\)"),)),
    FeatureRule("function.noexcept", (rx(r"\bnoexcept\b"),)),
    FeatureRule("lookup.adl", (rx(r"\bfriend\b|\boperator\s+(?!new\b|delete\b)"),)),
    FeatureRule("operator.overload", (rx(r"\boperator\s*(?!(?:new|delete)\b)(?:[+\-*/%<>=!&|^~,\[\]()]+|[A-Za-z_][A-Za-z0-9_:<>]*)"),)),
    FeatureRule("class.using_declaration", (rx(r"\busing\s+[A-Za-z_][A-Za-z0-9_:<>]*::[A-Za-z_]"),)),
    FeatureRule("class.inheriting_constructor",
                (rx(r"\busing\s+(?:[A-Za-z_][A-Za-z0-9_:<>]*::)*([A-Za-z_][A-Za-z0-9_]*)::\1\s*;"),)),
    FeatureRule("class.inheritance.multiple", (rx(r"\b(?:class|struct)\s+\w+\s*:[^{,]+,[^{]+"),)),
    FeatureRule("class.member_pointer", (rx(r"::\s*\*|\.\*|->\*"),)),
    FeatureRule("class.conversion_operator", (rx(r"\boperator\s+(?!new\b|delete\b)(?:const\s+)?[A-Za-z_][A-Za-z0-9_:<>*&\s]*\s*\("),)),
    FeatureRule("function.trailing_return", (rx(r"\)\s*->\s*[A-Za-z_][A-Za-z0-9_:<>*&\s]*"),)),
    FeatureRule("support.attribute", (rx(r"\[\[[^]]+\]\]|__attribute__\s*\("),)),
    FeatureRule("lifetime.ctor_dtor", (rx(r"\b~[A-Za-z_][A-Za-z0-9_]*\s*\(|\b[A-Za-z_][A-Za-z0-9_]*\s*\([^;{}]*\)\s*:"),),
                ref_patterns=(rx(r"C[12]E|D[012]E|__base_entry"),)),
    FeatureRule("value.copy_move",
                (rx(r"\b(?:copy|move)\b|operator\s*=(?!=)\s*\(\s*(?:const\s+)?[A-Za-z_][A-Za-z0-9_:<>]*\s*(?:&&|&)|"
                    r"\b([A-Za-z_][A-Za-z0-9_]*)\s*\(\s*(?:const\s+)?\1\s*(?:&&|&)"),),
                ref_patterns=(rx(r"\bcopyobj\b|aSERKS|aSEOS"),)),
    FeatureRule("value.by_value_abi", (rx(r"\b[A-Z][A-Za-z0-9_:<>]*\s+\w+\s*\([^)]*[A-Z][A-Za-z0-9_:<>]*\s+\w+[^)]*\)"),),
                ref_patterns=(rx(r"\bcopyobj\b|pass=by_value|return\s+obj<"),)),
    FeatureRule("value.temporary",
                (rx(r"\bconst\s+[A-Za-z_][A-Za-z0-9_:<>]*\s*&\s*\w+\s*=\s*[A-Z][A-Za-z0-9_:<>]*\s*\("),),
                ref_patterns=()),
    FeatureRule("value.ref_qualified_member", (rx(r"\)\s*(?:const\s*)?[&]{1,2}\s*(?:;|\{|->|noexcept)"),)),
    FeatureRule("value.delegating_ctor",
                (rx(r"\b([A-Za-z_][A-Za-z0-9_]*)\s*\([^)]*\)\s*:\s*\1\s*\("),)),
    FeatureRule("class.union", (rx(r"\bunion\b"),)),
    FeatureRule("expr.array_new_delete", (rx(r"\bnew\s+[^\[]*\[|\bdelete\s*\["),),
                ref_patterns=(rx(r"operator_(?:new|delete)_array|delete_array|array_cookie"),)),
    FeatureRule("lookup.using_directive", (rx(r"\busing\s+namespace\b|\busing\s+[A-Za-z_][A-Za-z0-9_:<>]*::[A-Za-z_]"),)),
    FeatureRule("support.lambda", (rx(r"\[[^]\n]*\]\s*\([^)]*\)\s*(?:mutable\s*)?(?:noexcept\s*)?(?:->|\{)"),)),
    FeatureRule("support.lambda.capture",
                (rx(r"\[[^]\n]*[A-Za-z_][A-Za-z0-9_]*[^]\n]*\]\s*\([^)]*\)\s*"
                    r"(?:mutable\s*)?(?:noexcept\s*)?(?:->[^{]+)?\{"),)),
    FeatureRule("support.range_for", (rx(r"\bfor\s*\([^:;()]+:[^)]*\)"),)),
    FeatureRule("support.decltype", (rx(r"\bdecltype\s*\("),)),
    FeatureRule("support.auto", (rx(r"\bauto\b"),)),
    FeatureRule("polymorphic.basic", (rx(r"\bvirtual\b"),),
                ref_patterns=(rx(r"__vtable|_ZTV|__rtti|_ZTI|typeinfo"),)),
    FeatureRule("polymorphic.override_final", (rx(r"\b(?:override|final)\b"),)),
    FeatureRule("polymorphic.vdtor", (rx(r"\bvirtual\s+~"),)),
    FeatureRule("polymorphic.vtable_order", (rx(r"\bvirtual\b"),),
                ref_patterns=(rx(r"__vtable|_ZTV"),)),
    FeatureRule("polymorphic.pointer_adjust",
                (rx(r"\bvirtual\b"), rx(r"\b(?:class|struct)\s+\w+\s*:[^{,]+,[^{]+|static_cast\s*<[^>]*[&*]")),
                all_patterns=True,
                ref_patterns=(rx(r"this_adjust|adjustor|thunk|vbase|vcall"),)),
    FeatureRule("template.type", (rx(r"\btemplate\s*<[^>]*(?:class|typename)\b"),)),
    FeatureRule("template.class", (rx(r"\btemplate\s*<[^>]*>\s*(?:class|struct)\b"),)),
    FeatureRule("template.function",
                (rx(r"\btemplate\s*<[^>]*>[^;{}()]*\b[A-Za-z_][A-Za-z0-9_:<>*&\s]*\s+[A-Za-z_][A-Za-z0-9_]*\s*\([^)]*\)\s*(?:\{|;)"),)),
    FeatureRule("template.default_argument", (rx(r"\btemplate\s*<[^>]*=[^>]*>"),)),
    FeatureRule("template.dependent_name", (rx(r"\btypename\s+[A-Za-z_][A-Za-z0-9_:<>]*::|[A-Za-z_][A-Za-z0-9_]*<[^>]*>::"),)),
    FeatureRule("template.friend", (rx(r"\bfriend\b[^;{]*\btemplate\b|\btemplate\s*<[^>]*>[^;{]*\bfriend\b"),)),
    FeatureRule("template.current_instantiation", (rx(r"\btypename\s+[A-Za-z_][A-Za-z0-9_]*::|[A-Za-z_][A-Za-z0-9_]*<[^>]*>::"),)),
    FeatureRule("template.disambiguator", (rx(r"\btypename\b|\btemplate\s+[A-Za-z_]"),)),
    FeatureRule("template.function_partial_ordering",
                (rx(r"\btemplate\s*<[^>]*>[^;{}()]*\b([A-Za-z_][A-Za-z0-9_]*)\s*\([^)]*\)\s*(?:\{|;).*?"
                    r"\btemplate\s*<[^>]*>[^;{}()]*\b\1\s*\("),)),
    FeatureRule("template.alignas_alignof", (rx(r"\balignas\s*\(|\balignof\s*\(|__alignof__\s*\("),)),
    FeatureRule("template.pack", (rx(r"\btemplate\s*<[^>]*\.\.\."),)),
    FeatureRule("template.template_parameter", (rx(r"\btemplate\s*<[^>]*template\s*<"),)),
    FeatureRule("template.member_template", (rx(r"\btemplate\s*<[^>]*>[^;{]*(?:operator|[A-Za-z_][A-Za-z0-9_]*::)"),)),
    FeatureRule("template.builtin_traits", (rx(r"\b__(?:is|has|is_trivially|is_nothrow|builtin)_[A-Za-z0-9_]*\b|__builtin_[A-Za-z0-9_]*"),)),
    FeatureRule("template.nttp", (rx(r"\btemplate\s*<[^>]*(?:int|bool|char|long|short|unsigned|signed|std::size_t|size_t)\s+\w+"),)),
    FeatureRule("template.nttp.pointer_member",
                (rx(r"\btemplate\s*<[^>]*(?:[A-Za-z_][A-Za-z0-9_:<>]*\s*[*&]\s*\w+|[A-Za-z_][A-Za-z0-9_:<>]*::\s*\*\s*\w+)"),)),
    FeatureRule("template.explicit_specialization", (rx(r"\btemplate\s*<\s*>\s*"),)),
    FeatureRule("template.specialization_timing", (rx(r"\btemplate\s*<\s*>\s*"),)),
    FeatureRule("constexpr.integral_subset",
                (rx(r"\bconstexpr\b|\bstatic_assert\b|\btemplate\s*<[^>]*(?:int|bool|char|long|short|unsigned|signed)\s+\w+"),)),
    FeatureRule("static_assert", (rx(r"\bstatic_assert\s*\("),)),
    FeatureRule("constexpr.full", (rx(r"\bconstexpr\b"),)),
    FeatureRule("constexpr.control_flow", (rx(r"\bconstexpr\b"), rx(r"\b(?:if|for|while|do|switch)\b"),), all_patterns=True),
    FeatureRule("constexpr.default_argument", (rx(r"\bconstexpr\b"), rx(r"\([^)]*=[^)]*\)")), all_patterns=True),
    FeatureRule("constexpr.floating", (rx(r"\bconstexpr\b"), rx(r"\b(?:float|double|long\s+double)\b|[0-9]*\.[0-9]+")), all_patterns=True),
    FeatureRule("constexpr.noexcept", (rx(r"\bconstexpr\b|\bstatic_assert\b"), rx(r"\bnoexcept\b")), all_patterns=True),
    FeatureRule("constexpr.object", (rx(r"\bconstexpr\b"), rx(r"\b(?:class|struct|\{)")), all_patterns=True),
    FeatureRule("template.partial_specialization", (rx(r"\btemplate\s*<[^>]+>\s*(?:class|struct)\s+[A-Za-z_][A-Za-z0-9_]*\s*<"),)),
    FeatureRule("template.alias", (rx(r"\btemplate\s*<[^>]*>\s*using\b|\busing\s+\w+\s*="),)),
    FeatureRule("template.variable", (rx(r"\btemplate\s*<[^>]*>\s*(?:constexpr\s+|const\s+|static\s+)?(?:bool|int|char|long|unsigned|signed|[A-Za-z_][A-Za-z0-9_:<>]*)\s+\w+\s*="),)),
    FeatureRule("template.current_specialization", (rx(r"\btypename\s+[A-Za-z_][A-Za-z0-9_]*::|[A-Za-z_][A-Za-z0-9_]*<[^>]*>::"),)),
    FeatureRule("template.explicit_instantiation", (rx(r"\bextern\s+template\b|\btemplate\s+(?:class|struct|[A-Za-z_])"),)),
    FeatureRule("template.specialization_partial_ordering", (rx(r"\btemplate\s*<[^>]+>\s*(?:class|struct)\s+[A-Za-z_][A-Za-z0-9_]*\s*<"),)),
    FeatureRule("template.deduction_full",
                (rx(r"\btemplate\s*<[^>]*>[^;{}()]*\b[A-Za-z_][A-Za-z0-9_:<>*&\s]*\s+[A-Za-z_][A-Za-z0-9_]*\s*\([^)]*(?:&&|decltype|enable_if|typename\s+[A-Za-z_][A-Za-z0-9_:<>]*::)[^)]*\)"),)),
    FeatureRule("template.detector_idiom", (rx(r"\b(?:void_t|detected_or|detector|is_detected)\b"),)),
    FeatureRule("template.substitution", (rx(r"\b(?:enable_if|void_t|typename\s+[A-Za-z_][A-Za-z0-9_:<>]*::|decltype\s*\()"),)),
    FeatureRule("template.conversion_deduction", (rx(r"\btemplate\s*<[^>]*>[^;{]*operator\s+[A-Za-z_]"),)),
    FeatureRule("template.constructor_deduction", (rx(r"\btemplate\s*<[^>]*>\s*[A-Za-z_][A-Za-z0-9_]*\s*\("),)),
    FeatureRule("template.initializer_list", (rx(r"\binitializer_list\b"),)),
    FeatureRule("template.no_eager_instantiation", (rx(r"\b(?:no_eager|unevaluated|dependent|static_assert\s*\(\s*false|sizeof\s*\([^)]*typename)"),)),
    FeatureRule("sfinae", (rx(r"\b(?:enable_if|void_t|sfinae|SFINAE|detected_or|detector|is_detected)\b"),)),
    FeatureRule("template.braced_init_deduction",
                (rx(r"\btemplate\s*<[^>]*>[^;{}()]*\b[A-Za-z_][A-Za-z0-9_:<>*&\s]*\s+"
                    r"[A-Za-z_][A-Za-z0-9_]*\s*\([^)]*\)\s*(?:\{|;).*?"
                    r"\b[A-Za-z_][A-Za-z0-9_]*\s*\(\s*\{"),)),
    FeatureRule("template.non_deduced_context", (rx(r"\b(?:non_deduced|identity|decltype\s*\(|typename\s+[A-Za-z_][A-Za-z0-9_:<>]*::)"),)),
)


def pa_number(pa: str) -> int | None:
    match = re.fullmatch(r"pa(\d+)", pa)
    return int(match.group(1)) if match else None


def split_markdown_row(line: str) -> list[str]:
    return [cell.strip() for cell in line.strip().strip("|").split("|")]


def strip_backticks(value: str) -> str:
    value = value.strip()
    if value.startswith("`") and value.endswith("`"):
        return value[1:-1]
    return value


def load_feature_table(path: Path) -> dict[str, FeatureMeta]:
    features: dict[str, FeatureMeta] = {}
    in_table = False
    with path.open("r", encoding="utf-8") as handle:
        for line in handle:
            if line.startswith("| Feature Family |"):
                in_table = True
                continue
            if not in_table:
                continue
            if line.startswith("| ---"):
                continue
            if not line.startswith("|"):
                if features:
                    break
                continue
            cells = split_markdown_row(line)
            if len(cells) < 6:
                continue
            feature_id = strip_backticks(cells[0])
            owner_pa = strip_backticks(cells[1])
            try:
                owner_cluster = int(cells[2])
            except ValueError:
                continue
            features[feature_id] = FeatureMeta(
                feature_id=feature_id,
                owner_pa=owner_pa,
                owner_cluster=owner_cluster,
                n3485_ref=cells[3],
                required_detections=cells[5],
            )
    if not features:
        raise ValueError(f"no feature table found in {path}")
    return features


def strip_comments(source: str) -> str:
    out: list[str] = []
    i = 0
    in_block = False
    in_line = False
    in_string: str | None = None
    while i < len(source):
        c = source[i]
        n = source[i + 1] if i + 1 < len(source) else ""
        if in_line:
            if c == "\n":
                in_line = False
                out.append(c)
            else:
                out.append(" ")
            i += 1
            continue
        if in_block:
            if c == "*" and n == "/":
                out.extend("  ")
                i += 2
                in_block = False
            else:
                out.append("\n" if c == "\n" else " ")
                i += 1
            continue
        if in_string:
            out.append(c)
            if c == "\\" and i + 1 < len(source):
                out.append(source[i + 1])
                i += 2
                continue
            if c == in_string:
                in_string = None
            i += 1
            continue
        if c == "/" and n == "/":
            out.extend("  ")
            in_line = True
            i += 2
            continue
        if c == "/" and n == "*":
            out.extend("  ")
            in_block = True
            i += 2
            continue
        if c in ("'", '"'):
            in_string = c
        out.append(c)
        i += 1
    return "".join(out)


def strip_string_literals(source: str) -> str:
    out: list[str] = []
    i = 0
    in_string: str | None = None
    while i < len(source):
        c = source[i]
        if in_string:
            if c == "\\" and i + 1 < len(source):
                out.extend("  ")
                i += 2
                continue
            out.append(" ")
            if c == in_string:
                in_string = None
            i += 1
            continue
        if c in ("'", '"'):
            in_string = c
            out.append(" ")
            i += 1
            continue
        out.append(c)
        i += 1
    return "".join(out)


def read_text(path: Path) -> str:
    try:
        return path.read_text(encoding="utf-8")
    except UnicodeDecodeError:
        return path.read_text(encoding="latin-1")


def match_rule_patterns(patterns: tuple[re.Pattern[str], ...],
                        haystack: str,
                        all_patterns: bool,
                        prefix: str) -> list[str]:
    if not patterns:
        return []
    matched: list[str] = []
    for pattern in patterns:
        match = pattern.search(haystack)
        if match:
            evidence = " ".join(match.group(0).split())
            matched.append(f"{prefix}:{evidence[:120]}")
    if all_patterns and len(matched) != len(patterns):
        return []
    return matched


def detect_features(source: str, ref_text: str = "") -> dict[str, FeatureHit]:
    no_comments = strip_comments(source)
    code = strip_string_literals(no_comments)
    hits: dict[str, FeatureHit] = {}
    for rule in RULES:
        haystack = no_comments if rule.use_raw else code
        matched = match_rule_patterns(rule.patterns, haystack, rule.all_patterns, "source")
        matched.extend(match_rule_patterns(rule.ref_patterns, ref_text, rule.all_patterns, "ref"))
        if matched:
            hits[rule.feature_id] = FeatureHit(rule.feature_id, matched)
    return hits


def iter_test_files(root: Path, pas: Iterable[str], include_course: bool) -> list[Path]:
    files: list[Path] = []
    for pa in pas:
        test_root = root / pa / "tests"
        if test_root.exists():
            files.extend(sorted(test_root.rglob("*.t")))
        if include_course:
            course_root = root / "cppgm.tests" / "course" / pa
            if course_root.exists():
                files.extend(sorted(course_root.rglob("*.t")))
    return files


def current_pa_for(path: Path) -> str:
    parts = path.parts
    if "cppgm.tests" in parts and "course" in parts:
        idx = parts.index("course")
        if idx + 1 < len(parts):
            return parts[idx + 1]
    for part in parts:
        if re.fullmatch(r"pa\d+", part):
            return part
    return "unknown"


def test_role_for(path: Path) -> str:
    parts = path.parts
    if "cppgm.tests" in parts and "course" in parts:
        return "course-current"
    if "general" in parts:
        return "general"
    if "spec" in parts:
        return "spec"
    return "other"


def cluster_for(path: Path) -> int | None:
    match = re.match(r"(\d{3})-", path.name)
    return int(match.group(1)) if match else None


def expected_exit_for(path: Path) -> str:
    ref_status = path.with_suffix(".ref.exit_status")
    if ref_status.exists():
        return read_text(ref_status).strip()
    return "unknown"


def ref_text_for(path: Path) -> str:
    ref = path.with_suffix(".ref")
    if ref.exists():
        return read_text(ref)
    return ""


def is_lowir_test(pa: str) -> bool:
    number = pa_number(pa)
    return number in LOWIR_SOURCE_PAS if number is not None else False


def placement_for(feature: FeatureMeta, current_pa: str, current_cluster: int | None) -> tuple[str, str]:
    current_num = pa_number(current_pa)
    owner_num = pa_number(feature.owner_pa)
    if current_num is None or owner_num is None:
        return "review", "non-PA path or owner"
    if current_num >= 14 and owner_num <= SEMANTIC_ONLY_PA_MAX:
        return (
            "semantic-owner",
            "semantic owner cannot own LowIR output; place by enclosing LowIR feature",
        )
    if owner_num in BACKEND_ONLY_PAS and is_lowir_test(current_pa):
        return (
            "backend-owner",
            "backend owner expects LowIR/backend input; source-to-LowIR owner must be chosen",
        )
    if current_num < owner_num:
        return "violation", f"feature owner {feature.owner_pa} is later than {current_pa}"
    if current_num == owner_num and current_cluster is not None and current_cluster < feature.owner_cluster:
        return (
            "cluster-early",
            f"feature first cluster {feature.owner_cluster} is later than {current_cluster}",
        )
    return "ok", "owner reached"


def row_for(path: Path,
            root: Path,
            features: dict[str, FeatureMeta]) -> dict[str, object]:
    source = read_text(path)
    ref_text = ref_text_for(path)
    current_pa = current_pa_for(path.relative_to(root))
    current_cluster = cluster_for(path)
    hits = detect_features(source, ref_text)
    detected = sorted(hits)
    placements = []
    for feature_id in detected:
        meta = features.get(feature_id)
        if not meta:
            placements.append({
                "feature": feature_id,
                "status": "unknown-feature",
                "reason": "rule has no feature-table row",
                "owner_pa": "",
                "owner_cluster": None,
            })
            continue
        status, reason = placement_for(meta, current_pa, current_cluster)
        placements.append({
            "feature": feature_id,
            "status": status,
            "reason": reason,
            "owner_pa": meta.owner_pa,
            "owner_cluster": meta.owner_cluster,
            "evidence": hits[feature_id].evidence,
        })
    review_statuses = {"violation", "cluster-early", "backend-owner", "unknown-feature"}
    semantic_notes = [p for p in placements if p["status"] == "semantic-owner"]
    return {
        "path": path.relative_to(root).as_posix(),
        "current_pa": current_pa,
        "current_cluster": current_cluster,
        "test_role": test_role_for(path.relative_to(root)),
        "expected_exit": expected_exit_for(path),
        "detected_features": detected,
        "placements": placements,
        "needs_review": any(p["status"] in review_statuses for p in placements),
        "semantic_owner_notes": len(semantic_notes),
    }


def markdown_report(rows: list[dict[str, object]],
                    missing_rules: list[str],
                    include_ok: bool) -> str:
    review_rows = [row for row in rows if row["needs_review"] or include_ok]
    violations = sum(
        1
        for row in rows
        for placement in row["placements"]  # type: ignore[index]
        if placement["status"] in {"violation", "cluster-early", "backend-owner", "unknown-feature"}
    )
    semantic_notes = sum(int(row["semantic_owner_notes"]) for row in rows)
    lines = [
        "# PA Feature Placement Audit",
        "",
        f"- tests scanned: {len(rows)}",
        f"- tests needing review: {sum(1 for row in rows if row['needs_review'])}",
        f"- placement findings: {violations}",
        f"- semantic-owner notes: {semantic_notes}",
        f"- feature table entries without detector rules: {len(missing_rules)}",
        "",
    ]
    if missing_rules:
        lines.append("## Missing Detector Rules")
        lines.append("")
        for feature_id in missing_rules:
            lines.append(f"- `{feature_id}`")
        lines.append("")
    lines.append("## Placement Findings")
    lines.append("")
    lines.append("| Test | Current | Feature | Owner | Status | Reason | Evidence |")
    lines.append("| --- | --- | --- | --- | --- | --- | --- |")
    for row in review_rows:
        for placement in row["placements"]:  # type: ignore[index]
            if not include_ok and placement["status"] not in {
                "violation",
                "cluster-early",
                "backend-owner",
                "unknown-feature",
            }:
                continue
            current = f"{row['current_pa']}:{row['current_cluster']}"
            owner_cluster = placement.get("owner_cluster")
            owner = f"{placement.get('owner_pa')}:{owner_cluster}" if owner_cluster else str(placement.get("owner_pa"))
            evidence = ", ".join(placement.get("evidence", [])[:2])
            lines.append(
                "| `{}` | `{}` | `{}` | `{}` | `{}` | {} | `{}` |".format(
                    row["path"],
                    current,
                    placement["feature"],
                    owner,
                    placement["status"],
                    str(placement["reason"]).replace("|", "\\|"),
                    evidence.replace("|", "\\|"),
                )
            )
    return "\n".join(lines) + "\n"


def write_json(path: Path, rows: list[dict[str, object]], missing_rules: list[str]) -> None:
    payload = {
        "tests": rows,
        "missing_detector_rules": missing_rules,
    }
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def write_csv(path: Path, rows: list[dict[str, object]]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.writer(handle)
        writer.writerow([
            "path",
            "current_pa",
            "current_cluster",
            "test_role",
            "expected_exit",
            "feature",
            "owner_pa",
            "owner_cluster",
            "status",
            "reason",
            "evidence",
        ])
        for row in rows:
            for placement in row["placements"]:  # type: ignore[index]
                writer.writerow([
                    row["path"],
                    row["current_pa"],
                    row["current_cluster"],
                    row["test_role"],
                    row["expected_exit"],
                    placement["feature"],
                    placement.get("owner_pa", ""),
                    placement.get("owner_cluster", ""),
                    placement["status"],
                    placement["reason"],
                    "; ".join(placement.get("evidence", [])),
                ])


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path("."))
    parser.add_argument("--tracker", type=Path, default=DEFAULT_TRACKER)
    parser.add_argument("--pa", action="append", choices=[f"pa{i}" for i in range(1, 38)])
    parser.add_argument("--feature", action="append", help="only report tests matching this feature id")
    parser.add_argument("--include-course", action="store_true", default=True)
    parser.add_argument("--no-course", action="store_false", dest="include_course")
    parser.add_argument("--include-ok", action="store_true", help="include ok/semantic-owner rows in markdown")
    parser.add_argument("--json-out", type=Path)
    parser.add_argument("--csv-out", type=Path)
    parser.add_argument("--markdown-out", type=Path)
    args = parser.parse_args(argv)

    root = args.root.resolve()
    tracker = args.tracker if args.tracker.is_absolute() else root / args.tracker
    if not tracker.exists():
        print(f"error: tracker not found: {tracker}", file=sys.stderr)
        return 2
    features = load_feature_table(tracker)
    rule_ids = {rule.feature_id for rule in RULES}
    missing_rules = sorted(set(features) - rule_ids)

    pas = tuple(args.pa) if args.pa else DEFAULT_PAS
    tests = iter_test_files(root, pas, args.include_course)
    rows = [row_for(path, root, features) for path in tests]
    if args.feature:
        wanted = set(args.feature)
        rows = [
            row for row in rows
            if wanted.intersection(set(row["detected_features"]))  # type: ignore[arg-type]
        ]
    if args.json_out:
        write_json(args.json_out if args.json_out.is_absolute() else root / args.json_out,
                   rows,
                   missing_rules)
    if args.csv_out:
        write_csv(args.csv_out if args.csv_out.is_absolute() else root / args.csv_out,
                  rows)
    report = markdown_report(rows, missing_rules, args.include_ok)
    if args.markdown_out:
        out = args.markdown_out if args.markdown_out.is_absolute() else root / args.markdown_out
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_text(report, encoding="utf-8")
    else:
        print(report, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
