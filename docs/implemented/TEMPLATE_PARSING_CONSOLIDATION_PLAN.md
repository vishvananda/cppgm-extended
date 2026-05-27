## Goal

Reduce the remaining places where template parsing, qualified-name parsing, and
template argument classification are implemented independently, so fixes land in
one place instead of being repeated across parser and semantic code.

## Working Rules

- Validate every stage with:
  - `make verify-fast-pa10-31 CXX=/usr/local/opt/llvm/bin/clang++ VERIFY_ASSIGNMENT_JOBS=6`
- If the fast path is unavailable or suspicious, rerun:
  - `make verify-pa10-31 CXX=/usr/local/opt/llvm/bin/clang++ VERIFY_ASSIGNMENT_JOBS=6 VERIFY_SUBTEST_JOBS=1`
- Commit after each completed stage before starting the next stage.

## Stages

- [x] Stage 1: Replace the active char-based qualified-name parser in
  `dev/src/cpp_decl_bridge.cpp` with a token-based implementation using the
  shared `template_angle_parser` APIs.
  Why:
  - this is the largest remaining duplicate angle parser
  - active semantic lookup paths still depend on it
  Success criteria:
  - `parse_qualified_name_string` no longer has its own `<` / `>` scanner
  - active semantic callers still work through the same public API
  - full `pa10`-`pa32` validation passes
  Notes:
  - implemented with tokenization + `CppAstParser`-backed qualified-name
    parsing, with a shared-angle-parser fallback only for components the normal
    parser rejects
  - operator lookup normalization in semantic lookup was updated so token-based
    qualified names do not require spelling-specific operator hacks at call
    sites

- [x] Stage 2: Extract a dedicated shared qualified-name parser.
  Why:
  - Stage 1 proved that layering `cpp_decl_bridge` on top of `CppAstParser`
    still leaves us composing two parsers at different abstraction levels
  - qualified-name parsing needs to be owned by one token-level parser, not a
    full AST parser plus fallback heuristics
  Success criteria:
  - a `qualified_name_parser` layer exists above `template_angle_parser` and
    below `CppAstParser`
  - `cpp_decl_bridge` no longer subclasses `CppAstParser` or falls back between
    parser implementations
  - `CppAstParser` name-text parsing delegates to the shared qualified-name
    parser where practical
  - full `pa10`-`pa32` validation passes
  Notes:
  - `qualified_name_parser` now owns token-level parsing of identifier
    components, destructors, operator function-ids, and `decltype(...)`
    qualifiers
  - `cpp_decl_bridge` and the relevant `CppAstParser` name-text entrypoints now
    share that parser instead of composing a full AST parser with a lower-level
    fallback

- [x] Stage 3: Consolidate template-angle lookup adapters.
  Why:
  - `CppAstParser`, scoped semantic parsing, and the legacy cursor path still
    each build their own `template_angle::NameLookup` adapters
  Success criteria:
  - one shared adapter/builder layer exists for parser-backed and scope-backed
    lookups
  - repeated local lookup structs are removed
  - full `pa10`-`pa32` validation passes
  Notes:
  - `template_angle_lookup::NameSetLookup` now provides the shared set-backed
    adapter used by `CppAstParser`, semantic fragment parsing, the qualified-name
    bridge, and the legacy cursor path
  - `CppAstParser::make_template_angle_lookup()` now snapshots the active parser
    name environment instead of rebuilding identical local adapter structs at
    every call site

- [x] Stage 4: Canonicalize operator and function-id name spelling.
  Why:
  - lookup currently still needs operator spelling aliases because builtin and
    parsed operator names are not normalized the same way
  Success criteria:
  - one canonical operator/function-id spelling is used for declaration
    collection and semantic lookup
  - transitional alias helpers are removed
  - full `pa10`-`pa32` validation passes
  Notes:
  - `semantic_lookup` now owns canonical function lookup keys and the shared
    function-set/function-template slot helpers
  - direct function registration, using-declaration injection, instantiated
    lookup, and diagnostic candidate dumping now all flow through those shared
    helpers instead of raw map access plus operator alias fallbacks

- [x] Stage 5: Consolidate fragment tokenization and parser seeding.
  Why:
  - expression fragments, type fragments, and template-id string parsing still
    duplicate tokenize -> posttokenize -> recog -> parser setup
  Success criteria:
  - one shared fragment parsing utility owns tokenization, parser construction,
    and parse-environment seeding
  - semantic fragment entrypoints delegate to it
  - full `pa10`-`pa32` validation passes
  Notes:
  - `semantic_fragment_parser` now owns the shared text-to-recog-token pipeline,
    fragment parser seeding, and the expression/type fragment wrappers
  - `callsemantic` now delegates its fragment parsing and scoped template-id
    tokenization setup to that shared utility instead of rebuilding the same
    tokenizer/parser stack inline

- [x] Stage 6: Consolidate template argument semantic classification.
  Why:
  - `template_resolution.cpp` and `template_specialization.cpp` still
    independently classify type, non-type, and template-template arguments
  Success criteria:
  - one shared helper layer owns argument text classification and dependent vs
    hard-failure policy
  - full `pa10`-`pa32` validation passes
  Notes:
  - `template_argument_semantics` now owns the shared text helpers for
    template-template classification, non-type evaluation, direct type
    resolution, deduction-sensitive type parsing, and bound-name rewriting
  - the helper surface is intentionally split between direct argument
    resolution and syntax-preserving deduction parsing so partial
    specialization matching keeps its pre-existing top-level-cv behavior while
    still sharing the surrounding text semantics

- [x] Stage 7: Collapse scoped vs unscoped template-id parsing decisions into
  one `SemanticContext` entrypoint.
  Why:
  - call sites still choose manually between scoped and unscoped parsing
  Success criteria:
  - callers use a single preferred entrypoint
  - full `pa10`-`pa32` validation passes
  Notes:
  - `SemanticContext::parse_template_id_string_in_scope(...)` is now the
    preferred entrypoint when the caller may or may not have a deduction/use
    scope
  - the remaining manual scoped/unscoped branch in `template_resolution` now
    delegates to that single API instead of choosing between two parser
    methods inline

- [x] Stage 8: Retire or quarantine the legacy cursor path.
  Why:
  - the cursor path still carries its own permissive parsing semantics even
    though it delegates to the shared angle parser
  Success criteria:
  - the path is either deleted, or clearly marked legacy-only with a minimal
    shared implementation
  - full `pa10`-`pa32` validation passes
  Notes:
  - the remaining cursor helper is now explicitly named
    `parse_legacy_template_id(...)` and documented as a PA6-only wrapper
  - the implementation remains minimal and continues to delegate all angle
    handling to the shared template-angle parser rather than carrying its own
    parsing logic

- [x] Stage 9: Performance cleanup for the consolidated parser/trace path.
  Why:
  - the consolidation work should not leave avoidable overhead on hot parser
    paths, especially when tracing is disabled
  - current trace probes still do some disabled-path work, including repeated
    env checks and eager location/message preparation at some call sites
  Success criteria:
  - disabled tracing avoids repeated environment parsing and unnecessary
    location/message construction on hot parser-angle paths
  - any parser/fragment performance regressions introduced by consolidation are
    measured and reduced where practical
  - full `pa10`-`pa32` validation passes
  Notes:
  - `parser_trace` now caches its environment-driven configuration once per
    process and short-circuits category-disabled calls before location lookup
  - hot `parser.angle` and `parser.fragment` sites now use lazy helpers so
    they do not build messages unless that trace category is enabled
  - the last unguarded `cppast_parser` fragment trace site now follows the
    same guarded pattern
  - `template_argument_semantics` now emits shared `template.resolve` trace
    events for type, non-type, template-template, and deduction-sensitive
    argument classification, which improves debugging without scattering ad hoc
    caller-side logs
