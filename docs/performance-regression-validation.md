# Performance Regression Validation

Use this check before keeping changes that could affect compiler performance. It
is designed to work on a loaded machine by gating on hardware counters and
memory, not wall time.

## Metrics

Primary gates:

- `instructions retired`: main signal for CPU work.
- `maximum resident set size`: resident memory pressure.
- `peak memory footprint`: total process footprint on macOS.

The script also records wall, user, system time, and cycles when available, but
wall time is informational only because it is too noisy under load.

## Create A Baseline

Create the baseline from a known-good commit on the same machine and with the
same build mode. A few runs are useful here because this is the reference other
workers will compare against.

```sh
scripts/validate_perf_regression.py record \
  --baseline /tmp/cppgm-semantic-overload-baseline.json \
  --runs 3 \
  -- ./dev/cppgm++ -I benchmarks/self_compile/stable/include \
     -c -o /tmp/cppgm-baseline.o \
     benchmarks/self_compile/stable/semantic_overload.cpp
```

The default command is the same semantic-overload compile test, so this shorter
form is equivalent except for the output filename:

```sh
scripts/validate_perf_regression.py record \
  --baseline /tmp/cppgm-semantic-overload-baseline.json \
  --runs 3
```

## Check A Candidate

Candidate validation collects three runs and compares the median against the
saved baseline median. Instruction count and peak footprint use hard limits.
Maximum RSS uses the confirmation rule below.

```sh
scripts/validate_perf_regression.py check \
  --baseline /tmp/cppgm-semantic-overload-baseline.json \
  -- ./dev/cppgm++ -I benchmarks/self_compile/stable/include \
     -c -o /tmp/cppgm-candidate.o \
     benchmarks/self_compile/stable/semantic_overload.cpp
```

Default tolerances are:

- `--instruction-tolerance 0.005`
- `--rss-warning-tolerance 0.03`
- `--footprint-tolerance 0.01`

Maximum RSS uses a warning-and-confirmation rule. A result at or above the RSS
threshold starts one more batch with the same run count. The candidate fails
when the confirmation batch also reaches or exceeds the threshold. The
`--rss-tolerance` spelling remains as a compatibility alias. Instruction and
footprint limits remain hard gates on each batch.

Each check collects a three-run median. An RSS warning collects three more
runs for the confirmation median. Do not start another batch after that
confirmation.

A failed hard gate or confirmed RSS warning starts a regression investigation.
The failure rejects the measured implementation; an in-scope correction may
preserve the underlying change.
Check the changed semantic work, allocation counts, executable layout, and the
other recorded counters before reverting the change. Amend the candidate when
an in-scope correction removes the cost, then validate and measure that changed
candidate. Do not rerun an unchanged failed commit. Revert after the
investigation finds no maintainable correction within the candidate's scope.

## Notes

- Run from the repository root after building the compiler under test.
- Keep the baseline command and candidate command equivalent. Changing only the
  `-o` path is fine. The gate rejects any other command difference, including a
  changed project include path or added compile flag, before running the
  measurement.
- The default self-compile workload uses the checked-in project headers under
  `benchmarks/self_compile/stable/include`; do not substitute live `dev/src`
  headers. Changing the frozen source or header closure starts a new benchmark
  epoch and requires a new baseline. Host standard-library headers are not
  vendored, so keep the host compiler configuration and standard library fixed.
- `benchmarks/self_compile/stable/PERF_EPOCH.json` pins epoch `9764b3835`, the
  source digest, exact 51-header membership, every header digest, and the
  aggregate closure digest. Both `record` and `check` verify it before invoking
  the compiler, so source drift and missing, added, changed, or symlinked headers
  cannot silently alter the gate. The pre-manifest baseline recorded at that
  exact epoch remains compatible; no parent remeasurement or rebaseline is
  required.
- The script uses `/usr/bin/time -lp`, including macOS hardware-counter fields.
  If those fields are missing, the check fails instead of silently passing.
- Store shared project baselines outside the worktree or in a deliberately
  tracked location; avoid committing local machine-specific baselines by
  accident.
