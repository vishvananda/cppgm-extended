# Internal C++ source compilation

Fixed qualified cohort: **5,614 fixture groups / 5,724 translation units**, at `-O3`.

This measures object compilation of the common positive source subset, including driver startup. It does not replace the original assignment IR, ABI, diagnostic or runtime oracles. The fixed source selection is independent of timing; any failed compilation fails the run.

Measured compiler: `04d2857867b5818d7af63b659039c796ca842ddc`. Input snapshot: `783dbfcf01360d923ff590d2048a0c1764761cc3`.

CPU: AMD EPYC 7763 64-Core Processor. Workers per compiler: 4.

| Compiler | Accepted objects | Compile CPU seconds | Speed vs cppgm ↑ |
|---|---:|---:|---:|
| cppgm | 5724 / 5724 | 52.336 | 1.00× |
| gcc | 5724 / 5724 | 203.816 | 3.89× |
| clang | 5724 / 5724 | 286.906 | 5.48× |

One complete observation per compiler; sums include compiler subprocesses. Linking, execution and harness time are excluded. This is a compile-throughput observation, with no application-runtime score or claim about small changes.

[Host and compiler identities](metadata.json) · [Measured results](results.json)

[CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039929) · [Shared comparison records](records.json)

[cppgm per-unit records](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039929) · [gcc per-unit records](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039929) · [clang per-unit records](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039929)
