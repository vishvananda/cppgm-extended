# Internal C++ source compilation

Fixed qualified cohort: **5,614 fixture groups / 5,724 translation units**, at `-O3`.

This measures object compilation of the common positive source subset, including driver startup. It does not replace the original assignment IR, ABI, diagnostic or runtime oracles. The fixed source selection is independent of timing; any failed compilation fails the run.

Measured compiler: `8287f480c7b5bdb2074c8941689aa36f9c233bd7`. Input snapshot: `783dbfcf01360d923ff590d2048a0c1764761cc3`.

CPU: AMD EPYC 9V74 80-Core Processor. Workers per compiler: 4.

| Compiler | Accepted objects | Compile CPU seconds | Speed vs cppgm ↑ |
|---|---:|---:|---:|
| cppgm | 5724 / 5724 | 52.786 | 1.00× |
| gcc | 5724 / 5724 | 199.348 | 3.78× |
| clang | 5724 / 5724 | 276.704 | 5.24× |

One complete observation per compiler; sums include compiler subprocesses. Linking, execution and harness time are excluded. This is a compile-throughput observation, with no application-runtime score or claim about small changes.

[Host and compiler identities](metadata.json) · [Measured results](results.json)

[CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798447) · [Shared comparison records](records.json)

[cppgm per-unit records](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798447) · [gcc per-unit records](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798447) · [clang per-unit records](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798447)
