# Callsemantic Split Plan

## Goal

`dev/src/callsemantic.cpp` is too large to keep compiling and reviewing as one
translation unit. The split should reduce compile cost and narrow ownership
without turning the file into textual `.inc` chunks.

## Layout

Use `dev/src/callsemantic/` for new internal implementation units rather than
adding more `callsemantic_*.cpp` files at the top level.

Reasons:

- The intended end state is a family of internal services, so a directory is
  clearer than a long global filename prefix.
- The public include surface stays stable: `callsemantic.h` remains public, and
  `callsemantic_internal.h` remains the shared internal boundary.
- Nested source paths make ownership obvious as larger chunks move out.

Build impact:

- Source-set entries may now contain slash-separated paths, for example
  `callsemantic/memory_census`.
- `dev/Makefile` and PA wrapper Makefiles need to create nested object and
  dependency directories for those entries.

## Extraction Rules

- Prefer real `.cpp` files with small headers over textual includes.
- Keep `Analyzer` as orchestration and state; move cohesive services out behind
  explicit inputs.
- Do not expose the full `Analyzer` type through a large private header just to
  move method bodies. That lowers line count but keeps rebuild coupling high.
- Extract low-risk helper services first, then use the smaller surface to split
  stateful services.
- Validate each slice with a dev build and targeted strict tests before moving
  to the next large chunk.

## Slices

1. Build infrastructure and source-set path support.
2. Memory census service.
   This is self-contained debug/telemetry code and only needs explicit ownership
   vectors plus cache/output references.
3. Source-location tracking.
   Move token/source table handling and qualified-use occurrence indexing into a
   small stateful helper.
4. Template body checks.
   Move template-body validation helpers that already depend only on explicit
   `SemanticContext`, `Scope`, AST, and parameter inputs.
5. Text-rewrite bridge services.
   Extract as bridge debt only. The long-term goal remains structured semantic
   data rather than expanding text rewriting.
6. Nothrow/type-trait analysis.
   Move once dependencies are explicit enough to avoid a giant Analyzer private
   header.
7. Type and function registries.
   These are larger stateful services and should be split after the low-risk
   helpers have reduced direct dependencies in `Analyzer`.

## Progress

- Added nested source/object/dependency support for `dev/src/callsemantic/`.
- Extracted memory census into `callsemantic/memory_census.*`.
- Extracted source-location parsing, token scanning, nested template-id token
  recovery, and qualified-use occurrence helpers into
  `callsemantic/source_location_utils.*`.
- Extracted template-body validation helpers into
  `callsemantic/template_body_checks.*`.
- Extracted the text mention/rewrite bridge into
  `callsemantic/text_rewrite_bridge.*`; `Analyzer` now supplies only explicit
  cache/state callbacks and thin `SemanticContext` wrappers for that bridge.
- Extracted type-trait predicates for destructibility/trivial special members,
  function-local classes, and empty-class analysis into
  `callsemantic/type_trait_analysis.*`.
- Extracted noexcept/nothrow initializer, special-member, and callsem-node
  analysis into `callsemantic/nothrow_analysis.*`.
- Extracted function internal-symbol lookup, indexing, symbol reservation, and
  class-function discard cleanup into `callsemantic/function_registry.*`.
- Extracted class/type registry lookup, class-info creation, class template
  instantiation creation, and instantiated-class tracking into
  `callsemantic/type_registry.*`.

The planned split slices are complete. Future callsemantic moves should follow
the same rule: do not move method bodies behind a private `Analyzer` header.
Extract a service with an explicit input surface so compile-time ownership
actually improves.

## Completion Criteria

- New callsemantic-specific files live under `dev/src/callsemantic/`.
- Source-set manifests and Makefiles can build nested source paths.
- Each extracted slice builds as an independent translation unit.
- Targeted strict suites show no behavioral drift for the extracted slice.
