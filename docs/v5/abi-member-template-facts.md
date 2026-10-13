# Dependent member template arguments in PA9 facts

PA9 documents `member <type-ref> <yes|no> <identifier> [<argument-ref>...]`,
but the fact reader accepted exactly three operands. It now resolves trailing
typed template-argument references and passes them to the existing ABI graph
and encoder. The three-operand path retains its original construction and
does not allocate an argument list. No compiler ABI producer or encoder changes.

Four PA9 `500-*member-template*` fixtures cover a dependent member-template
call inside `decltype`, a written qualifier with type and value arguments,
forward references containing nested arguments, and rejection of a type fact
used where an argument fact is required. References come from the ordinary
runner. The unchanged compiler rejects the three valid new inputs; the patched
reader accepts them. GCC independently emits the call fixture's encoding
`DTclsrT_8selectedIiEfp_EE` inside an instantiated function's mangled name.

## Performance

Frozen GCC host builds compare against main `883778fd` on wnx0008099.
Only `abimangle` changes. The `cppgm++` binaries before and after are byte
identical, SHA-256
`ded1b6800ce1bdaeaad45cd28c2903d7c96de7cdc29261f6834869a6eaf525fc`;
PA35 compilation and generated code therefore use exactly the same executable.

The affected tool is measured separately on CPU 80 with grouped user-mode
instructions/cycles, with counters running 100%. Each frozen input has one
warmup per binary, three A/A calibration blocks and six alternating ABBA/BAAB
blocks. Every output hash is checked for equality. The member input repeats
64,000 legacy three-operand expressions; the mixed input repeats all 115
accepted original PA9 fixtures 512 times. These inputs exercise existing
behavior supported by both binaries; new syntax has no successful baseline.

| Input | Patched/baseline instructions | Cycles (95% interval) | Wall (95% interval) |
| --- | --- | --- | --- |
| Legacy member | 1.0001414 | 0.9207 (0.8246–1.0045) | 0.9510 (0.8684–1.0326) |
| Mixed PA9 | 1.0000033 | 1.0214 (0.9940–1.0502) | 1.0204 (0.9915–1.0482) |

The instruction increases are measurable but small: 0.0142% on the targeted
member input and 0.00034% on the mixed corpus. Cycle and wall intervals include
parity; this experiment establishes no speedup or measurable throughput loss.
Intervals use 5,000 paired-block bootstrap samples. Load on this shared host
increases timing variance; instruction counts are the more stable observation.

Frozen `abimangle` SHA-256:

- Before: `829adef73867bd09b77837507fda98fa94bfe1ead1c726f7447ffa11a233f3e6`
- After: `43213f9937c4edc2c4059b7a49af94e7e5e4bbd6c931503f87bb8c2131a340f6`

The measurement script, all 76 observations, raw counter files, input/output
hashes and GCC cross-check source are retained in `obj/abi-member-validation/`.
Frozen binaries and full inputs are under `/tmp/abi-member-proof/`.

## Validation

PA9 passes 123/123, including all four new cases. The full strict course report
passes; all 312 harness tests, debug-info lanes, architecture,
placement and formatting audits pass. The file audit retains its four existing
header-body warnings. Self-built tools pass PA1–PA5 (58, 26, 20, 106 and
196 cases respectively). Check logs are under `/tmp/abi-member-*.log`.
