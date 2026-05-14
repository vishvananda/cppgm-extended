# Demand-Driven Semantic Analysis Baseline

Phase 0 records stable inputs and baseline measurements before changing
semantic behavior.

## Frozen Inputs

- `benchmarks/self_compile/stable/semantic_overload.cpp`
  - copied from `dev/src/semantic_overload.cpp`
  - verified identical to local `main:dev/src/semantic_overload.cpp`
- `benchmarks/self_compile/stable/callsemantic_frozen.cpp`
  - copied from `dev/src/callsemantic.cpp`
  - source commit: `a850082570c40d1b34bbc98c3c0919baaa1ce7cd`
  - this is the commit immediately before the reachability probe, so the
    checkpoint compiles against current headers without benchmarking the probe
    code itself

## Semantic Overload Baseline

Command:

```sh
scripts/run_structured_ast_perf_benchmarks.py \
  --skip-build \
  --benchmark self-semantic-overload \
  --repeat 3 \
  --timeout-sec 900 \
  --counters \
  --env CPPGM_REACHABILITY_PROBE=1 \
  --env CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ \
  --output-prefix /tmp/cppgm-demand-phase0-semantic-overload
```

Compiler head: `eb14053607699c5dbf9a279d49947d7097399051`

Results:

- runs: `203.475s`, `203.139s`, `205.794s`
- median wall time: `203.475s`
- median max RSS: `2258200 KB`
- reachable classes: `5399 / 12704` (`42.498%`)
- reachable functions: `14951 / 38661` (`38.672%`)
- reachable template instantiations: `6747 / 14484` (`46.582%`)
- `class-info-for-type-calls`: `9179831`
- `class-info-for-type-pointer-cache-hits`: `7683082-7683084`
- `complete-class-type-calls`: `475171`
- `complete-class-type-no-class`: `356357`
- `resolve-template-argument-calls`: `522281`
- `class-template-key-builds`: `308467`

Reachability decision: this sits in the 20-50% middle band. The demand-DAG
should help, but node compaction and fixpoint flattening may also matter.

## Callsemantic Frozen Checkpoint

Command:

```sh
scripts/run_structured_ast_perf_benchmarks.py \
  --skip-build \
  --benchmark self-callsemantic-frozen \
  --repeat 1 \
  --timeout-sec 2400 \
  --counters \
  --env CPPGM_REACHABILITY_PROBE=1 \
  --env CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ \
  --output-prefix /tmp/cppgm-demand-phase0-callsemantic-frozen
```

Compiler head: `eb14053607699c5dbf9a279d49947d7097399051`

Results:

- wall time: `700.959s`
- max RSS: `4648544 KB`
- reachable classes: `10012 / 22755` (`43.999%`)
- reachable functions: `31771 / 69576` (`45.664%`)
- reachable template instantiations: `14452 / 28885` (`50.033%`)
- `class-info-for-type-calls`: `18756296`
- `class-info-for-type-pointer-cache-hits`: `16122416`
- `complete-class-type-calls`: `886270`
- `complete-class-type-no-class`: `641598`
- `resolve-template-argument-calls`: `1057099`
- `class-template-key-builds`: `616807`

Reachability decision: this is at the upper edge of the 20-50% middle band.
The checkpoint still shows large avoidable query volume, especially no-class
completion calls, but the probe suggests demand-driven output roots alone may
not be enough without flattening repeated semantic scans.

## Phase 1a Negative-Class Counter Check

Command:

```sh
scripts/run_structured_ast_perf_benchmarks.py \
  --skip-build \
  --benchmark self-semantic-overload \
  --repeat 1 \
  --timeout-sec 900 \
  --counters \
  --env CPPGM_REACHABILITY_PROBE=1 \
  --env CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ \
  --output-prefix /tmp/cppgm-demand-phase1a-semantic-overload
```

Compiler head before committing the change:
`13ae61cc06e354b773167676962a603db517564c` plus the Phase 1a working tree.

Results:

- wall time: `460.291s`
- max RSS: `2043004 KB`
- wall time is not comparable to Phase 0; the host was under unrelated load
- reachable classes: `5399 / 12704` (`42.498%`)
- reachable functions: `14952 / 38662` (`38.674%`)
- reachable template instantiations: `6747 / 14484` (`46.582%`)
- `class-info-for-type-definitely-not-class-skips`: `1317464`
- `class-info-for-type-calls`: `7862373`
- `class-info-for-type-pointer-cache-hits`: `7683079`
- `complete-class-type-definitely-not-class-skips`: `340692`
- `complete-class-type-calls`: `134482`
- `complete-class-type-no-class`: `15668`
- `complete-class-type-materializations`: `1438`
- `resolve-template-argument-calls`: `522281`
- `class-template-key-builds`: `308467`

Counter interpretation:

- Phase 0 had `class-info-for-type-calls=9179831`; Phase 1a moves `1317464`
  of those into an explicit non-class skip, leaving `7862373` counted calls.
- Phase 0 had `complete-class-type-calls=475171`; Phase 1a moves `340692`
  obvious non-class queries into the explicit skip counter, leaving `134482`
  counted calls.
- This is mostly a representation/accounting cleanup. The old implementation
  already returned quickly for non-`TK_NAMED` types, so this is not expected to
  produce meaningful wall-time improvement by itself.
