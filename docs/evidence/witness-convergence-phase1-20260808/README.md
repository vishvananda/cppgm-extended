# Witness convergence Phase 1 evidence

This is the durable summary for Phase 1 of
`docs/witness-convergence-reset-and-recovery-plan.md`. Raw JSONL traces and the
full occurrence report are reproducible temporary artifacts; the analyzers,
tests, exact hashes, counts, and conclusions needed to recreate them are
checked in here so a reboot does not erase the result.

## Identity and commands

- Starting commit: `afcdb6ae29c402895cac2712bc763f2e964c83b3`
- Host compiler: `/usr/local/opt/llvm/bin/clang++`
- Ordinary object root: `../obj/witness-recovery-phase1-ordinary-20260808`
- Provenance object root: `../obj/witness-recovery-phase1-provenance-20260808`
- Strict: `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-strict-nobuild`
- Broad: `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 ORDERED=false make test-report-nobuild`
- Provenance analyzer: `scripts/analyze_witness_provenance.py`
- Occurrence analyzer: `scripts/analyze_witness_convergence.py`

The diagnostic corpus was also run one PA at a time into separate `pa19`,
`pa20`, `pa22`, `pa23`, and `pa24` trace directories. This makes provenance
correlation unambiguous for test basenames shared by multiple PAs.

## Exact artifact hashes

- 1,529-file relative trace manifest:
  `eabbb80e9c593917525cc3879f82ebbff8e6140e2d604cbdf8603b094387fc73`
- 63,229-record provenance report:
  `93719efd349ad1318e7f7c506472bfca4474b6a983d18bfd6a50ca523be4f37c`
- occurrence-level convergence report:
  `2592e39108c6b8f3e320966f59f8f5ada74a5f165f613581b667c36038f93f68`
- ordinary and diagnostic strict logs:
  `cd31be5615fe8965e2fa9e7a6d5b069e45ebf6b83fb62fe31452dad30280e11f`
- ordinary and diagnostic 3,060-output manifests:
  `3ff5c80c9a64f0b4bec8273ce678e0fd83c252e4c77aee66eafb7190bffb2d2b`
- PA1-PA38 broad report:
  `c483db8d53f385ee89a6b45f680d23581563758447b7095c9dc8ce8c4e02539d`
- three-run performance report:
  `4853c98df0047ea9c397151f5a4c15ff43ce90227e919e26eff1e3376cb2f5dc`

The only missing trace is
`pa23/tests/spec/300-nondeduced-partial-pattern-recursive-completion.t`; its
witness compilation exits before session finalization. It remains represented
in the strict mismatch report.

## Correctness result

Ordinary and diagnostic output are byte-for-byte identical. Both reproduce
the Phase 0 result: 1,339 of 1,530 expanded strict references pass, all 1,305
previously tracked references pass, and the same 191 outputs fail. The broad
PA1-PA38 direct-LowIR report passes 4,862 of 4,862.

The 191 failing outputs contain 565 occurrence-level differences:

| Family | Failing tests | Changed | Missing expected | Unexpected actual | Ordering only |
| --- | ---: | ---: | ---: | ---: | ---: |
| Alias | 24 | 18 | 16 | 2 | 0 |
| Class | 62 | 57 | 36 | 10 | 0 |
| Function | 86 | 40 | 62 | 35 | 1 |
| Variable | 3 | 1 | 0 | 2 | 0 |
| Lifecycle | 79 | 0 | 158 | 127 | 0 |

Lifecycle events have only a kind and entity in public witness output. An
event with a different entity is therefore a missing transition plus an extra
transition, not a synthetic changed pair.

Failing-test distribution by PA:

| Family | PA19 | PA20 | PA22 | PA23 | PA24 |
| --- | ---: | ---: | ---: | ---: | ---: |
| Alias | 0 | 0 | 5 | 14 | 5 |
| Class | 3 | 2 | 21 | 20 | 16 |
| Function | 1 | 4 | 14 | 50 | 17 |
| Variable | 0 | 0 | 0 | 0 | 3 |
| Lifecycle | 8 | 5 | 16 | 34 | 16 |

## Producer and route coverage

Every final producer is exercised and no source attempt has an unknown
producer or upstream route.

| Family | Attempts | Inserted | Exact duplicate | Table rows | Public rows |
| --- | ---: | ---: | ---: | ---: | ---: |
| Alias | 821 | 821 | 0 | 821 | 821 |
| Class | 2,573 | 2,572 | 1 | 2,572 | 2,572 |
| Function | 1,298 | 1,026 | 272 | 1,026 | 763 |
| Variable | 34 | 34 | 0 | 34 | 34 |
| Lifecycle | 6,030 | 6,030 | 0 | 6,030 | not a source-row public counter |

Typed upstream route counts:

- class: 1,620 resolved-template-id, 210 declaration-type, 33 explicit
  specialization, 18 qualified-value, and 692 nested-source attempts;
- function: 4 constant lookup, 19 conversion, 123 `declval`, and 1,152
  overload-resolution attempts;
- variable: 31 direct and 3 initializer-replay attempts;
- alias: all 821 publications use the canonical occurrence route.

The one class duplicate is a nested source-owned `root::lib::outer::result`
occurrence in
`pa22/tests/general/300-sibling-namespace-dependent-member-template-id-owner.t`.
It is a focused Phase 2 pruning target, not justification for general class
deduplication.

## Repeated semantic work

Alias completion receives 1,970 semantic candidates for 821 public rows:

- 821 first completions;
- 1,050 ignored repeats;
- 95 class-context upgrades;
- 4 current-specialization enrichments.

The ignored repeats are 625 parameterized resolutions, 408 concrete
resolutions, and 17 source-pattern resolutions. This is the primary Phase 3
algorithmic-convergence target; the table itself is already one-to-one and is
not hiding the repeated work.

Class consolidation records 2,919 completed candidates, 3,270 early repeats,
336 prepublication merges, 2,583 collected occurrences, and 2,573
publications. Fifteen of the 36 missing expected class occurrences correlate
with 49 rejected materialization decisions. All 49 have source-use mode 0,
typed owner `none`, and no typed materialization. Several occurrences are
reanalyzed many times (up to twelve decisions for one source ID), so Phase 2
must collapse repeated analysis while establishing the correct typed owner.
Another 21 missing class occurrences have neither a class source attempt nor
a materialization decision and require producer-boundary repair.

Function duplicates divide by semantic route:

- overload resolution: 975 insertions and 177 exact duplicates;
- `declval`: 32 insertions and 91 exact duplicates;
- conversion: 15 insertions and 4 exact duplicates;
- constant lookup: 4 insertions and no duplicates.

These duplicates and the 263 table rows later removed or combined by the
renderer are Phase 5 semantic-result convergence work. They are not an alias
or class visibility-policy obligation.

## Ordinary-build and performance invariants

All new route state and detailed records are diagnostic-only. No production
semantic structure gained a field. The warning-free ordinary binary contains
no `witness_provenance` symbol and retains the Phase 0 layout exactly:

- file size: 17,004,368 bytes;
- Mach-O `__TEXT`: 12,951,552 bytes;
- `__text`: 11,788,265 bytes;
- `__DATA_CONST`: 57,344 bytes;
- `__DATA`: 442,368 bytes.

The three-run median against the Phase 0 fixed artifact is:

| Metric | Phase 0 fixed | Phase 1 | Delta |
| --- | ---: | ---: | ---: |
| Instructions | 175,251,868,297 | 175,152,378,823 | -0.06% |
| Maximum RSS | 747,782,144 | 751,472,640 | +0.49% |
| Peak footprint | 575,848,448 | 575,700,992 | -0.03% |

All gates pass without an RSS confirmation run. The diagnostic implementation
adds 229 guarded source lines; Phase 6 owns their deletion after the semantic
paths converge.

## Ordered implementation consequences

1. Phase 2 starts with the 15 class occurrences that have rejected typed
   materialization evidence, then repairs the 21 occurrences with no semantic
   attempt. It must remove repeated source-ID analysis as owners migrate.
2. Phase 3 starts with the 16 missing aliases that have no completion attempt,
   then resolves 18 payload changes and two extra qualified member aliases.
   The 1,149 repeat/upgrade operations must fall as completion arms merge.
3. Phase 4 treats 158 missing and 127 extra lifecycle transitions as entity
   state ownership. It must not use lifecycle to admit class or alias rows.
4. Phase 5 makes overload resolution, conversion, `declval`, and constant
   lookup return one typed call result, then handles the three variable tests.
5. Renderer or table deduplication is removed only after the corresponding
   semantic repeat counts reach zero. Inception remains last.
