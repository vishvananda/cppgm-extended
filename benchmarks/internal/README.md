# Internal C++ compilation

This reference-only benchmark compiles the fixed, qualified positive subset of
the course tests: **5,614 fixture groups / 5,724 translation units**. It runs on
each commit with cppgm++, GCC and Clang at `-O3`; the optional dedicated Intel
lane uses the same producer and publisher.

The source snapshot is pinned to `783dbfcf01360d923ff590d2048a0c1764761cc3`.
`cohort.json` records every input checksum, language mode and flag. The runner
verifies the source archive, uses identical resource limits and host libstdc++
for all compilers, and requires a valid x86-64 ELF object for every unit.
Failures remain failures; they never shrink the timed cohort.

The badges report GCC or Clang total compilation CPU divided by cppgm++ CPU.
Compiler-driver startup and child processes count; linking, test execution
and harness time do not. Each compiler completes one whole cohort run, so
small changes need repeated measurements. Raw per-unit records, output hashes,
diagnostics, compiler identities and resource settings remain in the CI artifact.
Runtime is `n/a`: these short course fixtures are not an application-speed
benchmark. Their original IR, ABI, diagnostic and runtime contracts remain
separate from this source-only projection.

Run locally after building the compiler:

```sh
python3 scripts/internal_cpp_benchmarks.py run --output obj/internal --jobs 4
```

Published [host results](https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/internal/report.md)
retain each host's last measurement. The initial dedicated Intel entry preserves
the [historical qualified snapshot](../../docs/performance/preview/internal.md)
until a fresh run replaces it. The [measurement-age table](https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/freshness.md)
identifies old revisions, including that initial entry.
