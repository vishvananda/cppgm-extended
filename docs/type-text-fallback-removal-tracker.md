# Type Text Fallback Removal Tracker

Goal: remove the remaining semantic/template type text fallback paths and replace
them with structured syntax or already-resolved semantic data. Text may still
exist at the parser boundary as source spelling, but it should not be reparsed as
the recovery path after structured lookup fails.

Baseline checkpoint:

- Commit: `1a871f6e` (`Thread type syntax data through template resolution`)
- Validation: `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-strict-nobuild STRICT_SUBTEST_JOBS=8 CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`
- Result: PA18, PA19, PA21, and PA22 strict all passed with LowIR direct compare.
- Boundary audit note: `scripts/audit_semantic_template_boundary.py --strict`
  is clean. `scripts/audit_template_boundary.py` still reports broader
  pre-existing debt; use it to track the `text_recovery_bridge` count, not as a
  blocker for unrelated categories.

## Rules

- For each slice, remove the text fallback first, run strict with LowIR direct
  compare, and fix clustered failures by threading semantic data through the
  producer boundary.
- Do not update witness or LowIR refs for these changes unless the intended
  semantic behavior changes and that is explicitly reviewed.
- Do not replace one text parse with another. Acceptable replacements are
  `TemplateArgumentSyntax`, `CppAstNode` semantic annotations, `TemplateArgument`
  / `TypePtr` data, source argument scope, or instantiation argument vectors.
- Run the strict LowIR command after each source change that affects resolution
  behavior. Run a perf regression after each significant completed group against
  `/tmp/cppgm-perf-baseline-eca0d1dd-20260511.json`.

## Inventory

- [x] Preserve source argument scope for nested class-template specialization
  selection.
  - Files: `template_service_interfaces.h`, `template_services.h`,
    `template_argument_semantics.cpp`.
  - Fix: pass the source argument scope into specialization selection only, and
    annotate direct qualified type-name argument syntax with the resolved owner
    type at the leaf node.
  - Validation: full strict LowIR direct compare passed at `1a871f6e`.

- [x] Remove `resolve_simple_direct_type_lookup_text`.
  - Definition: `template_argument_semantics.cpp`.
  - Public declaration: `template_argument_semantics.h`.
  - Current caller: `template_resolution.cpp`
    `resolve_instantiated_dependent_template_owner_type_argument`.
  - Strategy: carry the structured actual argument or `TemplateArgumentSyntax`
    into the dependent-owner repair path. Use the argument's resolved `TypePtr`
    or direct qualified type-name AST annotations instead of resolving `text`.
  - Validation: full strict LowIR direct compare passed after replacing the
    caller with `resolve_type_argument_input`.
  - Perf: candidate `331dda41` vs
    `/tmp/cppgm-perf-baseline-eca0d1dd-20260511.json`: instructions `-2.15%`,
    max RSS `+0.06%`, peak footprint `-2.05%`.

- [x] Remove direct `resolve_direct_type_lookup_request_text` fallbacks from
  type-argument resolution.
  - Main sites: `template_argument_semantics.cpp`
    `resolve_type_argument_text`,
    `resolve_type_text_without_fragment_fallback`,
    `resolve_type_text_compound_without_fragment_fallback`, and helper callers
    around `resolve_wrapped_visible_type_text`.
  - Strategy: split real parser-boundary type parsing from semantic recovery.
    For template arguments, prefer `TemplateArgumentSyntax::resolved_type`,
    `type_id`, `template_id`, and direct scope-aware lookup nodes. For cv/ref,
    pointer, and function types, build from the AST/declarator shape or the
    already-expanded `TypePtr`.
  - Progress: removed the final arbitrary direct lookup from
    `resolve_type_text_without_fragment_fallback`; full strict LowIR direct
    compare still passes.
  - Progress: removed the final arbitrary direct lookup from
    `resolve_type_argument_text`; full strict LowIR direct compare still passes.
  - Progress: removed the two `typename` direct lookup recovery calls from
    `resolve_type_argument_text` and
    `resolve_type_text_without_fragment_fallback`; full strict LowIR direct
    compare still passes.
  - Progress: removed the text-expression value-initialization direct type
    lookup branch; full strict LowIR direct compare still passes.
  - Progress: replaced the remaining text member-value owner lookup with the
    structured `TemplateIdSyntax` semantic-context path, removed
    `resolve_direct_type_lookup_request_text`, and verified no references remain
    in `dev/src`. Full strict LowIR direct compare still passes.
  - Perf: candidate `6c226ba9` vs
    `/tmp/cppgm-perf-baseline-eca0d1dd-20260511.json`: instructions `-2.27%`,
    max RSS `+0.94%`, peak footprint `-1.96%`.
  - Audit: semantic/template strict audit clean; template-side audit still has
    broader pre-existing non-text failures and reports `text_recovery_bridge`
    count `91`.

- [x] Remove alias/template validation calls that re-resolve expanded type text.
  - Sites: `template_argument_semantics.cpp` around alias validation and
    dependent alias replay; `template_specialization.cpp` callers of
    `resolve_type_text_without_fragment_fallback`.
  - Strategy: have alias expansion return or retain the structural `TypePtr`
    alongside source spelling. Validation should inspect the type and only keep
    spelling for diagnostics/witness source identity.
  - Progress: replaced nested alias argument validation in
    `template_argument_semantics.cpp` with
    `expand_alias_template_pattern_type`; full strict LowIR direct compare
    still passes.
  - Progress: removed the canonical-pattern text reparse fallback from
    partial-specialization matching in `template_specialization.cpp`; full
    strict LowIR direct compare still passes.
  - Progress: removed the placeholder-pattern text reparse fallback from
    partial-specialization matching in `template_specialization.cpp`; full
    strict LowIR direct compare still passes.
  - Progress: removed the non-placeholder pattern text reparse fallback from
    partial-specialization matching in `template_specialization.cpp`; full
    strict LowIR direct compare still passes.
  - Progress: removed alias canonicalization's no-syntax
    `specialization_argument_text_type` fallback; full strict LowIR direct
    compare still passes.
  - Progress: removed partial-specialization actual/pattern argument text type
    fallbacks and the now-unused `specialization_argument_text_type` wrapper;
    full strict LowIR direct compare still passes.
  - Progress: replaced direct function-type pack deduction's text reparse with
    structural `TypePtr` deduction plus `TemplateArgumentSyntax` pack-expansion
    detection; full strict LowIR direct compare still passes.
  - Progress: removed transformed partial-specialization function-type text
    parsing; `TemplateArgumentSyntax` now covers the surviving path, and full
    strict LowIR direct compare still passes.
  - Perf: candidate `a329e01d` vs
    `/tmp/cppgm-perf-baseline-eca0d1dd-20260511.json`: instructions `-2.24%`,
    max RSS `+0.15%`, peak footprint `-1.76%`. Wall time was load-noisy.
  - Perf: candidate `52f5d4d1` vs
    `/tmp/cppgm-perf-baseline-eca0d1dd-20260511.json`: instructions `-2.68%`,
    max RSS `-1.42%`, peak footprint `-1.66%`. Wall time was load-noisy.

- [x] Remove external `resolve_type_argument_text` bridges.
  - Wrappers/users: `template_api.{h,cpp}`, `template_api_internal.h`,
    `semantic_dependent_type.{h,cpp}`, `template_type_ops.cpp`,
    `callsemantic.cpp`, and `template_resolution.cpp`.
  - Strategy: replace public text entry points with structured requests. Any
    remaining wrapper should be parser-boundary only and named accordingly.
  - Progress: removed unused top-level `template_api` declarations/definitions,
    unused `semantic_dependent_type` wrappers, and stale `template_resolution`
    forward declarations.
  - Progress: removed the top-level direct function-type splitter from
    `resolve_type_argument_text`; the strict suite did not need that fallback,
    and full strict LowIR direct compare still passes.
  - Progress: removed the lower-level function-type splitter from
    `resolve_type_text_compound_without_fragment_fallback` and deleted the now
    unused splitter helper; full strict LowIR direct compare still passes.
  - Progress: removed the remaining compound cv/ref/pointer type-string
    reconstruction fallback and its top-level token stripping helpers; full
    strict LowIR direct compare still passes.
  - Perf: candidate `9723bdfa` vs
    `/tmp/cppgm-perf-baseline-eca0d1dd-20260511.json`: instructions `-2.43%`,
    max RSS `-1.27%`, peak footprint `-1.93%`. Wall time was load-noisy.
  - Progress: removed `resolve_wrapped_visible_type_text` and the exact visible
    type-name/cv/ref/pointer string helpers that only supported it; full strict
    LowIR direct compare still passes.
  - Progress: removed bound-type and bound-pack text matching from the central
    type-argument text resolver; full strict LowIR direct compare still passes.
  - Progress: removed `lookup_type_impl`'s speculative bound-type text lookup
    in `callsemantic.cpp`; full strict LowIR direct compare still passes.
  - Progress: removed the remaining bound-type text matching APIs
    (`lookup_bound_type_by_text`, `lookup_bound_type_pack_element_by_text`,
    and template wrapper variants). Pack-expanded base template-ids now resolve
    through expanded `TemplateArgumentSyntax` carrying pack-element `TypePtr`s,
    with exact namespace-qualified visible lookup covering text-expanded
    qualified type names. Full strict LowIR direct compare still passes.
  - Progress: removed the external `semantic_dependent_type`
    `resolve_type_text_without_fragment_fallback` bridge. Builtin type
    transforms now use the same `SemanticContext::lookup_type` surface as
    builtin type traits; full strict LowIR direct compare still passes.
  - Progress: removed the now-unused public `template_api`
    `resolve_type_text_without_fragment_fallback` wrapper layer; `cppgm++`
    still builds.
  - Progress: removed the dependent `typename` re-resolution call from
    `resolve_type_argument_text`; unresolved/dependent `typename` now goes
    directly to the dependent type path. Full strict LowIR direct compare still
    passes.
  - Progress: replaced qualified-scope qualifier text re-resolution with
    structured `QualifiedName` direct lookup requests. Full strict LowIR direct
    compare still passes.
  - Progress: replaced concrete qualified member owner text re-resolution with
    direct structured owner lookup before opening the member scope. Full strict
    LowIR direct compare still passes.
  - Progress: removed builtin-trait AST pack expansion's fallback parse of
    expanded type text; expanded entries must now carry `TypePtr` data. Full
    strict LowIR direct compare still passes.
  - Progress: changed text-form builtin trait argument parsing to use the
    parser-boundary `SemanticContext::lookup_type` surface instead of the
    internal fallback resolver. Full strict LowIR direct compare still passes.
  - Progress: changed expression-text `declval<T>()` handling to resolve `T`
    through parser-boundary `SemanticContext::lookup_type` instead of the
    internal fallback resolver. Full strict LowIR direct compare still passes.
  - Progress: changed nodeless `typeof(type)` handling to use parser-boundary
    `SemanticContext::lookup_type`; AST-backed `typeof` still uses structured
    `type_id` parsing. Full strict LowIR direct compare still passes.
  - Progress: deleted `resolve_type_text_without_fragment_fallback` and dead
    helper predicates now that all callers have moved to structured data or a
    parser-boundary lookup. Full strict LowIR direct compare still passes.
  - Progress: renamed the final parser-boundary type text adapter from
    `resolve_type_argument_text` to `parse_type_argument_text`, leaving the
    `CallSemantic::parse_type_text` hook explicit and removing the last
    `resolve_*type*_text` API name from `dev/src`. Full strict LowIR direct
    compare still passes.
  - Perf: candidate `a30a01b6` vs
    `/tmp/cppgm-perf-baseline-eca0d1dd-20260511.json`: instructions `-5.45%`,
    max RSS `+1.96%`, peak footprint `-2.41%`. Wall/user/sys remain load-noisy.
  - Audit: semantic/template strict audit clean; template-side
    `text_recovery_bridge` count is down to `88`.
  - Perf: candidate `08c7d4f9` vs
    `/tmp/cppgm-perf-baseline-eca0d1dd-20260511.json`: instructions `-5.51%`,
    max RSS `+0.38%`, peak footprint `-2.46%`. Wall time was load-noisy.

- [ ] Audit raw semantic type lookup by string.
  - Examples: `callsemantic.cpp::lookup_type_impl`,
    `semantic_declaration.cpp`, `semantic_class_model.cpp`,
    `semantic_builtins.cpp`, `semantic_consteval.cpp`,
    `semantic_overload.cpp`, and `callsemantic/constant_value_lookup.cpp`.
  - Strategy: classify each site as parser-boundary lookup, ordinary identifier
    lookup, or semantic fallback. Only the fallback class belongs in this plan.

- [x] Remove local member-template owner source-string-to-syntax fallback.
  - Progress: `template_argument_semantics.cpp`
    `resolve_member_template_owner_type_text` /
    `resolve_member_template_template_argument_text` no longer synthesize
    `TemplateArgumentSyntax` entries from split owner text. Syntax-bearing
    callers must use carried `TemplateIdSyntax` / qualifier syntax; instantiated
    alias-template substitution can only recover member template-template
    arguments from already-bound typed `TemplateArgument` metadata.
  - Follow-up site: `template_id_syntax_from_text_component` and callers such as
    dependent qualified-component handling. These should use parser-captured
    `TemplateIdSyntax` / qualifier syntax when available, and only preserve text
    as spelling for diagnostics.

- [x] Remove misleading `resolve_*type*text` helper names.
  - Progress: renamed the remaining scoped lookup helpers in
    `template_resolution.cpp`, `semantic_class_model.cpp`, and
    `semantic_lookup.cpp` so `dev/src` no longer contains
    `resolve_direct_type_argument_text`, `resolve_type_argument_text`,
    `resolve_type_text_without_fragment_fallback`, or other
    `resolve_*type*text` helper names. Full strict LowIR direct compare still
    passes.
  - Audit: semantic/template strict audit is clean. Broad template audit still
    reports pre-existing non-text categories, with `text_recovery_bridge` at
    `85` against the baseline limit of `166`.
  - Perf: candidate `66a9294a` vs
    `/tmp/cppgm-perf-baseline-eca0d1dd-20260511.json`: instructions `-5.25%`,
    max RSS `-0.28%`, peak footprint `-2.50%`. Wall/user/sys remain load-noisy.

- [x] Remove `resolve_type_text_without_fragment_fallback`.
  - End state: no semantic recovery helper should promise "without fragment
    fallback" while still resolving arbitrary type text. The remaining parser
    helper, if any, should accept a `CppAstNode`/declarator shape or be clearly
    limited to source parsing at the true parser boundary.

- [x] Remove `try_build_type_text_ir` from Itanium symbol linkage.
  - Current scope: all remaining references are in `dev/src/symbol_linkage.cpp`.
    The helper is a semantic recovery parser for mangling: it reparses cv/ref/
    pointer spelling, template-ids, qualified names, builtin transforms, and
    identifier fallback names after structured type construction fails.
  - Baseline before this tranche: full strict LowIR direct compare passed on
    `2026-05-13` using
    `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-strict STRICT_SUBTEST_JOBS=8
    CXX=/usr/local/opt/llvm/bin/clang++
    CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`.
  - Perf baseline for this tranche:
    `/tmp/cppgm-perf-baseline-5bdf41bc-20260513.json`.
  - Slice 1: remove typed-object fallbacks where a `TypePtr` or
    `TemplateArgument` is already available. Fix `try_build_type_ir` or the
    producer that lost type information instead of falling back to spelling.
    Main sites: owner template arguments, member-owner specialization,
    `TemplateArgument::TA_TYPE`, non-type template parameter value type,
    named-type fallback, and conversion-operator target type.
  - Progress: removed the typed-object fallbacks. Class-scope enum types now
    record `named_member_owner_type`/`named_member_name`, so member enum types
    such as `Box<int>::Tag` and template-local anonymous enums use the typed
    member-type IR path. Qualified function-local class keys with `__local_`
    are now emitted from the typed local-name path instead of being recovered by
    template-argument text. Full strict LowIR direct compare still passes.
  - Perf: candidate `44441821` vs
    `/tmp/cppgm-perf-baseline-5bdf41bc-20260513.json`: instructions `-0.42%`,
    max RSS `+2.00%`, peak footprint `+0.32%`. Wall/user/sys were load-noisy.
  - Slice 2: remove `TemplateArgumentSyntax::text` type fallback. Use
    `resolved_type`, `type_id`, `template_id`, pack-expansion metadata, or
    exact template-parameter matching only. Any failure should be fixed by
    threading structured syntax/semantic type data into the syntax producer.
  - Progress: removed the `TemplateArgumentSyntax::text` fallback. The exposed
    PA18/PA19 LowIR drifts were dependent comparison expressions that had been
    hidden by text spelling; structured binary operator codes now cover the
    normal Itanium binary operator set used by those expressions. Updated the
    two LowIR refs after full strict showed no exit or witness failures.
  - Perf: candidate `42a17255` plus this slice vs
    `/tmp/cppgm-perf-baseline-5bdf41bc-20260513.json`: instructions `-0.32%`,
    max RSS `+1.45%`, peak footprint `+0.65%`. Wall/user/sys were load-noisy.
  - Slice 3: replace qualified-owner text reparsing. Build qualifier owner IR
    from `QualifiedName`, qualifier `TemplateIdSyntax`, qualifier semantic type
    metadata, and lookup scope data without assembling owner text.
  - Progress: replaced the qualified-owner loops in member template-id types,
    qualified member type specifiers, dependent member expressions, and lexical
    namespace-qualified type syntax. Namespace-qualified type arguments such as
    `_Algorithm::__copy` now use the lookup scope to build a structured named
    type instead of falling back to the type-text parser. Full strict LowIR
    direct compare still passes.
  - Perf: candidate `0850208` plus this slice vs
    `/tmp/cppgm-perf-baseline-5bdf41bc-20260513.json`: instructions `-0.30%`,
    max RSS `+1.01%`, peak footprint `+0.31%`. Wall/user/sys were load-noisy.
  - Slice 4: audit dependent-expression type arguments separately. Expression
    text may remain at the parser boundary, but type components inside those
    expressions should not route through `try_build_type_text_ir`.
  - Progress: replaced the generic `CppAstNode::value` fallback with a direct
    syntax helper that handles only builtin atoms, exact template parameters,
    pack expansion wrappers, scope-qualified named type syntax, and visible
    named-type lookup. This restored local typedef/alias constructor mangling
    without reintroducing recursive type text parsing. Full strict LowIR direct
    compare still passes.
  - Slice 5: delete `try_build_type_text_ir` and its recursive string parser
    once all external callers are gone. Rename any surviving exact
    template-parameter-name helper so it does not imply arbitrary type parsing.
  - Progress: replaced dependent-expression type-trait arguments and qualified
    member owner construction with direct syntax/type builders, then deleted
    `try_build_type_text_ir` and its owner-text helper. No
    `try_build_type_text_ir` references remain in `dev/src`.
  - Final validation: full strict LowIR direct compare passed after deletion,
    and `scripts/audit_semantic_template_boundary.py --strict` remains clean.
  - Final perf: candidate `c92d1e2` plus this deletion vs
    `/tmp/cppgm-perf-baseline-5bdf41bc-20260513.json`: instructions `-0.24%`,
    max RSS `+1.61%`, peak footprint `+0.25%`. Wall/user/sys were load-noisy.
  - Validation: run strict with LowIR direct compare after each source slice.
    Run `scripts/validate_perf_regression.py check --baseline
    /tmp/cppgm-perf-baseline-5bdf41bc-20260513.json` after significant groups
    and require the final candidate to be at or ahead of the baseline on
    retired instructions and within memory tolerance.

## Validation Commands

Strict after each behavior slice:

```sh
CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-strict-nobuild STRICT_SUBTEST_JOBS=8 \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

Boundary audits after each completed group:

```sh
python3 scripts/audit_semantic_template_boundary.py --strict
python3 scripts/audit_template_boundary.py
```

Perf after significant groups:

```sh
# Use the repo's standard perf regression driver, comparing against:
# /tmp/cppgm-perf-baseline-eca0d1dd-20260511.json
```
