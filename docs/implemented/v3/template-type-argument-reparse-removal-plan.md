# Template Type-Argument Re-parse Removal Plan

## Goal

Remove the remaining text **re-parse** in template type-argument resolution:
`parse_type_argument_text(...)` and the sibling text-keyed lookups in the
explicit type-argument resolver. A template type argument should resolve from
the structured syntax the parser already produced (`TemplateArgumentSyntax`
with `type_id` / `template_id` / `resolved_type`, carried via
`TemplateIdSyntax::argument_syntaxes`), never by splitting a template-id's
*name text* back into argument strings and re-parsing each one.

This is the type-argument analogue of the already-landed change
*"Remove text-based type-pack fallback; thread typed pack info"*
(`93707632f`), and a sibling of
[`resolve-type-lookup-text-removal-plan.md`](resolve-type-lookup-text-removal-plan.md)
and [`semantic-fallback-removal-plan.md`](semantic-fallback-removal-plan.md).

This is **not** a ref-update exercise. Witness / LowIR / exit-code drift means
the structured replacement lost information, unless a clang/g++ reference or an
assignment contract proves otherwise.

## Baseline to protect

- Branch `boost-updated`, commit **`9610a1ec9`** ("Remove vestigial
  `__check_valid_allocator` static_assert text fallback").
- Both lanes green: **3281 / 3281** (default clang/libc++ and gcc15
  g++/libstdc++). Re-verify with `make test-report`, run **alone** (concurrent
  load causes false timeout failures).

## What "the re-parse" is

In the explicit type-argument resolver (`template_resolution.cpp`,
`resolve_template_argument`, ~lines 10330–10385) the structured paths run first
(keyed on `syntax->resolved_type` / `syntax->type_id` / `syntax->template_id` /
`syntax->expression`). If none of those produced a type **and** there is no
structured syntax, five text paths run, all gated on
`!type && !has_structured_type_syntax`:

| Line (approx) | Path | Kind |
|---|---|---|
| 10339 | `resolve_non_dependent_direct_type_argument` | text |
| 10343 | `lookup_direct_bound_type_argument` | text-keyed lookup |
| 10353 | `lookup_rewritten_bound_type_argument` | text-keyed lookup |
| 10363 | `lookup_exact_visible_type_argument_text` | text-keyed lookup |
| 10373 | `parse_type_argument_text` | **text re-parse** |

`parse_type_argument_text` (`template_argument_semantics.cpp:28090`) is the core
one. It is *not* dumb name matching — it dispatches to
`parse_simple_type_specifier_argument_text`, `parse_template_id_argument_text`,
`parse_function_type_argument_text`, `parse_decltype_or_typeof_text`, i.e. it
re-lexes and re-parses the argument text into proper AST forms and resolves
them. It is functionally correct; the objection is that it **re-parses text
whose AST was already built once and then discarded**.

`has_structured_type_syntax` is defined at `template_resolution.cpp:10244`:

```cpp
const bool has_structured_type_syntax =
    syntax && (syntax->resolved_type || syntax->type_id || syntax->template_id);
```

An audit already marks the boundary: `semantic_fallback_audit::hard_fail` with
category `explicit-type-argument-text-fallback`
(`template_resolution.cpp:10454`) and `template-type-resolution-fallback`
(`template_argument_semantics.cpp:1121/1128`,
`template_resolution.cpp:11838/11846`). These only throw in strict mode; in
normal operation the text paths run.

## Root cause

The structs are **not** missing fields. `cpp_decl_model.h` already has:

```cpp
struct TemplateArgumentSyntax {
  std::string text; std::string source_text; ...
  std::shared_ptr<TemplateIdSyntax> template_id;
  std::shared_ptr<CppAstNode>       type_id;
  std::shared_ptr<CppAstNode>       expression;
  TypePtr                           resolved_type;
};
struct TemplateIdSyntax {
  QualifiedName name; ...
  std::vector<std::string>             arguments;        // text
  std::vector<TemplateArgumentSyntax>  argument_syntaxes; // structured
};
```

The failing arguments reach `resolve_template_argument` with **`syntax ==
nullptr`** — no `TemplateArgumentSyntax` at all, so there is no `type_id` AST to
use and text re-parse is the only option. They are dependent member types and
function types, e.g. (traced from `std::set<int>` + `std::vector<std::string>`):

```
_Rp(_ArgTypes...)                      (std::function signature)
typename allocator_type::value_type
typename __node_traits::size_type
typename __alloc_traits::size_type
```

### Why `syntax` is null: name-text resolution + the anchor mechanism

`lookup_type_impl` (`callsemantic.cpp:6556`) resolves a template-id from its
**name string** (`normalized_name`):

```cpp
// ~7322
const vector<TemplateArgumentSyntax> * arg_syntaxes = nullptr;
bool parsed_template_id =
    ... && split_top_level_template_id_text(normalized_name, template_id, arg_texts);
if (parsed_template_id) {
  arg_syntaxes = exact_template_type_lookup_anchor_arg_syntaxes(normalized_name, template_id.name);
} else if (... '<' ... '>' ...) {
  const ExactTemplateTypeLookupAnchor * anchor = current_exact_template_type_lookup_anchor();
  ...
}
```

So structured argument syntaxes are carried into name-based type lookup through
a **thread-context anchor** (`ExactTemplateTypeLookupAnchor`, registered with
`ScopedExactTemplateTypeLookupAnchor`). Callers that hold the parsed
`TemplateIdSyntax` register an anchor; callers that resolve a template-id by
name **without** registering one leave `arg_syntaxes == nullptr`, the args are
split out as text, and the text re-parse fires.

Reference registrations to copy: `callsemantic.cpp:11683-11709` and
`16400-16421`. The anchor carries
`arg_syntaxes = template_id_syntax->argument_syntaxes`.

The syntax-aware resolvers already exist but are commonly handed `nullptr` AST
nodes and so stay dormant — e.g.
`resolve_out_of_class_named_method_binding_with_syntax` is invoked with
`function_identifier == nullptr` at `callsemantic.cpp:22694`, and
`resolve_out_of_class_owner_class_from_template_id_syntax`
(`callsemantic.cpp:22376`, which *does* pass
`&owner_template_id->argument_syntaxes` at 22436) is only reached when a real
AST node is threaded in.

## Why it is load-bearing (do not just delete)

Probe (disable all five text paths, then `make test-report`):

- all five disabled → **3156 / 3281** (125 failures, almost all `pa35` hosted:
  `vector{,-bool,-char,-string,-class-brace}`, `set-insert`,
  `range-for-member-map`, `ostringstream-tellp`, `stringstream-insertion`,
  `use-facet-locale`, `shared-ptr-inline-odr`, `std-function-recursive-lambda`,
  `special-member-cross-tu`, `unreachable-inline-callee`, …)
- only the four lookups disabled, `parse_type_argument_text` kept → **3280 /
  3281** (1 failure: `pa22/500-array-type-argument-sfinae-static-value`)

So `parse_type_argument_text` carries the real work; the four lookups are nearly
redundant (one is needed for array-type args). That 125-test set is the
**acceptance gate** for the full removal.

## Fix pattern (per site)

Register a `ScopedExactTemplateTypeLookupAnchor` carrying the owner/type
`TemplateIdSyntax::argument_syntaxes`, in scope for the nested resolution that
reaches `lookup_type_impl`. Recipe (mirrors `callsemantic.cpp:11683`):

```cpp
ExactTemplateTypeLookupAnchor anchor;
anchor.template_text = template_id_syntax_text_preserving_spacing(*ts);
anchor.identifier    = template_lookup_fragment_identifier(anchor.template_text);
if (anchor.identifier.empty()) anchor.identifier = ts->name.name;
anchor.compact_key   = compact_lookup_text(anchor.template_text);
anchor.arg_texts     = ts->arguments;
anchor.arg_syntaxes  = ts->argument_syntaxes;   // <-- the structured payload
anchor.has_argument_list = true;
anchor.location      = /* normalize witness source location */;
const ScopedExactTemplateTypeLookupAnchor guard(anchor);
// ... existing by-name resolution call ...
```

`lookup_type_impl` then picks it up at 7331
(`exact_template_type_lookup_anchor_arg_syntaxes`) or 7337
(`current_exact_template_type_lookup_anchor`). The owner `TemplateIdSyntax`
comes from the AST node via
`cppast_qualifier_template_id_syntax(node, owner_qualifier_index)`
(`cppast_ast.h:532`), where `owner_qualifier_index = qualifiers.size() - 1`.

Where the by-name resolver is reached through a virtual interface that only
carries strings, register the anchor at the **caller that still holds the AST
node** (e.g. the collector), since the anchor is a thread-context guard that
flows down through the nested call. Alternatively thread the AST node
(`function_identifier`) through the interface (more invasive: virtual decl +
overrides + wrappers + call sites).

## Staged plan

Each stage builds, then iterates one failing test at a time using the tracing
below. The tree stays red between stages; that is expected — there is **no
incremental green milestone** until all distinct sites are done (each repro hits
several sites). Commit only when the full suite is green again.

- **Stage A — DONE (`9610a1ec9`).** Removed the vestigial
  `__check_valid_allocator` static_assert text hack
  (`evaluate_static_assert_text_fallback` + `compact_static_assert_condition_text`
  + call site in `semantic_declaration.cpp`). It was already covered by the
  structured path; clean deletion, both lanes green.

- **Stage 1 — Remove the fallback + instrument.** Disable the five text paths in
  `resolve_template_argument`. Add a backtrace trace in the plural
  `resolve_template_arguments` (`template_resolution.cpp` ~10637, gated on
  `CPPGM_STAGEC_TRACE`) that prints `join_template_texts(texts)` and a
  `backtrace`/`backtrace_symbols` dump when `syntaxes` is null/empty. Build.

- **Stage 2 — Site: out-of-class method template.** First failure
  (`texts=[bool,_Allocator]`) path:
  `template_api::resolve_template_arguments` (null syntaxes) ←
  `class_template_id_arguments_are_dependent` (`callsemantic.cpp:4485`) ←
  `lookup_type_impl` (call at 8044) ← `resolve_qualified_owner_class` ←
  `resolve_out_of_class_owner_class` (`22363`, string, drops syntax) ←
  `resolve_out_of_class_method_template` (`24371`/`24465`) ←
  `TemplateDeclarationCollector` (`template_declaration_collector.cpp:834`,
  `:2400`). Fix: register the anchor at the collector call sites (they hold the
  `inner` AST node), or thread `function_identifier` so
  `resolve_out_of_class_owner_class_from_template_id_syntax` is used.
  Verify: the `bool,_Allocator` failure clears (next failure appears).

- **Stage 3 — Remaining distinct sites.** Repeat the trace per failing test.
  Known additional chains: the `unordered_map`/`map`/`set` member-typedef bodies
  (`typename allocator_type::value_type`, etc. — a different chain than
  out-of-class method templates), and `std::function`'s `_Rp(_ArgTypes...)` at
  namespace scope. Distinct paths are few; the 125 tests share them.

- **Stage 4 — Clean up + verify + commit.** Delete the five text-path blocks and
  the now-dead helpers (`lookup_direct_bound_type_argument`,
  `lookup_rewritten_bound_type_argument`,
  `lookup_exact_visible_type_argument_text`,
  `resolve_non_dependent_direct_type_argument`, `replace_identifier_token_text`;
  `parse_type_argument_text` only if it has no other live callers — it has many,
  so likely keep it but ensure the explicit type-arg resolver no longer calls
  it). Remove the trace + `#include <execinfo.h>`. Run `make test-report`
  default **then** gcc15, each alone. Expect 3281/3281 both. Commit
  single-line, push `origin boost-updated`.

## Tracing method (how to find each site)

```sh
# one-time: libc++ include dirs for direct hosted compiles
isys=$(/usr/local/opt/llvm/bin/clang++ -stdlib=libc++ -E -v -x c++ /dev/null 2>&1 \
  | awk '/#include <...> search starts/{f=1;next} /End of search list/{f=0} f{gsub(/^ +/,"");print}' \
  | grep -v Framework | sed 's/^/-isystem /' | tr '\n' ' ')

# build with the fallback disabled + trace, then compile a repro
CPPGM_STAGEC_TRACE=1 ./dev/cppgm++ -std=c++17 $isys -c -o /tmp/x.o repro.cpp 2>&1 \
  | grep -A12 'STAGEC-NULLSYN'
```

The release build inlines the singular resolver, so put the trace in the
**plural** `resolve_template_arguments` (a real call boundary). Minimal repros:
`#include <functional>` + `std::function<int(int)>`; `#include <set>` +
`std::set<int>` + `std::vector<std::string>`.

## Pragmatic alternative

`parse_type_argument_text` already performs a *structured* re-parse and is
functionally correct. If the objective is correctness rather than eliminating
the re-parse, the only clearly dead code is three of the four lookups
(probe-verified: keeping `parse_type_argument_text` plus the one array-needed
lookup is green at 3280→3281). Removing those three is a small, committable
cleanup that does not require the multi-site anchor work.

## Status

- Stage A landed and pushed (`9610a1ec9`); both lanes green.
- Stages 1–4 designed and instrumented; site #1 traced to exact lines. The
  remaining work is a multi-site context-threading refactor (anchor registration
  / AST-node threading), not a localized field change. Live working notes:
  `memory/text-fallback-removal-plan.md`.
