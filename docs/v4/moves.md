# Phase 4: the course and regression lanes moved into the assignment suites

Applied on 2026-09-06 on branch `v4`, on top of the Phase 3 regeneration.

## What moved

5,282 tracked files left `cppgm.tests/course/paN/` and
`cppgm.tests/regression/paN/` by `git mv`; the `paN/course` symlinks and
the two shared directories are gone.  Destinations, by lane:

| destination | files | destination | files |
| --- | --- | --- | --- |
| pa1/tests | 70 | pa22/tests | 106 |
| pa2/tests | 39 | pa23/tests | 50 |
| pa3/tests | 38 | pa25/tests | 73 |
| pa4/tests | 142 | pa26/tests | 30 |
| pa5/tests | 29 | pa27/tests | 3 |
| pa6/tests | 64 | pa28/tests | 11 |
| pa7/tests | 71 | pa29/tests (strict, structural, behavior) | 591 |
| pa8/tests | 84 | pa29/tests/controls | 19 |
| pa9/tests | 61 | pa29/tests/regression | 707 |
| pa10/tests | 32 | pa30/tests | 66 |
| pa11/tests | 31 | pa31/tests | 73 |
| pa12/tests | 63 | pa32/tests | 105 |
| pa13/tests | 65 | pa32/tests/controls | 3 |
| pa14/tests | 16 | pa33/tests | 15 |
| pa15/tests | 44 | pa34/tests (preproc, compile, run) | 30 |
| pa15/tests/controls | 5 | pa35/tests/compile | 200 |
| pa16/tests | 189 | pa36/tests/link | 5 |
| pa16/tests/controls | 4 | pa37/tests (o0 to o3, driver, object-roundtrip, debuginfo) | 503 |
| pa17/tests | 63 | pa37/tests/regression | 768 |
| pa17/tests/controls | 10 | pa38/tests (o1 to o3, behavior, driver, debuginfo) | 227 |
| pa18/tests | 25 | pa38/tests/regression | 367 |
| pa19/tests | 81 | pa20/tests | 36 |
| pa21/tests | 68 | | |

A course fixture went to the bucket of the same name under `paN/tests/`
(flat suites stay flat).  Controls went to `paN/tests/controls/`; the
regression roots of PA29, PA37 and PA38 went to `paN/tests/regression/`
with their bucket structure intact, and the PA37 and PA38 regression
controls to `paN/tests/regression/controls/`.

Two PA1 fixtures collided with local fixtures of the same stem and were
renamed to say what distinguishes them: `100-raw-string-literal` became
`100-raw-string-literal-nested-delimiter`, and `200-header-name` became
`200-header-name-forms`.

`cppgm.tests/` keeps only `undefined/` (inputs the course leaves
unspecified; no lane runs them) and a README that says so.  The regression
lane's policy moved from `cppgm.tests/regression/README.md` into
`docs/student-export-root/TESTING_AND_REFERENCES.md`.

## Harness and Makefiles

- `scripts/run_all_tests_common.pl` and `scripts/CppgmBatchWorker.pm` prune
  `controls` and `regression` subdirectories from a sweep, so `make test`
  over `tests` reaches a control or a regression fixture only through the
  lane that knows how to judge it.
- Every assignment Makefile lost its `COURSE_*` roots and the
  `ifeq ($(COURSE_TEST_ROOT),)` fork: the suite is the one `tests` tree.
  PA15, PA16, PA17 and PA32 read their controls from `tests/controls`;
  PA29 from `tests/controls` and its regression oracles from
  `tests/regression/{strict,structural,behavior}`; PA37 and PA38 from
  `tests/regression/...` with the controls under
  `tests/regression/controls`.  PA31 re-runs the moved
  `330-host-eh-nested-catch-forward-at-o1` fixture at `-O1` after the
  default sweep, as the course lane did.  PA38's driver census root is
  `tests/driver`.
- `make ref-test` now regenerates the PA37 and PA38 regression roots too
  (PA29 already did), so a maintainer's regeneration leaves a clean tree.
- The `INCEPTION_COURSE_STAGE_paN` variables of PA39 had no reader and are
  gone.

## References

`make ref-test` after the move regenerated every reference.  Differences
against the moved course references are of three kinds: references that
embed the fixture path (`start translation unit tests/...`), the expected
byte-level drift of outputs the relaxed comparison already absorbed (the
course references were produced by an older build), and the
`.ref.stdout` of negative fixtures, which the harness rewrites from the
current diagnostics.  Success-case `.ref.stdout` files the course lane did
not track (73 of them) are tracked now, as the local suites always were.

## Results

- `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-report` (the CI setting) first
  passed 5,930 of 5,933.  The three failures were the special member order
  violations already recorded in `ref-deltas.md`
  (`pa22/tests/general/300-member-template-assignment-not-special-member`,
  `pa28/tests/general/100-diamond-virtual-destructor-slot-merge`,
  `pa28/tests/general/100-multibase-implicit-virtual-destructor-slot-merge`):
  the compiler emitted the move assignment before the copy assignment when
  the move was demanded first, and the complete-entry vtable thunk before
  the deleting-entry thunk, against the order `pa13/lowir.md` states.  A
  presentation pass in the lowering driver now orders each special member
  family (source commit "Open a constructor loop's cleanup segment before
  the loop"); the three references were regenerated and the report passes
  5,934 of 5,934.
- The PA39 selfhost lane failed on
  `dev/src/native/allocation/location_planning.cpp`: an array of more than
  eight objects with destructors, declared while another such object was
  alive, made the emitted construction loop open the enclosing cleanup
  region once per element and close it once.  The same source commit opens
  the segment before the loop; `pa16/tests/general/300-constructor-array-loop-enclosing-cleanup`
  is the new fixture, and no other reference changed.
- The PA38 regression control
  `tests/regression/controls/459-o3-parameter-address-rematerialization`
  reported that native `-O3` did not reduce call-preserved address
  pressure.  It last passed at `f59dcabf` and failed from `7b97d1e9` ("Keep
  a replayed index base live until its last replay"), which deliberately
  keeps only the base of a replayed index address across a call from `-O1`
  on, so the O2 baseline already has the pressure the control expected O3
  to remove.  The control now pins that decision (only the base is
  preserved at either level, no derived address is rebuilt before the
  call, O3 adds no homes or frame) and the PA38 handout paragraph says the
  same.
- `make -C pa16 test-seams` passes (18 rewrites classified).

## Phase 5: placement and numbering

The placement auditor (`scripts/audit_pa_feature_placement.py`) is now the
only judge of where a fixture sits, and it judges the whole tree:

- The course lane is gone from it (`--include-course`/`--no-course`
  removed); every `paN/tests/**/*.t` outside `regression/` and
  `controls/` is a placed fixture.  The regression lane pins the course
  solution's own outputs and the controls are judged by their checkers,
  so neither is a placed suite.
- PA1 to PA9 must carry a three-digit prefix; from PA10 on the prefix is a
  cluster, a multiple of one hundred (the rule that already existed).
- A numbered companion unit (`x.t.1`) of a host-interop lane (PA31 to
  PA34, PA36) is compiled by the host compiler, so its hosted includes are
  not early.

After the move the auditor reported 740 findings (93 fixtures using a
feature before its owning assignment or cluster, 647 hygiene findings).
`scripts/v4_renumber.py` (a scratch tool, not kept) turned the findings
into 620 renames, applied with `git mv` together with every sidecar:

| reason | fixtures |
| --- | ---: |
| individual number to its cluster (`320-x` to `300-x`) | 253 |
| flat course fixture into the suite's `general/` bucket | 297 (overlapping the rows above and below) |
| unprefixed LowIR-input fixture, cluster by the words its numbered siblings use | 120 |
| unprefixed source fixture, cluster of the latest feature its assignment owns | 43 |
| same assignment, later owning cluster | 29 |
| feature owned by a later assignment: moved there | 24 |
| PA25 fixture whose reference carries unwind lowering: moved to PA26 | 1 |

Two PA17 fixtures used `__attribute__((noinline))` only decoratively; the
attribute went instead of the fixtures.  The ledgers
(`doc/lowir-contract-ledger.tsv`, `doc/compiler-refactor-output-cases.tsv`),
the survivor-property checkers, the PA29 and PA31 Makefiles, the PA13
handout and the backend review records name the fixtures by their new
paths.  `make ref-test` and `make ref-test-debuginfo` regenerated the
references that embed a path.

After the renames and three further moves the auditor found nothing
(`--fail-on-early` exits 0), and the byte-exact `make test-report` passes
with the regenerated references.

## The selfhost lane

`make -C pa39 test-through-pa10 CXX=../dev/cppgm++` (a CI gate) failed at
PA9: every test of the self-compiled `cy86` assembler died with "invalid
ELF header size".  Two compiler defects, both older than the move, both
fixed in the source tree and synced here:

- The optimizer forwarded a load through a phi of addresses to the value a
  retyping store had stored (`store i64 %n` with `%n` a pointer
  difference, as `vector::size()` inlined into `_M_check_len` leaves it),
  so its own -O1 output carried an `i64` phi with a `ptr` incoming and the
  LowIR reader rejected it.  `pa37/tests/regression/o1/549-retyping-store-through-address-phi`
  and `pa38/tests/behavior/o1/500-retyping-store-through-address-phi` hold
  the shape.  `make -C pa38 ref-test` now regenerates the behaviour buckets
  it had skipped, and the behaviour lane's `.ref.mir`/`.ref.cmir` dumps are
  ignored like its `.ref.program`.
- The native backend folded a constant index into a local's frame operand
  even when the result reached one past the local's last byte, abstract
  offset zero, which the encoder reads as the caller's frame and does not
  move past the saved registers: the `last` pointer of a local array passed
  to a range insert arrived 8 bytes too high whenever the function preserved
  a register.  `pa29/tests/behavior/200-one-past-local-array-call-argument`
  holds the shape (it fails before the fix at `-O0`).

To reach the defects, `cppgm++ --emit-*` now accepts `-I`, `-D`, `-U` and
`--hosted`, so one translation unit of the compiler itself can be emitted
as LowIR and optimized or lowered on its own; the reader's phi mismatch
error names the value, both types and the function.

One frontend defect the lane exposed is recorded rather than fixed: our
compiler cannot resolve `std::pair::swap` for a pair whose first member is
itself a pair (`std::sort` over
`std::vector<std::pair<std::pair<int, std::size_t>, std::size_t>>` fails
with "no viable overload for swap"; the inner pair's specialization
receives two class-template identities).  The ordering pass was rewritten
around it; the eight-line reducer is
`docs/v4/reducers/nested-pair-sort.cpp`, to become a PA35 compile fixture
once the frontend resolves it.

## Phase 7: the export

`scripts/export_student_repo.sh` copies the student support files at their
paths in this tree (`dev/src/abi/itanium/abi_mangle*.h`,
`dev/src/preprocess/tokens/*PPTokenStream.h`, `dev/src/support/not_implemented.h`,
`dev/src/support/tool_help_text.h`, `dev/src/support/testing/test_runner.cpp`,
`dev/src/ir_symbol_model.h`), writes the student `frontend_source_sets.mk`
with the test runner's source id, ships the two seams scripts PA16's lane
runs, and no longer validates a witness lane.  The scaffolds include those
paths.  A local run exported 18,651 verified reference files (7,059 exit
statuses, 411 retained failed-case diagnostics), packaged the reference
bundle, and the exported `dev/` builds its scaffolds.  CI gained an
`audits` job (the architecture audits, the file audit, `make test-harness`).

## Phase 7: the host cells

The first CI runs of `v4` were green only on the cell the local machine
matches (Ubuntu 26.04, gcc 15, libstdc++ 15).  The other three cells build
the compiler with a different host compiler and compile the hosted lanes
(PA35, PA36) against a different standard library, and each exposed defects
the local run could not.  Every fix landed in `~/work/v3codex` first and was
synced here; every language defect has a fixture in its owning assignment.

| cell | symptom | cause | fix | fixture |
| --- | --- | --- | --- | --- |
| all but 26.04 gcc | garbage field offsets, `invalid PA11 type identity`, a selfhost segfault | three references held across table growth (`PublishUsingAccess`, `TryAnalyzeFloatingIntrinsicCall`, `PublishFunctionTemplateSpecialMemberRole`) | copy the record, or evaluate before taking the reference | existing PA16 and PA30 fixtures; the selfhost lane |
| 24.04 gcc (libstdc++ 13) | `expression kind 31 does not designate scalar storage` in `_Hashtable::_M_insert_unique_node` | `const T& r = call();` never materialized storage for a scalar prvalue | the reference-initialization path materializes any non-class prvalue | `pa17/tests/general/200-scalar-prvalue-reference-binding.t` |
| 24.04 gcc | `ambiguous overload` in `_Rb_tree::_M_erase` | `f(B*)` and `f(const B*)` ranked equal for a `D*` argument | derived-to-base conversions of equal depth prefer the less qualified target | `pa12/tests/general/200-derived-to-base-pointer-prefers-less-qualified-overload.t` |
| 24.04 gcc | `no viable overload for _S_right` | a pointer to a class template specialization the program had only named was never completed for the derived-to-base check | call conversions complete the pointee on demand | covered by the PA35 map and set fixtures on that cell |
| 24.04 clang (libc++ 18) | `expected binary operand` at `pair.h:125` (86 fixtures) | `name<>()` read as less-than when another class had declared a non-template member of that name | an empty angle pair after a name is a template-id | `pa22/tests/general/300-member-template-empty-argument-list-after-same-named-member.t` |
| 24.04 clang | `duplicate default template argument` on `__invoke` | two function templates differing only in their decltype result matched as one declaration | the whole decltype result takes part in redeclaration identity; roots still decide leading-versus-trailing spellings | `pa23/tests/general/300-decltype-result-distinguishes-function-template-overloads.t`, `300-friend-function-template-alias-result-definition.t` |
| 24.04 clang | `expected OP_RPAREN` at `__math/traits.h:50` | `(typename T::type)x` not parsed as a cast | `typename` starts a cast type-id | `pa22/tests/general/300-dependent-typename-cast-expression.t` |
| 24.04 clang | `invalid universal character name` in `<sstream>` | `\u{` inside a comment | a backslash without a hex quad stays a backslash | `pa1/tests/200-malformed-universal-name-in-comment.t` |
| 24.04 clang | `expected parameter declaration` at `vector:2601` | `vector(*this, …).swap(*this);` read as a declaration in a template member | `T(*this, …)` and `T(this, …)` are expressions | `pa17/tests/general/100-injected-class-name-functional-cast-this-statement.t` |
| 24.04 clang | `structured template type was not found`, `invalid signedness transform operand`, `unknown expression name` while registering `__allocate_at_least` | a shape-only completion (parameters standing in for arguments) treated members the stand-ins cannot reach as errors | such members stay dependent shapes or keep no constant until a concrete specialization is completed | the libc++ cell (no reduction reproduces it outside the header chain) |
| 24.04 clang | `unknown expression name: __libcpp_compute_min<type,digits,is_signed>::value` | a non-type argument spelled as a bare name rejected because a namespace-scope class template shared the name of the class's own static constant | a value in a nearer scope hides the type | `pa22/tests/general/100-nontype-argument-member-hides-namespace-template.t` |
| 24.04 clang | `direct base must name a complete non-union class` in `constexpr_c_functions.h` | libc++'s `__libcpp_datasizeof` takes its `template <> struct _FirstPaddingByte<true>` branch when `__has_cpp_attribute(no_unique_address)` reads as absent | the probe answers true for the attribute the compiler implements | `pa5/tests/500-attribute-probe.t` |
| 24.04 clang (PA36) | `multiple definition of __do_deallocate_handle_size` | a function template specialization whose only arguments sit in an empty pack was emitted as a plain strong function with a non-template mangling and without its abi tag | the specialization keeps its template identity (`IJEE`, weak) and the pattern's abi tags | `pa32/tests/general/100-empty-pack-function-template-duplicate.t` |
| every cell (latent) | member function template symbols one substitution short of the host's (`S2_` where clang writes `S3_`) | the `<template-prefix>` of a member template never took a substitution number | the encoder numbers it before the template arguments | `pa32/tests/general/100-member-template-prefix-substitution.t`; every LowIR reference naming such a symbol changed |
| 24.04 clang | `ambiguous overload` in `__tuple_leaf`'s reference-binding assertion (`tuple:346`, six fixtures) | retained call facts are replayed for every specialization of a class template, and an unqualified call recorded no naming class to check the facts against, so a sibling specialization's member stayed a candidate | the enclosing class stands in for the naming class | `pa24/tests/general/200-member-template-call-per-specialization.t` |
| 24.04 clang | `base polymorphism facts are incomplete` in `std::function` | a lambda closure made an empty base of libc++'s compressed pair never had its facts computed | a complete base's facts are computed on demand; the diagnostic names both classes | `700-hosted-function-nullary-base-reentry-compile` on that cell |
| 24.04 gcc | `no viable overload for _S_right` in `_Rb_tree::_M_erase` | a derived-to-base pointer conversion on a class template specialization the program had only named | call conversions complete the pointee on demand | the PA35 map fixtures on that cell |

With these fixes the Ubuntu 24.04 clang cell's test-report is green, and
all four build cells, both audit jobs, every test-debuginfo job and three of
four test-report jobs pass.  Four CI checks remain red, root-caused to two
classes:

**Retained dependent-qualifier resolution (test-report, 24.04 gcc).**
`700-libstdcxx-regex-compiler-member-alias-call` fails because
`__copy_move_backward<_IsMove, true, rait>::__copy_move_b` calls
`std::__copy_move<_IsMove, false, rait>::__assign_one`, a qualifier
dependent on the enclosing template's `_IsMove`.  `retained_call_template_sets_`
caches the member-template pattern (and `retained_call_naming_classes_` the
naming class) from whichever specialization is instantiated first
(`__copy_move<false,false>`, entity 604).  A later instantiation
(`__copy_move<true,false>`, 605) replays the same callee node; the
naming-class guard in `RetainedFunctionCallCandidates` compares the pattern
owner (604) against the *recorded* naming class (604) and so accepts it,
and the qualified-call rebuild in `CompleteFunctionCallTemplateCandidates`
does not fire because it looks up `active_name[0]` ("std") in the active
class's member scope and finds nothing.  A fix must re-resolve the
dependent qualifier in the active specialization's scope rather than reuse
the recorded set; it touches the same retained-replay logic that a broad
first attempt regressed by ~350 tests, so it needs its own validated pass.

**Host-config self-host codegen (test-through-pa10, 24.04 gcc + clang, 26.04
clang).**  A `cppgm++` built by gcc 13 or clang miscompiles the recognizer
so its grammar-terminal map drops `KW_TRUE`, and PA6's empty test fails at
grammar load.  Reproduced with a fresh gcc-13-configured self-build; the
obvious suspects each compile correctly in isolation (`SimpleTokenKindName`'s
122-entry static `const char*` table, the enum-derived `kSimpleTokenCount`,
and the `unordered_map` fill), so it is a subtler uninitialized-read or
codegen bug in `cppgm++` itself, in the same class as the three
table-growth use-after-frees already fixed, not yet isolated to a function.
The 24.04 clang lane also hits `unknown type name: __alloc` in
`basic_string::__assign_with_sentinel`, a declaration-vs-expression
ambiguity where a value-name parameter (`u` in `holder temp(tag(), u,
__alloc())`) is wrongly accepted as a parameter type; the narrow guard for
it must not disturb genuine parameter declarations (a first attempt did).

Conversion function templates also still mangle their conversion type from
the deduced argument (`cvi` where the host writes `cvT_`).

### Update: two distinct gcc self-host issues

A first investigation, conducted with the local host `g++` (which is g++-15,
not g++-13 -- passing `CPPGM_HOST_CXX=g++-13` sets only the *embedded* config,
not the build compiler), found that g++-15 at `-O3` miscompiles the parser
through a strict-aliasing assumption when it later parses int128-configured
headers.  `-fno-strict-aliasing` on the tool build fixes that and is retained
(`dev/Makefile`, commit 63c4f466); it is byte-exact-neutral.  It does **not**
fix the CI 24.04 gcc cell, which builds with the real g++-13.

Building with the real g++-13.4 (in the Ubuntu 24.04 container, and locally
with `CXX=g++-13`) reproduces the actual 24.04 failure: the compiler builds
cleanly but the `recog` it produces cannot map the grammar terminal
`KW_TRUE`, so pa6 reports every test failing with byte-identical-looking
`BAD` output.  It is deterministic (`recog` fails 48/48; `-j1` and `-j4`
alike), and the compiler itself is deterministic across repeated runs, so it
is not the flaky race and not host non-determinism.  Isolated reductions of
the token table (the static `const char*` array, the enum-derived count, and
the `unordered_map` fill) all compile correctly under the g++-13-built
compiler, so it is a heap-state-dependent codegen defect in cppgm++ specific
to the g++-13 host configuration, exercised only by the full recognizer and
not yet isolated to a construct.  It is the real remaining 24.04 gcc
self-host blocker.

Two items remain on the self-host lanes after that fix:

- **`__alloc` declaration-versus-expression ambiguity (clang cells) — fixed.**
  libc++'s `basic_string::__assign_with_sentinel` writes
  `const basic_string __temp(__init_with_sentinel_tag(), std::move(__first),
  std::move(__last), __alloc());`.  The multi-argument direct-initializer
  disambiguation already reads `std::move(a)` as a call, but treated the
  last argument `__alloc()` -- a `name()` with an empty parameter clause --
  as a value-initialized temporary of type `__alloc`, reporting
  `unknown type name: __alloc`.  A `name()` whose name is callable and not a
  type is a nullary call, so the statement is an initialization; recognized
  now in `AnalyzeAmbiguousMultiDirectInitializer` (both passes) via
  `NamesCallableNonType`.  Fixture: `pa17/tests/general/200-local-class-
  direct-init-nullary-member-call.t`.  This unblocked the clang and libc++
  self-host lanes at `post_tokenizer.cpp`.

- **libc++ `basic_streambuf::seekpos` vtable reference (clang cells, PA10).
  RESOLVED** (commit "Name undefined vtable relocations by their object
  symbol").  With `__alloc` fixed the clang/libc++ self-host reaches PA10
  (compiling `cppgm++` with itself), which failed at link: `driver.o` and
  `lowering/core/driver.o` held a `.data` vtable slot with an undefined
  reference to `std::__1::basic_streambuf<char>::seekpos`, a virtual the
  driver's stream type inherits from libc++ but that no object defines.
  Reduced to eight lines (`docs/v4/reducers/streambuf-vtable-seekpos.cpp`).
  The earlier "`MangleFunction` returns empty" diagnosis was wrong:
  `MangleFunction` returns the correct Itanium mangling
  `_ZNSt3__1...7seekpos...`, and the vtable slot's symbol carries it as its
  object name.  The real cause was in the LowIR adapter
  (`dev/src/lowir/io/frontend_adapter.cpp`): it built the program's
  `symbol_names` from the internal presentation names only, dropping the
  mangled object name for a symbol that has no local definition and no
  declaration (an inherited virtual reached only through a vtable slot is
  never called, never defined, so nothing puts it in `function_declarations`).
  Native object emission then had no object symbol for the fixup and spelled
  the undefined relocation with the internal name.  Fix: carry each external
  reference's mangled object name in a parallel `symbol_object_names` channel
  (populated only for symbols with no local definition, retained in the
  object-only pruning pass) and, in `elf_format.cpp`, name an undefined fixup
  by that object symbol when it is neither locally defined nor declared.  The
  channel is consumed only by native object emission, so LowIR text is
  unchanged (byte-exact report identical); under libstdc++ `seekpos` is
  inline and stays a weak definition, so the branch never fires there.
  Validated: the reduction compiles, links against libc++, and runs; the
  24.04 clang self-host now links the PA10 stage and passes pa1-pa9.

- **AST-writer self-host miscompile (clang cells, PA10, exposed by the
  seekpos fix).  RESOLVED** (commit "Reach a reference or pointer parameter's
  virtual bases through the vtable").  Root cause: cppgm++ carried a hidden
  companion pointer ("__pvbptr") for a virtual base of every parameter,
  including references and pointers.  How many it carried was derived from the
  parameter's virtual-base count, which requires the complete class type; a
  caller seeing only a forward declaration (a unit passing `std::ostream&`
  through) computed zero while the definition, compiled with the complete
  type, expected the carry-all set, and the two disagreed across the
  translation-unit boundary, so the callee read a companion pointer the caller
  never passed.  Fix: restrict the companion pointer to by-value class
  parameters (whose complete type both sides must see) and reach a reference or
  pointer parameter's virtual bases through the object's vtable at each use
  (`VirtualBoundaryEntity` returns no entity for reference/pointer types, so the
  access site falls through to the existing `RuntimeVirtualBaseAddress` /
  vtable path).  The clang self-host now passes PA1-PA10 (158/158), `make
  inception` still matches byte for byte, and the byte-exact report is green
  (5951/5951) after regenerating the 18 PA28 references that carried the old
  companion arguments; the PA28 handout documents the two shapes.  Original
  investigation notes retained below.  The crash is a SIGSEGV in libc++'s `__pad_and_output` reached from
  `SyntaxArena::Write`'s `output << strings_.Get(node.tag)`; `--emit-types`,
  `--emit-semantics` and `--emit-lowir` all work, so it is specific to the
  AST text dump.  It is not the seekpos fix: the self-compiled `arena.o` is
  byte-identical whether compiled by the fixed or unfixed compiler.  It is
  not `arena.o` or the front end: swapping clang-compiled `arena.o`,
  `frontend_intern.o`, or the whole preprocess+syntax+support group into the
  self link does not fix it.  It is a weak libc++ template instantiation:
  `__pad_and_output` is emitted weak by many objects, ld takes the first
  (`cppgm++-runner.o`), and that instantiation is miscompiled; forcing a
  clang-built `operator<<(ostream, string)` object to win the weak
  resolution makes the crash vanish and the AST dump correct.
  Runtime tracing of the crashing call shows the miscompiled function is one
  frame up: `__put_character_sequence` forwards the string bytes correctly
  (`__ob`/`__ep` point at "translation-unit") but passes a garbage ostream
  (`__iob` is a code address, `__s` is garbage), so `__pad_and_output`
  dereferences a bad streambuf.  The backtrace is itself corrupt (a
  constructor "calls" `WriteTranslationUnit`), i.e. the call chain's stack is
  smashed.

  Deeper root cause: cppgm++ accesses a virtual base's members through a
  companion `__pvbptr` pointer parameter it threads alongside every
  `basic_ostream&`/`basic_ostream*`, rather than reading the virtual-base
  offset from the object's vtable the way clang does
  (`mov rax,[obj]; mov rax,[rax-0x18]; obj+rax+field`).  A five-line
  reproducer confirms the ABI: `long get_width(basic_ostream& os){return
  os.width_;}` where `basic_ostream : virtual basic_ios : ios_base` lowers to
  `@get_width(%os, %__pvbptr0)` and reads `%__pvbptr0 + 8`; clang emits the
  vtable vbase-offset load.  Machinery: `dev/src/lowering/objects/virtual_bases.h`
  (the `__vbptr`/`__pvbptr` parameter synthesis, `CarriesVirtualBase`,
  `VirtualBoundaryEntity`, and the contract built in `CacheVirtualBaseBoundary`).

  Two compounding defects were isolated, and this crash needs both fixed:

  1. **Contract inconsistent across translation units.**  `CacheVirtualBaseBoundary`
     narrows a function's carried virtual bases below the carry-all default by
     scanning the body (which bases are forwarded/demanded).  That scan is not
     TU-stable for a weak/inline function: `arena.o` calls
     `__put_character_sequence` with three arguments (demand-reduced, no
     `__pvbptr` in `rcx`) while the definition the linker keeps, from
     `cppgm++-runner.o`, reads `rcx` as `__pvbptr`.  Restricting the reduction
     to TU-local (static / unnamed-namespace) functions and keeping carry-all
     for weak/external ones makes `arena.o` pass `rcx` and matches the
     definition -- verified at the object level -- but it changes the LowIR
     parameter lists of cross-TU virtual-base functions (reference churn) and
     did NOT by itself stop the crash, so it was reverted pending (2).

  2. **`__pvbptr` corrupts as a stack argument.**  With (1) applied the pointer
     is threaded correctly down to `WriteTranslationUnit` (gdb: its `__pvbptr`
     equals the vtable-computed ios subobject exactly).  `RunTranslationUnit`
     and `SyntaxArena::Write` have more than six parameters, so their appended
     `__pvbptr` lands in a stack argument slot; by `__put_character_sequence`
     it has become a code address (a return address, `Stats::Stats`), and the
     backtrace is stack-smashed.  Traced to the origin: `WriteTranslationUnit`
     receives the correct `__pvbptr` in `r9`, then calls `RunTranslationUnit`
     (declared in `driver_detail.h`, defined in another TU) and stores nothing
     into that call's `__pvbptr` stack slot -- it clobbers `r9` with `stats`
     and passes no companion pointer at all.

  The two defects share one cause: the contract is computed from two sources
  that disagree.  A definition's contract is built by scanning the body
  (`CacheVirtualBaseBoundary` -> forwarded/demanded -> a possibly reduced
  carry set); a caller that sees only the declaration counts hidden pointers
  from the parameter *types* instead (`CountVirtualBaseParameters` /
  `VirtualBaseParameterCount`, effectively carry-all).  Body-derived and
  type-derived counts need not match, and neither is visible to the other
  side of a translation-unit boundary, so caller and definition pass and read
  different numbers of `__pvbptr` arguments.  Restricting the reduction to
  TU-local functions (defect 1) was necessary but not sufficient, because the
  caller-side count for a declaration-only external callee is computed by a
  different routine that still diverged; it was reverted.

  This is the `__pvbptr` companion-pointer scheme, a cppgm++ invention in
  place of the Itanium vtable vbase-offset.  Same class as the gcc-cell
  KW_TRUE self-host miscompile.  The robust fix makes the contract a pure
  function of the signature (so declaration and definition agree without
  seeing each other's body), or -- better -- reads virtual-base offsets from
  the vtable when the most-derived type is not statically known and retires
  the companion pointer.  Either is a large, high-risk change to a core ABI
  subsystem needing a dedicated pass and full byte-exact + audit validation.

- **A flaky build/test race in the self-host chain.**  `make -C pa39
  test-pa6 CXX=../dev/cppgm++` under `-j` intermittently reports the pa6
  suite failing (0/N) while the freshly built `recog-self` is byte-correct
  and `test-pa6-nobuild` passes 48/48; `-j1` always passes.  A missing
  ordering between the checkpoint link and the sub-make test lets the test
  occasionally run before the staged binary is in place.
