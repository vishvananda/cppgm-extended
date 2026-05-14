# Class-Use Canonical Emission Plan

## Baseline

Work starts from the repair worktree `HEAD` on branch
`codex/class-use-canonical-20260501`. Before changing compiler behavior, run:

```sh
CPPGM_STRICT_SEMANTIC_FALLBACKS=1 make test-strict STRICT_PAS='pa18 pa19 pa21 pa22' STRICT_SUBTEST_JOBS=8 CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

If that baseline fails, test `HEAD^` and then `HEAD^^` in separate clean
worktrees before implementing against the first passing revision.

## Baseline Shape

Class-use emission started with three layers of policy:

1. Semantic and template-analysis call sites decide whether to build a
   `ClassUseEmitRequest`, whether the use is `Direct`, `NestedDerived`, or
   `SourceOwned`, and in some cases set `allow_suppressed_calls`.
2. `witness_api` converts requests into `ClassUseSourceDecision`s, gates normal
   class-use recording on `function_call_source_capture_enabled()`, and gates
   the allow-suppressed path on broader `source_capture_enabled()`.
3. The semantic source-use table and C++ renderer dedupe/canonicalize later.
   The source-use table already prefers exact spelling anchors, merges
   equivalent class uses ignoring binding spacing, and gives `SourceOwned`
   precedence over `Direct`, which in turn takes precedence over
   `NestedDerived`. The renderer still has additional class-use cleanup,
   including redundant nested drops, explicit-specialization preference, less
   specific binding duplicate removal, and visible-event dedupe.

The practical problem is that suppression is not intrinsic to class-use
semantics. It is inherited from function-call source capture. The code then adds
`allow_suppressed_calls` escape hatches for class uses that must still be
visible while function-call capture is paused, which spreads policy into
callers and makes emission order-sensitive.

## Target Rule

A class-use frame should be emitted when semantic analysis has an actual
source-backed class template-id use, or a canonical source-owned class use that
represents the user spelling currently being interpreted. It should not be
suppressed merely because function-call witness capture is suppressed.

A class-use frame should not be emitted when:

- source witness capture is disabled for the session/context;
- the request has no source location;
- the request is only a nested/derived recovery and an equivalent direct or
  source-owned class use is already known for the same source site;
- the request is less specific than another equivalent request for the same
  source spelling, such as a fallback binding list that is superseded by an
  explicit specialization binding list;
- the request describes an implementation artifact with no independent source
  spelling and no source-owned role.

The renderer can remain a final safety net, but the semantic source-use table
should become the canonical home for class-use identity and precedence.

## Implementation Steps

1. Make class-use capture independent of function-call capture.
   `class_use_source_capture_enabled()` should mean "semantic source capture is
   enabled for class uses", not "function-call source capture is currently
   enabled". The context overloads should use the same rule.

2. Remove `allow_suppressed_calls` from the public class-use request API.
   Call sites should not decide whether a class use bypasses function-call
   suppression. All class-use requests use one path through `emit_class_use`.

3. Centralize request-to-recording policy in `witness_api`.
   `emit_class_use` should build a decision, apply the single class-use capture
   gate, record the source use, and note the legacy decision only after the
   source-use table has accepted the event.

4. Keep ownership as semantic provenance, not suppression control.
   Preserve `Direct`, `NestedDerived`, and `SourceOwned` while moving any
   repeated "force source owned" decisions toward helpers that derive ownership
   from the source anchor, template-id occurrence, and caller provenance.

5. Strengthen source-use canonicalization only as tests require.
   Prefer doing duplicate elimination in `semantic_source_use` when the
   duplicate relationship is semantic. Leave renderer dedupe in place at first,
   then reduce renderer-only cleanup only after strict tests show the semantic
   table is producing canonical events.

6. Validate incrementally.
   After each behavior slice, run focused builds/tests for the touched binary if
   available, then run the requested strict command as the acceptance test.

## Implemented Direction

The implementation keeps `class_use_source_capture_enabled()` as the plain
semantic source-capture gate and adds a named `ClassUseEmissionOrigin` for the
few cases that have independent source ownership while function-call candidate
analysis is in progress. Ordinary resolved template-id observations remain
non-recording during that speculative window, because those can be artifacts of
overload/SFINAE checking rather than standalone user-facing class-use frames.

This replaces `allow_suppressed_calls` with provenance:

- `ResolvedTemplateId`: default; records when normal class-use recording is open.
- `ExplicitSpecializationSource`: records an explicit-specialization source use.
- `QualifiedValueSource`: records class uses discovered through qualified
  template value lookup.
- `NestedSourceTemplateId`: records source-owned nested template-id uses.

Renderer dedupe remains in place as the final safety net.

The follow-up cleanup removes the remaining class-use-specific suppression
flag from class template reference/type lookup. Those call paths now pass
`ClassTemplateSourceUseMode::EmitClassUse` for the canonical outer class-use
event, or `ClassTemplateSourceUseMode::NestedArgumentsOnly` when analysis is
only recovering nested template-id uses from the argument spelling.

The final cleanup keeps the same canonical behavior but removes the remaining
class-use suppression terminology from the implementation. Source-dependent
class template uses that cannot produce a canonical source frame are tracked as
noncanonical drops, and qualified value class-use recovery uses the same
noncanonical predicate language.

## Completion

The implementation is split into staged commits:

- `75ac723f` `Canonicalize class use emission provenance`
- `053507db` `Replace nested class-use force flag with ownership`
- `47d73cf4` `Replace class template source suppression with mode`
- `f690a125` `Rename class-use noncanonical drops`

Final validation command:

```sh
CPPGM_STRICT_SEMANTIC_FALLBACKS=1 make test-strict STRICT_PAS='pa18 pa19 pa21 pa22' STRICT_SUBTEST_JOBS=8 CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

Result: all requested strict tests passed.
