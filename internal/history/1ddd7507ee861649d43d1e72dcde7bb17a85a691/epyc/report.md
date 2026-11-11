# Internal C++ source compilation

Fixed qualified cohort: **5,614 fixture groups / 5,724 translation units**, at `-O3`.

This measures object compilation of the common positive source subset, including driver startup. It does not replace the original assignment IR, ABI, diagnostic or runtime oracles. The fixed source selection is independent of timing; any failed compilation fails the run.

Measured compiler: `1ddd7507ee861649d43d1e72dcde7bb17a85a691`. Input snapshot: `783dbfcf01360d923ff590d2048a0c1764761cc3`.

CPU: AMD EPYC 9V45 96-Core Processor. Workers per compiler: 4.

| Compiler | Accepted objects | Compile CPU seconds | Speed vs cppgm ↑ |
|---|---:|---:|---:|
| cppgm | 5724 / 5724 | 36.767 | 1.00× |
| gcc | 5724 / 5724 | 141.178 | 3.84× |
| clang | 5724 / 5724 | 184.441 | 5.02× |

One complete observation per compiler; sums include compiler subprocesses. Linking, execution and harness time are excluded. This is a compile-throughput observation, with no application-runtime score or claim about small changes.

[Host and compiler identities](metadata.json) · [Measured results](results.json)

[CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/42988175076) · [Shared comparison records](records.json)

[Raw per-unit observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42988175076)
