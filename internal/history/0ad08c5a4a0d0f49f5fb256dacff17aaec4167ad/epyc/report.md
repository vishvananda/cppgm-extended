# Internal C++ source compilation

Fixed qualified cohort: **5,614 fixture groups / 5,724 translation units**, at `-O3`.

This measures object compilation of the common positive source subset, including driver startup. It does not replace the original assignment IR, ABI, diagnostic or runtime oracles. The fixed source selection is independent of timing; any failed compilation fails the run.

Measured compiler: `0ad08c5a4a0d0f49f5fb256dacff17aaec4167ad`. Input snapshot: `783dbfcf01360d923ff590d2048a0c1764761cc3`.

CPU: AMD EPYC 9V74 80-Core Processor. Workers per compiler: 4.

| Compiler | Accepted objects | Compile CPU seconds | Speed vs cppgm ↑ |
|---|---:|---:|---:|
| cppgm | 5724 / 5724 | 40.883 | 1.00× |
| gcc | 5724 / 5724 | 156.836 | 3.84× |
| clang | 5724 / 5724 | 228.755 | 5.60× |

One complete observation per compiler; sums include compiler subprocesses. Linking, execution and harness time are excluded. This is a compile-throughput observation, with no application-runtime score or claim about small changes.

[Host and compiler identities](metadata.json) · [Measured results](results.json)

[CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/42770606270) · [Shared comparison records](records.json)

[cppgm per-unit records](https://github.com/vishvananda/cppgm-extended/actions/runs/42770606270) · [gcc per-unit records](https://github.com/vishvananda/cppgm-extended/actions/runs/42770606270) · [clang per-unit records](https://github.com/vishvananda/cppgm-extended/actions/runs/42770606270)
