# Boost B2 Frontier V2 Tracker

This is the live tracker for `docs/boost-b2-frontier-v2-plan.md`. It starts from
zero credited Boost suites. V1 pass/fail state is historical only.

## Start State

- started: `2026-07-13`
- compiler baseline: `db9879223b4249a9e5516de0205e76fa158d5549`
- base branch: `main`
- frontier branch: `boost-frontier-v2`
- Boost tree: `/Users/vishvananda/boost_1_91_0`
- Boost release: `1.91.0`
- suite inventory: `docs/boost-b2-suite-status-20260511.md`
- suite count: `147`
- completed suites: `0 / 147`
- current cursor: `#1 libs/accumulators/test`
- active compiler frontier: unqualified `format_report` lookup while compiling
  Boost.Test `test_tools.o`

## Baseline Gates

| Gate | Status | Evidence |
|---|---|---|
| Exact compiler start commit | pass | `main`, `origin/main`, and the merged PR head were verified at `db9879223` before this branch was created. |
| PR Tests workflow | pass | GitHub Actions run `29298098410` completed successfully with all 21 checks green across GCC/libstdc++ and Clang/libc++ on Ubuntu 24.04 and 26.04. |
| PR-triggered inception | pass | Run `29299551532` completed `Compare cppgm++ inception` successfully in `56m53s`. |
| Warning-clean `dev/cppgm++` build | pass | `make -C dev -j8 cppgm++` relinked the compiler at `2acedcec7` without warnings; production compiler sources are unchanged from `db9879223`. |
| Full direct-LowIR report | pass | `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 ORDERED=false make test-report` passed `3818 / 3819`; the only miss was the known load-sensitive PA9 generated-program timeout. Its immediate isolated direct-LowIR rerun passed `11 / 11`, and the timeout was accepted as load noise. |
| Strict direct-LowIR report | pass | `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-strict` passed all configured PA18, PA19, PA21, PA22, and PA23 comparisons. |
| Strict text-reparse audit | pass | `audit_text_reparse.py --strict --list-sites` reported all 23 categories at zero; all 14 audit unit tests passed. |
| PA placement audit | pass | PR CI placement audit passed. |
| Fixed V2 performance baseline | pass | Three runs were recorded from the detached worktree `/tmp/cppgm-boost-v2-baseline-db9879223` at exact commit `db9879223`; medians are recorded below. |

## Environment

Fill this before the first suite run. Keep these values stable for comparable
suite evidence and performance measurements.

| Item | Value |
|---|---|
| host OS/version | macOS 26.3 (25D125), Darwin 25.3.0 x86_64 |
| CPU | Intel Core i7-9750H, 6 physical / 12 logical cores, 16 GiB memory |
| host C compiler | `/usr/local/opt/llvm/bin/clang`, Homebrew clang 22.1.0 |
| host C++ compiler | `/usr/local/opt/llvm/bin/clang++`, Homebrew clang 22.1.0 |
| C++ standard library | system libc++ (`/usr/lib/libc++.1.dylib`, current version 2000.67.0; headers `_LIBCPP_VERSION=220100`) |
| B2 wrapper | `/Users/vishvananda/boost_1_91_0/run-cppgm-b2.sh` |
| compiler under test | `/Users/vishvananda/cppgm-extended/dev/cppgm++` |
| default suite jobs | `8` |
| default suite timeout | `1800s`, adjusted upward for known large suites |

## Fixed Performance Baseline

| Commit | Baseline file | Instructions | Max RSS | Peak footprint | Wall time | Status |
|---|---|---:|---:|---:|---:|---|
| `db9879223` | `/tmp/cppgm-boost-frontier-v2-db9879223-baseline.json` | 271,651,249,439 | 1,293,901,824 B | 1,024,110,592 B | 60.43s | recorded, immutable |

## Performance Ledger

Every production compiler commit gets a row against the fixed baseline. Add a
rolling delta only when it helps isolate the incremental cost.

| Commit | Frontier | Fixed instruction delta | Max RSS delta | Footprint delta | Rolling delta | Report | Decision |
|---|---|---:|---:|---:|---:|---|---|
| `(qualified-friend fix)` | Qualified friend class template-id initial parse | +0.17% | +1.66% | +0.00% | n/a | `/tmp/cppgm-boost-frontier-v2-qualified-friend-perf.json` | pass; instruction and memory gates remain within tolerance |
| `(typed-qualified-call fix)` | Substituted qualified static member-template call | -0.08% | +1.15% | -0.01% | n/a | `/tmp/cppgm-boost-frontier-v2-typed-qualified-call-perf.json` | pass; below hotspot threshold |

## Suite Cursor

The inventory order comes from `docs/boost-b2-suite-status-20260511.md`. Add a
row when a suite is attempted. Do not prepopulate passes from V1.

| # | Suite | V2 status | Commit | Forced run evidence | Notes |
|---:|---|---|---|---|---|
| 1 | `libs/accumulators/test` | frontier | `(typed-qualified-call fix)` | Forced survey run, 8 jobs, 1800s timeout, completed `mixed` with rc `1` in 491.7s; initial log `/tmp/boost-frontier-v2-suite-001-db9879223/libs__accumulators__test.log`. Forced single-job `count` rebuild after the typed qualifier fix used a 900s timeout; log `/tmp/boost-v2-acc-count-typed-qualifier-fix.log`. | The qualified-friend parser and `mp_bool` alias-template frontiers are fixed. `count.o` compiles; the ordered dependency build now fails first on Boost.Test `format_report`, then also exposes the later Boost.Regex `perl_matcher<BidiIterator,Allocator,traits>` structured-anchor failure. |

Allowed statuses are `pending`, `running`, `frontier`, `blocked-external`, and
`pass`. A timeout is evidence, not a pass.

## Active Frontier

- suite: `#1 libs/accumulators/test`
- focused target: `libs/accumulators/test//count`
- failure phase: semantic unqualified function lookup in an instantiated body
- diagnostic: `ERROR: unknown function format_report` while completing
  `boost::test_tools::tt_detail::report_assertion` from
  `boost/test/impl/test_tools.ipp:389:5`
- reduced repro: pending
- owning PA/cluster: pending reduction
- implementation area: function declaration visibility and instantiated-body lookup
- performance risk: function lookup normal path; use hotspot counters and sampling if
  the production fix moves instructions by more than 0.25%
- next action: reduce the Boost.Test `format_report` failure and add the smallest
  non-STL regression at the owning PA

## Fix Ledger

Add one row per coherent compiler fix. Keep detailed logs in `/tmp`; keep the
stable command, diagnostic, reducer, validation, and measured deltas here.

| Status | Suite/target | Root cause and typed fix | Owner regression | Pre-fix evidence | Validation | Perf vs fixed baseline | Commit |
|---|---|---|---|---|---|---|---|
| fixed | `libs/accumulators/test//count` dependency build | The initial parser treated an unknown qualified template suffix before `;` as expression-ambiguous even after an elaborated class-key. `parse_class_specifier` now selects the existing typed qualified-name mode that permits the final template-id, preserving `QualifiedName`, `TemplateIdSyntax`, qualifier template syntax, and qualifier type syntax on the AST node. | `pa21/tests/general/300-qualified-friend-class-template-id.t`, placed at the `template.friend` owner `pa21:300` | Non-STL reducer and Boost `friend class detail::interface_iarchive<Archive>;` both failed during initial parse; parser trace rejected the class member at `KW_FRIEND`. Focused pre-fix log: `/tmp/boost-v2-acc-count-parser-frontier.log`. | Warning-clean build; reducer accepted by Clang and `cppgm++`; PA10/PA21 direct LowIR report `352/352`; PA21 strict `216/216`, compared `161`, failures `0`; placement audit marks all reducer features `ok`; all 23 text-reparse categories remain zero and 14 audit tests pass; forced Boost rerun clears the parser diagnostic and builds Serialization. | instructions +0.17%; max RSS +1.66%; peak footprint +0.00%; pass | `(this commit)` |
| fixed | `libs/accumulators/test//count` | Substitution had already attached the concrete `support::empty_list` type to `Next` through `qualifier_type_syntax`, but both qualified function lookup paths discarded that semantic type and tried to look up its fully qualified rendering as one name component. `lookup_function_templates_node` and `lookup_functions_node` now consume `cppast_qualifier_type_syntax(...)->semantic_type` first and retain the existing structured template-id/type lookup only as a fallback. This corrects the uncached algorithm; no cache, text parser, or spelling special case is added. | `pa21/tests/general/300-qualified-owner-static-template-nttp.t`, placed at the `template.member_template` owner `pa21:300` | Exact Boost-header reducer `/tmp/cppgm-boost-v2-parameter-arg-list.cpp` and non-STL reducer `/tmp/cppgm-boost-v2-qualified-dependent-static-template.cpp` failed resolving `mp_bool`/`bool_constant`. Semantic trace showed the substituted qualifier type was present while call lookup reported `template_count=0` only for the namespace-qualified concrete owner. | Warning-clean compiler build; exact Boost and non-STL reducers accepted by `cppgm++`, and the non-STL reducer accepted by Clang; focused PA21 check passes; direct PA21 report `217/217`; PA21 strict `217/217`, compared `161`, failures `0`; placement audit reports zero findings and marks all reducer features `ok`; all 23 text-reparse categories remain zero and 14 audit tests pass; forced single-job `count` rebuild compiles `count.o` and advances to Boost.Test `format_report`. | instructions -0.08%; max RSS +1.15%; peak footprint -0.01%; pass, below hotspot threshold | `(this commit)` |

## Decision Log

- `2026-07-13`: Started V2 at `db9879223` because removal of semantic text
  reparsing and the following template-resolution fixes may have changed suites
  previously marked passing.
- `2026-07-13`: Reset suite credit to zero. The V1 tracker remains historical
  evidence and is not a V2 cursor.
- `2026-07-13`: Chose `libs/accumulators/test`, inventory row 1, as the first
  suite.
- `2026-07-13`: Made the start-commit performance baseline immutable. Rolling
  baselines may characterize a fix but cannot replace the fixed cumulative
  gate.
- `2026-07-13`: Completed the baseline bootstrap. The fixed three-run baseline
  was measured in a detached worktree at exact commit `db9879223`; the active
  and main branch tips at `2acedcec7` contain only V2 process documents and
  refreshed assignment references beyond that production compiler baseline.
- `2026-07-13`: Accepted one PA9 generated-program timeout in the full report
  as the known load-sensitive test behavior after its isolated direct-LowIR
  report passed `11 / 11` immediately.
- `2026-07-13`: The first forced V2 Accumulators run completed `mixed` in
  491.7s. Classified its earliest causal failure as an initial-parser frontier
  in a Boost.Serialization dependency rather than the later parallel target
  diagnostics.
- `2026-07-13`: Reduced the first frontier to an initial-parser failure on
  `friend class detail::interface_archive<T>;`. The typed class-specifier parse
  now accepts the unambiguous qualified template-id without reparsing text. The
  regression lives at the `pa21:300` template-friend owner, not PA10, under the
  current placement rules.
- `2026-07-13`: The forced `count` rerun cleared the parser failure and advanced
  `count.o` to `mp_bool` alias-template resolution. The same parallel run also
  observed independent Boost.Test `format_report` lookup and Boost.Regex
  template-id lookup failures; these are later dependency frontiers, not the
  earliest causal failure for the focused target.
- `2026-07-13`: Reduced `mp_bool` to a non-STL qualified static member-template
  call whose substituted owner type lives in `qualifier_type_syntax`. Both call
  lookup paths now use that typed owner directly. The forced single-job rebuild
  compiled `count.o` and established Boost.Test `format_report` as the next
  ordered target frontier; Boost.Regex remains a separate later dependency
  frontier.

## Next Commands

```sh
cd /Users/vishvananda/boost_1_91_0
env CPPGM_BOOST_B2_FRONTIER=1 \
  CPPGM_B2_CXX=/Users/vishvananda/cppgm-extended/dev/cppgm++ \
  CPPGM_B2_HOST_CC=/usr/local/opt/llvm/bin/clang \
  CPPGM_B2_HOST_CXX=/usr/local/opt/llvm/bin/clang++ \
  JOBS=1 \
  /usr/local/bin/timeout 600 \
  ./run-cppgm-b2.sh libs/accumulators/test//count
```
