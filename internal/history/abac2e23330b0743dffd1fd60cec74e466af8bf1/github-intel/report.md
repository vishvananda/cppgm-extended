# Internal C++ source compilation

Fixed qualified cohort: **5,614 fixture groups / 5,724 translation units**, at `-O3`.

This measures object compilation of the common positive source subset, including driver startup. It does not replace the original assignment IR, ABI, diagnostic or runtime oracles. The fixed source selection is independent of timing; any failed compilation fails the run.

Measured compiler: `abac2e23330b0743dffd1fd60cec74e466af8bf1`. Input snapshot: `783dbfcf01360d923ff590d2048a0c1764761cc3`.

CPU: Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz. Workers per compiler: 4.

| Compiler | Accepted objects | Compile CPU seconds | Speed vs cppgm ↑ |
|---|---:|---:|---:|
| cppgm | 5724 / 5724 | 44.354 | 1.00× |
| gcc | 5724 / 5724 | 185.453 | 4.18× |
| clang | 5724 / 5724 | 250.282 | 5.64× |

One complete observation per compiler; sums include compiler subprocesses. Linking, execution and harness time are excluded. This is a compile-throughput observation, with no application-runtime score or claim about small changes.

[Host and compiler identities](metadata.json) · [Measured results](results.json)

[CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924706) · [Shared comparison records](records.json)

[cppgm per-unit records](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924706) · [gcc per-unit records](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924706) · [clang per-unit records](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924706)
