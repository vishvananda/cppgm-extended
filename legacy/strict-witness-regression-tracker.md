# Strict Witness Regression Tracker

This tracker is for the strict owner-suite witness cleanup in the integration
worktree. Keep it updated before and after each behavior-changing witness or
semantic-source-use edit so newly introduced drift is visible instead of being
mixed into the older frontier.

## Validation Command

```sh
make test-strict-nobuild STRICT_PAS='pa18 pa19 pa21 pa22' STRICT_SUBTEST_JOBS=8 \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

## Baselines

- Previous focused baseline: 9 known witness failures:
  `pa19/spec/033`, `pa21/general/207`, `pa21/general/476`,
  `pa22/general/474`, `pa22/general/501`, `pa22/general/507`,
  `pa22/general/509`, `pa22/general/511`, and `pa22/general/512`.
- Current refreshed strict run, 2026-05-01 after typed source-use propagation,
  template-entity owner parsing, and duplicate constructor-call collapse:
  all strict witness suites pass.
  `pa18`: `111` compared, `0` failures, `3` skipped.
  `pa19`: `91` compared, `0` failures, `3` skipped.
  `pa21`: `103` compared, `0` failures, `2` skipped.
  `pa22`: `268` compared, `0` failures, `11` skipped.

## Previously Known Rows

These are the nine rows that were already part of the known witness frontier.
Keep them listed even when currently green; a later edit that makes one fail
again is a regression.

| Test | Previous baseline | Current status | Drift class | Notes / owner |
| --- | --- | --- | --- | --- |
| `pa19/tests/spec/033-late-explicit-specialization-type-argument-does-not-eagerly-instantiate.t` | Failing | Green in full strict | Defaulted class-template argument spelling | Keep tracked because it was the original late-specialization item. Current behavior matches the checked-in ref with `n::plus<>`. |
| `pa21/tests/general/207-late-explicit-specialization-type-argument-does-not-eagerly-instantiate.t` | Failing | Green in full strict | Defaulted class-template argument spelling | Keep tracked because it was the original late-specialization item. |
| `pa21/tests/general/476-current-specialization-display-name-member-alias.t` | Failing | Green in full strict | Current-specialization alias owner rendering | Keep tracked to catch regressions in `function<type-parameter...>::member_alias` owner selection. |
| `pa22/tests/general/474-defaulted-decltype-empty-pack-instantiation.t` | Failing | Green in full strict | Paused source-capture function-call witness | Keep tracked because it was the original `declval`/empty-pack source-capture item. |
| `pa22/tests/general/501-internal-remove-cvref-alias-sfinae.t` | Failing | Green in full strict | Current-specialization template-id reconstruction | Fixed by carrying typed source-use metadata through witness events and preserving source spelling for semantic pack aggregates. |
| `pa22/tests/general/507-defaulted-nontype-enable-if-constructible-ref.t` | Failing | Green in full strict | Current-specialization template-id reconstruction | Fixed by the same typed source-use and pack-aggregate handling as `501`. |
| `pa22/tests/general/509-defaulted-class-template-arg-deduction.t` | Failing | Green in full strict | Extra closure class-instantiation | Fixed by making template witness owner parsing angle-depth aware so `::` inside template arguments does not corrupt owner metadata. |
| `pa22/tests/general/511-index-sequence-alias-constructor-deduction.t` | Failing | Green in full strict | Extra constructor-call event | Fixed by collapsing duplicate same-location constructor-call events where the longer event only adds deduced trailing template bindings from an alternate declaration. |
| `pa22/tests/general/512-dependent-local-typedef-qualified-member-type-witness.t` | Failing | Green in full strict | Lost dependent member owner | Fixed by threading a typed preserve-qualified-member source-use fact into the rendered witness binding. |

## New Rows Since Baseline

These rows were green in the previous focused baseline and are now failing.
Fixes should remove these without re-opening the previously known rows.

| Test | Previous baseline | Current status | Drift class | Notes / owner |
| --- | --- | --- | --- | --- |
| `pa19/tests/spec/032-explicit-specialization-type-argument-does-not-eagerly-instantiate.t` | Green | Green in full strict | Defaulted class-template argument spelling | Fixed by preventing the generic defaulted class alias map from rewriting explicit-specialization spelling. |
| `pa21/tests/general/206-explicit-specialization-type-argument-does-not-eagerly-instantiate.t` | Green | Green in full strict | Defaulted class-template argument spelling | Fixed by preventing the generic defaulted class alias map from rewriting explicit-specialization spelling. |
| `pa22/tests/general/293-explicit-template-call-dependent-alias-conversion.t` | Green | Green in full strict | Extra dependent alias-use event | Fixed by carrying a source-dependent qualified-owner fact through qualified member alias instantiation. |
| `pa22/tests/general/294-explicit-template-call-dependent-alias-sfinae-overload.t` | Green | Green in full strict | Extra dependent alias-use event | Fixed by the dependent-qualified member alias source-use suppression. |
| `pa22/tests/general/366-function-template-nested-alias-explicit-call.t` | Green | Green in full strict | Extra dependent alias-use event | Fixed by the dependent-qualified member alias source-use suppression. |
| `pa22/tests/spec/037-explicit-template-call-dependent-alias-sfinae-overload.t` | Green | Green in full strict | Extra dependent alias-use event | Fixed by the dependent-qualified member alias source-use suppression. |
| `pa22/tests/spec/117-explicit-template-call-dependent-alias-conversion.t` | Green | Green in full strict | Extra dependent alias-use event | Fixed by the dependent-qualified member alias source-use suppression. |

## Working Rules

- Re-run this tracker command after each fix-sized edit and update every row
  whose status changes.
- New failures are regressions until proven to be corrected refs from the
  patched-Clang witness generator.
- Do not fix witness mismatches by editing tests or refs unless the clang-side
  extractor is proven wrong.
- Prefer typed semantic/source-use facts over renderer text parsing,
  filename/path tests, or source spelling suppressions.
- If a fix moves one row green and another row red, stop and classify that
  tradeoff before continuing.
