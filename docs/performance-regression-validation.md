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
  -- ./dev/cppgm++ -I dev/src -c -o /tmp/cppgm-baseline.o \
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

Candidate validation defaults to one run because repeated compile tests are
expensive. The script compares the candidate median against the saved baseline
median and fails if instruction count or memory rises past tolerance.

```sh
scripts/validate_perf_regression.py check \
  --baseline /tmp/cppgm-semantic-overload-baseline.json \
  -- ./dev/cppgm++ -I dev/src -c -o /tmp/cppgm-candidate.o \
     benchmarks/self_compile/stable/semantic_overload.cpp
```

Default tolerances are intentionally tight:

- `--instruction-tolerance 0.01`
- `--rss-tolerance 0.03`
- `--footprint-tolerance 0.03`

For a marginal failure, rerun the candidate with `--runs 2` or `--runs 3`.
Do not relax tolerances to keep a change unless the delta has been explained.

## Notes

- Run from the repository root after building the compiler under test.
- Keep the baseline command and candidate command equivalent. Changing only the
  `-o` path is fine.
- The script uses `/usr/bin/time -lp`, including macOS hardware-counter fields.
  If those fields are missing, the check fails instead of silently passing.
- Store shared project baselines outside the worktree or in a deliberately
  tracked location; avoid committing local machine-specific baselines by
  accident.
