# The regression lane

The fixtures under this directory are the compiler team's own memory of
its decisions: an optimized LowIR reference or a machine-IR dump that our
implementation produced, kept so that a later change cannot silently move
it.  They are compared exactly, after the harness's presentation
canonicalization, and they are not part of any assignment's contract.

A student's implementation is judged by the course suites under
`course/`, which state each fixture's contract, outcome and size envelope
(`x.ref.expect`, `x.ref.ir`, program behaviour, object inspection), not by
this lane.  When a course fixture's only justification is the shape one of
our passes produces, it belongs here.

Layout mirrors the assignments: `regression/pa29/{strict,structural,behavior}`,
`regression/pa37/{o0,o1,o2,o3,driver/o1,driver/o2,driver/o3}`,
`regression/pa38/{o1,o2,o3,behavior/o1,behavior/o2,behavior/o3}`.  Each
assignment's `make test` runs its regression roots after its course roots.
