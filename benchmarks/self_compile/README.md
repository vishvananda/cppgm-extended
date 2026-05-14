# Frozen Self-Compile Benchmark Corpus

This directory holds stable self-compile benchmark inputs used for
performance comparisons across compiler checkpoints.

Why these files exist:

- compiling `dev/src/*.cpp` directly is useful for real-world signal, but the
  source text changes as the compiler evolves
- that makes before/after timing comparisons noisy, because the benchmark
  input changes at the same time as the compiler
- these snapshots keep the benchmark source text fixed while still compiling
  against the current `dev/src` headers and implementation

Per-slice frozen corpus:

- `stable/constant_value.cpp`
- `stable/semantic_model.cpp`
- `stable/semantic_overload.cpp`
- `stable/recog_token_buffer.cpp`
- `stable/parser_trace.cpp`
- `stable/rtti_names.cpp`
- `stable/cppast_dump.cpp`

Checkpoint-only frozen corpus:

- `stable/callsemantic_frozen.cpp`

`callsemantic_frozen.cpp` is intentionally not part of the normal
`--include-self-compile` benchmark set. It is large enough that it should be
used as a checkpoint anchor at major semantic-analysis phase boundaries, not
as a per-slice benchmark.

Frozen LowIR corpus for optimizer-only benchmarking:

- `frozen_lowir/semantic_overload.o0.lowir`

Why the frozen LowIR corpus exists:

- optimizer hotspots can shift on larger source files
- freezing `--emit-lowir -O0` output removes front-end and codegen noise from
  optimizer-only measurements
- these inputs can be fed directly to `scripts/profile_frozen_optimizer_phase.py`
  with `--lowir ... --source ...`

Most files were frozen from:

- source paths under `dev/src/`
- repository `HEAD` at the time this corpus was created

`callsemantic_frozen.cpp` was frozen from `dev/src/callsemantic.cpp` at commit
`a850082570c40d1b34bbc98c3c0919baaa1ce7cd`, immediately before the
reachability probe implementation. That keeps the benchmark source fixed
without benchmarking the probe code itself.

Use `scripts/run_frozen_self_compile_benchmarks.sh` to run the corpus against
either the current compiler or a compiler binary from another worktree.

Example optimizer-only run on the larger frozen LowIR input:

```sh
scripts/profile_frozen_optimizer_phase.py \
  --repo-root . \
  --source benchmarks/self_compile/stable/semantic_overload.cpp \
  --lowir benchmarks/self_compile/frozen_lowir/semantic_overload.o0.lowir \
  --optimizer ./dev/lowiropt \
  --repeat 3 \
  --output-prefix /tmp/cppgm-optimizer-phase-semantic_overload
```
