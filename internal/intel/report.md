# Internal C++ source compilation

Fixed qualified cohort: **5,614 fixture groups / 5,724 translation units**, at `-O3`.

This measures object compilation of the common positive source subset, including driver startup. It does not replace the original assignment IR, ABI, diagnostic or runtime oracles. The fixed source selection is independent of timing; any failed compilation fails the run.

Measured compiler: `783dbfcf01360d923ff590d2048a0c1764761cc3`. Input snapshot: `783dbfcf01360d923ff590d2048a0c1764761cc3`.

CPU: Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz. Workers per compiler: 4.

| Compiler | Accepted objects | Compile CPU seconds | Speed vs cppgm ↑ |
|---|---:|---:|---:|
| cppgm | 5724 / 5724 | 50.269 | 1.00× |
| gcc | 5724 / 5724 | 224.273 | 4.46× |
| clang | 5724 / 5724 | 344.306 | 6.85× |

One complete observation per compiler; sums include compiler subprocesses. Linking, execution and harness time are excluded. This is a compile-throughput observation, with no application-runtime score or claim about small changes.

[Host and compiler identities](metadata.json) · [Measured results](results.json)

Historical local snapshot; [original report](https://github.com/vishvananda/cppgm-extended/blob/main/docs/performance/preview/internal.md).
