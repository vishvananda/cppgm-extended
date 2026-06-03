#!/usr/bin/env python3
"""Detect PA feature usage and report placement risks.

This is a source/ref audit helper. It is intentionally conservative: a match
means "review this test", not "move this test without reading it".

Source and reference-output matches drive placement failures. Filename matches
are retained as path hints for review, but they do not make CI fail.
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
STRICT_TEMPLATE_PAS = ("pa18", "pa19", "pa21", "pa22", "pa23")
SEMANTIC_ONLY_PA_MAX = 12
LOWIR_SOURCE_PAS = set(range(14, 28))
BACKEND_ONLY_PAS = {28}
EARLY_PLACEMENT_STATUSES = {"violation", "cluster-early"}
LATE_PLACEMENT_IGNORED_FEATURES = {
    "lowir.procedural",
}
LATE_PLACEMENT_BROAD_FEATURES = {
    "stmt.condition_declaration",
    "expr.reference",
    "expr.array_pointer",
    "expr.cast.builtin",
    "lang.enum",
    "class.basic",
    "template.type",
    "template.class",
    "template.function",
}

TEMPLATE_CONCEPT_BY_FEATURE = {
    "template.type": "basic-template",
    "template.class": "basic-template",
    "template.function": "basic-template",
    "template.default_argument": "basic-template",
    "template.dependent_name": "dependent-name",
    "template.friend": "friend-template",
    "template.current_instantiation": "current-instantiation",
    "template.disambiguator": "dependent-name",
    "template.function_partial_ordering": "function-partial-ordering",
    "template.pack": "pack-expansion",
    "template.template_parameter": "template-template-parameter",
    "template.member_template": "member-template",
    "template.nttp": "integral-nttp",
    "template.nttp.pointer_member": "value-nttp",
    "template.explicit_specialization": "explicit-specialization",
    "template.specialization_timing": "specialization-timing",
    "template.partial_specialization": "partial-specialization",
    "template.alias": "alias-template",
    "template.variable": "variable-template",
    "template.current_specialization": "current-specialization",
    "template.explicit_instantiation": "explicit-instantiation",
    "template.specialization_partial_ordering": "specialization-partial-ordering",
    "template.deduction_full": "function-deduction",
    "template.detector_idiom": "detector-idiom",
    "template.substitution": "substitution",
    "template.conversion_deduction": "conversion-deduction",
    "template.constructor_deduction": "constructor-deduction",
    "template.no_eager_instantiation": "no-eager-instantiation",
    "sfinae": "sfinae",
    "template.braced_init_deduction": "braced-init-deduction",
    "template.non_deduced_context": "non-deduced-context",
}

TEMPLATE_LATER_OR_COMPAT_FEATURES = {
    "template.alignas_alignof",
    "template.builtin_traits",
    "template.initializer_list",
    "class.inheritance.multiple",
    "class.member_pointer",
    "support.lambda",
    "support.lambda.capture",
    "support.attribute",
    "support.auto",
    "support.range_for",
    "exception.try_catch",
}

TEMPLATE_INTEGRATION_BASIC_SUPPORT = {"basic-template"}
TEMPLATE_PRE_INTEGRATION_SUPPORT = {
    "basic-template",
    "dependent-name",
    "current-instantiation",
}


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
    path_patterns: tuple[re.Pattern[str], ...] = ()


@dataclass
class FeatureHit:
    feature_id: str
    evidence: list[str] = field(default_factory=list)


@dataclass(frozen=True)
class TemplateHeader:
    start: int
    end: int
    params: tuple[str, ...]


@dataclass(frozen=True)
class ClassSpan:
    start: int
    body_start: int
    body_end: int
    is_template: bool


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
    FeatureRule("lang.extended_integer", (rx(r"\b__u?int128(?:_t)?\b"),)),
    FeatureRule("stmt.condition_declaration",
                (rx(r"\b(?:if|switch)\s*\(\s*(?:const\s+|volatile\s+|unsigned\s+|signed\s+|long\s+|short\s+|int\b|bool\b|char\b|[A-Za-z_][A-Za-z0-9_:<>]*\s+[&*]?\s*[A-Za-z_])"),)),
    FeatureRule("expr.reference", (rx(r"(?:^|[^\w])(?:const\s+)?[A-Za-z_][A-Za-z0-9_:<>]*\s*&\s*[A-Za-z_]|\bstatic_cast\s*<[^>]*&"),)),
    FeatureRule("expr.array_pointer", (rx(r"\[[^]]*\]|\bnullptr\b|->|\b[A-Za-z_][A-Za-z0-9_]*\s*\+"),)),
    FeatureRule("lang.enum", (rx(r"\benum\b"),)),
    FeatureRule("expr.cast.builtin", (rx(r"\b(?:static_cast|const_cast|reinterpret_cast|dynamic_cast)\s*<|\([A-Za-z_][A-Za-z0-9_:<>\s*&]*\)\s*[A-Za-z_(0-9]"),)),
    FeatureRule("call.variadic_promotions",
                (rx(r"\b(?:int|void|char|short|long|float|double|signed|unsigned|[A-Za-z_][A-Za-z0-9_:<>*&\s]+)\s+"
                    r"[A-Za-z_][A-Za-z0-9_:]*\s*\([^)]*(?:,\s*)?\.\.\.\s*\)"),)),
    FeatureRule("class.basic",
                (rx(r"(?<!enum )\b(?:class|struct)\s+[A-Za-z_][A-Za-z0-9_]*(?=\s*(?:final\s*)?(?:[:{;]))"),)),
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
    FeatureRule("exception.try_catch",
                (rx(r"\btry\s*\{"), rx(r"\bcatch\s*\("), rx(r"\bthrow\b")),
                ref_patterns=(rx(r"\b__cxa_(?:throw|begin_catch|rethrow)\b|\bexception_selector\b"),)),
    FeatureRule("lookup.adl", (rx(r"\bfriend\b|\boperator\s+(?!new\b|delete\b)"),)),
    FeatureRule("operator.overload", (rx(r"\boperator\s*(?!(?:new|delete)\b)(?:[+\-*/%<>=!&|^~,\[\]()]+|[A-Za-z_][A-Za-z0-9_:<>]*)"),)),
    FeatureRule("class.using_declaration", (rx(r"\busing\s+[A-Za-z_][A-Za-z0-9_:<>]*::[A-Za-z_]"),)),
    FeatureRule("class.inheriting_constructor",
                (rx(r"\busing\s+(?:[A-Za-z_][A-Za-z0-9_:<>]*::)*([A-Za-z_][A-Za-z0-9_]*)::\1\s*;"),)),
    FeatureRule("class.inheritance.multiple", (),
                path_patterns=(rx(r"(?:multiple-inheritance|multiple-base|multibase|diamond|nonprimary-base|downcast-nonprimary|secondary-primary|repeated-base)"),)),
    FeatureRule("class.member_pointer", (rx(r"::\s*\*|\.\*|->\*"),)),
    FeatureRule("class.conversion_operator",
                (rx(r"\boperator\s+(?!new\b|delete\b)(?:const\s+)?[A-Za-z_][A-Za-z0-9_:<>*&\s]*\s*\("),),
                use_raw=True),
    FeatureRule("function.trailing_return", (rx(r"\)\s*->\s*[A-Za-z_][A-Za-z0-9_:<>*&\s]*"),)),
    FeatureRule("support.attribute", (rx(r"\[\[[^]]+\]\]|__attribute__\s*\("),)),
    FeatureRule("lifetime.ctor_dtor",
                (rx(r"\b~[A-Za-z_][A-Za-z0-9_]*\s*\("),),
                path_patterns=(rx(r"(?:lifetime|constructor|destructor|global-constructor|member-object-lifetime)"),)),
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
    FeatureRule("support.lambda", (rx(r"(?<!operator)\[[^]\n]*\]\s*\([^)]*\)\s*(?:mutable\s*)?(?:noexcept\s*)?(?:->|\{)"),)),
    FeatureRule("support.lambda.capture",
                (rx(r"\[[^]\n]*[A-Za-z_][A-Za-z0-9_]*[^]\n]*\]\s*\([^)]*\)\s*"
                    r"(?:mutable\s*)?(?:noexcept\s*)?(?:->[^{]+)?\{"),)),
    FeatureRule("support.range_for", (rx(r"\bfor\s*\([^:;()]+:[^)]*\)"),)),
    FeatureRule("support.decltype", (rx(r"\bdecltype\s*\("),)),
    FeatureRule("support.auto",
                (rx(r"\bauto\s+(?:[*&]\s*)?[A-Za-z_][A-Za-z0-9_]*\s*(?:=|;|,|\[)|"
                    r"\bauto\s+[A-Za-z_][A-Za-z0-9_]*\s*\([^)]*\)\s*(?!->)(?:noexcept\s*)?(?:\{|;)"),)),
    FeatureRule("polymorphic.basic", (rx(r"\bvirtual\b"),),
                ref_patterns=(rx(r"__vtable|_ZTV|__rtti|_ZTI|typeinfo"),)),
    FeatureRule("polymorphic.override_final", (rx(r"\b(?:override|final)\b"),)),
    FeatureRule("polymorphic.vdtor", (rx(r"\bvirtual\s+~"),)),
    FeatureRule("polymorphic.vtable_order", (),
                path_patterns=(rx(r"(?:vtable-order|destructor-slot|slot-merge|vptr-overwrite|vtable-offset)"),)),
    FeatureRule("polymorphic.pointer_adjust",
                (rx(r"\bvirtual\b"), rx(r"\b(?:class|struct)\s+\w+\s*:[^{,]+,[^{]+|static_cast\s*<[^>]*[&*]")),
                all_patterns=True,
                ref_patterns=(rx(r"this_adjust|adjustor|thunk|vbase|vcall"),)),
    FeatureRule("template.type", (rx(r"\btemplate\s*<[^>]*(?:class|typename)\b"),)),
    FeatureRule("template.class", (rx(r"\btemplate\s*<[^>]*>\s*(?:class|struct)\b"),)),
    FeatureRule("template.function",
                (rx(r"\btemplate\s*<[^>]*>[^;{}()]*\b[A-Za-z_][A-Za-z0-9_:<>*&\s]*\s+[A-Za-z_][A-Za-z0-9_]*\s*\([^)]*\)\s*(?:\{|;)"),)),
    FeatureRule("template.default_argument",
                (rx(r"\btemplate\s*<[^>]*=[^>]*>"),),
                path_patterns=(rx(r"(?:default-template-arg|template-default|progressive-template-defaults|parameter-default)"),)),
    FeatureRule("template.dependent_name",
                (rx(r"\btypename\s+[A-Za-z_][A-Za-z0-9_:<>]*::|[A-Za-z_][A-Za-z0-9_]*<[^>]*>::"),),
                path_patterns=(rx(r"(?:^|[-_/])(?:dependent|typename|template-body)(?:[-_/]|$)"),)),
    FeatureRule("template.friend", (rx(r"\bfriend\b[^;{]*\btemplate\b|\btemplate\s*<[^>]*>[^;{]*\bfriend\b"),)),
    FeatureRule("template.current_instantiation", (),
                path_patterns=(rx(r"current-instantiation|current-owner|source-owner|owner-type|owner-param|owner-key"),)),
    FeatureRule("template.disambiguator",
                (rx(r"\btypename\s+[A-Za-z_][A-Za-z0-9_:<>]*::|(?:\.|->|::)\s*template\s+[A-Za-z_]"),),
                path_patterns=(rx(r"(?:^|[-_/])(?:typename|template-keyword|missing-(?:typename|keyword))(?:[-_/]|$)"),)),
    FeatureRule("template.function_partial_ordering", (),
                path_patterns=(rx(r"(?:function-partial-order|partial-order|partial_order|pack-fallback|more-specialized)"),)),
    FeatureRule("template.alignas_alignof", (rx(r"\balignas\s*\(|\balignof\s*\(|__alignof__\s*\("),)),
    FeatureRule("template.pack", (rx(r"\btemplate\s*<[^>]*\.\.\."),)),
    FeatureRule("template.template_parameter", (rx(r"\btemplate\s*<[^>]*template\s*<"),)),
    FeatureRule("template.member_template",
                (rx(r"\btemplate\s*<[^>]*>\s*template\s*<"),),
                path_patterns=(rx(r"(?:member-template|templated-member)"),)),
    FeatureRule("template.builtin_traits",
                (rx(r"\b__(?:is|has|is_trivially|is_nothrow)_[A-Za-z0-9_]*\b|"
                    r"\b__builtin_(?:is|has|types_compatible_p|classify_type)[A-Za-z0-9_]*"),)),
    FeatureRule("template.nttp", (rx(r"\btemplate\s*<[^>]*(?:int|bool|char|long|short|unsigned|signed|std::size_t|size_t)\s+\w+"),)),
    FeatureRule("template.nttp.pointer_member",
                (rx(r"\btemplate\s*<[^>]*(?:[A-Za-z_][A-Za-z0-9_:<>]*\s*[*&]\s*\w+|[A-Za-z_][A-Za-z0-9_:<>]*::\s*\*\s*\w+)"),)),
    FeatureRule("template.explicit_specialization", (rx(r"\btemplate\s*<\s*>\s*"),)),
    FeatureRule("template.specialization_timing", (),
                path_patterns=(rx(r"(?:^|[-_/])(?:stale|refresh|late[-_]|after-instantiation|keeps-definition|"
                                  r"(?:explicit-specialization|specialization)[-_][A-Za-z0-9_-]*redeclaration|"
                                  r"redeclaration[-_][A-Za-z0-9_-]*(?:explicit-specialization|specialization))"),)),
    FeatureRule("constexpr.integral_subset",
                (rx(r"\bconstexpr\b|\bstatic_assert\b|\btemplate\s*<[^>]*(?:int|bool|char|long|short|unsigned|signed)\s+\w+"),)),
    FeatureRule("static_assert", (rx(r"\bstatic_assert\s*\("),)),
    FeatureRule("constexpr.full",
                (rx(r"\bconstexpr\s+[A-Za-z_][A-Za-z0-9_:<>*&\s]+\s+[A-Za-z_][A-Za-z0-9_]*\s*\([^)]*\)\s*\{"),)),
    FeatureRule("constexpr.control_flow", (rx(r"\bconstexpr\b"), rx(r"\b(?:if|for|while|do|switch)\b"),), all_patterns=True),
    FeatureRule("constexpr.default_argument",
                (rx(r"\bconstexpr\s+[A-Za-z_][A-Za-z0-9_:<>*&\s]+\s+[A-Za-z_][A-Za-z0-9_]*\s*\([^)]*=[^)]*\)"),)),
    FeatureRule("constexpr.floating", (rx(r"\bconstexpr\b"), rx(r"\b(?:float|double|long\s+double)\b|[0-9]*\.[0-9]+")), all_patterns=True),
    FeatureRule("constexpr.noexcept", (),
                path_patterns=(rx(r"constexpr-noexcept"),)),
    FeatureRule("constexpr.object", (),
                path_patterns=(rx(r"constexpr-object|constexpr-class|constexpr-struct"),)),
    FeatureRule("template.partial_specialization",
                (rx(r"\btemplate\s*<[^>]+>\s*(?:class|struct)\s+[A-Za-z_][A-Za-z0-9_]*\s*<[^>]*>\s*(?!::)"),)),
    FeatureRule("template.alias", (rx(r"\btemplate\s*<[^>]*>\s*using\b"),)),
    FeatureRule("template.variable",
                (rx(r"\btemplate\s*<[^>]*>\s*(?:constexpr\s+|const\s+|static\s+)?"
                    r"(?!using\b)(?:bool|int|char|long|unsigned|signed|[A-Za-z_][A-Za-z0-9_:<>]*)\s+"
                    r"(?!operator\b)\w+\s*="),)),
    FeatureRule("template.current_specialization", (),
                path_patterns=(rx(r"current-specialization"),)),
    FeatureRule("template.explicit_instantiation",
                (rx(r"\bextern\s+template\b"),),
                path_patterns=(rx(r"(?:explicit-instantiation|extern-template)"),)),
    FeatureRule("template.specialization_partial_ordering", (),
                path_patterns=(rx(r"(?:partial-specialization-order|class-partial-order|partial_order_class|function-type-partial-specialization-preference|repeated-argument)"),)),
    FeatureRule("template.deduction_full",
                (rx(r"\btemplate\s*<[^>]*>[^;{}()]*\b[A-Za-z_][A-Za-z0-9_:<>*&\s]*\s+[A-Za-z_][A-Za-z0-9_]*\s*\([^)]*(?:&&|decltype|enable_if|typename\s+[A-Za-z_][A-Za-z0-9_:<>]*::)[^)]*\)"),),
                path_patterns=(rx(r"(?:deduc|forwarding-reference|explicit-template-(?:args|id)|nondeduced|non-deduced|reference-cv|array-bound)"),)),
    FeatureRule("template.detector_idiom",
                (rx(r"\b(?:void_t|detected_or|detector|is_detected)\b"),),
                path_patterns=(rx(r"(?:detected|detector|void-t|void_t)"),)),
    FeatureRule("template.substitution",
                (rx(r"\b(?:enable_if|void_t|sfinae|SFINAE|detected_or|detector|is_detected)\b"),)),
    FeatureRule("template.conversion_deduction",
                (rx(r"\btemplate\s*<[^>]*>[^;{]*operator\s+[A-Za-z_]"),),
                use_raw=True),
    FeatureRule("template.constructor_deduction", (),
                path_patterns=(rx(r"constructor-template|converting-ctor|ctor-template"),)),
    FeatureRule("template.initializer_list", (rx(r"\binitializer_list\b"),)),
    FeatureRule("template.no_eager_instantiation",
                (rx(r"\b(?:no_eager|unevaluated|static_assert\s*\(\s*false|sizeof\s*\([^)]*typename)"),),
                path_patterns=(rx(r"(?:no[-_]?eager|no[-_]?body|does-not-eagerly|not-instantiat|body-skip|unused-body)"),)),
    FeatureRule("sfinae", (rx(r"\b(?:enable_if|void_t|sfinae|SFINAE|detected_or|detector|is_detected)\b"),)),
    FeatureRule("template.braced_init_deduction",
                (rx(r"\btemplate\s*<[^>]*>[^;{}()]*\b[A-Za-z_][A-Za-z0-9_:<>*&\s]*\s+"
                    r"[A-Za-z_][A-Za-z0-9_]*\s*\([^)]*\)\s*(?:\{|;).*?"
                    r"\b[A-Za-z_][A-Za-z0-9_]*\s*\(\s*\{"),)),
    FeatureRule("template.non_deduced_context",
                (rx(r"\b(?:non_deduced|nondeduced)\b"),),
                path_patterns=(rx(r"non[-]?deduced|nondeduced"),)),
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


def has_top_level_base_comma(code: str) -> bool:
    class_header = re.compile(
        r"\b(?:class|struct)\s+[A-Za-z_][A-Za-z0-9_]*(?:\s+final)?\s*:\s*([^{;]+)\{",
        re.MULTILINE | re.DOTALL,
    )
    for match in class_header.finditer(code):
        depth_angle = 0
        depth_paren = 0
        depth_bracket = 0
        base_clause = match.group(1)
        index = 0
        while index < len(base_clause):
            char = base_clause[index]
            if char == "<":
                depth_angle += 1
            elif char == ">" and depth_angle and index + 1 < len(base_clause) and base_clause[index + 1] == "=":
                pass
            elif char == ">" and depth_angle:
                depth_angle -= 1
            elif char == "(":
                depth_paren += 1
            elif char == ")" and depth_paren:
                depth_paren -= 1
            elif char == "[":
                depth_bracket += 1
            elif char == "]" and depth_bracket:
                depth_bracket -= 1
            elif char == "," and depth_angle == 0 and depth_paren == 0 and depth_bracket == 0:
                return True
            index += 1
    return False


def declared_intrinsic_like_names(code: str) -> set[str]:
    names: set[str] = set()
    for match in re.finditer(
        r"\b(?:class|struct|union)\s+(__[A-Za-z_][A-Za-z0-9_]*)\b",
        code,
        re.MULTILINE | re.DOTALL,
    ):
        names.add(match.group(1))
    for match in re.finditer(
        r"\b(?:template\s*<[^>]*>\s*)?using\s+(__[A-Za-z_][A-Za-z0-9_]*)\b",
        code,
        re.MULTILINE | re.DOTALL,
    ):
        names.add(match.group(1))
    for match in re.finditer(
        r"\btypedef\b[^;]*\s+(__[A-Za-z_][A-Za-z0-9_]*)\s*;",
        code,
        re.MULTILINE | re.DOTALL,
    ):
        names.add(match.group(1))
    for match in re.finditer(
        r"\b(?:static\s+|constexpr\s+|inline\s+|extern\s+|friend\s+|virtual\s+)*"
        r"(?:bool|int|char|short|long|unsigned|signed|void|[A-Za-z_][A-Za-z0-9_:<>]*)\s+"
        r"(__[A-Za-z_][A-Za-z0-9_]*)\s*\(",
        code,
        re.MULTILINE | re.DOTALL,
    ):
        names.add(match.group(1))
    for match in re.finditer(
        r"\b(?:template\s*<[^>]*>\s*)?(?:static\s+)?(?:constexpr\s+|const\s+)?"
        r"(?:bool|int|char|short|long|unsigned|signed|[A-Za-z_][A-Za-z0-9_:<>]*)\s+"
        r"(__[A-Za-z_][A-Za-z0-9_]*)\s*(?:=|;)",
        code,
        re.MULTILINE | re.DOTALL,
    ):
        names.add(match.group(1))
    return names


def overloaded_arrow_star_without_member_pointer(code: str) -> bool:
    if not re.search(r"\boperator\s*->\s*\*", code):
        return False
    return not re.search(r"::\s*\*|\.\*", code)


def matching_delimiter(code: str, start: int, open_char: str, close_char: str) -> int | None:
    depth = 0
    i = start
    while i < len(code):
        c = code[i]
        if c == open_char:
            depth += 1
        elif c == close_char:
            depth -= 1
            if depth == 0:
                return i
        i += 1
    return None


def split_top_level_commas(text: str) -> tuple[str, ...]:
    parts: list[str] = []
    start = 0
    angle = paren = bracket = brace = 0
    for i, c in enumerate(text):
        if c == "<":
            angle += 1
        elif c == ">" and angle:
            angle -= 1
        elif c == "(":
            paren += 1
        elif c == ")" and paren:
            paren -= 1
        elif c == "[":
            bracket += 1
        elif c == "]" and bracket:
            bracket -= 1
        elif c == "{":
            brace += 1
        elif c == "}" and brace:
            brace -= 1
        elif c == "," and not (angle or paren or bracket or brace):
            parts.append(text[start:i].strip())
            start = i + 1
    tail = text[start:].strip()
    if tail:
        parts.append(tail)
    return tuple(parts)


def has_top_level_equal(text: str) -> bool:
    angle = paren = bracket = brace = 0
    for c in text:
        if c == "<":
            angle += 1
        elif c == ">" and angle:
            angle -= 1
        elif c == "(":
            paren += 1
        elif c == ")" and paren:
            paren -= 1
        elif c == "[":
            bracket += 1
        elif c == "]" and bracket:
            bracket -= 1
        elif c == "{":
            brace += 1
        elif c == "}" and brace:
            brace -= 1
        elif c == "=" and not (angle or paren or bracket or brace):
            return True
    return False


def find_template_headers(code: str) -> list[TemplateHeader]:
    headers: list[TemplateHeader] = []
    for match in re.finditer(r"\btemplate\b", code):
        i = match.end()
        while i < len(code) and code[i].isspace():
            i += 1
        if i >= len(code) or code[i] != "<":
            continue
        end = matching_delimiter(code, i, "<", ">")
        if end is None:
            continue
        params = split_top_level_commas(code[i + 1:end])
        headers.append(TemplateHeader(match.start(), end + 1, params))
    return headers


def template_header_before(headers: Iterable[TemplateHeader], pos: int, code: str) -> TemplateHeader | None:
    candidate: TemplateHeader | None = None
    for header in headers:
        if header.end <= pos and code[header.end:pos].strip() == "":
            if candidate is None or header.start > candidate.start:
                candidate = header
    return candidate


def find_class_spans(code: str, headers: Iterable[TemplateHeader]) -> list[ClassSpan]:
    spans: list[ClassSpan] = []
    for match in re.finditer(r"(?<!enum\s)\b(?:class|struct)\s+[A-Za-z_][A-Za-z0-9_]*", code):
        open_idx = code.find("{", match.end())
        semi_idx = code.find(";", match.end())
        if open_idx == -1 or (semi_idx != -1 and semi_idx < open_idx):
            continue
        close_idx = matching_delimiter(code, open_idx, "{", "}")
        if close_idx is None:
            continue
        spans.append(ClassSpan(
            start=match.start(),
            body_start=open_idx + 1,
            body_end=close_idx,
            is_template=template_header_before(headers, match.start(), code) is not None,
        ))
    return spans


def inside_class_body(pos: int, spans: Iterable[ClassSpan]) -> bool:
    return any(span.body_start <= pos < span.body_end for span in spans)


def is_type_template_parameter(param: str) -> bool:
    return bool(re.match(r"(?:class|typename)\s*(?:\.\.\.)?\s*(?:[A-Za-z_][A-Za-z0-9_]*)?(?:\s*=.*)?$", param))


def is_template_template_parameter(param: str) -> bool:
    return bool(re.match(r"template\s*<", param))


def is_pointer_or_reference_nttp(param: str) -> bool:
    return bool(re.search(r"::\s*\*|\(\s*\*|\*|&", param))


def add_source_hit(hits: dict[str, FeatureHit], feature_id: str, evidence: str) -> None:
    hit = hits.setdefault(feature_id, FeatureHit(feature_id))
    if evidence not in hit.evidence:
        hit.evidence.append(evidence)


def detect_structured_template_features(code: str) -> dict[str, FeatureHit]:
    hits: dict[str, FeatureHit] = {}
    headers = find_template_headers(code)
    spans = find_class_spans(code, headers)
    for header in headers:
        for param in header.params:
            if "..." in param:
                add_source_hit(hits, "template.pack", f"source:template parameter pack `{param}`")
            if has_top_level_equal(param):
                add_source_hit(hits, "template.default_argument", f"source:template parameter default `{param}`")
            if is_template_template_parameter(param):
                add_source_hit(hits, "template.template_parameter", f"source:template-template parameter `{param}`")
                continue
            if is_type_template_parameter(param):
                continue
            add_source_hit(hits, "template.nttp", f"source:non-type template parameter `{param}`")
            if is_pointer_or_reference_nttp(param):
                add_source_hit(hits, "template.nttp.pointer_member", f"source:pointer/reference NTTP `{param}`")
        if inside_class_body(header.start, spans):
            add_source_hit(hits, "template.member_template", "source:template declaration inside class body")
    for span in spans:
        if span.is_template:
            body = code[span.body_start:span.body_end]
            if re.search(r"\bfriend\b", body):
                add_source_hit(hits, "template.friend", "source:friend declaration inside class template")
    if re.search(r"\bsizeof\s*\.\.\.", code):
        add_source_hit(hits, "template.pack", "source:sizeof... pack expression")
    return hits


def evidence_partition(evidence: Iterable[str]) -> tuple[list[str], list[str]]:
    source_ref: list[str] = []
    path: list[str] = []
    for item in evidence:
        if item.startswith("path:"):
            path.append(item)
        else:
            source_ref.append(item)
    return source_ref, path


def detect_features(source: str, ref_text: str = "", test_path: str = "") -> dict[str, FeatureHit]:
    no_comments = strip_comments(source)
    code = strip_string_literals(no_comments)
    declared_intrinsics = declared_intrinsic_like_names(code)
    hits: dict[str, FeatureHit] = {}
    for rule in RULES:
        haystack = no_comments if rule.use_raw else code
        matched = match_rule_patterns(rule.patterns, haystack, rule.all_patterns, "source")
        matched.extend(match_rule_patterns(rule.ref_patterns, ref_text, rule.all_patterns, "ref"))
        matched.extend(match_rule_patterns(rule.path_patterns, test_path, rule.all_patterns, "path"))
        if rule.feature_id == "class.member_pointer" and overloaded_arrow_star_without_member_pointer(code):
            matched = [evidence for evidence in matched if "->*" not in evidence]
        if rule.feature_id == "class.inheritance.multiple" and has_top_level_base_comma(code):
            matched.append("source:<multiple base-specifiers>")
        if rule.feature_id == "template.builtin_traits" and declared_intrinsics:
            filtered: list[str] = []
            for evidence in matched:
                name = re.search(r"source:(__[A-Za-z_][A-Za-z0-9_]*)", evidence)
                if name and name.group(1) in declared_intrinsics:
                    continue
                filtered.append(evidence)
            matched = filtered
        if matched:
            hits[rule.feature_id] = FeatureHit(rule.feature_id, matched)
    for feature_id, structured_hit in detect_structured_template_features(code).items():
        hit = hits.setdefault(feature_id, FeatureHit(feature_id))
        for evidence in structured_hit.evidence:
            if evidence not in hit.evidence:
                hit.evidence.append(evidence)
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


def companion_source_text_for(path: Path) -> str:
    """Return same-stem source sidecars that can carry the tested behavior."""
    sidecar_suffixes = (
        ".h",
        ".hh",
        ".hpp",
        ".shared.h",
        ".shared.hh",
        ".shared.hpp",
        ".cc",
        ".cpp",
        ".cxx",
    )
    chunks: list[str] = []
    for sidecar in sorted(path.parent.glob(f"{path.stem}.*")):
        if sidecar == path or sidecar.suffix.startswith(".ref"):
            continue
        if sidecar.name.endswith(sidecar_suffixes):
            chunks.append(read_text(sidecar))
    return "\n".join(chunks)


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


def template_concepts_for(detected_features: Iterable[str]) -> list[str]:
    return sorted({
        concept
        for feature_id in detected_features
        if (concept := TEMPLATE_CONCEPT_BY_FEATURE.get(feature_id))
    })


def review_template_concepts(concepts: Iterable[str], current_pa: str) -> list[str]:
    review = set(concepts)
    if len(review) > 1:
        support = (
            TEMPLATE_INTEGRATION_BASIC_SUPPORT
            if current_pa == "pa23"
            else TEMPLATE_PRE_INTEGRATION_SUPPORT
        )
        non_support = review.difference(support)
        if non_support:
            review = non_support
        else:
            review.difference_update(TEMPLATE_INTEGRATION_BASIC_SUPPORT)
    return sorted(review)


def later_template_dependency_features(placements: Iterable[dict[str, object]]) -> list[str]:
    later: list[str] = []
    for placement in placements:
        feature_id = str(placement["feature"])
        if feature_id in TEMPLATE_LATER_OR_COMPAT_FEATURES:
            later.append(feature_id)
    return sorted(set(later))


def latest_owner_label(feature_ids: Iterable[str], features: dict[str, FeatureMeta]) -> str:
    owners: list[tuple[int, int, str]] = []
    for feature_id in feature_ids:
        meta = features.get(feature_id)
        if not meta:
            continue
        owner_num = pa_number(meta.owner_pa)
        if owner_num is None:
            continue
        owners.append((owner_num, meta.owner_cluster, meta.owner_pa))
    if not owners:
        return ""
    owner_num, owner_cluster, owner_pa = max(owners)
    return f"{owner_pa}:{owner_cluster}"


def placement_key(pa: str, cluster: int | None) -> tuple[int, int] | None:
    number = pa_number(pa)
    if number is None:
        return None
    return (number, cluster or 0)


def late_audit_feature(meta: FeatureMeta) -> bool:
    if meta.feature_id in LATE_PLACEMENT_IGNORED_FEATURES:
        return False
    owner_num = pa_number(meta.owner_pa)
    if owner_num is None:
        return False
    return owner_num > SEMANTIC_ONLY_PA_MAX


def late_placement_candidate(
    placements: list[dict[str, object]],
    current_pa: str,
    current_cluster: int | None,
    features: dict[str, FeatureMeta],
) -> dict[str, object] | None:
    current_key = placement_key(current_pa, current_cluster)
    if current_key is None:
        return None
    owner_entries: list[tuple[tuple[int, int], str, FeatureMeta, dict[str, object]]] = []
    for placement in placements:
        feature_id = str(placement["feature"])
        meta = features.get(feature_id)
        if not meta or not late_audit_feature(meta):
            continue
        owner_key = placement_key(meta.owner_pa, meta.owner_cluster)
        if owner_key is None:
            continue
        owner_entries.append((owner_key, feature_id, meta, placement))
    if not owner_entries:
        return None
    latest_key = max(entry[0] for entry in owner_entries)
    if latest_key >= current_key:
        return None
    latest_entries = [entry for entry in owner_entries if entry[0] == latest_key]
    latest_meta = latest_entries[0][2]
    latest_features = sorted({feature_id for _, feature_id, _, _ in latest_entries})
    confidence = (
        "broad-owner-only"
        if all(feature_id in LATE_PLACEMENT_BROAD_FEATURES for feature_id in latest_features)
        else "specific"
    )
    evidence: list[str] = []
    for _, _, _, placement in latest_entries:
        evidence.extend(str(item) for item in placement.get("evidence", [])[:2])
    return {
        "status": "late-placement-candidate",
        "suggested_pa": latest_meta.owner_pa,
        "suggested_cluster": latest_meta.owner_cluster,
        "features": latest_features,
        "confidence": confidence,
        "reason": (
            f"latest source/ref-backed owner is {latest_meta.owner_pa}:"
            f"{latest_meta.owner_cluster}, earlier than {current_pa}:"
            f"{current_cluster}"
        ),
        "evidence": evidence[:4],
    }


def suggest_integration_cluster(concepts: Iterable[str], current_cluster: int | None) -> int:
    concept_set = set(concepts)
    if current_cluster is not None and current_cluster >= 500:
        return 500
    if concept_set & {"detector-idiom", "sfinae", "substitution", "no-eager-instantiation"}:
        return 300
    if concept_set & {
        "function-deduction",
        "function-partial-ordering",
        "specialization-partial-ordering",
        "non-deduced-context",
        "braced-init-deduction",
    }:
        return 200
    if concept_set & {
        "alias-template",
        "variable-template",
        "member-template",
        "pack-expansion",
        "template-template-parameter",
    }:
        return 400
    return 100


def template_review_for(
    detected_features: list[str],
    placements: list[dict[str, object]],
    current_cluster: int | None,
    current_pa: str,
    features: dict[str, FeatureMeta],
) -> dict[str, object]:
    concepts = template_concepts_for(detected_features)
    review_concepts = review_template_concepts(concepts, current_pa)
    template_features = sorted(
        feature_id for feature_id in detected_features
        if feature_id in TEMPLATE_CONCEPT_BY_FEATURE
    )
    later_features = later_template_dependency_features(placements)
    owner = latest_owner_label(template_features, features)
    suggested_cluster: int | None = None
    if not review_concepts:
        bucket = "manual-review"
        action = "Classify by source/ref review; no template concept was detected."
    elif later_features:
        bucket = "later-owner-or-split"
        action = "Move later-owned behavior, or split/reduce to keep only the PA22 template assertion."
    elif len(review_concepts) >= 2:
        bucket = "pa23-integration-candidate"
        suggested_cluster = suggest_integration_cluster(review_concepts, current_cluster)
        action = "Review as multi-feature template integration; move to PA23 if concepts are essential together."
    elif owner.startswith(("pa18", "pa19", "pa21")):
        bucket = "basic-owner-candidate"
        action = "Place in the owning basic template PA; keep if already there, otherwise move or renumber after review."
    elif owner.startswith("pa22"):
        bucket = "pa22-advanced-single-candidate"
        action = "Place in PA22 and renumber if the current cluster is earlier than the owner cluster."
    else:
        bucket = "manual-review"
        action = "Review manually; ownership is not resolved by the template classifier."
    return {
        "template_features": template_features,
        "template_concepts": concepts,
        "review_template_concepts": review_concepts,
        "template_concept_arity": len(review_concepts),
        "later_or_compat_features": later_features,
        "latest_template_owner": owner,
        "template_bucket": bucket,
        "suggested_pa23_cluster": suggested_cluster,
        "template_action": action,
    }


def row_for(path: Path,
            root: Path,
            features: dict[str, FeatureMeta]) -> dict[str, object]:
    source = read_text(path)
    sidecar_source = companion_source_text_for(path)
    detection_source = source if not sidecar_source else f"{source}\n{sidecar_source}"
    ref_text = ref_text_for(path)
    relative_path = path.relative_to(root)
    current_pa = current_pa_for(relative_path)
    current_cluster = cluster_for(path)
    raw_hits = detect_features(detection_source, ref_text, relative_path.as_posix())
    hits: dict[str, FeatureHit] = {}
    path_hints: dict[str, list[str]] = {}
    for feature_id, hit in raw_hits.items():
        source_ref_evidence, path_evidence = evidence_partition(hit.evidence)
        if source_ref_evidence:
            hits[feature_id] = FeatureHit(feature_id, source_ref_evidence)
        if path_evidence:
            path_hints[feature_id] = path_evidence
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
    late_candidate = late_placement_candidate(placements, current_pa, current_cluster, features)
    row = {
        "path": path.relative_to(root).as_posix(),
        "current_pa": current_pa,
        "current_cluster": current_cluster,
        "test_role": test_role_for(path.relative_to(root)),
        "expected_exit": expected_exit_for(path),
        "detected_features": detected,
        "path_hint_features": sorted(path_hints),
        "path_hints": path_hints,
        "placements": placements,
        "needs_review": any(p["status"] in review_statuses for p in placements),
        "semantic_owner_notes": len(semantic_notes),
        "late_placement_candidate": late_candidate,
        "source_sidecars_scanned": bool(sidecar_source),
    }
    row.update(template_review_for(detected, placements, current_cluster, current_pa, features))
    return row


def markdown_report(rows: list[dict[str, object]],
                    missing_rules: list[str],
                    include_ok: bool) -> str:
    review_rows = [row for row in rows if row["needs_review"] or include_ok]
    late_rows = [row for row in rows if row.get("late_placement_candidate")]
    violations = sum(
        1
        for row in rows
        for placement in row["placements"]  # type: ignore[index]
        if placement["status"] in {"violation", "cluster-early", "backend-owner", "unknown-feature"}
    )
    semantic_notes = sum(int(row["semantic_owner_notes"]) for row in rows)
    path_hint_count = sum(len(row["path_hint_features"]) for row in rows)  # type: ignore[arg-type]
    lines = [
        "# PA Feature Placement Audit",
        "",
        f"- tests scanned: {len(rows)}",
        f"- tests needing review: {sum(1 for row in rows if row['needs_review'])}",
        f"- placement findings: {violations}",
        f"- semantic-owner notes: {semantic_notes}",
        f"- path-only hints: {path_hint_count}",
        f"- late placement candidates: {len(late_rows)}",
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
    if late_rows:
        lines.extend([
            "",
            "## Late Placement Candidates",
            "",
            "These rows are non-failing review leads. They use only source/ref-backed evidence; filename-only matches remain path hints.",
            "",
            "| Test | Current | Suggested Earliest Owner | Confidence | Features | Reason | Evidence | Path Hints |",
            "| --- | --- | --- | --- | --- | --- | --- | --- |",
        ])
        for row in late_rows:
            candidate = row["late_placement_candidate"]  # type: ignore[index]
            current = f"{row['current_pa']}:{row['current_cluster']}"
            suggested = f"{candidate['suggested_pa']}:{candidate['suggested_cluster']}"  # type: ignore[index]
            evidence = ", ".join(candidate.get("evidence", [])[:4])  # type: ignore[union-attr]
            lines.append(
                "| `{}` | `{}` | `{}` | `{}` | {} | {} | `{}` | {} |".format(
                    row["path"],
                    current,
                    suggested,
                    candidate.get("confidence", ""),  # type: ignore[union-attr]
                    markdown_cell(candidate.get("features", [])),  # type: ignore[union-attr]
                    markdown_cell(candidate.get("reason", "")),  # type: ignore[union-attr]
                    evidence.replace("|", "\\|"),
                    markdown_cell(row["path_hint_features"]),
                )
            )
    return "\n".join(lines) + "\n"


def markdown_cell(value: object) -> str:
    if isinstance(value, list):
        text = ", ".join(str(item) for item in value)
    elif value is None:
        text = ""
    else:
        text = str(value)
    return text.replace("|", "\\|")


def selected_pas_for_rows(rows: Iterable[dict[str, object]]) -> list[str]:
    pas = {str(row["current_pa"]) for row in rows}
    return sorted(pas, key=lambda pa: pa_number(pa) or 0)


def template_tracker_title(pas: list[str]) -> str:
    if pas == ["pa22"]:
        return "PA22 Template Placement Tracker"
    if tuple(pas) == STRICT_TEMPLATE_PAS:
        return "Strict Template Placement Tracker"
    return "Template Placement Tracker"


def template_tracker_output_path(pas: list[str]) -> str:
    if pas == ["pa22"]:
        return "docs/pa22-template-placement-tracker.md"
    if tuple(pas) == STRICT_TEMPLATE_PAS:
        return "docs/template-strict-placement-tracker.md"
    return "docs/template-placement-tracker.md"


def template_tracker_scope_label(pas: list[str]) -> str:
    if pas == ["pa22"]:
        return "PA22"
    if tuple(pas) == STRICT_TEMPLATE_PAS:
        return "the strict template PAs (`pa18 pa19 pa21 pa22 pa23`)"
    return "the selected template PAs (`{}`)".format(" ".join(pas))


def template_tracker_report(rows: list[dict[str, object]], missing_rules: list[str]) -> str:
    bucket_counts: dict[str, int] = {}
    for row in rows:
        bucket = str(row["template_bucket"])
        bucket_counts[bucket] = bucket_counts.get(bucket, 0) + 1
    pas = selected_pas_for_rows(rows)
    pa_args = " ".join(f"--pa {pa}" for pa in pas)
    output_path = template_tracker_output_path(pas)
    lines = [
        f"# {template_tracker_title(pas)}",
        "",
        f"This tracker is the review queue for template test placement across {template_tracker_scope_label(pas)}.",
        "It supports the split into:",
        "",
        "- PA18/PA19/PA21 basic template owners",
        "- PA22 advanced single-feature template completion",
        "- PA23 template integration",
        "- later owners, split/reduce, or drop decisions",
        "",
        "The table below was seeded by the template-placement audit mode.",
        "Treat the bucket and cluster as review leads, not final move decisions.",
        "Filename-only matches are retained as path hints and do not drive placement failures.",
        "After review starts, do not overwrite this tracker without preserving status and notes.",
        "",
        "Seed command:",
        "",
        "```sh",
        f"python3 scripts/audit_pa_feature_placement.py {pa_args} --no-course --template-placement \\",
        f"  --markdown-out {output_path} \\",
        "  --csv-out /tmp/template-placement.csv \\",
        "  --json-out /tmp/template-placement.json",
        "```",
        "",
        "Status legend: `[ ]` todo · `[~]` in progress · `[x]` placed · `[D]` dropped · `[-]` deferred",
        "",
        "## Review Rules",
        "",
        "- A test goes to the earliest PA/cluster that owns the behavior it asserts.",
        "- Support syntax does not control placement when it is already implemented and not essential to the expected output.",
        "- If two or more template concepts are essential together, place the test in PA23 integration and cluster it by the feature combination.",
        "- If a later non-template feature is essential, move later or split/reduce the test before keeping template coverage.",
        "- Witness refs are golden; do not regenerate witness refs while moving tests.",
        "",
        "## PA23 Candidate Clusters",
        "",
        "| Cluster | Intended integration shape |",
        "| --- | --- |",
        "| 100 | dependent-name/entity interactions that do not fit a narrower later cluster |",
        "| 200 | deduction, partial ordering, non-deduced contexts, and braced-init deduction combinations |",
        "| 300 | SFINAE, substitution, detector idiom, and no-eager instantiation combinations |",
        "| 400 | pack, member-template, template-template-parameter, alias-template, and variable-template compositions |",
        "| 500 | library-shaped end-to-end reducers without hosted/builtin dependencies |",
        "",
        "## Generated Summary",
        "",
        f"- tests scanned: {len(rows)}",
        f"- feature table entries without detector rules: {len(missing_rules)}",
    ]
    for bucket in sorted(bucket_counts):
        lines.append(f"- {bucket}: {bucket_counts[bucket]}")
    lines.extend([
        "",
        "## Review Queue",
        "",
        "| Status | Test | Current | Bucket | Concepts For Review | Later/Compat Features | Latest Template Owner | PA23 Cluster | Late Candidate | Late Confidence | Path Hints | Action | Notes |",
        "| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |",
    ])
    for row in sorted(rows, key=lambda item: (str(item["template_bucket"]), str(item["path"]))):
        current = f"{row['current_pa']}:{row['current_cluster']}"
        late_candidate = row.get("late_placement_candidate") or {}
        late_label = (
            placement_label(
                late_candidate.get("suggested_pa", ""),  # type: ignore[union-attr]
                late_candidate.get("suggested_cluster"),  # type: ignore[union-attr]
            )
            if late_candidate else ""
        )
        lines.append(
            "| [ ] | `{}` | `{}` | `{}` | {} | {} | `{}` | {} | `{}` | `{}` | {} | {} |  |".format(
                row["path"],
                current,
                row["template_bucket"],
                markdown_cell(row["review_template_concepts"]),
                markdown_cell(row["later_or_compat_features"]),
                row["latest_template_owner"],
                markdown_cell(row["suggested_pa23_cluster"]),
                late_label,
                late_candidate.get("confidence", "") if late_candidate else "",  # type: ignore[union-attr]
                markdown_cell(row["path_hint_features"]),
                markdown_cell(row["template_action"]),
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


def write_csv(path: Path, rows: list[dict[str, object]], template_placement: bool = False) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.writer(handle)
        if template_placement:
            writer.writerow([
                "path",
                "current_pa",
                "current_cluster",
                "test_role",
                "expected_exit",
                "template_bucket",
                "template_concept_arity",
                "review_template_concepts",
                "template_features",
                "later_or_compat_features",
                "latest_template_owner",
                "suggested_pa23_cluster",
                "template_action",
                "late_candidate_owner",
                "late_candidate_confidence",
                "late_candidate_features",
                "path_hint_features",
                "source_sidecars_scanned",
            ])
            for row in rows:
                late_candidate = row.get("late_placement_candidate") or {}
                writer.writerow([
                    row["path"],
                    row["current_pa"],
                    row["current_cluster"],
                    row["test_role"],
                    row["expected_exit"],
                    row["template_bucket"],
                    row["template_concept_arity"],
                    "; ".join(row["review_template_concepts"]),  # type: ignore[arg-type]
                    "; ".join(row["template_features"]),  # type: ignore[arg-type]
                    "; ".join(row["later_or_compat_features"]),  # type: ignore[arg-type]
                    row["latest_template_owner"],
                    row["suggested_pa23_cluster"] or "",
                    row["template_action"],
                    placement_label(
                        late_candidate.get("suggested_pa", ""),  # type: ignore[union-attr]
                        late_candidate.get("suggested_cluster"),  # type: ignore[union-attr]
                    ) if late_candidate else "",
                    late_candidate.get("confidence", "") if late_candidate else "",  # type: ignore[union-attr]
                    "; ".join(late_candidate.get("features", [])) if late_candidate else "",  # type: ignore[union-attr]
                    "; ".join(row["path_hint_features"]),  # type: ignore[arg-type]
                    row["source_sidecars_scanned"],
                ])
            return
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
            "late_candidate_owner",
            "late_candidate_confidence",
            "late_candidate_features",
            "path_hint_features",
        ])
        for row in rows:
            late_candidate = row.get("late_placement_candidate") or {}
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
                    placement_label(
                        late_candidate.get("suggested_pa", ""),  # type: ignore[union-attr]
                        late_candidate.get("suggested_cluster"),  # type: ignore[union-attr]
                    ) if late_candidate else "",
                    late_candidate.get("confidence", "") if late_candidate else "",  # type: ignore[union-attr]
                    "; ".join(late_candidate.get("features", [])) if late_candidate else "",  # type: ignore[union-attr]
                    "; ".join(row["path_hint_features"]),  # type: ignore[arg-type]
                ])


def early_placement_findings(rows: list[dict[str, object]]) -> list[tuple[dict[str, object], dict[str, object]]]:
    findings: list[tuple[dict[str, object], dict[str, object]]] = []
    for row in rows:
        for placement in row["placements"]:  # type: ignore[index]
            if placement["status"] in EARLY_PLACEMENT_STATUSES:
                findings.append((row, placement))
    return findings


def github_escape_message(text: object) -> str:
    return str(text).replace("%", "%25").replace("\r", "%0D").replace("\n", "%0A")


def github_escape_property(text: object) -> str:
    escaped = github_escape_message(text)
    return escaped.replace(":", "%3A").replace(",", "%2C")


def placement_label(pa: object, cluster: object) -> str:
    if cluster is None:
        return str(pa)
    return f"{pa}:{cluster}"


def emit_early_placement_errors(findings: list[tuple[dict[str, object], dict[str, object]]]) -> None:
    count = len(findings)
    plural = "" if count == 1 else "s"
    print(
        f"error: test placement audit found {count} feature use{plural} placed "
        "before the owning PA/cluster.",
        file=sys.stderr,
    )
    print(
        "Move the test to the owner shown below, or reduce the test so it no "
        "longer depends on the later feature.",
        file=sys.stderr,
    )
    for row, placement in findings:
        current = placement_label(row["current_pa"], row["current_cluster"])
        owner = placement_label(placement.get("owner_pa", ""), placement.get("owner_cluster"))
        evidence = "; ".join(placement.get("evidence", [])[:2])
        message = (
            f"{placement['feature']} belongs in {owner}, but this test is in "
            f"{current}: {placement['reason']}. Evidence: {evidence}"
        )
        path = row["path"]
        print(
            "::error file={},title={}::{}".format(
                github_escape_property(path),
                github_escape_property("Test placed before owning feature"),
                github_escape_message(message),
            ),
            file=sys.stderr,
        )
        print(f"- {path}: {message}", file=sys.stderr)


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path("."))
    parser.add_argument("--tracker", type=Path, default=DEFAULT_TRACKER)
    parser.add_argument("--pa", action="append", choices=[f"pa{i}" for i in range(1, 38)])
    parser.add_argument("--feature", action="append", help="only report tests matching this feature id")
    parser.add_argument("--include-course", action="store_true", default=True)
    parser.add_argument("--no-course", action="store_false", dest="include_course")
    parser.add_argument("--include-ok", action="store_true", help="include ok/semantic-owner rows in markdown")
    parser.add_argument(
        "--template-placement",
        action="store_true",
        help="emit a per-test template-placement review queue instead of placement findings",
    )
    parser.add_argument("--json-out", type=Path)
    parser.add_argument("--csv-out", type=Path)
    parser.add_argument("--markdown-out", type=Path)
    parser.add_argument(
        "--fail-on-early",
        action="store_true",
        help="exit nonzero when a test uses a feature before its owning PA/cluster",
    )
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
                  rows,
                  args.template_placement)
    if args.template_placement:
        report = template_tracker_report(rows, missing_rules)
    else:
        report = markdown_report(rows, missing_rules, args.include_ok)
    if args.markdown_out:
        out = args.markdown_out if args.markdown_out.is_absolute() else root / args.markdown_out
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_text(report, encoding="utf-8")
    else:
        print(report, end="")
    if args.fail_on_early:
        early_findings = early_placement_findings(rows)
        if early_findings:
            emit_early_placement_errors(early_findings)
            return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
