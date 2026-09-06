# Brief: implement PA38's planned register allocation as a newcomer

You are a student implementing the register allocator of Programming
Assignment 38 (`lowir2native -O*`) against the course material only.  The
course solution's allocator exists in this tree; you are not allowed to read
it, and the point of the exercise is to find out whether the assignment's
own instructions and tests are enough to write a different one that passes.

## What you may read

- `pa38/README.md`, in particular "How PA38 Is Specified", "Quality Bar",
  "Validation Modes", "Testing", and in "One Design" only the paragraph
  "Where a different allocator plugs in".
- `dev/src/native/allocation/planning_seam.h`: the seam you implement, with
  the contract a correct plan must respect.
- `dev/src/native/allocation/location_planning.h`: the timeline types
  (`PlannedLocationSegment`, `PlannedLocationKind`, `FunctionLocationTimeline`).
- `dev/src/native/analysis/function.h`: the facts (`FunctionFacts`) you consume.
- `dev/src/native/allocation/registers.h` and `dev/src/native/mir/registers.h`
  (or wherever `X64Register` is declared): register names and
  `is_callee_saved`.
- `dev/src/lowir/model/program.h`: the `LowirFunction` and `Instruction`
  types, as far as you need positions, kinds and value ids.
- `dev/src/native/driver/stats.h`: only if you want to count something.
- `dev/src/backend_variant.h`.
- The fixtures under `pa38/tests/`, including their
  `.ref.expect` sidecars, `.ref.mir` dumps and `.my.mir` outputs.
  `scripts/expect_ir.pl <file.mir> <file.ref.expect>` evaluates a sidecar.

## What you may not read

- `dev/src/native/allocation/location_planning.cpp` (the course solution's
  allocator), any other `.cpp` under `dev/src/native/allocation/` except
  `linear_scan.cpp`, and anything under `dev/src/native/lowering/`.
- Git history or diffs of those files.

If you find you cannot proceed without a fact from a forbidden file, do not
read it: write down exactly what you needed in your notes and make the most
conservative choice that keeps the plan correct (planning nothing for that
case is always correct).

## What you deliver

1. `dev/src/native/allocation/linear_scan.cpp`: implement
   `plan_value_locations_linear_scan` as a linear-scan register allocator
   over the candidate intervals the seam header describes.  Determinism is
   required (no hashing by pointer, no unordered iteration order).  Do not
   change any other source file.
2. `doc/backend-review/newcomer-notes.md`: what you needed that the README
   and the seam header did not tell you; where the wording misled you;
   which forbidden file you would have opened and for what fact; how long
   each part took in rough terms.  This file is the real deliverable: its
   contents become README changes.

## How you know you are done

Build with `make -C dev -j32`.  Then, from the repository root:

```sh
CPPGM_BACKEND_VARIANT=linear-scan make -C pa38 test-course
CPPGM_BACKEND_VARIANT=linear-scan make -C pa38 test-perf
CPPGM_BACKEND_VARIANT=linear-scan make -C pa37 test-course
CPPGM_BACKEND_VARIANT=linear-scan make -C pa29 test-course
```

All four must pass.  With the stub that plans nothing, the first fails two
budgets (`tests/o1/420-loop-and-eh-placement`: 35 memory operands
against 31, and 19 instructions in `@walk_unavoidable` against 17;
`tests/o2/410-eh-edge-placement-barrier`: 4 memory operands against 3)
and the second fails two behaviour programs (the fill loops in
`tests/behavior/o1/520-*`, which need their loop-carried values in
registers to stay within 10% of the reference count).  Those are the bar.
You may not edit fixtures or sidecars.  Do not run `make test-cells`, and
do not run `make test-variants`.

When a run fails, `pa38/<lane>/<fixture>.my.mir` is your output and the
message names the predicate; `perl scripts/expect_ir.pl` reruns it.
