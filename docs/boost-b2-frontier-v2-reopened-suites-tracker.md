# Boost B2 Frontier V2 Reopened Suites Tracker

This tracker is intentionally separate from
`docs/boost-b2-frontier-v2-tracker.md`. It records the independent validation
of the three V2 suites that were previously blocked by missing host
dependencies, so the results can be merged into the canonical tracker without
interleaving with its active suite cursor.

## Isolation

- branch: `boost-frontier-v2-test-intake`
- starting commit: `3373a8dcc`
- finalized upstream base: `222a0ab0f`
- compiler under test:
  `/Users/vishvananda/cppgm-test-intake/dev/cppgm++`
- final compiler commit: `58d4643c9`
- host compiler: Homebrew Clang 22.1.0 at
  `/usr/local/opt/llvm/bin/clang++`
- Boost tree: `/Users/vishvananda/boost_1_91_0`
- B2 build roots: `/private/tmp/cppgm-boost-reopened-v2/build-*`
- logs: `/private/tmp/cppgm-boost-reopened-v2/logs`
- B2 configuration:
  `/private/tmp/cppgm-boost-reopened-v2-user-config.jam`
- canonical tracker policy: unchanged until these independent rows are ready
  to merge

Every credited run uses an isolated `--build-dir`, `-a`, `pch=off`, forced
C++11, `CPPGM_B2_CXX` pinned to this worktree's absolute compiler path, and
explicitly pinned Homebrew Clang paths for host C/assembly/link actions. The
validation and performance ladder follows `docs/boost-b2-frontier-v2-plan.md`.

## Suite Status

| Suite | Path | Prior blocker | Status | Evidence |
|---:|---|---|---|---|
| 57 | `libs/leaf/test` | missing `nlohmann/json.hpp` | pass | Final forced graph updated 482 targets: 123 tests passed, all 22 negative compilations failed as expected, and there were no skips or unexpected failures. The nlohmann header and positive/negative serialization cases were included. |
| 51 | `libs/graph_parallel/test` | missing MPI, runner, and Python target | pass | The final forced C++11 replay updated all 207 targets: 25 MPI-backed runtime tests passed, both examples compiled and linked, and there were no failures or skips. The run took 4,341.72 seconds with 2,318,970,880 B maximum RSS and zero swaps. |
| 68 | `libs/mpi/test` | missing MPI compiler and runner | pass | The final pinned forced C++11 replay updated all 458 requested targets. All 94 MPI runtime tests passed, including the ten paths that did not pass in the initial survey; there were no failures or skips. The run took 7,501.86 seconds with 1,363,877,888 B maximum RSS and zero swaps. |

## Frontier Log

- `2026-08-03`: Created the independent reopened-suite lane after rebasing the
  intake branch onto the finalized Boost.JSON frontier and passing the full
  direct-LowIR report `4734/4734`. The canonical V2 tracker remains untouched.
- `2026-08-03`: Reopened `libs/leaf/test` with the installed Homebrew
  nlohmann-json headers. The initial full graph exposed three independent
  compiler defects: contextual logical-bool probing instantiated a worse
  conversion-template body; two same-spelled function-local polymorphic
  classes shared internal vtable identity; and retained dependent `decltype`
  syntax did not expand a pack nested in a structured template-id. Reduced
  owning regressions landed in PA21, PA17, and PA23 respectively. The related
  placement-audit false positive was narrowed so `__vmi_class_type_info`
  requires source `dynamic_cast` or `typeid` evidence before implying PA27.
- `2026-08-03`: Closed LEAF on the final narrowed compiler. The exact forced
  B2 run (`-a -j2`, C++11, PCH off, static, hidden visibility) updated 482
  targets with 123 passes, 22 failures-as-expected, no skips, no unexpected
  failures, zero swaps, and 648,364,032 B maximum RSS. Repository validation
  passed the direct-LowIR report `4737/4737`; strict comparisons passed PA18
  `263`, PA19 `145`, PA21 `234`, PA22 `290`, and PA23 `369`; all 23 strict
  text-reparse categories remained zero and all 14 audit unit tests passed;
  the placement-audit unit suite passed all 35 tests; and normal, every
  individual cache-disabled mode, and all-disabled mode emitted byte-identical
  LowIR for all three reducers. The frozen three-run performance gate passed at
  -37.01% instructions, -41.60% max RSS, and -42.34% peak footprint
  (`/private/tmp/cppgm-boost-reopened-v2/leaf-final-candidate.json`). PA17,
  PA21, and PA23 already document the tested assignment surfaces, so no README
  expansion was needed. The canonical V2 tracker remains unchanged.
- `2026-08-03`: Advanced the independent cursor to
  `libs/graph_parallel/test` after committing the LEAF closure. The suite uses
  the isolated `build-graph-parallel` root and leaves the canonical tracker and
  active frontier build roots untouched.
- `2026-08-03`: The first Graph Parallel replay exposed three compiler defects.
  Dependent elaborated template-ids in function-local typedefs lost their
  retained lookup nodes; partial ordering tried to deduce a qualified
  non-deduced parameter; and output resolution collapsed repeated member
  template specializations with the same function type onto the first body.
  The fixes and owning regressions landed in PA18, PA22, and PA26 as commits
  `ae9ddf1db`, `2f09d66c2`, and `d157f251b`. The last fix also corrected the
  distributed CSR `vertex` overload, whose selected specialization previously
  emitted the less-specialized sequential body. A header-free PA22 regression
  now covers that overlapping free-function-template output identity in
  `9e17a42e9`.
- `2026-08-03`: Focused Graph validation passes on the final compiler. The
  exact forced `distributed_csr_algorithm_test-1` target rebuilt 107 targets
  and passed compile, link, and runtime with zero swaps
  (`graph-parallel-csr-forced-after-template-identity-output-fix.log`). After
  replacing only its stale generated test object, the two-rank
  `distributed_connected_components_test-2` target passed; an independent
  Homebrew Clang C++11 build of the same target also passed
  (`graph-parallel-connected-components-current-incremental.log` and
  `graph-parallel-connected-components-clang.log`). The PA22 direct-LowIR
  report passes `388/388`, strict comparison passes `290` with `98` expected
  witness skips, and the placement audit reports zero findings. The fixed
  cumulative performance gate remains passing at -36.97% instructions,
  -40.09% maximum RSS, and -42.28% peak footprint
  (`perf-template-identity-output.json`). The final forced full Graph Parallel
  graph remains the suite-closing gate.
- `2026-08-03`: Rebased the independent lane onto finalized upstream commit
  `222a0ab0f` and rebuilt `dev/cppgm++` incrementally with Homebrew Clang 22.1.0.
  The combined PA18/PA22/PA24 hard placement audit scanned 792 tests with zero
  placement or hygiene findings; manual template-composition review keeps the
  new PA18 lookup and PA22 substitution tests at their single-feature owners,
  and keeps the revised repeated local declaration probe in PA24's
  post-template language-closure integration suite. The focused direct-LowIR
  report passes `792/792`; strict comparison passes PA18 `263`, PA19 `145`,
  PA21 `234`, PA22 `290`, and PA23 `369`; all 23 text-reparse categories remain
  zero and all 14 audit unit tests pass; and the full direct-LowIR report passes
  `4746/4746`. Existing PA18, PA22, and PA24 assignment text already covers the
  tested lookup, substitution, and integration surfaces, so no README change
  is needed. The final forced Graph Parallel graph and refreshed performance
  gate remain pending after the active canonical frontier replay finishes.
- `2026-08-03`: Closed `libs/graph_parallel/test` with the isolated final
  forced replay (`-a -j1`, C++11, PCH off, static, hidden visibility). B2
  updated all 207 targets; all 25 MPI-backed runtime tests passed; both Graph
  Parallel examples compiled and linked; and there were no failures or skips.
  This forced pass includes the previously failing strong-components,
  connected-components, shortest-paths, distributed CSR algorithm, and
  betweenness-centrality cases, so it exercises every compiler fix found by
  the suite. The run took 4,341.72 seconds with 2,318,970,880 B maximum RSS and
  zero swaps
  (`graph-parallel-full-forced-after-final-rebase.log`). The post-rebase
  repository gates remain the placement, focused, strict, text-reparse, and
  full direct-LowIR passes recorded above. The refreshed frozen three-run
  performance gate also passes at -37.12% instructions, -41.11% maximum RSS,
  and -42.28% peak footprint
  (`/private/tmp/cppgm-boost-reopened-v2/perf-final-rebase-graph.json`). The
  independent cursor now advances to `libs/mpi/test`.
- `2026-08-03`: The initial full `libs/mpi/test` survey found 8,828 targets,
  scheduled 1,360 updates, updated 1,329, passed 83 MPI tests, failed ten
  targets, and skipped 21 dependents. Nine link failures covered
  `broadcast_test` ranks 2 and 17 plus all seven `skeleton_content_test`
  ranks: namespace-scope explicit function-template specializations were
  retained as local object bindings and five definitions were not required
  for output, so the static MPI archive lacked externally linkable broadcast
  symbols. `sendrecv_vector-2` failed earlier in compilation because the
  ambiguous inherited `marker` lookup used by a partial-specialization
  constraint escaped instead of discarding that candidate.
- `2026-08-03`: Commit `58d4643c9` fixes both typed causes. Externally linked
  explicit specializations no longer prefer local object binding, and defined
  namespace-scope explicit specializations require output while unused inline
  specializations from include paths retain lazy suppression. Ambiguous member
  lookup now has a typed soft-failure category that partial-specialization
  matching rejects as substitution failure; ordinary ambiguous lookup remains
  a hard diagnostic. Header-free PA29 and PA22 regressions cover the separate
  object and SFINAE behavior. Clang 22 and GCC 13 accept and run both controls.
  PA22 and PA29 already document the owning substitution and separate-object
  surfaces, so no README edit is needed. The multiple-inheritance placement
  detector was narrowed to require materialized object behavior instead of a
  type-only multiply-derived declaration; all 36 placement-audit tests pass.
- `2026-08-03`: An unpinned verification attempt was discarded after it
  exposed the B2 wrapper's default compiler path: it points at the active
  `cppgm-extended` worktree, whose binary was rebuilt during that attempt and
  produced the known `check_const_loading<const boost::mpi::content>` static
  assertion. The attempt was stopped immediately and receives no validation
  credit. The plan now requires every survey and direct B2 command to pin
  `CPPGM_B2_CXX` and both host tools explicitly. The correctly pinned focused
  replay rebuilt 98 targets and passed `version_test-1`,
  `sendrecv_vector-2`, `broadcast_test-2`, and `skeleton_content_test-2`
  through compile, link, and MPI runtime in 695.61 seconds, with
  1,072,766,976 B maximum RSS and zero swaps.
- `2026-08-03`: Closed `libs/mpi/test` on the explicitly pinned compiler at
  `58d4643c9`. The exact serialized forced replay (`-a -j1`, C++11, PCH off,
  static, hidden visibility) found 8,828 targets and updated all 458 requested
  targets. All 94 MPI runtime tests passed with no failure or skip, including
  both broadcast ranks, all seven skeleton/content ranks, and
  `sendrecv_vector-2`. The run took 7,501.86 seconds with 1,363,877,888 B
  maximum RSS and zero swaps
  (`mpi-full-forced-pinned-intake-final.log`). Repository validation passes
  the full direct-LowIR report `4748/4748`, all 1,301 configured strict
  comparisons, all 23 zero-count text-reparse categories and 14 audit tests,
  all 36 placement-audit tests, and clean PA22/PA29 placement and hygiene
  scans. The frozen three-run performance gate passes at -36.80%
  instructions, -44.35% maximum RSS, and -41.43% peak footprint
  (`/private/tmp/cppgm-boost-reopened-v2/perf-final-mpi.json`). All three
  reopened suites are now closed; the canonical V2 tracker remains unchanged
  for later merge.
