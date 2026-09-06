# Plan: Clang and libc++ as Supported Toolchains

Status: proposed

Date: 2026-09-03

## Objective

Make `clang++` a supported host compiler and libc++ a supported target standard
library, so that the assignment ladder, the self build, and the inception build
all pass in that shape.  Every gap found on the way is reduced to the earliest
assignment whose stated surface already owns the construct, fixed there, and
covered by a fixture in that assignment.  The ladder keeps teaching what the
compiler has to do; it does not accumulate a private compatibility list that no
assignment describes.

Supporting a second standard library is the real content.  Supporting a second
host compiler is nearly free, which the probe below establishes rather than
assumes.  The plan is sized accordingly.

## Two axes, kept separate

The word "clang support" hides two independent things, and conflating them is
the fastest way to misread a failure:

- **Host axis.**  `cppgm++` is *built by* `clang++` instead of `g++`.  This
  exercises our own source against a second C++ implementation.
- **Target axis.**  `cppgm++` *compiles source that includes* libc++ headers
  instead of libstdc++ headers.  This exercises our front end against a second
  standard library's use of the language.

They are coupled in exactly one place: `CPPGM_STDLIB_FLAGS` is passed both to
the host build and to `dev/gen_builtin_host_config.pl`, so building with
`-stdlib=libc++` also bakes libc++'s include paths into the compiler.  That
coupling is correct and stays.  The failure surfaces are still distinct and
every reduction in this plan names which axis it belongs to.

The supported matrix is three cells, not a cross product:

| host | target stdlib | status |
| --- | --- | --- |
| `g++` | libstdc++ | current default, 5,500/5,500 through PA38 |
| `clang++` | libstdc++ | probe says already passing; needs a lane and gates |
| `clang++` | libc++ | the work |

`g++` with libc++ is not a supported cell and no lane will be added for it.

## Evidence from the probe

Reproduce with a detached worktree so the main tree's `dev/cppgm++` is never
clobbered:

```sh
git worktree add -f ~/clangprobe HEAD
cd ~/clangprobe/dev && make -j32 cppgm++ CXX=clang++ CPPGM_HOST_CXX=clang++
```

Toolchain on this host: Ubuntu clang 21.1.8, target `x86_64-pc-linux-gnu`,
with `libc++` and `libc++abi` present.

### Host axis with libstdc++: already at parity

- Every target builds clean under `clang++`.  Only warnings, the loudest being
  one `-Wreorder-ctor` in `lowering/core/program_lowerer.cpp:114`.
- The `clang++`-built `cppgm++` compiles a `<vector>`/`<string>`/`<map>`/
  `<sstream>` translation unit to an object **byte-identical** to the one the
  `g++`-built compiler produces.
- `make -C pa39 -j32 cppgm++-self CXX=clang++ CPPGM_HOST_CXX=clang++` succeeds,
  and the resulting self-built compiler's object output is also byte-identical
  to the `g++` baseline.

So this cell needs a lane, gates, and a pinned image.  It does not need fixes.
That is a finding worth keeping: it means any later breakage in this cell is a
regression with a known-good starting point, not an unfinished port.

One pre-existing limitation was confirmed **not** to be clang-related: linking
that same translation unit fails with `undefined native symbol:
std____throw_logic_error` under both the `g++`-built and the `clang++`-built
compiler.  It belongs to link mode, not to this plan, and is recorded here only
so the next person does not re-reduce it.

### Host axis with libc++: one failure class

Building with `CPPGM_STDLIB_FLAGS=-stdlib=libc++` fails on absolute size
budgets.  libc++'s `std::string` is 24 bytes where libstdc++'s is 32, so every
structure that embeds one shrinks:

```text
src/abi/itanium/abi_mangle_model.cpp:280: sizeof(AbiType) == 400 -> 344
src/abi/itanium/abi_mangle_model.cpp:282: sizeof(AbiTemplateArgument) == 1912 -> 1640
src/abi/itanium/abi_mangle_model.cpp:284: sizeof(AbiDependentExpression) == 1024 -> 888
src/abi/itanium/abi_mangle_model.cpp:286: sizeof(AbiFunctionTarget) == 1072 -> 920
src/abi/itanium/abi_mangle_model.cpp:288: sizeof(AbiFunctionRecord) == 824 -> 696
```

There are 21 such assertions across 12 files.  With them neutralized the entire
compiler builds clean under clang and libc++, so this is the only host-axis
issue in that cell.

These assertions are load-bearing: they are the guard that keeps typed
presentation storage from silently widening, and several optimizer plans cite
them.  The fix is to keep the guard while making it stdlib-relative, not to
delete it.

### Target axis: three root causes block 30 of 38 headers

`dev/gen_builtin_host_config.pl` already bakes the correct libc++ table with no
change:

```text
kHostCxx    = "clang++"
kStdlibFlags= "-stdlib=libc++"
kVersion    = "21.1.8"
kStandardIncludePaths = {
  "/usr/lib/llvm-21/bin/../include/c++/v1",
  "/usr/lib/llvm-21/lib/clang/21/include",
  "/usr/local/include", "/usr/include/x86_64-linux-gnu", "/usr/include" }
```

Probing each of the 38 standard headers `dev/src` includes, one per translation
unit, gives a small and bounded inventory.  Before any fix, every header fails
in preprocessing.  After the first fix, 8 pass and the remaining 30 fall into
exactly three causes.

**B1 — scoped attribute-token in `__has_cpp_attribute`.**  `libc++/__config`
writes `__has_cpp_attribute(_Clang::__no_destroy__)` and thirteen more like it.
`EvaluateBuiltinProbe` in `preprocess/macros/macro_processor.cpp:1318` requires
the operand to be exactly one identifier-like token and otherwise throws
`invalid builtin probe operand`.  Since the result for this probe is already a
hard `false`, the parse is the whole problem.  Blocks all 38 headers including
`<cstddef>`.  Also missing from the probe family, by inspection of libc++'s
usage: `__has_warning`, `__has_declspec_attribute`, `__has_constexpr_builtin`,
`__has_rmw_builtin`.

**B2 — conditional `explicit`.**  `__utility/pair.h:140` writes
`explicit(!_CheckArgsDep::__enable_implicit_default())`.  This is the C++20
conditional explicit, and libc++ 21 emits it unconditionally outside its C++03
branch: `_LIBCPP_STD_VER` is 11 under `-std=gnu++11` and the construct still
appears.  Both `g++` and `clang++` accept it in C++11 mode as an extension with
`-Wc++20-extensions`, so this is a concession the hosted headers require, not a
standard-mode feature.  Blocks 23 headers: `algorithm array chrono deque
fstream functional iomanip iostream iterator locale map memory numeric ostream
queue set sstream streambuf string unordered_map unordered_set utility vector`.

**B3 — missing type-trait intrinsics.**  `__type_traits/is_const.h` writes
`_BoolConstant<__is_const(_Tp)>`; we answer `type name used as retained value:
_Tp`, meaning the trait is not registered and the type operand parses as a
value.  We register 38 `__is_*` traits, tuned to libstdc++, which spells several
of these as partial specializations instead.  Confirmed missing and directly
observed: `__is_const`, `__is_volatile`, `__is_array`, `__is_object`.  Screening
libc++'s usage against our registry names roughly seventeen genuine trait
builtins we lack, including `__is_bounded_array`, `__is_compound`,
`__is_fundamental`, `__is_lvalue_reference`, `__is_rvalue_reference`,
`__is_nothrow_convertible`, `__is_nothrow_destructible`, `__is_referenceable`,
`__is_scoped_enum`, `__is_unsigned`, and
`__has_unique_object_representations`.  The exact set is established in C4 by
re-probing, not by trusting that screen, which also matched libc++'s own helper
function names.  Blocks `exception limits type_traits` directly and more behind
B2.

**B4 — missing math builtins.**  First is `__builtin_fabsf`.  Blocks `cmath
cstdlib new stdexcept`.

Passing today after B1 alone: `cctype cerrno climits cstddef cstdint cstring
ctime iosfwd`.

The diagnostics that carry a file, line, and column come from the source token
ranges landed in `70aac8a4`.  The two that do not — B3 and the probe error —
are themselves a finding: an unlocated diagnostic costs a bisect every time.
C2 and C4 attach locations to the diagnostics they touch.

## Standard-library discovery

The mechanism is already the right shape and this plan mostly writes down its
rules and closes its gaps.  Nothing in `dev/src` runs a host compiler at run
time today, and nothing may start.

1. **Build-time baking is the only default source.**
   `dev/gen_builtin_host_config.pl` probes `CPPGM_HOST_CXX` plus
   `CPPGM_STDLIB_FLAGS` once, at build time, and emits the include table, the
   predefined macros, the target triple, the version, and the search dirs into
   a generated header.  `ConfigureHostedPreprocessing` reads only that.
2. **Explicit flags beat the baked table.**  `-nostdinc` clears it and
   `-isystem` appends; both already exist in the driver.  This is the supported
   way to compile against a different libc++ or libstdc++ than the one the
   compiler was built against, with no rebuild, and it is what the
   multi-version test lane uses.
3. **One environment fallback, and it names paths.**  A
   `CPPGM_SYSTEM_INCLUDE_PATH` list is consulted only when the baked table is
   empty, which happens when the generator's probe failed at build time.  It
   never invokes a compiler.  A build whose probe failed emits a warning rather
   than silently producing a compiler that cannot find a header.
4. **The compiler can report what it will search.**  Add
   `cppgm++ --print-search-paths`, printing the effective list with its
   provenance — baked, flag, or fallback — plus `kHostCxx`, `kStdlibFlags`, and
   `kVersion`.  A version mismatch then costs one command instead of a
   header-not-found hunt.  This is also the fixture oracle for rule 2.

For version skew across libraries:

- **Pin the toolchains.**  Add `docker/clang-libcxx/Dockerfile` in the shape the
  extended repository already uses: install a fixed clang and matching
  `libc++-dev`/`libc++abi-dev`, and set `CXX`, `CPPGM_HOST_CXX`, and
  `CPPGM_STDLIB_FLAGS` in the image.  Pinning is what makes a libc++ reference
  reproducible at all.
- **Default references stay standard-library agnostic.**  PA34 already enforces
  this for preprocessor references through
  `pa34/scripts/check_preproc_refs_portable.sh`.  Extend the same rule to the
  compile lane and state it in the PA34 readme.
- **Library-pinned cases live in their own root** that the default lane does not
  run, named for the library and version it pins.
- **Do not chase every libc++ release.**  The plan targets one pinned libc++ and
  keeps the ladder's fixtures written against the *language construct* the
  library used, not against the library.  A fixture that reduces
  `explicit(bool)` is durable; a fixture that includes `<vector>` is not.

## Ownership rule for reductions

Each failure is reduced to a minimal source fixture and placed in the earliest
assignment whose **stated required surface** already covers the construct — not
the earliest assignment that happens to touch the code.

PA34 states hosted compatibility as its surface: predefined macro import,
`__has_*`, "GNU/Clang parser concessions commonly exercised by the selected
hosted headers", and "builtin traits, transforms, intrinsics, and builtin
families used during hosted compile acceptance".  B1 through B4 all land there
on that reading, and each fix must cite the readme clause it satisfies.

Two rules keep that from becoming a dumping ground:

- If the construct is one an earlier assignment already owns and the failure is
  a defect in our handling of it, it reduces to that earlier assignment.  A
  template-lookup bug uncovered by a libc++ header belongs to the assignment
  that owns the lookup, not to PA34.
- If PA34's readme does not already describe the surface, the readme changes in
  the same commit as the fixture.  A fixture whose rule is written nowhere is
  the `bad-removed-spaghetti` failure mode: it reads to a student as an
  arbitrary requirement.

Every fixture is written against the construct, minimally, with no standard
header included.  The libc++ header is what found the bug; it is not the test.

## Execution program

Each increment: reduce, place the fixture, fix, run the assignment lane, run the
cumulative gate, run the performance gate, commit.  No increment lands with a
red lane or an unexplained regression.

### C0. Lanes, image, and baselines

Add the clang cells as first-class lanes before changing any compiler code, so
every later increment has somewhere to report.

- `docker/clang-libcxx/Dockerfile` pinning clang, libc++, and libc++abi.
- Root make targets for the two clang cells that forward `CXX`,
  `CPPGM_HOST_CXX`, and `CPPGM_STDLIB_FLAGS`, so a lane is one command.
- Record the baselines: `make test-report-through-pa38` for gcc+libstdc++ and
  for clang+libstdc++, `make inception` for both, and the header probe
  inventory for clang+libc++ committed as a checked-in report so progress is
  measurable rather than remembered.

**Exit:** clang+libstdc++ is green on the full report and on inception, and its
result is reproducible from the image.

### C1. Make the host build standard-library agnostic

Convert the 21 absolute `sizeof` budgets into guards that still fail when
storage widens but do not encode one library's `std::string`.  Options in
preference order: assert against a computed budget expressed in terms of the
members' own sizes; or key the expected value on the detected library with both
values written down.  Deleting the assertion is not an option — it is the
guard several optimizer plans depend on.

**Exit:** clang+libc++ builds every target clean; the guard still fires when a
member is widened deliberately, proven by a temporary widening in review.

### C2. Preprocessor probe family

Accept a scoped attribute-token operand in `__has_cpp_attribute`, and add
`__has_warning`, `__has_declspec_attribute`, `__has_constexpr_builtin`, and
`__has_rmw_builtin`.  Attach a source location to `invalid builtin probe
operand` while in this code.

**Fixtures:** PA34 preproc lane — a scoped attribute probe, a probe with a
malformed operand that must still be rejected, and one per added probe.  The
rejection fixture matters: without it, "accept everything" passes.

**Exit:** all 38 headers reach the parser; 8 compile.

### C3. Conditional explicit

Implement `explicit(constant-expression)` as a hosted concession, matching what
`g++` and `clang++` accept in C++11 mode.  Both the grammar and the semantic
effect on implicit conversion sequences are in scope; a parse that accepts the
form and then ignores the condition would silently mis-rank overloads.

**Fixtures:** PA34 compile lane — a class whose constructor is explicit only
when the condition is true, exercised in both directions through copy
initialization, plus a rejection fixture for a non-constant condition.

**Exit:** the 23 headers blocked at `pair.h:140` get past it; re-probe.

### C4. Type-trait intrinsics

Establish the exact missing set by re-probing rather than by the screen in this
document, then register each trait with its evaluation.  Each trait needs a
semantic answer, not just a registry entry: `__is_const` on a reference type,
on a cv-qualified array, and on a dependent type all have specified answers.

**Fixtures:** PA34 compile lane — one per trait, each covering the interesting
answers rather than a single true case, and a rejection fixture for a trait
applied to the wrong operand kind.

**Exit:** `exception limits type_traits` compile; re-probe the rest.

### C5. Builtin families

Add the math and other builtin families the remaining headers need, starting
from `__builtin_fabsf`.  Establish the set by re-probing.

**Exit:** all 38 headers compile individually.

### C6. Converge on real translation units

Individual headers compiling is necessary, not sufficient.  Compile
progressively larger real units under clang+libc++ — a program using the
containers, then a `dev/src` file, then the whole of `dev/src` — reducing each
new failure through the same loop.  Expect this to be the longest phase and the
one with the least predictable content; the previous phases exist to make its
failures legible.

**Exit:** every `dev/src` translation unit compiles under clang+libc++.

### C7. Self build

`make -C pa39 cppgm++-self` in the clang+libc++ shape.  Note that pa39 only
treats `../dev/cppgm++` of its own worktree as the self-host producer; any
other path becomes the "host" flavor and the link fails with *native or invalid
object cannot be linked by cppgm++*.  Run this in the producer's own worktree.

**Exit:** the self build links and the self-built compiler passes
`make test-report-through-pa38`.

### C8. Inception

`make inception` in the clang+libc++ shape.  Inception compares a compiler
built by a compiler built by a compiler; it is the phase that surfaces
divergences the self build tolerates, because the second generation's output
must match the third's.

**Exit:** `compare-cppgm++-inception` reports MATCH.

### C9. Close

- The three supported cells are lanes anyone can run in one command.
- PA34's readme states every surface the new fixtures test.
- The discovery rules above are written in the readme that owns them, including
  the prohibition on run-time host probing.
- A short report records the final inventory and what each fix cost.

## Testing requirements

- Every fix ships with at least one fixture in the assignment that owns it, and
  every new accepting rule ships with a rejection fixture so the rule cannot be
  satisfied by accepting everything.
- Fixtures are minimal and include no standard header.
- The full `make test-report-through-pa38` is green in the gcc+libstdc++ cell
  after every increment.  That cell is the one with 5,500 passing tests and it
  is the regression surface that matters most.
- `make inception` is run before any semantic change is called safe.
  `test-report-through-pa38` does not self-host and will not catch a self-host
  regression.
- The audits stay green: `audit-lowir-contract`, `audit-compiler-layout`,
  `audit-frontend-source-sets`, `audit-semantic-owners`.
- `make test-debuginfo` is a known-red lane with four PA13 failures whose
  references were generated by the reference compiler.  Do not read it as a
  regression signal; do check that the count and diff shape are unchanged.

## Performance protocol

Additions in this plan land in the preprocessor's probe path and the semantic
trait path, both of which are hot.  A trait registry that becomes a linear scan
would not fail a single test.

- Use `scripts/run_ab_compile_benchmark.py`, run from the source-layout working
  directory with `--repo-root` set to it.  Compiler arguments take the
  `--compiler-arg=-O1` form; the space-separated form is rejected.  The script
  screens load and PSI; a hand-rolled loop does not, and this box is shared.
- Four lanes, as in the O2/O3 protocol: gcc-O1, gcc-O3, self-O1, self-O3, timed
  through pa39 `compare-cppgm++-inception` after an untimed self-host build.
- **Aggregate CPU is the gate for the full build**, not wall.  Two 3-block
  samples of the same change once gave +1.21% and -0.90% on wall while CPU
  agreed at +0.17% and -0.12%.  Take at least 6 blocks.
- For the frozen translation unit, use cachegrind Ir with `--vgdb=no`, and pass
  `--cache-sim=yes` explicitly for movement-class changes; the default is
  `--cache-sim=no` and yields Ir only.  The self-policy Ir noise floor is about
  0.001%, so this lane resolves small regressions the timing lanes cannot.
- **Thresholds.**  No lane may regress more than 1% aggregate CPU, and the
  frozen-TU Ir may not regress more than 0.5%, without a written justification
  naming the feature the cost buys.  Anything unexplained is reverted and
  re-approached, not accepted.
- Measurement outputs go under `$HOME`, not the scratchpad, which has a shared
  quota that surfaces as spurious compiler failures.

## Commit sequence

One commit per increment, in the repository's style: a bare imperative subject
and a body that says what was wrong and why the fix is shaped as it is.  A
commit that adds a fixture and the fix it covers stays together; a commit that
changes a readme surface includes the fixture that tests it.

## Rollback rule

If an increment cannot be made green in the gcc+libstdc++ cell, it is reverted
rather than carried.  The default cell's 5,500 passing tests are worth more
than progress on a second cell, and a half-landed concession is harder to
reduce than an absent one.

## Completion criteria

1. All three supported cells build, self-host, and reach inception MATCH at
   both `-O1` and `-O3`.  (The `-O1` half was added in the third run: a cell
   whose `-O1` self build miscompiles itself is not supported.)
2. `make test-report-through-pa38` is green in every supported cell.
3. Every fix has a fixture in the assignment that owns it, and every surface
   those fixtures test is described in that assignment's readme.
4. No lane regressed past the thresholds above, or each exception is written
   down with what it bought.
5. The compiler never probes a host compiler at run time, and
   `--print-search-paths` reports what it will search and why.

## Non-goals

- Supporting `g++` with libc++.
- Tracking libc++ releases.  One pinned version, with fixtures written against
  constructs rather than headers.
- Implementing C++20.  `explicit(bool)` is taken as a hosted concession that
  both host compilers already grant in C++11 mode, on the same footing as the
  other GNU and Clang concessions PA34 already requires.
- Matching Clang's diagnostics.  That was the strict witness experiment, and it
  was dropped for good reasons recorded elsewhere.

## Execution record

### C0 — lanes, image, and baselines

Landed in `d7262e8d` and, for the relink defect below, `cfea22c2`.

- Cell prefixes `with-clang-` and `with-clang-libcxx-` run any target against a
  toolchain pair, each with its own object roots, guarded by a toolchain-present
  check.
- `docker/clang-libcxx/Dockerfile` pins clang, libc++, and libc++abi.  Not built
  here: no docker on this host.
- `scripts/probe_hosted_headers.pl` compiles each standard header the compiler's
  own sources include as its own translation unit and groups the failures by
  first diagnostic.

Baselines: g++/libstdc++ and clang/libstdc++ both accept 38/38 headers, and
clang/libstdc++ passes 5527/5527 through PA38.  Its inception failed on the
defect recorded under C0a and reports MATCH once that is fixed, so both C0 exit
criteria are met.

A defect in the first cut of the lanes is worth recording, because it produced a
confidently wrong conclusion before it was found.  The tool binaries live in
`dev/` and are shared by every cell, while the objects they link from live under
`$(OBJ)` and are not.  Switching cells therefore left a binary newer than the
objects make would relink it from, so make skipped the link and the previous
cell's compiler stayed in place under the new cell's name.  Two experiments were
run against the wrong binary, and the first attributed a failure to a commit
that turned out to be innocent.  `dev/.toolchain` now stamps the toolchain
identity next to the binaries and the link rule depends on it.

### C0a — use-after-free in constexpr constructor evaluation

Not anticipated by the plan.  C0's inception gate found it, which is the reason
that gate exists: `test-report-through-pa38` passed 5527/5527 in the same cell.

`EvaluateConstexprConstructorInitializers` held a reference to
`entity_data_members_[entity]` across the initializer loops.  Evaluating an
initializer re-enters analysis, which can lay out a class the translation unit
has not seen yet and grow that vector, leaving the reference dangling.  The loop
then read a stale size and the function reported success with no complete
object, which its three callers published into the dump.

The bug is a latent one in our own source, not a clang defect: it compiled under
a g++-built `cppgm++` and failed under a clang-built one, because whether the
stale read matters depends on allocator behaviour.  UBSan is clean on it; ASan
names it directly as a heap-use-after-free on `members.size()`.  That asymmetry
is the argument for `make asan-build`, added with the fix: a fixture only
catches this bug in the cell that happens to expose it, while the sanitizer
catches it in either.

Reduced to `cppgm.tests/course/pa21/301-constexpr-constructor-instantiating-
initializer`, which fails in the clang cell before the fix and passes after.
PA21 owns constexpr constructors and object-valued constant evaluation.  Fixed
in `cfea22c2`; the gcc cell went 5527 to 5528 with the new fixture and no
regression.

Two smaller findings were left alone as out of scope, recorded so they are not
re-derived: a `constexpr` array whose elements are never odr-used is rejected
with "constexpr object initializer is not constant", and a namespace-scope
`static_assert` placed after such an array makes the array's own initializer
stop being constant.

### C1 — standard-library-agnostic size budgets

Landed in `f09dc211`.  The five typed-ABI budgets were absolute byte counts and
all five failed against libc++, whose `std::string` is 24 bytes against
libstdc++'s 32.  Each budget now subtracts the shrinkage the current library
implies, scaled by the string subobjects the structure holds, so they stay
exact rather than becoming ceilings and are unchanged under libstdc++.  Adding
a member to `AbiType` still fails all five in both cells, checked by doing it.
That was the only thing blocking the libc++ build.

### C2 — preprocessor probe family

Landed in `a24fc83c`.  Accept a scoped attribute-token in
`__has_cpp_attribute`, and answer `__has_warning`,
`__has_declspec_attribute`, `__has_constexpr_builtin` and `__has_rmw_builtin`;
libc++ uses all four and libstdc++ uses none.  Operand errors now carry a file,
line and column.

Registering them exposed a second name list inside `IsBuiltinProbe`, and a name
in the marker map but not that list is worse than an unknown name: the header
sees the probe as defined, skips its own fallback macro, and the unsubstituted
call reaches the controlling expression.  The map is now the whole answer.
Inventory 0 to 8 of 38, one cause to three.

### C3 — conditional explicit

Landed in `db75a5c5`.  The specifier already parsed; the libc++ failure was an
attribute sitting between it and the rest of the declaration, which the
special-member specifier loop stopped at.  Fixing the parse exposed that the
condition was then discarded, so every `explicit(...)` constructor was
explicit.  Evaluate it, and reject a non-constant condition.

A condition that depends on template parameters is still unanswered: the
pattern carries only a bool, and answering it needs a deferred fact like the
one exception specifications use.  libc++'s `pair` needs it.  Behaviour is
unchanged there, so this is a gap rather than a regression.  Inventory stays at
8 of 38 while the wall moves from one blocker to five deeper ones.

### C4 — type-shape traits

Landed in `091391f7`.  Fourteen traits libc++ spells directly rather than
deriving from partial specializations, each checked against clang's answers.

The sharper finding was underneath: the registry answers every lookup with a
binary search, its tables must stay sorted by spelling, and nothing said so.
Inserting the new entries as a block unsorted the table and silently knocked
out `__is_class` and its neighbours -- a builtin that had worked for months
stopped being recognised and failed far away as an unknown name.
`audit-builtin-registry-tables` now checks it, since no fixture catches the
class.  Inventory 8 to 10 of 38, and the trait cause is gone.

### Performance through C4

Measured against `22ae72a0`, the last commit before any compiler change in this
plan.  Paired ABBA over 6 blocks on `semantic/templates/validation.cpp`:
+0.32% wall, +0.71% user.  Cachegrind Ir on two translation units: +0.40% on
`validation.cpp` and +0.47% on `lowir/optimize/pipeline.cpp`.  Both are inside
the thresholds this plan set, but only just, so the remaining phases have to be
lean.

Attribution: the constexpr fix costs +0.006%, C1 emits no code, and C3 and C4
are free -- the whole cost is the C2 commit.  It has no per-token algorithmic
addition, and the one path that runs per token got cheaper rather than dearer;
removing four of its registrations moves the figure by 0.16% and re-adding them
restores it, which is the signature of code layout rather than of work.  What
it buys is that every libc++ header preprocesses at all.  Recorded rather than
chased further.

### C5 — builtin families

The uniform libm family: 129 intrinsics across `float`, `double` and `long
double`, each taking its own result type in every operand and lowering to the C
function of the same name.  The operation kind that already did this was called
`FLOATING_OPERATION_EXTERNAL_CEIL` although nothing about it was ceil-specific;
it is `FLOATING_OPERATION_EXTERNAL_LIBM` now, and its signature is built from
the entry's arity rather than assuming one operand.  Inventory 10 to 14 of 38.

Adding them found a second parallel-structure hazard next to the sortedness one
from C4, and a worse-behaved one.  `GetFloatingIntrinsic` indexes the table by
the enumerator's own position, so the enum has to list the kinds in the table's
order.  Inserting the new enumerators as a block while the table stayed sorted
by spelling made every intrinsic answer as some other one: it built cleanly,
then segfaulted on one fixture and failed an unrelated `__builtin_flt_rounds`
test.  The enum is generated from the table's order now, and the audit checks
the correspondence as well as the sortedness.

The libm entries whose operands are not all the floating type came second:
`frexp`, `ldexp`, `scalbn`, `scalbln`, `modf`, `remquo`, `nexttoward`,
`ilogb`, `lrint`, `lround`, `llrint` and `llround`.  Each names its own
signature shape and some return an integral type rather than the operand's.
Three things had to give way, taken one at a time with `frexp` alone first:
the operand checker assumed every operand was floating, which the declared
signature already answers correctly; the lowering treated an unrecognised
operation as something to lower itself rather than as an ordinary call; and
the set of operations that mean "call the C function of the same name" was
named twice, once on each side, so it is a predicate on the registry now.

That exposed the last thing holding those headers: libc++ writes
`::__builtin_copysignf`, and the builtin dispatch matched on the callee's
spelling with the qualification still attached.  A builtin is a
global-namespace name, so `::__builtin_x` names the same one; any deeper
qualification does not, and still fails.

Inventory 14 to 17 of 38, and the causes drop from five to four.

### C6 — parser disambiguation, and the diagnostics that hid it

Three causes went in this phase, and the shape of all three is the same: the
construct that failed was never the problem.

The complete-class context (`af7a988d`) and the template-parameter kind leak
(`c4f58f2c`) are written up beside their entries in the list above.  Between
them they moved 13 headers past their blocker and left the remaining ones all
stopping in one place.

That place was an internal error with nothing to go on.  A gdb break on the
throw named it in one step, which is worth remembering: for an internal error
the stack is the diagnostic, and reaching for a reducer first wastes the run.
`0bf3d56a` makes it a semantic error and repairs the location mechanism that
made the first attempt at locating it actively misleading.

### C7 — the substitution failure that outlived its candidate

The `unique_ptr` blocker was not about `unique_ptr`, which is why six reducers
of that construct all compiled.  A gdb break at the throw showed
`CandidateSubstitutionFailed()` already true on arrival: the compiler was
completing a class template specialization *inside* an enclosing candidate
substitution that had already failed, and every consumer of the resulting empty
type read that failure as its own.  Guarding each consumer was whack-a-mole --
three guards, three new sites -- which is the tell that the root cause is
upstream.

N3485 14.8.2/8 settles it: instantiating a class template specialization is not
in the immediate context of a deduction.  The instantiation neither gets
excused by a substitution in progress nor gets to inherit its failure.
Suspending the substitution for the duration of the completion is the whole fix,
and it made all three consumer guards unnecessary, checked by removing them.

That exposed the next one immediately.  `__has_builtin` consulted the integer,
floating, memory and atomic registries but not the type-trait or transform
ones, so we denied builtins we implement.  libc++ then took its fallback for
`is_signed`, which is written as a C++17 variable template -- machinery we would
have had to support instead of the builtin we already had.  Denying a builtin
you have is worse than lacking one.  Both defects are the same family as C2's
and C4's: a fact known in one place and answered from another.

Together these moved all 18 headers off that cause.  The remaining one is a
variable template read in a constant expression, at `unique_ptr.h:149:51`.

**The suspension has no minimal fixture.**  Eight reducers were tried; the ones
that fail turn out to fail for a different reason, which is recorded below.  It
is verified by the header inventory moving and by 5542/5542 in the default
cell, and that is weaker evidence than this plan asks for.  Recorded as a debt
rather than dressed up.

A separate defect surfaced while looking for that fixture and has a clean
five-line reducer that clang accepts: a template-id argument list whose first
argument is a dependent `typename T::type` and whose second names a member of
a class template specialization fails with *incomplete named type:
__retained_template_parameter_shape_0*.  It is unaffected by the suspension fix,
so it is its own bug.

### C7a -- the dependent value that had to survive an operator

The fourth attempt landed it.  The three that failed all tried to make the
*argument* dependent at the point it is formed; what was missing is that the
dependence has to be a property of the **expression**, because the argument
libc++ actually writes is not a bare read.

`unique_ptr.h:149` forms it as
`__is_replaceable_v<pointer> && __is_replaceable_v<deleter_type>`.  A retention
test keyed on the operand's binding sees a variable template specialization and
retains correctly -- for a single read.  Through `&&` the binding is gone: the
operator reports `constant=false` and `binding=kNoBinding`, and the two facts
that would have identified the operands as *waiting* rather than *not constant*
are exactly what the operator dropped.  That is why the earlier attempts each
cleared a hand-written reducer and left the header where it was.

So the bit is carried on `ExpressionInfo` as `dependent_value`, computed by
`Analyzer::IsDependentValueExpression` (a read of a variable template
specialization that has no value yet, or anything built from one) and
propagated at the single place a builtin binary expression is constructed.
`AppendTemplateArgument` then retains on the predicate rather than on the
binding, and `ApplyImplicitConversion` returns a value whose type is a retained
nondeduced shape unchanged instead of diagnosing it -- a retained argument
stands for a value nobody knows yet, so there is nothing to convert it to.

Owner is PA22, which is where variable templates become modelled entities
(PA19 and PA20 both list them as out of scope).  Fixture
`cppgm.tests/course/pa22/363-dependent-variable-template-value-argument.t`
asserts the compound form and, as a positive control, that the *selection* is
right rather than merely accepted: `pair_kind<char, char>` picks `char` and
`pair_kind<char, double>` picks `double`, which shows in the LowIR as
`slot $both_small : i8` against `slot $one_large : f64`.  clang and g++ both
run it to exit 0, and `cppgm++-ref` reproduces the same LowIR.

The result moves the libc++ inventory's dominant cause rather than the header
count: all 18 headers now pass `unique_ptr.h:149` and stop at the *next*
construct in the same file, `__attribute__((__aligned__(::std::__compressed_pair_alignment<T2>)))`
from `_LIBCPP_COMPRESSED_PAIR`.  That is the gap recorded above as parallel,
where accepting it "does not clear the `unique_ptr` failure" -- true when it
was measured, because this one was in front of it.  It is now the frontier, and
it wants a real constant evaluation of the attribute argument in the
instantiation's scope; `RequestedAlignment` currently parses the argument as an
integer literal token and refuses anything else outright.

### C7b -- the alignment nobody could compute yet

`_LIBCPP_COMPRESSED_PAIR` writes
`__attribute__((__aligned__(::std::__compressed_pair_alignment<T2>)))`, and
this was rejected outright because a GNU attribute is consumed as raw tokens:
each argument became a space-joined spelling, and anything that was not a
literal set a `gnu-attribute-nonliteral-argument` marker that every consumer
treats as an error.  There was no grammar in the attribute scanner to give the
argument meaning.

`alignas` already evaluates exactly this shape, variable-template operand
included, so the fix is to stop having two mechanisms.  The scanner now also
records each top-level argument's `[begin, end)` token range; for `aligned` it
hands those ranges back, and the parser -- which does have the grammar --
re-parses the range into the same `alignment-specifier` node `alignas`
produces.  `RequestedAlignment` prefers that node when present, and the
evaluation both spellings share moved into `AlignmentSpecifierValue`.

That got as far as *nonconstant alignment specifier*, which the instrumented
diagnostic then explained: `vt=1 vts=1 bindconst=0 dep=1` -- the argument is a
variable template specialization with no value, and C7a's predicate already
recognises it as dependent rather than non-constant.  The alignment simply is
not knowable while `deleter_type` is still `_Dp`.  So a dependent argument is
**deferred**, returning "no alignment requested", and layout computes it again
at instantiation with concrete arguments.  A genuinely non-constant argument is
still an error, and the two diagnostics gained source locations, which is what
made the cause legible at all.

Deferral is only safe if instantiation really does recompute, so that was
checked rather than assumed: six reducers spanning the static-member and
variable-template spellings, a nested unnamed struct, an inline variable
template with a partial specialization, and the real `_LIBCPP_COMPRESSED_PAIR`
macro all still fold to alignment 8 rather than 1.

Owner is PA34, which holds the GNU/Clang parser concessions the hosted headers
exercise and already carries `600-gnu-aligned-class-attribute`.  Fixture
`cppgm.tests/course/pa34/compile/204-gnu-aligned-dependent-argument.t` covers
the non-literal argument, both dependent spellings, and -- since a reference
deleter selects the variable template's partial specialization -- that the
deferred argument is re-evaluated per specialization instead of cached.
`cppgm++-ref` accepts it, so the fixture asserts the reference behaviour.

The 18 headers advance again, to `sizeof(_ToPad) - __datasizeof_v<_ToPad>` as
the bound of `__compressed_pair_padding`'s array member: *invalid array bound*.
That one is a model question rather than a missing evaluation --
`TryDependentArray` represents a dependent bound only as "template parameter
N", so an arbitrary dependent bound expression has nowhere to live.

### C7c -- the array bound, diagnosed but not fixed

Recorded because the diagnosis is most of the work and the wrong conclusion is
easy to reach twice.

The frontier after C7b is `compressed_pair.h:77:34` --
`char __padding_[sizeof(_ToPad) - __datasizeof_v<_ToPad>]`.  My first reading,
that this was a missing evaluation like the previous two, is wrong; so is the
opposite reading that the construct is unsupported.  Both
`std::__datasizeof_v<int>` and `std::__compressed_pair_padding<int>` compile
standalone, and a class template with `char b[sizeof(T) - 1]`, or with a
variable-template bound, compiles and lays out correctly.  `__datasizeof_v` is
not wrong either: it answers 16 for a padded POD, which is what clang answers.

What is actually different is the context.  `BuildArrayDeclaratorType` defers a
dependent bound only when it is handed `template_parameter_names`, and that set
is a *function* template mechanism -- `function_instantiation.cpp` builds it --
so it is null for a class template's data member.  The instrumented probe put
the failure at `class_template_completion_suppressed_depth_ == 2` inside
`__cppgm_class_template_identity_198_68_0`: the member is being analysed for
identity, with class template completion suppressed.

Lifting the suppression around the bound does not help, which is the useful
negative result: `_ToPad` is itself `pointer`, still dependent, so the bound
genuinely has no value there -- it is not merely blocked by our own traversal,
the way C7b's alignment was.

Neither placeholder is free, and this is what rules out the cheap fix:

- `TryArray(element, 0)` reaches layout and throws *invalid array size*.  That
  is the informative outcome -- the identity-built type **is** used for layout,
  so a placeholder is not a throwaway.
- `TryZeroLengthArray(element)` gets past and exposes the
  `incomplete named type: __retained_template_parameter_shape_1` bug already
  recorded at the end of C7 -- but it silently makes the padding zero bytes,
  and unlike C7b's deferral there is no evidence instantiation recomputes it.
  C7b's deferral was only landed because six reducers showed the alignment came
  back at instantiation; the equivalent evidence does not exist here, and a
  silent layout claim is exactly what this plan says not to trade away.

So it needs the type model to carry an *opaque* dependent bound -- an array
known to be dependent whose bound is an expression rather than "template
parameter N" -- with substitution recomputing it.  `TryDependentArray` today
encodes only `dependent_bound_type` plus `dependent_bound_parameter`, and that
representation is threaded through interning, qualification, identity,
mangling, and the deduction paths in `function_deduction.cpp`.  That is the
next piece of work, and it is a model change rather than another missing
evaluation.

**Instrumentation note.**  Two probes in this investigation produced false
trails.  Inserting a statement before a `return` that is the body of a
brace-less `if` makes the return unconditional; the resulting compiler rejected
`struct s { int v[2]; };`, and the "failure" at `bits/types.h:155` came from
that build, not from the compiler under test.  Replacing repeated occurrences
of a literal with text that contains the literal nests them all into the first
site, leaving the other two silent.  Probes into this code want braces and a
sanity case that must still pass.

### C7d -- locations in diagnostics, and the cause they immediately cleared

Prompted by the obvious question after C7b and C7c: would these be easier to
diagnose if the message carried a file, line, and column?  Every blocker so far
had been located by hand.  C7b's cause was unreadable until a location was
added to that single message; C7c needed a probe built only to recover one, and
two of the probe attempts were themselves wrong in ways that cost a false
trail.

Threading a node through nine hundred `ThrowSemanticError` sites is not the
shape of the problem, so the analyzer publishes the node it is working on and
the throw helpers append its location, with a hook keeping `support` free of a
dependency on the syntax model.  The guard sits at the expression and type-id
entry points and costs two stores and two restores; six ABBA blocks put it
below this harness's noise (paired wall -0.10%, user -0.22%).  Nothing in the
suite compares diagnostic text -- `remove_nonportable_reference_stdout` drops a
failing reference run's stdout precisely because it is nonportable -- so the
5,545 tests were unaffected, and the improvement cannot be asserted by a
fixture.

It paid for itself immediately.  The `__uint128_t` cause, previously just a
name with no site, resolved to `__charconv/tables.h:119:5` --
`__uint128_t(UINT64_C(...)) * UINT64_C(10)` -- and from there to a two-line
reducer: `__uint128_t(10)` is rejected while `__uint128_t x = 1;` is accepted.
The hosted type specifiers spell themselves as identifiers rather than
keywords, so they never reach the fundamental-spelling table in
`calls.cpp` that turns `int(x)` into a conversion, and that table ended in
`return kNoType`.  `HostedSpecifierType` already maps exactly those spellings,
so the fix asks it rather than restating the list -- this repo has been bitten
repeatedly by two structures that must agree.

That cleared a whole cause: **three become two**, with `ostream` and `sstream`
advancing to join the array-bound group.  Fixture
`cppgm.tests/course/pa34/compile/205-hosted-type-specifier-functional-cast.t`
checks values rather than acceptance -- the multiply overflows 64 bits and only
survives in 128.  Two things were deliberately left out of it: `_Float16(1)`
compared against an integer, which `cppgm++-ref` rejects with *fpext requires
wider destination type* and this build handles, and `sizeof(_Float16(1))`,
which is the reverse -- a `sizeof(T(...))` disambiguation this build does not
yet do.  Both are real and neither is this fix.

The location also re-attributed the array bound.  It is reported at
`unique_ptr.h:797:57`, the partial specialization
`hash<__enable_hash_helper<unique_ptr<_Tp, _Dp>, typename unique_ptr<_Tp, _Dp>::pointer> >`
-- which is the same shape as the eighteen-line reducer already recorded, and
the same `typename T::type` template-id that fails with *incomplete named type:
__retained_template_parameter_shape*.  So C7c's array bound is reached by
completing `unique_ptr<_Tp, _Dp>` with **dependent** arguments in order to
resolve `::pointer`.  That reframes it: the question is why a specialization
with dependent arguments is being completed at all, which
`ClassTemplateArgumentsAreLayoutReady` is supposed to prevent.  The array bound
is merely the first thing that cannot be computed once it is.  That is a better
lead than the type-model change C7c proposed, and it is where the next attempt
should start.

### C7e -- the array bound, fixed by following C7d's reattribution

C7d's located diagnostic pointed at
`hash<__enable_hash_helper<unique_ptr<_Tp, _Dp>, typename unique_ptr<_Tp, _Dp>::pointer> >`,
and that turned out to be the whole story.  A nine-line reducer isolates it:
naming `typename holder<T, D>::pointer` **nested inside another template-id**
in a partial specialization's argument list completes `holder<T, D>` while `T`
and `D` are still parameters.  The same typename is fine in a member typedef,
in a function parameter, and even alone as a specialization argument -- only
nested in a template-id does it force the completion.

C7c's conclusion that this needs an opaque dependent bound in the type model
was wrong, and so was the first fix here.  Two attempts are worth recording
because each failed in an instructive way:

- **Skipping the completion** when the arguments are not layout-ready cost 487
  tests.  That predicate is broader than dependence: it also refuses an
  argument whose own class is not complete yet.
- **Skipping it when the arguments are merely dependent** still cost four
  tests, all in the family the fix targets -- `400-dependent-alias-helper-
  partial-specialization` and its PA23/PA24 siblings.  The partial
  specialization machinery *needs* that completion in order to record a
  pattern containing `typename X<...>::y`.  Removing it is not an option.

So the completion stays and becomes tolerant instead.  The mechanism turned out
to be simpler than assumed: the bound is not non-constant, it is **constant and
meaningless**.  `sizeof(T) - 1` folds to `-1` because the stand-in parameter
shape has no size, and libc++'s `sizeof(_ToPad) - __datasizeof_v<_ToPad>` folds
to zero the same way.  Two traversals reach a member declaration without
wanting a layout -- completing a specialization whose arguments are still
dependent, and building a type for identity only, which suppresses completion
-- and in both a meaningless bound stands in rather than erroring.

The placement matters, and getting it wrong cost a fifth test.  Applied before
the existing branches it swallowed `T[N]`, whose bound is a parameter with a
dependent-array representation of its own (`range_iterator<T[N]>` in
`pa23/tests/general/200-range-array-reference-mutable-begin`).  It now applies
only where the function would otherwise reject the bound, so the dependent
array path is untouched and a genuinely bad bound is still an error.

Fixture `cppgm.tests/course/pa22/364-dependent-completion-tolerates-shape-only-member.t`
pins all three: the specialization matches, a concrete `holder<int, char>` is
still laid out at 3 bytes -- the tolerance must not leak into a real layout --
and `bound_kind<char[4]>` still selects the `T[N]` partial specialization.

**Twenty headers move past the array bound onto the
`__retained_template_parameter_shape` failure already recorded at the end of
C7**, which is now the single blocker for all of them, with `<set>`'s constexpr
constructor the only other cause.  Two causes remain, both known.

A note found while writing the fixture: this build folds a constant `&&` that
`cppgm++-ref` does not, so any fixture whose LowIR contains one cannot match a
generated reference.  It is a divergence in our favour at `-O0` and no existing
test covers it; the fixture uses sequential `if`s instead.

### C7f -- the retained parameter shape, narrowed

The single blocker for twenty of the twenty-one failing headers, narrowed by
bisecting the real header rather than by writing reducers, which is what
finally worked in C7e as well.  Five standalone reducers of the apparent shape
all compile, so the construct is again not the problem.

Editing line 797 of a copy of `unique_ptr.h` in place isolates the trigger
sharply:

- `hash<__enable_hash_helper<unique_ptr<_Tp, _Dp>, typename unique_ptr<_Tp, _Dp>::pointer> >` fails.
- `hash<unique_ptr<_Tp, _Dp> >` alone is fine, so it is not the specialization.
- `hash<__enable_hash_helper<unique_ptr<_Tp, _Dp>, _Tp> >` is fine, so it is
  the second argument.
- `__pointer<_Tp, _Dp>` -- a plain alias -- as that argument is fine.
- `typename unique_ptr<_Tp, _Dp>::element_type` and `::deleter_type` fail
  exactly like `::pointer`, so it is *any* dependent qualified member, not that
  member's own machinery.

The sharpest fact: at C++11 `__enable_hash_helper` **discards** its keys --
`template <class _Type, class...> using __enable_hash_helper = _Type;` -- so
the argument that fails is one the alias immediately throws away.  Forming a
dependent `typename X<...>::y` as a discarded alias argument is what produces
the incomplete `__retained_template_parameter_shape`.

Bisecting the class body rules out the obvious suspects: replacing
`_LIBCPP_COMPRESSED_PAIR` with two plain members still fails, and so does
dropping `__trivially_relocatable`, `__replaceable`, or both.  One neighbouring
observation worth keeping: replacing `using pointer = __pointer<_Tp, deleter_type>`
with `typedef _Tp *pointer;` changes the diagnostic to *duplicate retained
template declaration*, which is a different defect in the same area.

Next attempt should start from the discarded-argument fact -- an alias template
parameter that is never used still has its argument formed, and forming a
dependent qualified type in that position is where the shape leaks out.

### C7g -- the shape completion that tried to lay itself out

C7f's narrowing pointed straight at it.  The diagnostic came from `SizeOf`, so
something was asking a retained parameter shape for a size, and the shape-only
completion introduced in C7e was the thing asking: it runs the member
declarations, and member layout accumulation takes each member's size.  A
member whose type is one of the template's own parameters has a stand-in for
its type, and a stand-in has no size, so a specialization that only had to
exist became an incomplete-type error.

C7e already said the right thing in a comment -- "a shape completion, not a
layout" -- but only the array bound had been made to honour it.  Member layout
now honours it too: during a shape-only completion a member whose type has no
known size is skipped rather than measured.

Fixture `cppgm.tests/course/pa22/365-shape-completion-skips-member-layout.t`
has a proven negative control, which the earlier reducers in this cause never
did: built without the fix it reproduces
*incomplete named type: __retained_template_parameter_shape_1* exactly, and
with it, it passes.  It also pins two concrete specializations' sizes, since
skipping a member's layout must not leak into a real one.

`__memory/unique_ptr.h` now compiles, and the whole twenty-header group moves
off the retained-shape failure onto `<set>`'s existing cause, *constexpr
constructor has non-literal subobjects* -- so twenty-one of the twenty-one
failing headers now share **one** cause, with `<memory>` alone on a second,
*delete operand is not a unique pointer*.  The inventory is still 17 of 38, but
the tail has collapsed from five causes at the start of this run to two.

### C7h -- _Atomic is not volatile, for the literal-type rule

`<set>`'s long-standing cause, and after C7g the cause all twenty-one failing
headers shared.  The diagnostic did not say which class, so it got the same
treatment as C7d's locations: it now names the type, and answered immediately
with `struct std::__1::atomic_flag`.  An eleven-line reducer followed at once --
a class with an `_Atomic(bool)` member and a constexpr constructor -- which
clang accepts.

N3485 3.9/10 disqualifies a class with a **volatile** non-static data member
from being a literal type.  `_Atomic` is a separate extension and carries no
such rule, but `IsVolatileSubobjectType` answers for `CV_VOLATILE | CV_ATOMIC`
together.  That conflation is correct for its other reader -- zero
initialization of a whole object, where treating atomic like volatile is
conservative and safe -- so the fix is not to change it but to give the
literal-type rule the narrower question it actually asks.  The new predicate
walks bases and members rather than trusting the conflated entity flag, since
that flag is set on any class transitively containing either.

Fixture `cppgm.tests/course/pa34/compile/206-atomic-member-is-a-literal-subobject.t`
has a proven negative control -- without the change it reproduces
*constexpr constructor has non-literal subobjects* -- covers the member and the
base, and keeps a volatile member that must still be rejected, so the narrower
question is being asked rather than skipped.  It deliberately avoids a
`constexpr` *variable* of the type, which `cppgm++-ref` rejects with
*unsupported constexpr variable initializer*: a real limit, but not this one.

**The libc++ tail is now a single cause.**  All twenty-one failing headers stop
on *delete operand is not a unique pointer* from
`initialization/analysis.cpp`, where a delete operand that is not a pointer
finds no unique pointer conversion target.  It has no source location even with
a guard added at `AnalyzeDeleteExpression`, because that node carries none --
the same range-less-node problem recorded earlier.  Five causes at the start of
this run have become one.

### C7i -- the delete nobody could check yet

The last of the causes, and the one that finally moved the header count.  The
diagnostic named neither the type nor a location, so it got the same treatment
as the two before it, and answered at once: the operand is
`rvalue-reference to __retained_template_parameter_shape_0` with zero
conversions to a pointer.  That is a function template body being analysed with
the parameters standing in for types -- nothing about the delete is decidable
there, and every check below the pointer test is asking about a type nobody
has yet.  It is checked when the body is instantiated.

**The inventory finally moves: 17 of 38 becomes 21 of 38**, and the seventeen
that remain share a single new cause, *unknown type name: value_type*.

**This fix has no fixture, which is a debt rather than an omission.**  Six
reducers were tried -- a delete of a template parameter, of a dependent member
typedef, of `typename D::pointer`, of a forwarding reference, of an rvalue
reference, and libc++'s own `default_delete` shape including its static_asserts
-- and none reaches the branch.  Instrumenting it settles why: it fires exactly
twice while compiling `<memory>` and never for any of them, so the context that
produces a stand-in typed delete operand is not one a small program reproduces.
The change is backed by 5,549 tests, inception MATCH, and the four headers it
unblocks, and its condition is narrow enough to be low risk -- an operand only
has a `NAMED_TYPENAME_PARAMETER` type during shape analysis.  It should get a
fixture as soon as the context is understood well enough to reproduce.

### C7k -- the explicit instantiation that never saw its class

C7j's four-line reducer, fixed.  `AnalyzeExplicitFunctionInstantiation` built
the declarator in the namespace scope, so a parameter list naming the class's
own member types had nowhere to find them.  The out-of-class definition path
already does this correctly, and the recipe for a template-id qualifier was
already written a few hundred lines away in `arguments.cpp`: walk the
name components, and for one carrying a template argument list, resolve the
pattern, build its arguments, instantiate it, and take its `member_scope`.

One step had to be added to that recipe.  The owner comes back instantiated but
**incomplete**, so its `member_scope` is `kNoScope` and the members are still
invisible; an explicit instantiation is often the first thing in a translation
unit to name a specialization, so it has to complete it.  `ResolveOwner` remains
the fallback for a qualifier that is a plain class rather than a template-id.

Fixture `cppgm.tests/course/pa22/366-explicit-instantiation-member-type-scope.t`
carries a proven negative control -- without the change it reproduces
*unknown type name: value_type* -- and covers both spellings of the member
types, a parameter naming the class itself, and the matching explicit
instantiation definitions, so both sides of the declaration are exercised.

The seventeen remaining headers keep their single cause but move deeper into
`<string>`, from line 2503 to line 1060:
*structured template type was not found: `__is_allocator<_Allocator>::value`*.

### C7j -- the reducer that pointed at C7k

The seventeen headers that still fail share one cause, and it reduced to four
lines once the diagnostic could name a place.  Two refinements to C7d's
mechanism got it there: a guard whose node has no source range no longer
overwrites a located outer one, and the unknown-type-name diagnostic publishes
the specifier node it is actually complaining about.  That pointed at
`string:2503`, the `_LIBCPP_STRING_V1_EXTERN_TEMPLATE_LIST` expansion, whose
entries read

```
extern template void basic_string<char>::__init(const value_type*, size_type);
```

`value_type` and `size_type` are members of `basic_string<char>`, named
unqualified after the declarator-id, where C++ looks them up in the class's
scope.  The out-of-class *definition* does this correctly today; the
`extern template` explicit instantiation declaration does not:

```c++
template<class T> struct box { typedef T value_type; void init(const value_type *); };
template<class T> void box<T>::init(const value_type *) {}   // accepted
extern template void box<char>::init(const value_type *);    // rejected
```

`AnalyzeExplicitFunctionInstantiation` calls `BuildDeclarator` with the
namespace scope, so the parameter list never sees the class.  The fix is to
give it the qualifier's `member_scope`, which needs the nested-name-specifier
of a possibly template-id qualifier resolved first -- that is where this run
stops, with the reducer and the rule both in hand.

### C7l -- the last cause, pinned to three conditions

The single cause holding the remaining seventeen headers, reduced to fourteen
lines on the first try because the diagnostic now names a place
(`string:1060:27`).  libc++ writes

```c++
template <__enable_if_t<__is_allocator<_Allocator>::value, int> = 0>
basic_string(const _CharT* __s);
```

and the reducer is the same shape:

```c++
template<class C, class A> struct str {
  template<enable_if_t<is_alloc<A>::value, int> = 0> void f() {}
};
int main(){ str<char, int> s; return 0; }   // instantiating is what fails
```

Three conditions are each necessary, which is what makes this findable:

- The member template's **parameter type** must be a dependent alias-template
  use.  `enable_if_t<true, int>` is fine.
- Its argument must depend on the **enclosing class's** parameter, either of
  them.  A parameter of the member template's own list is fine --
  `template<class A, enable_if_t<is_alloc<A>::value, int> = 0>` compiles.
- The class must actually be **instantiated**.  Declared and never used, it
  compiles; `str<char, int> s;` alone is enough to fail, without calling
  anything.

It is not the replay that misclassifies it -- that was the first guess and it
is wrong.  Dumping the syntax with `--emit-ast` shows the working and failing
cases parse *identically*, and instrumenting the failure shows the
decl-specifier-seq being analysed has a single child: `type-name` with the
payload `is_alloc<A>::value`.  So the **argument was parsed as a type-id**, and
the failure is simply the first moment anyone asks what type it is.  The
working case never asks, because a member template depending only on its own
parameters is not replayed at class completion.

`ParseMatchedTemplateTypeArguments` asks `TemplateArgumentStartsType()`, which
returns true at `parser.cpp:591` because `is_alloc` is a known template;
`ParseTypeId` then happily consumes `is_alloc<A>::value` including the
`::value`, and the following `,` makes it look well formed.

N3485 14.6/3 says otherwise: a dependent qualified name without `typename` is
not a type, so that argument must be an expression.  This repo already applies
that clause elsewhere -- `pa19/332-dependent-value-direct-initializer` exists
for exactly it -- so the rule is settled; what is missing is a way to apply it
here.  The obstacle is that the parser has no notion of *dependence*: a
template type parameter carries the same `kKnownType` fact as `int`, so
`is_alloc<A>::value` and `is_alloc<int>::type` are indistinguishable at
`TemplateArgumentStartsType`, and the second is a genuine type that must keep
working.

Two ways out looked available.  **The first was tried and does not work**,
which is worth recording so the next attempt does not repeat it.

Giving the parser a `kActiveTypeParameter` fact -- set alongside `kKnownType`
in `ParseTypeTemplateParameter`, unwound in `ParseTemplate` exactly the way
`kActiveNonTypeParameter` already is -- and rejecting a parsed type-id whose
top-level qualifier mentions such a name is straightforward to write and
reduces the reducer immediately.  It does not survive the suite:

- The naive scan over the argument's tokens cost **210 tests**: it never
  unwound the fact, so every later `X<T>::y` in the file was read as a value.
- Scoping the fact to its template declaration and ignoring anything inside
  parentheses -- `bool_<(1 <= size<L>::value)>` is a type whose inner
  expression says nothing about the outer one -- brought that to **37**.
- Excluding `X::template f`, which names a template rather than a value, fixed
  those but broke a different set that had been passing, turning previously
  green tests into *structured template type was not found*.

Four iterations, oscillating between over- and under-applying, all in the
member-alias and template-template families.  The lesson is that a token scan
cannot stand in for dependence: the same spelling is a type, a value, or a
template name depending on what the qualifier resolves to, and the parser
cannot know that.  The change was reverted.

The second route was then instrumented rather than written, and it moved the
diagnosis again -- **this is not a disambiguation problem at all.**

`AppendTemplateArgument` already re-interprets a non-type argument whose syntax
is a type-id: `arguments.cpp:1615` calls `AnalyzeNamedValue` for exactly that
shape.  It is never reached here.  Printing the parameter each argument is
matched against while compiling the reducer shows `A`, `T`, `T`, `A` -- the
parameters of `is_alloc` and of the enclosing class -- and **never `B`**, the
`bool` parameter of `enable_if_t`.  The alias's own parameter kinds are never
consulted for its argument list on this path, so argument 0 is processed as
`TEMPLATE_ARGUMENT_TYPE`, `BuildCanonicalTemplateTypeArgument` builds
`is_alloc<A>` as a type, and `::value` is then looked up as a member type and
fails.

That reading was wrong too, and the correction is the useful part.  Probing
`FindAliasTemplateIndex` shows `enable_if_t` **is** found, with two arguments,
and `classes.cpp:391` does build them against `alias_pattern.parameters`.  The
earlier "never `B`" observation was an artefact of where the probe sat: it only
printed arguments taking the `TEMPLATE_ARGUMENT_TYPE` branch, and `B` correctly
takes the non-type one.  So the alias, its parameters, and the argument's
parameter kind are all right.

`arguments.cpp:1589` then looked like the answer.  It is a speculative probe --
"is this argument really a type?  then it is not a value" -- and resolving
`is_alloc<A>::value` as a type is exactly what fails there.  A probe that threw
instead of answering no would explain everything.  It is not that either:
instrumenting the throw site shows `CandidateSubstitutionActive() == 0`, so the
throw happens outside any substitution scope, and wrapping that probe in one
does not catch it.  Some other path builds the argument as a type first.

**Five investigations, each disproving the one before it.**  Every confident
statement of the mechanism has been wrong, including three in this document
before they were corrected.  The next attempt should therefore not begin with
another reducer or another printf: it should build a debug binary of the cell
and take a backtrace at `analysis.cpp:1647`, which identifies the caller
directly.  That is the one approach none of the five rounds substituted for,
and it is why each hypothesis survived only until it was tested.

### C7m -- found by the backtrace, in none of the five places

The previous entry said to stop guessing and take a backtrace.  Doing that
answered it in one step, and the caller was in none of the five places the
reducer-and-probe rounds had proposed.

A debug build of the default cell -- the reducer fails there too, so the
libc++ cell is not needed -- and a breakpoint at `analysis.cpp:1647` names the
chain immediately: `InternExpandedFunctionTemplateResult`, walking result
syntax references, reaches `result_identity.cpp:271` and calls
`BuildCanonicalTemplateTypeArgument` on the argument.

That call is **identity, not analysis**.  It asks whether a piece of syntax
happens to name a concrete type and falls through when it does not -- `kNoType`
is already handled two lines below.  It is guarded by
`!SyntaxUsesAnyTemplateParameter(node, dependent_names)`, and `dependent_names`
holds only this template's own parameters and its function parameter names.  A
member template whose parameter type is written over the **enclosing class's**
parameter therefore passes the guard, the syntax is built as a type, and
`is_alloc<A>::value` fails on the `::value`.

Every one of the three necessary conditions falls out of that: the enclosing
class's parameter rather than the member template's own, because only the
latter is in `dependent_names`; instantiating the class, because that is when
the result is interned; and a dependent alias use, because that is what makes
the parameter type a retained type-id reference here.

The fix is three lines: ask in the mode that reports a failure instead of
throwing one, using the `ScopedContainerPush` over
`candidate_substitution_failures_` that three other sites already use for
exactly this.  Fixture
`cppgm.tests/course/pa22/367-member-template-parameter-type-over-class-parameter.t`
has a proven negative control and carries both spellings -- over the class's
parameter, which failed, and over the member template's own, which already
worked and must keep working.

**23 of 38 headers, `<string>` among them.**  The one remaining cause split
into two: *inaccessible qualified type: string_type* for twelve, and
*duplicate or conflicting class member* for `iostream`, `ostream`, `sstream`.

**The lesson, since it cost five rounds:** a reducer tells you what fails and a
printf tells you what a value is, but neither tells you who called.  When two
successive hypotheses about a caller turn out wrong, the next step is a
backtrace, not a third hypothesis.

### C7n -- the two causes now in front, both already located

The backtrace habit was applied immediately to the larger of the two remaining
causes, and it is already placed.

**`inaccessible qualified type: string_type` (twelve headers.)**  libc++ writes

```c++
extern template __time_get_storage<_CharT>::string_type
__time_get_storage<_CharT>::__analyze(char, const ctype<_CharT>&);
```

and `string_type` is `protected` in that class, which is fine: the explicit
instantiation names it in a member's own return type, where the member's access
applies, and clang accepts it.  The chain is
`AnalyzeExplicitTemplateSpecialization` -> `BuildSpecifiers` ->
`LookupStructuredTypeSpecifier` -> `LookupStructuredName`, throwing at
`classes.cpp:372`.

The obvious reading -- that `ResolveOwner` fails on the template-id qualifier
and so never sets `current_class_context_`, exactly the C7k defect -- was tried
and **is not it**.  Factoring C7k's resolution into a shared helper and using it
here changes nothing.  The diagnostic, once it names the class, says why: the
naming class is `__cppgm_class_template_identity_436_101_0`, a synthetic
identity entity.  The *lookup* resolved `string_type` against the shape rather
than against the real `__time_get_storage<char>`, and access is then judged
against a class that has no access to anything.  That is the same identity
entity that C7e and C7g had to teach not to demand a layout, now leaking into
name lookup.  The experiment was reverted; only the located diagnostic is worth
keeping from it.

Probing narrowed it further, and the narrowing matters because it moves the
defect off the obvious suspect.  The lookup is **not** the retained
declaration-time one -- `FindFunctionTemplateResultLookup` is not the branch
taken -- it is a live `LookupQualified` whose *carrier* is the identity
entity's scope, reached from `classes.cpp:446`
(`carrier = ScopeForType(found.type)`) after `__time_get_storage<char>`
resolves to `__cppgm_class_template_identity_436_101_0`.

The important observation is that this is **normal**: compiling `<vector>`
produces 1,979 identity carriers and only this one fails.  So an identity
carrier is how the compiler routinely models a specialization here, and the
defect is not that the carrier is an identity -- it is that
`CanAccessMember(found.type_declaration, found.naming_class)` is being asked
about a *protected* member with an identity entity as the naming class, in a
declaration whose declarator-id names a member of that very class and therefore
has its access.  That question was then answered, and it narrows to one thing.

`current_class_context_` is **`<none>`** at the check, and the member's access
is `1` (protected).  The debugger puts the caller at `arguments.cpp:397`, in
`AnalyzeExplicitTemplateSpecialization`'s member branch, where
`BuildSpecifiers` runs **before** `current_class_context_ = entity` a few lines
below -- so the declared type, which belongs to the member as much as the
declarator does, is analysed from no class at all.

Moving the context assignment above the specifier build is correct on its own
terms and was written, but **it does not clear this case**: the probe still
reports `context=<none>`, so on this path `entity` itself is not yielding a
class.  That is the single remaining question for this cause -- what supplies
`entity` here, and why it is empty for
`__time_get_storage<char>::__analyze` -- and it is one probe away.  The
experiment was reverted rather than committed, since a correct-looking
reordering that does not fix its target and is not demanded by a fixture is
scope creep.

**`duplicate or conflicting class member` (three headers: `iostream`,
`ostream`, `sstream`)** is not yet located; `analysis.cpp:1136` and `:1271` are
the two throw sites.

That is where this run stops.

### C8 -- attempted, and it had a blocker of its own

C8 was run rather than predicted, and the prediction was wrong in a useful way.
It does not fail on the headers first: `make with-clang-libcxx-inception` stops
before reaching them with

```
ERROR: unsupported driver option: -stdlib=libc++
```

The selfhost build passes cppgm++ the same `CPPGM_STDLIB_FLAGS` it was
configured with, and cppgm++ did not accept `-stdlib=` at all.  That is a
driver gap independent of every header cause, and it would have blocked C8 even
with all thirty-eight headers compiling.

The fix keeps the plan's rule that the standard library is chosen at build time
and its include paths baked then, never scanned at runtime.  `-stdlib=` on the
command line can therefore only *agree or disagree* with that choice, and it is
now checked against the baked `kStdlibFlags`: a flag matching the build is
accepted, and one that differs is refused with a message naming what the
compiler was actually built for.  Silently honouring a mismatch would compile
against one library's headers while claiming another.  Both directions are
covered -- the default-cell binary refuses `-stdlib=libc++`, the libc++ cell
accepts it -- and the g++ and clang/libstdc++ cells are unaffected.

With that cleared, C8 reaches the headers and stops on the two causes C7n
records, plus one more not yet examined:
*unknown class member at `string:2092:76`*.  So C8's remaining distance is
exactly C7's remaining distance, which is what the plan assumed but had not
until now been demonstrated.

### C7o -- the access that was judged from no class

The `string_type` cause, fixed, and the previous three entries about it were
each wrong in a way the debugger settled in one step -- the same lesson C7m
taught, relearned because it was not applied soon enough.

An explicit instantiation names its member after the class, and the member's
**declared type** is named with that member's access, exactly as its declarator
is.  libc++ declares

```c++
extern template __time_get_storage<char>::string_type
__time_get_storage<char>::__analyze(char, const ctype<char>&);
```

where `string_type` is `protected`.  Two functions build such a declaration's
specifiers outside the class: `AnalyzeExplicitTemplateSpecialization` set
`current_class_context_` only *after* the specifier build, a few lines above the
declarator; and `AnalyzeExplicitFunctionInstantiation` -- the C7k function --
never set it at all and built the specifiers in the namespace scope.  Both now
enter the class first, the latter reusing the owner scope C7k already resolves,
with `ScopedValueRestore` so the restore is not a stray assignment.

The reason this took four entries is worth keeping: fixing the first function
made the *second* one throw, from a different file, with the same message.  Each
round therefore looked like "the fix did nothing" when it had in fact moved the
failure one caller sideways.  A breakpoint distinguishes those two outcomes
immediately; a printf at the throw site does not.

**33 of 38 headers, up from 23.**  One cause remains, on five:
*duplicate or conflicting class member* in `fstream`, `iomanip`, `iostream`,
`ostream`, `sstream`.

### C7p -- the last cause, located on the first try

The debugger was reached for first this time rather than after five rounds, and
the cause was placed immediately.  `declarations/analysis.cpp:1136` refuses to
add `__bits_per_word` to `std::__bitset<0, 0>` because a `BIND_VARIABLE` of that
name already occupies the class scope; the chain is `LookupStructuredName` ->
`EnsureClassDefinition` -> `CompleteClassTemplateSpecialization` ->
`AnalyzeClass` -> `CompleteClassDefinition` -> `AnalyzeClassMember`.

The obvious reading -- a class completed twice, its members added twice -- is
**wrong**, and checking cost one probe.  Instrumenting every completion shows
`__bitset<0, 0>` completed exactly once (binding 45698, index 420, entity 1434,
scope 27027).  Repeated completion of one binding does happen elsewhere and is
evidently normal, but not here.  So the class scope is **pre-populated** before
its completion runs, by a path that is not
`CompleteClassTemplateSpecialization`, and identifying that path is the whole of
what remains.

Three reducers of the apparent shape -- a full specialization repeating the
primary's member, with a self-friend, with a self typedef -- all compile, which
by now is the expected result, so the binding itself was instrumented instead.

That settles the mechanism.  `__bits_per_word` is added to scope 27027 exactly
once, and the two `AnalyzeClassMember` calls come from **different places**:

- the first, which succeeds, from
  `AnalyzeExplicitTemplateSpecialization` -> `AnalyzeClass` ->
  `CompleteClassDefinition` -- the explicit specialization
  `template<> class __bitset<0, 0>` being defined at its point of declaration;
- the second, which throws, from `EnsureClassDefinition` ->
  `CompleteClassTemplateSpecialization` -> `AnalyzeClass` -- the template
  machinery completing the same class again.

An explicit specialization owns its member declarations, and defining it already
ran them; completing it again adds every member a second time.  The other two
completion sites already refuse this, testing
`class_template_explicit_specialization_states_`; the `EnsureClassDefinition`
site does not.

Adding that guard there is written and **does not work yet**, and the reason is
the next step rather than a puzzle: at that site neither
`class_template_explicit_specialization_states_[declaration]` nor the entity's
own `explicit_template_specialization` flag is set for `__bitset<0, 0>`
(`expl=0`, `complete=0`, entity 1434).  Both are set together in
`arguments.cpp:704-710`, on the branch that handles a specialization it
recognises -- so the question is why this class specialization does not reach
that branch, or reaches it under a different binding.  The experiment was
reverted; the guard is correct and is waiting on the flag that should already
be true.

### C9 -- three of its four items were never gated on C7

Having been wrong about C8's reachability, the same question was asked of C9
rather than assumed, and three of its four bullets turned out to need nothing
from C7:

- **Lanes.**  Each cell was already one command by prefix; `make test-cells`
  now walks all three, keeps going past a failing cell and names which failed,
  so "does this hold everywhere" is one thing to type.
- **PA34's readme states the new surfaces.**  The three GNU/Clang concessions
  the new fixtures test are now written there: the `aligned` attribute taking a
  constant expression that may not be knowable until instantiation, a hosted
  type specifier introducing a functional cast, and `_Atomic` not being
  `volatile` for the literal-type rule.
- **The discovery rule is written in the readme that owns it.**  PA34 now says
  the host answers are baked when `cppgm++` is built and never probed at run
  time, why (a compiler that decided where the standard library lives each time
  it ran would depend on whatever compiler happened to be installed), and the
  consequence a student meets directly: `-stdlib=` may agree with the build but
  cannot change it.

The fourth, a report of the **final** inventory, is the only part that waits on
C7.  `doc/clang-libcxx-report.md` records the inventory as it stands, what each
fix bought, the two costs that were not fixes, and what remains -- so the shape
is in place and the numbers are the ones that change.

## State at the end of the second run

Twenty-one of thirty-eight libc++ headers compile, up from seventeen, and the
five causes this run began with have become one.  Every increment kept
`make test-report-through-pa38` green -- 5,543 at the start, 5,550 now -- with
`make inception` MATCH after each, and each capability has a fixture in the
assignment whose surface owns it, except the delete deferral, whose missing
fixture is recorded as debt in C7i.

The one remaining cause is `__is_allocator<_Allocator>::value` at
`string:1060`, a dependent structured template type.  C8 and C9 sit behind it.

## State at the end of the first run

Landed: C0, C1, C2, C4, C5, C6, the working half of C3, plus C0a, which the
plan did not anticipate.  `make test-report-through-pa38` is green at 5542/5542
in the default cell after every increment, and every fix has a fixture in the
assignment whose stated surface owns it.

The clang/libstdc++ cell is done: 38 of 38 headers, 5527/5527 through PA38, and
inception MATCH.  The clang/libc++ cell went from 0 to 17 of 38 headers, and
its five causes became three.

Three causes hold the remaining 21, each narrowed to a single header that
reproduces it alone: `__memory/unique_ptr.h` for the dominant one,
`__charconv/traits.h` for the 128-bit one, and `<set>`.  All three resist
hand-written reducers of the construct, which by now is the expected shape --
the construct is rarely the problem.

On the dominant one, gdb has taken it further than reduction did.  The failing
expression is
`__is_replaceable_v<pointer> && __is_replaceable_v<deleter_type>` at
`unique_ptr.h:149`, and **both** operands come back non-constant, with
constant evaluation not suppressed.  Reading the same variable template
outside that context is constant: `static_assert(std::__is_replaceable_v<int*>)`
compiles against the real header.  So a variable template specialization read
inside a class template's own completion does not produce a constant, while the
same read elsewhere does -- which is the same shape as the C7 defect, where
completing a class inside another operation inherited state that was never
about it.  Ten reducers of the construct have all compiled; what is missing is
the surrounding condition, not the construct.

gdb then took it to the mechanism.  Both operands are non-constant because
`InstantiateVariableTemplate` only analyses the initializer when the arguments
are not dependent, and here `pointer` is still `_Tp*`: the class is being read
as a template definition, so `__is_replaceable_v<_Tp*>` is genuinely dependent
and the `&&` genuinely has no value yet.  The correct answer is to keep the
argument as dependent, and that is where the model runs out: `TemplateArgument`
carries a `dependent_parameter` ordinal, which can say *this argument is
parameter N* but not *this argument is an expression over parameters*.  The
throw at `arguments.cpp:1662` is what happens when an argument is dependent in
a way the structure cannot hold.

So this is a missing capability rather than a defect: **a dependent non-type
template argument that is a compound expression**.  `TemplateArgumentKind` has
only `TYPE`, `INTEGRAL` and `TEMPLATE`, and a dependent non-type argument keeps
a deduction *shape* in `type` alongside a single `dependent_parameter` ordinal.
An expression over parameters has no ordinal and no shape, so there is nowhere
to put it.  Giving it one is a feature with its own design -- the
nondeduced-shape replay machinery in `InstantiateVariableTemplate` is probably
where it belongs -- not an increment on the way to something else.

It does now have an eighteen-line reproducer that clang accepts, which took
the include-reduction plus two ddmin passes to find and which no amount of
writing the construct by hand produced:

```c++
template <bool B, class X, class Y> struct conditional { typedef X type; };
template <class X, class Y> struct conditional<false, X, Y> { typedef Y type; };
template <bool B, class X, class Y> using cond_t = typename conditional<B, X, Y>::type;

template <class T> struct rep { static const bool value = true; };
template <class T> inline const bool rep_v = rep<T>::value;

template <class T, class D> struct holder {
  typedef T* pointer;
  typedef D deleter_type;
  using kind = cond_t<rep_v<pointer> && rep_v<deleter_type>, holder, void>;
};

template <class A, class B> struct helper {};
template <class T> struct hash;
template <class T, class D>
struct hash<helper<holder<T, D>, typename holder<T, D>::pointer> > {};
```

The last three lines are the whole trigger, and they are why every hand-written
reducer passed: naming `typename holder<T, D>::pointer` in a partial
specialization's argument list forces `holder<T, D>`'s members to be analysed
with its own parameters as arguments, so `kind` is formed while `pointer` is
still `T*`.  Without that, nothing ever asks for `kind` in a dependent context.
It is not a fixture: we reject code clang accepts, so a `-bad-` fixture would
enshrine the wrong answer.  It goes in the suite when the capability lands.

Narrowing it further changed the verdict, so the "missing capability" reading
above is wrong and is left standing only because the correction is worth
reading.  Replacing the `&&` with a single `rep_v<pointer>` fails identically,
so nothing about compound expressions is involved: **a dependent variable
template read is not accepted as a non-type template argument**.

And the machinery for that already exists.  `AppendTemplateArgument` has a
`retained_dependent` path that marks such an argument with
`FunctionTemplateNondeducedTypeShape()` and `kNondeducedTemplateParameter`,
which is exactly the representation the earlier note claimed was missing.  It
is gated on `source_dependent_names != 0`, and the alias-template branch of
`LookupStructuredName` -- the path `cond_t<...>` takes -- calls
`BuildTemplateArguments` without that set.  `MaterializeTemplatePartialArguments`
builds one from its own parameters and passes it; the alias branch has no
equivalent.

Supplying the in-scope template parameter names on that path was the obvious
fix, and it was tried: a `Program::CollectTemplateParameterNames` walking the
enclosing `SCOPE_TEMPLATE_PARAMETERS` scopes, passed to the alias branch.  It
does not work, and the reason is worth keeping.  `retained_dependent` decides
by asking `SyntaxUsesTemplateParameter` whether the argument's *syntax* names a
parameter, and here it does not: the argument is `rep_v<pointer>`, where
`pointer` is a member typedef.  The dependence is semantic, not lexical.

A semantic test was tried next -- retain when the expression reads a variable
template specialization that still has no value, which is exactly the dependent
case -- mirroring the existing retention including the bind.  Retention then
happens, and the failure moves to *invalid implicit conversion from
`__function_template_nondeduced_type_shape` to bool*: the retained argument
reaches `conditional<B, X, Y>` and the class specialization machinery converts
it to the parameter's type rather than staying dependent.  Both attempts are
reverted.

Using `ClassTemplateNondeducedTypeShape()` instead of the function one fails
identically, so the conversion is downstream of which shape is chosen: the
alias `cond_t` substitutes into `conditional<B, X, Y>`, and *that* argument
list converts the retained shape to `bool`.

That is the confirmed shape of the work, no longer a guess.  Three attempts
each moved the failure rather than fixing it, and where each landed is the
useful part: the argument side can produce a nondeduced shape, but every
template-id the retained argument then flows through has to pass it along
rather than convert it -- for `cond_t<...>` that is at least the alias
substitution and the `conditional` specialization behind it.  The function
template path already does this; the class path does not.  That is where the
next run starts, and it is what C8 and C9 sit behind.
  Two of the five below are done and are
kept with their findings; the numbering is the original order:

1. `__builtin_frexpf` and the rest of the non-uniform libm signatures — 8
   headers.  The shape of the work is known and written up under C5.
2. `expected parameter declaration` at `__functional/mem_fun_ref.h:27:8` —
   **done**.  Nothing was wrong with the construct, which is why eight
   hand-written reducers all passed: parser name facts are keyed by spelling,
   and libc++ names a template template-parameter `_Sp` in
   `__pointer_traits_rebind_impl` and then reuses `_Sp` as an ordinary type
   parameter.  The stale template kind stopped `_Sp (` from starting a
   declarator, so the parenthesis parsed as a parameter clause.  A type
   parameter now says which kind it is rather than only adding to whatever the
   spelling already carried.  PA10 owns it; the reducer is three lines.  This
   and the `pointer_to_binary_function.h` sibling both went, moving 13 headers
   past their blocker.
3. `expected OP_SEMICOLON` at `__hash_table:859:19` — **done**.  A member
   function body is a complete-class context (N3485 3.3.7/1) and the class
   body pre-scan predeclared the class-key and enum member names for that
   reason but not the alias names, so a body naming one parsed it as an
   expression.  `__hash_table` uses `__node_holder` eighty lines before
   declaring it.  Fixed in `af7a988d` with a PA11 fixture; the reducer is five
   lines.  Causes four to three.
4. `invalid PA11 type identity` — now 18 headers, since the parser fixes above
   moved the others onto it.  A gdb break on the throw named it in one step:
   `RegisterFunctionTemplatePattern` asks `IsFunction` about a declarator whose
   type is `kNoType`, and `TypeTable::Get` answers an absent type with an
   internal error, losing the diagnostic that check exists to give.  It reports
   the semantic error now.

   Locating it took two goes.  The declarator, the specifiers and the target
   all lack a token range, and `SourceFile` read `tokens_[0]` for a node with
   no range, so the first attempt confidently named whichever header came
   first: `<locale>` was blamed on `__cstddef/size_t.h`, which contains no
   template at all, and that sent me looking at variable templates in
   `desugars_to.h` for a while.  An absent range says nothing now, and the
   enclosing template declaration -- which does carry a range -- is what gets
   reported.  All eighteen headers name the same construct:
   `__memory/unique_ptr.h:185:3`, the constructor template
   `template <bool _Dummy = true, class = _EnableIfDeleterDefaultConstructible<_Dummy>>
   constexpr unique_ptr() noexcept : __ptr_(), __deleter_() {}`.

   Four reducers of that shape all compile, so the trigger is again elsewhere
   in the unit.  What is now known: it is a special-member template whose
   declarator produces no type, inside a class template specialization being
   completed, and `<__memory/unique_ptr.h>` on its own reproduces it, so
   nothing before it is needed.

   **Reducing it cannot go through `clang -E`,** which is worth writing down
   because it cost a detour.  A preprocessed unit bakes in *clang's* answers to
   `__has_builtin`, and libc++ guards a good deal of itself on those: it takes
   the builtin branch of `is_nothrow_destructible`, `is_trivially_relocatable`
   and others that we answer no to and that libc++ therefore never asks us to
   compile.  The preprocessed form then fails on a construct the real header
   never presents.  That also means those traits do not need implementing: the
   fallbacks are there precisely because a compiler may lack them.  Reduction
   has to work on the unpreprocessed header, which is slower because every
   candidate re-parses the whole include set.  Reducing the includes first
   (43 down to 4) makes each step cheap but costs clang's validity check, since
   the dropped headers are ones clang needs, and reducing with our own error
   alone over-reduces to malformed input that produces the same message.  Both
   oracles are needed, on the full include set.

   A separate real defect turned up while probing this one:
   `__attribute__((__aligned__(::ns::alignment_of<D>::value)))` is rejected
   with *invalid aligned attribute argument* where clang accepts it, because
   attribute arguments are captured as token spellings rather than expressions
   and anything but a literal is refused outright.  libc++ writes exactly that
   through `_LIBCPP_COMPRESSED_PAIR`, and a twenty-line reducer reproduces it.
   Accepting it does **not** clear the `unique_ptr` failure -- checked by
   trying -- so it is a parallel gap, and it wants a real constant evaluation
   rather than ignoring the alignment, which would silently change layout.
5. **(Confirmed in the second run as C8's blocker.)** The dependent
   conditional-explicit condition from C3, which no header
   currently stops on but `std::pair` needs for correct copy initialization.

C6 through C9 -- real translation units, the self build, inception, and the
close -- all sit behind those.

## State at the end of the third run

Three commits on top of the second run's head, each gated on the full default
suite and on `make inception`:

- `6219f22c` **Bound an explicit instantiation to the members it names.**  The
  libc++ link failures had one root: the preprocessor answered
  `__has_attribute(__exclude_from_explicit_instantiation__)` with 0, so
  `_LIBCPP_HIDE_FROM_ABI` fell back to always-inline and every hidden member
  of an `extern template` class was treated as covered by the declaration.
  The probe is answered now; the attribute is carried on the binding; an
  explicit instantiation declaration suppresses only the members it names --
  out-of-class definitions and nested-class members -- and never an excluded
  member, an implicit special member, or an in-class definition.  Conversion
  functions, which had no attributes at all, get them.  Fixtures in PA34
  (run) and PA15 (LowIR).
- `dd012371` **Make an inline variable one object across translation units.**
  A namespace-scope `inline` variable was given internal linkage by the
  `const` rule and each unit got its own copy; it is weak now.  PA34 fixture
  with two units.
- `9c8c85d4` **Copy nothing for an empty class.**  libc++'s
  `_LIBCPP_COMPRESSED_PAIR` places a `[[no_unique_address]]` empty member at
  offset 0 of an anonymous struct, and copying that empty member wrote a byte
  over the member that shares the offset; an `unordered_map` copy then
  crashed.  `EmitClassObjectCopy` emits nothing for an empty class.  Fifty-nine
  LowIR references lost exactly their `copyobj 1x1` lines and nothing else,
  checked by diff; PA15 fixture.

**C8's exit is met at the default level: `make with-clang-libcxx-inception`
reports MATCH** for all 224 objects and the compiler, twice, the second time
from a clean rebuild of the committed tree.  The -O3 remainder the second run
left (23 `missing lowered temporary`, 3 EH state mismatches) was cleared by
`04b4754c` and `00c11516` before this run.

The cell is not finished, and the list is exact:

1. **-O1 inception fails**, on one object, `semantic/model/program.o`: in
   `Program::EnsureVisibleName` the self-built `-O1` compiler keeps `%r8` live
   across a call to `operator delete` and stores through it afterwards.  It
   reproduces standalone with `dev/cppgm++ -c -O1` on the retained LowIR of
   that function; the reduced input, the mixing script, and an objdump
   checker that finds caller-saved reads after a call are in
   `$HOME/perf-libcxx/o1-bug/`.  Ruled out by instrumentation:
   `recolor_call_free_callee_saved` (never fires), `rewrite_local_operands`,
   the whole MIR optimizer (`CPPGM_NO_MIR_OPT` still shows it), deferred
   `OP_DEREF` formation, and any `set_value` placing a value in a caller-saved
   register with `crosses=1`.  The faulting address is a rematerialised
   constant index (`OP_IMM`, `deferred=1`) whose base should be re-materialised
   at each use by `emit_rematerialized_index_base` and is not.  The default
   cell's `-O1` inception matches, so the shape is libc++-specific.  PA38 owns
   the fixture.
2. **The libc++ suite is 5,541 of 5,555.**  Fourteen failures, five causes,
   none of which the compiler's own sources exercise:
   - five PA36 link tests (`600-hosted-string-assign-scope-guard-link-smoke`,
     `700-hosted-ostream-char-sequence-parameter-runtime-smoke`,
     `700-hosted-ostream-ctor-entry-coalescing`,
     `700-hosted-string-move-assignment-runtime`,
     `700-hosted-vector-string-pushback-link-smoke`): *undefined native
     symbol* for a `basic_string<char>` constructor whose emitted name still
     carries a `__cppgm_class_template_identity` placeholder, at every
     optimisation level.  A constructor template of an `extern template`
     class, most likely the `enable_if`-defaulted `basic_string(const _CharT*)`;
     it is the same family as `6219f22c` and the first thing to reduce.
   - PA32 `200-host-extern-template-vtable-reference`: the compile fails
     through the harness; the name says which family.
   - two PA35 compile tests (`600-random-to-address-qualified-call`,
     `700-hosted-random-mersenne-rshift-compile`): parse error at
     `__random/shuffle_order_engine.h:151:52`, where
     `__enable_if_t<(__uratio<_Np, _Dp>::num > 0xFFFF...ull / (_Max - _Min)), int>`
     has a `>` inside parentheses that the parser takes as the end of the
     template argument list.  N3485 14.2/3 says the first *non-nested* `>`
     ends it.  PA10 owns the fixture; `<random>` is not among the 38 headers
     the inventory tracked, which is why it was never seen.
   - PA35 `700-hosted-codecvt-wstring-convert-char16-compile`: *duplicate
     native object-symbol label*.
   - PA35 `700-libstdcxx-regex-compiler-member-alias-call`: *no viable
     overload for `__match_at_start_ecma`* in libc++'s `<regex>`.
   - four of seven PA37 `object-roundtrip` cases fail in this cell; not yet
     looked at.

   The other two cells were re-run today at this tree: clang/libstdc++
   5,572 of 5,572; the default cell 5,571 of 5,571 after the reference edits.
3. **The performance gates were not run for the three commits.**  Frozen-TU
   Ir and the inception CPU lanes are owed before anything else lands on top.
4. **`doc/clang-libcxx-report.md`** carried "gated on the self build" for this
   cell; its inventory table is corrected in this run and the narrative for
   these three fixes lives here.

### C10. Remaining to completion

In the order they should be taken:

- C10a. **Done.**  `516fe8c7` against `9c8c85d4`, frozen translation unit
  at -O1: Ir **+0.115%** (gate 0.5%); paired wall +1.14%, user +0.73%, RSS
  +0.07%, three ABBA blocks.  The Ir lane decides at this size; the gate is
  met.
- C10b. **Done** (`4e594d82`).  Not the constructor family at all: the
  symbol the five PA36 links missed was `basic_string::__fits_in_sso`,
  reached only through `__builtin_constant_p(len) && !__fits_in_sso(len)`
  in `operator==(const basic_string&, const char*)`.  Our front end folds
  `__builtin_constant_p` to 0, the right operand is analyzed with constant
  evaluation suppressed, and the resolved-call demand rule read that as
  "unevaluated" -- so the in-class inline member was never emitted while
  the lowering emitted the call.  A short-circuited operand keeps its
  callees now; only an unevaluated operand demands nothing.
- C10c. **Done.**  Two native-backend defects, not one: a replayed index base
  retired at its last direct use (`7b97d1e9`) and an in-place `v op v` on a
  frame home (`83f44163`), both reproducible from plain LowIR at -O1.
  `make with-clang-libcxx-inception INCEPTION_SELFHOST_OPT_LEVEL=1`
  **MATCHes** at `83f44163`; the -O1 gold-standard ratio is 1.747x.
- C10d. **Done**, seven commits `4e594d82`..`9cc4c6ee`, each with its
  fixture in the lane that owns the shape:
  - the `>` inside parentheses (`af29ab3f`): the parser stopped an
    expression at `>` while any template-argument list was open and a
    parenthesised operand lowered that depth by one only, so a list two
    deep (`shuffle_order_engine`'s template parameter clause) still ended
    early; the stop is suspended inside parentheses, call arguments,
    subscripts, braces, decltype and array bounds at any depth (PA10, ref
    from the reference parser);
  - the duplicate object-symbol label (`3f29050f`): the deleting-destructor
    entry derived its symbol by replacing `D1E` with `D0E`, which an ABI tag
    (`D1B8ne210108Ev`, every `_LIBCPP_HIDE_FROM_ABI` virtual destructor)
    defeated, so two entries shared one label (PA36);
  - the `<regex>` overload (`eda9d8cb`): `operator~` was not classified,
    so it never entered the enum-operator index ADL consults and was
    mangled as a source name instead of `co`; `~regex_constants::__full_match`
    inside namespace std has only ADL to find it (PA36, host-built
    definition linked under the correct name);
  - behind it, two more in the same test (`ec03db8c`, `9cc4c6ee`): the
    statement sequencer dropped every statement after a terminator up to
    the end of the sequence, so `thrower(); break;` in a case lost the
    following `default:` and the switch's default block stayed unfilled
    ("unbound native local label"); and `__builtin_ctzg`, which
    `__countr_zero` calls, was missing from the builtin registry (PA34 run
    fixtures for both);
  - the PA37 object-roundtrip cases (`1caa5e61`): the LowIR text writer
    rendered integer operands from their low word, so libc++'s
    `unsigned __int128` `__pow10_128` table lost its high halves through
    `--emit-lowir`; the writer is type-directed now and an `i128` prints
    both words (PA37 O0 and roundtrip fixtures);
  - the "extern-template vtable" PA32 entry of the earlier list does not
    exist under that name in the suite; the fourteen failures were the
    thirteen above plus the roundtrip cases.
- C10e. **Suites done**: `make test-cells` reports **5596 / 5596 in all
  three cells** at `ba29256c` (default, clang/libstdc++, clang/libc++),
  the first fully green libc++ cell.  Inception in all three cells at
  both levels: see the ledger of `PLAN-LIBCXX-PERFORMANCE.md` for the
  timing run at this tree.

Performance in this cell is its own plan: `PLAN-LIBCXX-PERFORMANCE.md`
records the decomposition -- the front end re-lexing every guarded libc++
header, and three loop shapes libc++ hands the optimizer that libstdc++ does
not -- and the program that takes the self-built ratio from 2.4x to 1.5x.
