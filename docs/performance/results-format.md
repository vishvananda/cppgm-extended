# Correctness and performance results

This is the proposed shared reporting contract. The initial implementation is
`scripts/benchmark_report.py`; the workload inventory is
[`benchmarks/catalog.json`](../../benchmarks/catalog.json). It accepts normalized
records without knowing the workload's build system. Existing benchmark producers
keep their measurements and statistical methods; adapters preserve those methods
when exporting this format. Migration of the existing publishers is a separate step.

## README summary and linked detail

The README should first list correctness coverage: required internal contracts,
qualified internal C++ projections, GCC and Clang language profiles, Boost,
external builds, and benchmark output checks. Show selected, executed, passed and
excluded counts with their units and measured commit. A source-only projection is
separate from its original IR, ABI, linker or driver oracle.

Performance tables are grouped by **exact CPU model and execution environment**.
The existing published data already contains two EPYC models: public benchmarks on
9V74, and index-cpp/simdjson on 7763. Dedicated Intel and GitHub-hosted Intel are
also separate profiles. The target ISA is an additional configuration property.

Within each host table, use:

| Workload / configuration | Optimization | Compile GCC / Clang | Runtime GCC / Clang | Correctness | Details |
|---|---|---|---|---|---|
| simdjson / scalar | O3 | two labeled speedups | two labeled speedups | 2 inputs | linked cases and provenance |
| simdjson / SSE | O3 | two labeled speedups | two labeled speedups | 2 inputs | linked cases and provenance |

These are column-layout examples, not measurements. Every measured row includes
its compiler revision. Missing configurations remain visible as **not measured**,
**not applicable**, **failed** or **planned**, never zero or an implicit pass.
Unsupported O0 measurements must not be inferred from O3 data.

All displayed speedups use **reference time / cppgm time**. Both GCC and Clang
are named. CPU seconds and elapsed seconds remain distinct metrics. Suite elapsed
time includes harness work and cannot silently become pure compilation time.
Link time is separate from object compilation; a complete Godot build can report
`build` time. Correctness test execution, with process startup, is separate from
an application runtime benchmark. Inception is cppgm's codegen benchmark: the
self-built and GCC-built compilers process identical compiler source inputs.
Its speedup is GCC-built compiler CPU time divided by self-built compiler CPU
time. This appears as the cppgm++ runtime result beside its compilation result.

Each linked detail table includes every test/input, absolute times when available,
paired ratios and their methods, validation results, flags, language mode, target
ISA, worker/affinity settings, exact CPU/OS, toolchain and library versions,
source/input/compiler hashes, compiler revision, CI URL and raw observations.
Ratios from paired block medians are retained; they need not equal a quotient of
independently summarized absolute times.

## Stable identities

A record identifies workload, named configuration, test or explicit aggregate,
host (including CPU model and environment), optimization level, baseline compiler,
phase and metric. A named configuration has arbitrary parameters: for example
simdjson's backend and ISA, or a future Godot build preset. Tests/inputs are children
of that configuration. Compiling one library is one measurement even when several
runtime inputs use it.

Do not aggregate across hosts, CPU models, configurations, optimization levels,
baselines, units, or measured compiler revisions. An aggregate names its exact
case set and exclusions. Both compilers must complete and pass that same case set
before a speedup is eligible for summary publication. Geometric means explicitly
state the included tests, weights and count; excluded or unavailable measurements
remain listed. Calibration warnings remain visible and observations are retained.

## Normalized record, version 1

The input document is `{"schema":1,"records":[...]}`. Each record contains:

- `workload`, `configuration`: IDs from the catalog; `test`: stable case/aggregate ID.
- `scope`: `summary` for an explicit producer-declared rollup/featured result, or
  `test` for detail-only cases. Child cases do not expand the README summary.
- `host`: `id`, exact `cpu`, `architecture`, `environment`; IDs cannot merge CPUs.
- `optimization`: explicit `O0`, `O1`, `O2`, `O3`, `Os`, `Oz` or `mixed`.
- `baseline`: `gcc` or `clang`; `revision`: full measured compiler source commit.
- `context`: explicit `language`, `target_isa`, `standard_library`, positive
  `workers`, CPU `affinity` list (empty means unpinned), and per-compiler `flags`.
- `phase`: `compile`, `link`, `build`, `runtime`, `test-execution` or `suite`.
- `metric`: `cpu_seconds` or `elapsed_seconds`; `method`: original estimator.
- `status`: `passed`, `failed`, `partial`, `not-measured` or `not-applicable`.
- `coverage`: `selected`, `candidate_executed`, `candidate_passed`,
  `reference_executed`, `reference_passed`, `excluded`, `unit`, `case_set_sha256`.
- `speedup`: positive reference/cppgm ratio, or null. Failed/partial/unmeasured
  records cannot publish a speedup. `candidate_seconds`, `reference_seconds`:
  positive absolute times or null when not provided by a historical aggregate.
- `details_url`, `raw_url`, `run_url`: retained evidence links.
- `provenance`: producer-owned flags, toolchain/library/input/binary identities,
  parameter values, observation counts, quality and warnings. Historical unknowns
  must be explicit; adapters must not invent them.

The renderer validates matrix identity, complete matching correctness counts,
units, finite positive measurements and evidence links. It produces a summary and
per-workload detailed tables from the same records. It does not establish that a
producer's tests or measurements were valid; the producer's own validation remains
required. The normalized records and raw artifacts are retained together.

The document can also contain `validations: [...]` for standalone correctness
results, such as the required internal stage/IR/ABI harness. These records need
no native baseline or timing measurement. Each identifies `workload`,
`configuration`, `host`, `revision`, `optimization`, a named `oracle`, `status`,
the three evidence URLs and `provenance`. Its `coverage` contains `selected`,
`executed`, `passed`, `excluded`, `unit` and `case_set_sha256`. A passing record
requires complete execution and validation of every selected case. These rows
populate the correctness overview and linked details without inventing a native
translation or performance score for a course-specific contract.

## Add Godot or another workload

1. Add one catalog entry with its status, documentation and named configurations.
   Godot is registered as planned; its presets and test inputs are not invented.
2. The workload runner builds and validates every declared compiler/configuration,
   freezes the inputs and records raw observations using the appropriate method.
3. Emit version-1 records for each measured phase/test/baseline. Missing compiler,
   optimization or host combinations are explicit records, not successful zeros.
4. Validate and generate tables:

   ```sh
   python3 scripts/benchmark_report.py --catalog benchmarks/catalog.json \
     --results /path/to/normalized.json --output /path/to/report-directory
   ```

5. Publish the records, summary, detailed tables and raw artifacts through the
   trusted benchmark publisher. Regenerate the README's catalog-driven rows when
   adding a configuration. No workload-specific branches belong in the renderer.

This contract and prototype are ready for review. It does not claim that the
existing publishers have already migrated or that Godot has been measured.

## Producer migration checks

The current upstream-suite artifacts retain platform/kernel, pinned sources,
compiler hashes, commands and stage CPU measurements. They do not record the
exact CPU model. Capture that identity before placing their times in a named
hardware summary; do not infer a CPU from the GitHub runner label. Keep the
existing CI run's measured source/merge commit distinct from its upstream corpus
revision. Historical unknowns stay explicit and cannot be grouped into a known
CPU profile.

Public benchmark speed ratios currently use the opposite direction from this
contract and need inversion on import. index-cpp's runtime score measures loop
elapsed time; it must retain that metric. simdjson's existing paired speedups
already use reference/cppgm. Each adapter must preserve those producers' original
estimators, input membership and validation before shared publication is enabled.
