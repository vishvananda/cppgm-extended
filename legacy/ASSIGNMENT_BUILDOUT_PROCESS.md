# Assignment Buildout Process

This is the repo-specific checklist for adding post-PA9 assignments.

## Core Repo Rules

- Real implementation lives in `dev/`.
- Assignment wrappers, specs, and tests live in `paN/`.
- Every milestone should run from the repo root with `make test-paN`.
- Prefer deterministic outputs and checked-in `.ref` files when no external reference
  implementation exists.
- Keep milestones week-sized. If one assignment owns more than one major subsystem, split it.

## 1. Freeze The Contract First

Before writing code, define:

- binary name
- command-line contract
- output contract
- dependencies on earlier assignments
- explicit non-goals
- smallest first passing scope

If the milestone is syntax-driven, add `paN/paN.gram`.

If the milestone defines or adopts a long-lived IR, state explicitly:

- whether the assignment owns the full IR contract or only a required subset
- which later assignments extend that same IR
- whether there is a temporary scaffold backend and whether it is part of the public contract

### Contract Rules

- If a grammar file exists, it is the authoritative syntax contract.
- README prose should explain the boundary and ownership, not act as a second grammar.
- New syntax goes forward into a later assignment’s `.gram`; do not silently grow older
  grammars except for errata or missing in-scope items.
- If later milestones depend on new IR forms, either:
  - keep them reserved, or
  - promote them into the defining assignment’s required subset and add tests there.

### Compatibility Rule

Later assignments must extend earlier ones monotonically:

- if a program is entirely inside an earlier assignment’s supported subset, its observable
  output for that earlier milestone should not change just because later shared code exists
- prefer demand-driven or source-triggered later features
- “undefined outside the subset” is not enough if the later implementation perturbs programs
  that are still inside the earlier subset

### Out-Of-Scope Rule

Unless the assignment contract explicitly requires deterministic rejection, features owned by
later assignments should be treated as undefined in earlier milestones.

That means:

- do not build earlier tests around “not yet supported” failures by default
- only add negative tests for later-feature syntax/semantics when the README explicitly says
  the earlier milestone must reject them
- if a later shared implementation causes an earlier binary to start accepting out-of-scope
  inputs, trim or rewrite the earlier tests rather than adding boundary guards just to keep
  those tests failing

## 2. Choose The Oracle

Use one or more of these:

1. external reference binary
2. checked-in golden `.ref` files
3. derived oracle script for regeneration
4. structural backend oracle
   - AST dump
   - LowIR dump
   - machine-IR dump

Default for PA10+:

- checked-in `.ref` outputs

### Test Layout

Final assignment layout should default to a single:

- `tests/`

directory.

Subdirectories should only be introduced when they represent a real difference in
test role or oracle, for example:

- strict vs structural backend oracles
- compile vs link vs runtime test families
- fixture-heavy integration tests kept separate from the default local suite

The older `tests/spec/` and `tests/derived/` split was useful during buildout, but
it should be treated as a transitional maintainer aid rather than the final
student-facing assignment shape.

If a buildout temporarily uses separate trusted and generated buckets, the cleanup
pass should normally collapse them back into one coherent `tests/` directory unless
the distinction still matters to the assignment contract.

### Backend Rule

If a milestone is supposed to replace a scaffold backend, behaviour alone is not enough.
Add a structural oracle proving the direct path exists.

### Post-Linker Rule

After the repo has an object/link pipeline, do not automatically switch every later
assignment to object-link-run.

Primary oracle should stay at the first new boundary the assignment owns:

- syntax -> AST
- semantics -> semantic dump
- lowering -> LowIR
- native backend -> machine IR / native output
- linker/runtime -> object/link/run

Use object-link-run as the primary contract only when the new feature set materially depends
on:

- cross-object behaviour
- linker resolution/relocation
- runtime support injected or coordinated at link time
- final executable ABI/runtime behaviour that cannot be meaningfully validated earlier

For later lowering assignments after PA21, a small secondary toolchain smoke suite is fine,
but only when it adds value.

## 3. Scaffold The Implementation In `dev/`

Required steps:

1. Add `dev/<target>.cpp`.
2. Add shared modules in `dev/src/` as needed.
3. Register the target in `dev/Makefile`.
4. Update `dev/.gitignore`.
5. Verify `make -C dev <target>`.

Use unique basenames in `dev/src/` to avoid object-file collisions.

## 4. Scaffold The Assignment Wrapper In `paN/`

Required structure:

1. `paN/<target>.cpp` -> symlink to `../dev/<target>.cpp`
2. `paN/course` -> symlink to `../cppgm.tests/course` when appropriate
3. `paN/.gitignore`
4. `paN/Makefile`
5. `paN/scripts/`
6. `paN/tests/`
7. `paN/grammar/` if the assignment has a `.gram`

Wrapper rules:

- Use `$(CXX)`, never hard-coded `g++`
- Use `../dev/src` and `../obj`
- Check in generated grammar HTML when a `.gram` exists
- Document how to regenerate grammar HTML, preferably with a Docker path that works on macOS

## 5. Wire The Harness

Default PA10+ model:

- run local implementation only
- compare `.my` against checked-in `.ref`
- compare `.my.exit_status` against `.ref.exit_status`

If `course/paN` exists and has checked-in refs, run it the same way.

The harness should default to a single `tests/` directory.

If an assignment truly needs multiple test families, the harness may recurse into
subdirectories, but those subdirectories should correspond to real contract
distinctions rather than “ground truth vs generated” bookkeeping.

## 6. Add Tests In Layers

Add tests in this order:

1. smoke tests
2. one smallest-possible positive test per feature
3. negative/boundary tests
4. regression tests for every bug fix
5. direct-route backend tests when replacing a scaffold path

Test rules:

- keep tests as small as possible
- prefer deterministic text outputs
- when tightening a contract, add both a positive and a negative test
- do not use negative tests to enforce undefined future-feature behavior unless rejection is
  part of the assignment contract
- if fallback parsing/nodes exist, add tests proving they only apply where the contract
  allows them

## 7. Implement In Vertical Slices

Use this loop:

1. make the binary build
2. make one smoke test pass
3. add one feature
4. add one positive and one negative test
5. repeat

Avoid landing large parser/semantic/backend batches without tests.

If a milestone introduces a scaffold backend, keep that explicit in the contract. If a later
milestone replaces it, update the README, tests, and oracle so the direct path is actually
required.

## 8. Root Makefile Integration

New assignments should start in `EXPERIMENTAL_TARGETS`.

Sequence:

1. add `paN` to `EXPERIMENTAL_TARGETS`
2. verify `make test-paN`
3. keep it out of default root `make` until stable
4. promote later if appropriate

## 9. Documentation And Promotion

Once stable:

- update `AGENTS.md`
- update `ROADMAP.md` if the actual scope changed
- regenerate grammar HTML if the syntax contract changed
- update adjacent README handoffs if the staged flow changed

## 10. Repo Finalization

Once the assignment is functionally complete:

1. keep `paN/` as an ordinary directory in the main repo
2. commit the implementation work under `dev/` in the main repo
3. do a cleanup pass focused on:
   - code reuse from earlier assignments
   - deduplication
   - simplifications that are obvious only after the assignment is complete
4. if that changes `dev/`, commit the follow-on cleanup in the main repo
5. do an end-to-end README flow review across the surrounding assignments
6. commit the final assignment-wide doc and harness updates in the main repo

### Finalization Rule

The assignment README is for implementers.
This file is for repo maintainers building and evolving assignments.

## Key Lessons To Preserve

- Freeze grammar and assignment boundary early.
- Keep grammar authoritative and README explanatory.
- Define long-lived IR boundaries explicitly.
- If later milestones require new IR forms, update the defining IR assignment too.
- Use structural backend oracles when replacing a scaffold backend.
- Keep later features monotonic and demand-driven so earlier outputs stay stable.
- Treat out-of-scope future features as undefined unless the assignment explicitly requires
  rejection.
- Use object/link-run as the primary oracle only when the milestone actually depends on that
  layer.
