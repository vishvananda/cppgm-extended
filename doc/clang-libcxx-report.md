# Clang and libc++ support: inventory and costs

Records where `PLAN-CLANG-LIBCXX-SUPPORT.md` stands and what each fix bought.
Numbers are from the end of the second run; the plan's execution record carries
the reasoning behind each entry.

## Inventory

| Cell | Standard headers | Suite | Inception |
| --- | --- | --- | --- |
| g++ / libstdc++ (default) | 38 of 38 | 5,596 / 5,596 | MATCH at -O1 and -O3 |
| clang / libstdc++ | 38 of 38 | 5,596 / 5,596 | MATCH |
| clang / libc++ | **38 of 38** | **5,596 / 5,596** (since `ba29256c`) | **MATCH at -O1 and -O3** (since `83f44163`) |

(Counts as of the third run, 2026-09-04, tree `9c8c85d4`; the earlier runs'
counts are in the sections that recorded them.)

`make test-cells` walks all three in one command.  The header inventory is
`scripts/probe_hosted_headers.pl`, which compiles each header the compiler's own
sources include as its own translation unit, so a count moves as fixes land
instead of stopping at the first failure.

## What the libc++ work cost

The clang/libstdc++ cell was finished in the first run.  The libc++ cell went
from 0 to 17 headers there, and from 17 to **all 38** in the second run, during
which its five distinct causes were cleared.  Each entry below is one commit, gated on
the full suite and on `make inception` before it landed.

| Fix | Bought |
| --- | --- |
| Retain dependent variable template values across operators | The dominant cause's first half; 18 headers advance |
| Evaluate GNU aligned attribute arguments as expressions | The same 18 advance again |
| Name a source location in semantic diagnostics | No headers directly, and the largest single lever on everything after it |
| Accept hosted type specifiers in a functional cast | `ostream`, `sstream` join the main group; 3 causes become 2 |
| Tolerate a shape-only member when completing a dependent specialization | The array bound clears; 20 headers advance |
| Skip member layout during a shape-only completion | `unique_ptr.h` compiles; 21 headers share one cause |
| Treat `_Atomic` as literal when judging a constexpr constructor | `<set>`'s long-standing cause clears |
| Defer a delete whose operand type is a parameter stand-in | 17 to 21 headers |
| Build an explicit instantiation declarator in its class scope | `<string>`'s first cause clears |
| Let result identity ask for a type without throwing | 21 to 23 headers, `<string>` among them |
| Enter the class before building an instantiated member's type | 23 to 33 headers |
| Mark an explicit specialization before analysing its body | 33 to 38 -- every standard header compiles |
| Check a command-line `-stdlib` against the baked one | C8 reaches the headers instead of stopping at the driver |

Two costs are worth naming because they were not fixes.  A parser-fact route for
the last C7 cause was written and reverted after four iterations (210 tests
failing, then 37, then a different set); the plan records why a token scan
cannot stand in for dependence.  And five successive explanations of that same
cause were each wrong until a debugger backtrace settled it in one step -- the
lesson, now in the plan, is that a reducer says what fails and a printf says
what a value is, but neither says who called.

## What remains

Every standard header now compiles.  What remains is the **self build**, which
uses more of the library than the headers do on their own: compiling
`macro_operator_code.cpp` against libc++ stops at

```
unknown class member __is_long_ in
struct std::__1::__cppgm_class_template_identity_189_66_0::__long
  at string:2092:76
```

`__builtin_constant_p(__rep_.__l.__is_long_)` resolves `__rep_.__l` to the
nested `__long` of `basic_string`'s **identity shape** rather than of the real
specialization, and the shape's nested class has no members.  It is the same
identity leak as the `string_type` cause, one level further in, reached through
`AnalyzeCondition` -> `AnalyzeBinary` -> `AnalyzeCall` ->
`TryAnalyzeImmediateBuiltinCall`.

Probing narrows it to one statement.  This is **not** inside a shape
completion -- both `dependent_shape_completion_depth_` and
`class_template_completion_suppressed_depth_` are zero at the failure -- but
`current_class_context_` *is* the identity entity.  So a member function body
belonging to the identity shape is being analysed, and inside it `__rep_.__l`
names the shape's nested `__long`, which has no members.  The frames above settle what that body is being analysed *for*, and it is not
identity work: `EmitDemandedFunction` <- `CompleteTranslationUnitDemand`.  The
function was demanded for **emission** -- it is odr-used and its body must be
generated -- and the demand names the identity shape's member rather than the
real specialization's.  So the shape is not merely carrying a body; it is
carrying one the translation unit intends to emit, whose enclosing class has
empty nested classes.

That check was run, and its answer rules out the fix that looked likeliest.
Instrumenting every demand of `__is_long` in that translation unit shows the
identity shape's member demanded **twice and the real specialization's never**.
So there is nothing to redirect the demand *to*: by the time emission is
demanded, the object's class is already the shape.  The demand is a symptom.

The cause is therefore one step earlier, and tracing the demands to the first
one names it.  The earliest shape member demanded in that translation unit is
`identity_184_57_0::operator()`, reached through
`TryAnalyzeCallOperator` -> `TryAnalyzeOverloadedOperator` ->
`BuildResolvedCall`: an object whose type is an identity shape is **called as a
functor**, `operator()` is resolved on the shape, and every demand after that
cascades from its body.

So the whole cause reduces to one thing: a call expression whose callee object
has a shape type where it should have the specialization's.  Everything
downstream -- the shape's `__is_long` demanded twice, the real one never, the
body analysed in the shape, `__rep_.__l` naming an empty nested `__long` -- is
that single substitution propagating.  Naming the shape's underlying template identifies the construct: it is
`std::__scalar_hash`, called with two arguments.  libc++ declares its primary
without ever defining it --

```c++
template <class _Tp, size_t = sizeof(_Tp) / sizeof(size_t)>
struct __scalar_hash;
```

-- and defines only the partial specializations for 0 through 4, selected by
that **computed** default argument.  So the failure is a partial specialization
that is not being selected: the callee gets the undefined primary's identity
shape, whose `operator()` is then resolved and demanded, and the rest cascades.

A standalone reducer of that shape -- a primary declared but not defined, two
partial specializations, a `sizeof`-derived default argument, called as a
functor -- compiles correctly, which by now is the expected result for this
subsystem.  Inspecting the selection rules that out too.  When
`SelectClassTemplatePartial` runs for `__scalar_hash` it is given concrete
arguments -- `(_PairT, 2)`, `(long double, 2)` -- `ClassTemplateArguments\
AllowPartialSelection` returns true, all five partial specializations are
considered, and exactly **one** matches each time.  Selection is not broken.

The whole "identity shape" framing was then found to be wrong, and correcting
it produced the cause.  `__cppgm_class_template_identity_N_M_P` is not a shape:
`ClassTemplateSpecializationStorageName` gives *every* class template
specialization that internal storage name.  Nothing was leaking a shape -- these
were ordinary specializations all along, and the question was only ever why one
of them was missing a member.

libc++ writes `basic_string`'s `__long` and `__short` with an **anonymous
struct** inside each:

```c++
struct __long {
  struct _LIBCPP_PACKED {
    size_type __is_long_ : 1;
    size_type __cap_ : sizeof(size_type) * CHAR_BIT - 1;
  };
  size_type __size_;
  pointer __data_;
};
```

A twenty-three line reducer reproduces the failure exactly: the same shape at
namespace scope compiles, with or without the packed attribute, and fails as
soon as the enclosing class is nested in a **class template**.

Instrumenting it found the defect, and it was not replay at all.  The injection
*does* run for the instantiated class; it reads back an **empty** member list
(`variants=0, complete=0`) because the anonymous class is not complete yet, so
it injects nothing and says nothing.  At namespace scope the anonymous class is
already complete by that point (`variants=2, complete=1`), which is why only the
nested-in-a-template case fails.  Completing it first is the fix, and it is
fixtured; the defect does not need bit fields, which the fixture avoids because
reading one diverges from `cppgm++-ref` on load signedness.

### C8, as it now stands

With that landed, `make with-clang-libcxx-inception` gets deep into the self
build and stops on **three** causes, all in libc++ headers the isolated-header
probe does not exercise this way:

- `nonconstant noexcept expression` at `__vector/vector.h:1390` -- the C++11
  branch of `vector::swap`, whose specification is
  `!propagate_on_container_swap::value || __is_nothrow_swappable_v<allocator_type>`,
  a dependent `||` over a variable template.  Same shape as C7a, now in a
  noexcept-specification.
- `structured template type was not found: __is_invocable_v<_Args...>` at
  `__type_traits/invoke.h:110`.
- `ambiguous overloaded operator` at `__ostream/basic_ostream.h:564`, and a
  `no viable overload` in `__algorithm/move_backward.h:44`.

Attributing them needs a **serial** build: the parallel log interleaves compile
lines with errors, and the TUs it appears to name compile cleanly on their own.
`make with-clang-libcxx-inception -j1` attributes them properly, and the first
failing unit is `semantic/constants/address_evaluator.cpp`.  It reproduces in
one command, which is where the next session should start:

```
cd pa39 && ../dev/cppgm++ -std=gnu++11 -Wall -O3 -stdlib=libc++ \
  -I../dev/src -I../obj-clang-libcxx/pa39/generated \
  -c -o /tmp/ae.o ../dev/src/semantic/constants/address_evaluator.cpp
```

Note that the error it gives that way -- *invalid implicit conversion* between
two `unordered_map` specializations at `unordered_map:1167` -- is not the one
the build reports for the same file, so the flag set matters and the build's
own command line (it adds `-DCPPGM_DEFAULT_HOST_CXX`, `-DCPPGM_DEFAULT_OBJECT_ROOT`
and the dependency-file options) should be used verbatim.  Two libc++ failures
are reachable from one translation unit, which is the useful part: the self
build is no longer a wall, it is a queue.

The front of that queue is now named.  Rendering both sides of the failing
conversion shows they are both `std::pair`, differing only in their first
argument -- two different iterator specializations:

```
from pair<__hash_iterator-ish, bool>
to   pair<__hash_map_iterator-ish, bool>
```

That is `unordered_map::insert` returning `__table_.__emplace_unique(...)`,
whose `pair` is over the table's iterator, as a `pair` over the map's iterator.
The conversion is legitimate and goes through `pair`'s **converting constructor
template**, and printing the target's constructor list shows the striking part:
**it is already there, instantiated, and takes exactly the source type.**  The
target `pair` has four constructors --

```
pair(const pair<...5740>&)      // copy
pair(pair<...5740>&&)           // move
pair(const pair<...5714>&)      // converting, from the source type
pair(pair<...5714>&&)           // converting, from the source type
```

-- the class is complete, and the value being converted is the prvalue result
of `__emplace_unique`, which binds to either of the last two.  So the candidate
is present and viable on its face, and `CallConversion(value, target, 0, 0)`
still reports no conversion.

Printing the candidates as overload resolution sees them answers it, and the
answer is a gap this plan already knew about.  The two converting constructors
come back marked **explicit**, while the copy and move constructors of the same
class do not:

```
ctor explicit=0  pair(const pair<...5740>&)
ctor explicit=0  pair(pair<...5740>&&)
ctor explicit=1  pair(const pair<...5714>&)
ctor explicit=1  pair(pair<...5714>&&)
```

`ConvertingConstructor` skips an explicit candidate, correctly, so the
conversion has no candidate left.  libc++ declares it

```c++
explicit(!__check_pair_construction<_T1, _T2>::template __is_implicit<_U1, _U2>())
```

-- a **conditional explicit** whose condition is dependent.  Evaluating it
conservatively as explicit is exactly the C3 leftover recorded at the end of the
first run: *"the dependent conditional-explicit condition, which no header
currently stops on but `std::pair` needs for correct copy initialization."*

So C8's head is not a new defect.  It is that item, and it is no longer
hypothetical -- it is what blocks the libc++ self build.

Counting on both sides of the pipeline then narrows it further, and moves it off
the reading the C3 note suggests.  Compiling that one translation unit,
`ParseExplicitSpecifier` parses **284** explicit-specifiers of which **37 carry
a condition** -- so the parse is fine, and the `explicit(...)` form is being read
correctly.  `EvaluateExplicitSpecifier` is then called **710** times and sees a
condition **zero** times.

Two facts fall out of that.  Evaluation runs more often than parsing, so it is
running per-instantiation over a specifier that parsing produced once; and the
condition child is not visible by the time it runs.  So this is not "the
condition is dependent and cannot be answered yet" -- it is that the condition
is *gone*, and `EvaluateExplicitSpecifier` therefore takes its
`condition_syntax == kNoNode` path and answers `true`, i.e. bare `explicit`.

The child is not lost.  Printing the specifier's children at the evaluator
shows it is never called with one at all -- so the conditional specifiers reach
a **different consumer**, and that consumer is the defect:

```c++
// function_instantiation.cpp:245, scanning a function template pattern
else if (value == "explicit") pattern->explicit_specifier = true;
```

A function template pattern records explicitness by matching the specifier's
payload string and setting a bool.  It never looks at the condition child, so
every `explicit(...)` on a function template becomes unconditionally explicit.
`pair`'s converting constructor is a member template of a class template, so
that is the path it takes, and `PublishFunctionTemplateSpecialMemberRole` then
ORs the bool into the instantiation.

That is the whole cause, and it also explains why the fix is not a one-liner.
The condition depends on the template's own parameters, so it can only be
answered at instantiation -- and `PublishFunctionTemplateSpecialMemberRole` has
no scope parameter to answer it in.  The pattern has to carry the condition's
syntax instead of a bool, and the instantiation has to evaluate it with the
arguments known: exactly the "deferred fact like the one exception
specifications use" that the C3 note called for.  The work is scoped, the test
is the one command above, and `function_instantiation.cpp:245` is where it
starts.

### C8's queue after the conditional-explicit and variable-template fixes

Both of those landed, and the self build advances into `__algorithm/sort.h`.
Two causes remain, and both now have small reducers:

**A functional cast to a reference type.**  libc++'s sort passes its comparator
on as `_Comp_ref(__comp)`, where `_Comp_ref` is a reference typedef.  N3485
5.2.3/1 makes `T(expr)` with one operand the cast `(T)expr`, which for a
reference binds the operand; `analyzer.cpp` sends it to the class path instead,
which tries to construct.  Three lines reproduce it:

```c++
struct S { int v; };
typedef S &Ref;
int main(){ S s; s.v = 1; Ref r = Ref(s); return r.v == 1 ? 0 : 1; }
```

Adding a reference branch ahead of the class check was tried and does **not**
work on its own -- neither `ApplyTarget(operand, reference_type)` nor returning
the operand unchanged binds it -- so reference binding for this form needs more
than routing, and that attempt was reverted.

**A neighbouring gap found while reducing it, worth its own fixture:**
`S b = S(a);` for a plain aggregate `S` is rejected with *invalid implicit
conversion from struct S to int*, which clang accepts.  It is unrelated to
libc++ and predates all of tonight's work.

The reference cast is fixed.  A serial run of the libc++ inception build now
reports four causes, in descending frequency:

| Count | Cause |
| --- | --- |
| 13 | `nonconstant noexcept expression` at `__vector/vector.h:1390` |
| 4 | `ambiguous overloaded operator` |
| 2 | `semantic expression kind 65 is outside the active PA15 checkpoint` |
| 1 | `structured template type was not found: std::move` |

The first is `vector::swap`'s C++11 branch, specifying
`!propagate_on_container_swap::value || __is_nothrow_swappable_v<allocator_type>`
-- a dependent `||` over a variable template in a noexcept-specification, the
same shape as the very first fix of this run but in a context that requires a
constant.  It has resisted a standalone reducer.

Kind 65 is `DUMP_TEMPORARY_OBJECT` reaching scalar lowering, which
`lowering/expressions/core.h` does not handle.  It is not a consequence of the
reference-cast fix: a reference cast over a temporary, in both the const-lvalue
and rvalue spellings, compiles correctly.

Every one of these is reached from `semantic/templates/result_identity.cpp`,
which is where a serial build stops first, so one translation unit is again the
whole queue -- and it reproduces in the same one-command form.

The noexcept cause is placed but not explained.  A breakpoint at the throw
gives the chain `BuildResolvedCall` -> `DemandFunction` ->
`DemandRuntimeFunction` -> `EnsureFunctionExceptionSpecification` ->
`IsNonthrowing`: demanding the function for emission is what forces its
exception specification, and the specification does **not** fold
(`expression.constant == false`).

What makes this different from the conditional-explicit fix, which had the same
shape, is that the machinery it would need already exists:
`EnsureFunctionExceptionSpecification` evaluates in a stored
`function.exception_specification_scope` -- there is already a deferred fact and
already a scope to answer it in.  So the question is not "where does the scope
come from" but why the `||` still fails to fold in a scope that should have the
arguments bound.

Probing at the throw answers half of that.  The expression is **dependent**, not
merely non-constant: the C7a predicate reports `dep=1`, meaning it reads a
variable template specialization that has no value yet, and there is a binding
for that specialization.  So the read reached the variable template and came
back uninstantiated -- `__is_nothrow_swappable_v<allocator_type>` never got a
value, even though the exception-specification scope should have
`allocator_type` concrete.

The obvious suspect was `InstantiateVariableTemplate`'s recursion guard, which
returns `kNoBinding` when a request for the same specialization is already in
progress.  **Measured, and it is not that**: instrumenting that return shows it
fires zero times while compiling the failing translation unit.

What the earlier probe already established narrows it instead.  The binding is
present and is a variable template specialization that is *not constant* -- so
the specialization was instantiated, and its **initializer** did not fold.  The
failure is therefore one level further in than the read:
`__is_nothrow_swappable_v<allocator_type>` exists as a specialization whose
value never became constant, and the next step is to find which link of the
trait chain behind it stops folding.

C8's remaining distance is exactly C7's: with the driver gap cleared, the libc++
inception build reaches the headers and stops on those causes.

## Fixtures

Every fix above has a fixture in the assignment whose stated surface owns it --
PA22 for the template-argument and completion work, PA34 for the GNU/Clang
concessions the hosted headers exercise.  One exception is recorded as debt: the
delete deferral has no fixture, because six reducers failed to reach the branch
and instrumenting it showed it fires twice while compiling `<memory>` and never
for any of them.

### C8 measured across the whole tree, not one translation unit

Every earlier count in this file came from a serial build, which stops at the
first translation unit that fails and so reports whatever that one file hits.
Compiling all 233 sources independently gives the real queue.  With the
short-circuit fix in, **60 of 233 fail**, in these groups:

| TUs | Cause |
| --- | --- |
| 14 | `missing lowered temporary %<internal-N> in function @...` |
| 10 | `class template name requires template arguments` |
| 10 | `ambiguous overloaded operator` |
| 9 | `semantic expression kind 65 is outside the active PA15 checkpoint` |
| 6 | `no viable overload for next in operator()` |
| 5 | `invalid implicit conversion from rvalue-reference to X to lvalue-reference to X` |
| 3 | `no viable overload for __iter_move in operator()` |
| 2 | `host EH protected-region state mismatch` |
| 1 | `structured template type was not found: std::move` |

Reproduce with `scripts`-free shell: compile each `dev/src/**/*.cpp` with
`-std=gnu++11 -Wall -O3 -stdlib=libc++ -I../dev/src -I../obj-clang-libcxx/pa39/generated`
from `pa39/`, after `make with-clang-libcxx-build`.  Note that `dev/cppgm++` is
shared between cells: the repro line in the section above names it without
saying the libc++ cell has to be the one built, and against the default build
it fails with *this compiler was built for the default standard library*.

The `nonconstant noexcept expression` group is gone; that was the 13-site
group above and is fixed.  The counts here are larger than the serial ones
because a serial build never reaches most of these files.

### The `class template name requires template arguments` group is a trap

All 10 are one site, `tuple:560:55` -- the `_And` in

```c++
explicit(_Not<_Lazy<_And, _IsImpDefault<_Tp>...> >::value)
```

which is a class template name passed as a **template template argument**.  A
template template argument is spelled exactly like a type argument, so the
walker that interns a function template's result identity
(`InternExpandedFunctionTemplateResult`) asks whether it names a concrete type.
It does not, and `BuildSpecifiers` throws.  Confirmed by backtrace:

```
BuildSpecifiers (declarations/analysis.cpp:1744)
  <- BuildTypeId
  <- BuildCanonicalTemplateTypeArgument (templates/arguments.cpp:1939)
  <- InternExpandedFunctionTemplateResult's walker (result_identity.cpp:285)
```

Reduces to 20 lines; the ingredients are a class template name as a template
template argument **inside an `explicit(...)` specifier of a constructor
template**.  Neither piece alone does it: the same expression outside
`explicit(...)` is fine, and `explicit(...)` over the same trait without a
template template argument is fine.

The obvious fix does not work, and fails in a way worth recording.  The walker
already asks in the mode that reports a substitution failure rather than
throwing one -- it pushes a frame and checks `CandidateSubstitutionFailed()` --
so the throw site was made to use that idiom, exactly like
`CandidateTypeFormation` and `CandidateExpressionFailure` next to it.  That
compiles the group, and a backtrace confirms the failure is recorded in the
walker's own frame and nobody else's (`depth=1`, twice, once per
specialization).  But the emitted code is then **wrong**: for the
specialization whose specifier evaluates to true, the constructor loses its
mem-initializer entirely.

```
function @box_int___...    object=_ZN3boxIJiEEC1IiEEv
    ++calls                      ; no `store i32 7`
function @box_double___...  object=_ZN3boxIJdEEC1IiEEv
    store i32 7, tag             ; correct
    ++calls
```

so `box<int> s; s.tag` reads 0 where clang reads 7.  The mechanism connecting a
failure contained in the walker's frame to a dropped mem-initializer is not
identified, and a hard error is better than silent wrong codegen, so the change
was reverted rather than shipped.  Anyone picking this up should start there
and not at the throw site.

Two neighbouring observations from the same reduction, both unrelated to
libc++ and neither yet filed: conditional `explicit` on a **non-template**
constructor fails with *constructor action has no emitted binding*, and on a
class template with a plain constructor with *class has no usable default
constructor*.

## Compile-time cost of this plan (measured late, which was a mistake)

spec.md is titled *Compile-Time-First* and is normative: compile-time
performance is a property of the semantic design, not a mode.  No commit in
this plan was measured against that until the whole thing was already written,
so nothing below is attributable to a particular change -- only to all of them
together.  That is the finding as much as the numbers are.

Frozen compile, `benchmarks/self_compile/stable/semantic_overload.cpp` at
`-O1`, pinned source **and** pinned headers so the workload is constant, 6 ABBA
blocks per lane, pre-plan `e0547a29` against the tree after the plan:

| lane | wall | user | peak RSS |
| --- | --- | --- | --- |
| gcc-built | 5.300 -> 5.315 s (+0.57%) | 4.825 -> 4.855 s (+0.62%) | 363,200 -> 369,356 KiB (+1.78%) |
| self-built | 8.345 -> 8.395 s (+0.66%) | 7.865 -> 7.900 s (+0.54%) | 362,698 -> 370,572 KiB (+2.24%) |

self/gcc ratio 1.574 -> 1.580 wall, 1.630 -> 1.627 user: the parity number did
not move.  The self binary grew 11,530,552 -> 11,591,912 bytes (+0.53%) and the
frozen object grew 64 bytes.

Read it as: time is ~0.6% worse and consistent across two independent lanes, so
it is signal rather than noise, but small; **RSS at ~+2% is the real mover**,
against a plan whose own tables treat +-0.1% as unchanged.  Which commit spent
it is unknown, because the measurements that would have said were never taken.

Run the check with

```sh
scripts/run_frozen_compile_benchmark.sh <compiler-a> <compiler-b> [blocks] [-Olevel]
```

and read the self/gcc ratio by passing the gcc-built `dev/cppgm++` and the
`obj/pa39/bin/selfhost/cppgm++-self` from `make -C pa39 -j32 cppgm++-self`.
Prefer the paired user figure: this host's wall is the noisiest metric.  The
corpus lives outside the repository, so set `CPPGM_FROZEN_ROOT` if it moves.

Two traps worth keeping: `dev/cppgm++` is one path shared by all three
toolchain cells, so a build for another cell part-way through a measurement
silently swaps the binary under it; and the frozen corpus carries its own
headers, so pointing `-I` at the current `dev/src` fails with a missing header
rather than measuring anything.

## C8 after the session of 2026-09-04

Six commits, each gated on the full report and `make inception`, and the last
three carrying a frozen-compile A/B in their message.  Compiling all 233
sources independently, before and after:

| level | before | after |
| --- | --- | --- |
| -O0 | 184 fail | **22 fail** |
| -O3 | 60 fail | **41 fail** |

The -O0 collapse is almost entirely one fix.  What each bought, measured by
re-running the census between commits:

| fix | -O0 files cleared |
| --- | --- |
| force-inline call with no continuation | 140 |
| deduced class variable is initialized | 10 |
| is_base_of rejects a reference operand | 12 |
| candidate discarded on an ambiguous operator | 0 net |

The ambiguous-operator fix is worth keeping despite the zero: it collapsed its
own group from 10 files to 1, and those files then failed later in the same
compile rather than earlier.  Progress inside a file does not show up in a
file-level count, which is a reason to read the cause table and not just the
total.

Two of these are not libc++ defects and not hosted ones.  Both reproduce in
the default cell, at the pre-plan baseline, in a handful of lines of plain
C++11: a force-inline `noreturn` call, and `auto p = E();` for any class type.
libc++ only made them unavoidable, by writing `__throw_length_error` as a
force-inline noreturn and `auto __proj = __identity();` in three algorithms.

### What is left

| files | cause | state |
| --- | --- | --- |
| 10 | `class template name requires template arguments` | diagnosed; the obvious fix is a trap, see above |
| 6 | `no viable overload for next` | undiagnosed |
| 3 | `no viable overload for __iter_move` | undiagnosed |
| 2 | `invalid binary arithmetic operands` | narrowed, below |
| 1 | `structured template type was not found: std::move` | undiagnosed |

At -O3, `missing lowered temporary` (16 files) is on top of that list.  It is
**not** downstream of the force-inline bug: an earlier note in this file said
it was, and re-measuring after that fix landed shows the identical fourteen
files unchanged, so the claim was a hypothesis stated as a fact.  It is a
native-lowering value used before its definition and needs its own reduction.

### The stream-manipulator cause, narrowed but not solved

`std::ostringstream t; t << std::setfill('0');` is enough; `t << "0x"` and
`t << std::hex` are both fine.  libc++ declares setfill's inserter as a hidden
friend -- a function template on `_Traits` inside `__iom_t4<_CharT>` -- so it
is reachable only by ADL.

Instrumenting the lookup shows the expression resolves `operator<<` twice:

```
[SEL] candidates=27 selected=1     op0/op1 are *references*   (the SFINAE probe)
[SEL] candidates=26 selected=0     op0/op1 are bare classes   (the real call)
```

The first is `__is_ostreamable`'s probe over `declval<_Stream&>() << declval<_Tp>()`.
It gets the hidden friend, deduces it (`specs=1`) and selects it.  The real
call gets one candidate fewer and never reaches
`AppendHiddenFriendCandidates` for that owner at all, so the friend is not in
its set and resolution falls back to the builtin shift -- which is what
"invalid binary arithmetic operands" is reporting.

So the friend is registered and findable; the associated entity computed for
the argument as a *prvalue* is not the entity it is registered under, while the
one computed for a reference to it is.  `AddAssociatedType` unwraps references
to the same `TYPE_NAMED`, so the difference is further in, in which entity the
specialization resolves to.  That is where the next attempt should start.

Ruled out along the way, each by measurement rather than reasoning: it is not
the inline namespace (`std::__1`), not the hidden-friend-inside-a-class-template
shape, not deduction against a base class, and not a regression from the
ambiguous-operator fix -- disabling that branch leaves the failure unchanged.

## C8 at the end of the session

| level | session start | now |
| --- | --- | --- |
| -O0 | 184 fail | **14 fail** |
| -O3 | 60 fail | 41 fail, last measured before the final three fixes |

Nine commits.  Per-fix, measured by re-running the census between commits:

| fix | -O0 files cleared |
| --- | --- |
| force-inline call with no continuation | 140 |
| is_base_of rejects a reference operand | 12 |
| deduced class variable is initialized | 10 |
| bare template-name reported as a substitution failure | 0 net |
| sizeof over a shape pack is value-dependent | 8 |
| candidate discarded on an ambiguous operator | 0 net |

The two zeroes are both real progress that a file count cannot show: each moved
its files to a later cause in the same compile.  The template-name fix in
particular moved its ten `tuple` files onto the sizeof-pack cause, which the
next fix then cleared -- three defects stacked behind one diagnostic.

### Reopening the template-name "trap" was right

An earlier section of this file recorded that fix as unsafe because it compiled
the files and then emitted a constructor with no mem-initializer.  That
reasoning was wrong in a specific way: before the fix, compilation *failed*, so
there was no working behaviour to regress.  The dropped mem-initializer was a
second defect standing behind the first -- of the two places that wire an
instantiated definition, `InstantiateFunctionTemplate` took the body without
the constructor's mem-initializers -- and a third (the shape pack) stood behind
that.  "The fix compiles the input and the output is wrong" means look for the
next defect, not that the fix is wrong.

### What is left at -O0

| files | cause | state |
| --- | --- | --- |
| 8 | `no viable overload for next` | narrowed, below |
| 3 | `no viable overload for __iter_move` | same family |
| 2 | `invalid binary arithmetic operands` | narrowed earlier: hidden friend not in the real call's candidate set |
| 1 | `structured template type was not found: std::move` | undiagnosed |

`missing lowered temporary` remains the largest -O3-only cause and is a
native-lowering value used before its definition.

### The `next` cause, narrowed but not solved

Site is `__algorithm/move_backward.h:44`, `_IterOps<_AlgPolicy>::next(__first,
__last)`, and the diagnostic prints only `candidates[0]`, so the message
understates it.  Printing the whole set at the failure:

```
[NV] name=next candidates=4
  cand0 function of (pointer to unsigned long int, pointer to unsigned long int) -> pointer to unsigned long int
  cand1 function of (lvalue-reference to pointer to unsigned long int, long int) -> pointer to unsigned long int
  cand2 function of (pointer to const char, pointer to const char) -> pointer to const char
  cand3 function of (lvalue-reference to pointer to const char, long int) -> pointer to const char
```

while the call's arguments are `pointer to enum AbiFunctionQualifier`.  Every
candidate is a specialization deduced for an *earlier* call in the same
translation unit; the member template patterns are not in the set, so no fresh
deduction happens.  The backtrace puts the call in the placeholder-deduction
path -- `auto __last_iter = ...` -- through `BuildVariableDeclarator`.

The suspect is the retained call-candidate cache keyed by callee syntax node:
one node, `_IterOps<_AlgPolicy>::next(__first, __last)`, replayed by every
instantiation of `__move_backward_impl<_AlgPolicy>::operator()`, with a naming
class (`_IterOps<_ClassicAlgPolicy>`) that is identical across them because the
policy is fixed, so a guard that validates the naming class would pass while
the cached candidates belong to another argument type.  That is a suspect, not
a finding.

Five reducers failed to reproduce it, which is worth recording so the next
attempt does not repeat them: two overloads named `next`; `_IterOps` declared
undefined with full specializations per policy rather than a primary template;
the qualified name dependent on the enclosing template's parameter; the call as
the initializer of a deduced variable; and the whole thing as a member template
of a class template instantiated twice with different iterator types.  All five
compile and run correctly.  Something about libc++'s context is still missing,
and the next step is to instrument the candidate collection itself rather than
to guess a sixth shape.

## C9. Close

**Inventory.**  Independent compiles of all 233 sources in the libc++ cell:

| level | pre-plan | now |
| --- | --- | --- |
| -O0 | 184 fail | 14 fail |
| -O3 | 60 fail | 38 fail |

All 38 hosted libc++ headers compile.  The three cells are one command each
(`make test-cells`), PA34's readme states the surfaces its fixtures test, and
the discovery rules including the prohibition on run-time host probing are in
the readme that owns them.

**Audits.**  All nine pass.  `audit-semantic-owners` did not: it passed before
this plan and was red at the plan's head with nine missing rows, which nothing
caught until C9 ran it.  Eight were real definitions added across the plan; the
ninth was a comment naming `Analyzer::EntityOf`, because the audit scans every
line and does not know a comment from a definition.

**Compile-time cost, and a threshold this plan exceeded.**  The plan's own
protocol sets two gates: no lane may regress more than 1% aggregate CPU, and
the frozen-TU cachegrind Ir may not regress more than 0.5% without a written
justification naming the feature the cost buys.

Frozen TU (`semantic_overload.cpp` at -O1, pinned source and headers):

| span | Ir | verdict |
| --- | --- | --- |
| pre-plan -> plan head before this session | **+0.627%** | over the 0.5% gate |
| that point -> after this session's nine fixes | **-0.002%** | neutral |
| pre-plan -> now | **+0.626%** | over the gate |

The CPU gate is met: paired user time is +0.62% gcc-built and +0.54%
self-built, both under 1%, and the self/gcc ratio is unmoved at 1.574 -> 1.580
wall.  The Ir gate is **not** met, and the attribution is unambiguous: every
instruction of it was spent before this session, by C0-C7.  The nine fixes in
this session cost nothing measurable -- -0.002% against a lane whose noise
floor is about 0.001%.

Stating the justification the plan asks for, rather than treating the overage
as closed: what the 0.627% bought is hosted libc++ support -- 38 headers, and
the libc++ build going from every translation unit failing to 14 -- plus three
defects that were never about libc++ at all and reproduce in the default cell
in a few lines of plain C++11 (a force-inline non-returning call, `auto p =
E();` for any class type, and `is_base_of` on a reference operand).  That is a
real feature and a real correctness gain, but the overage was never measured
while it was being spent, so it was never a decision.  It should be treated as
a debt to pay down, not a settled price: the per-commit Ir that would say which
of C0-C7's changes cost it was never taken and would have to be reconstructed
by bisecting the plan's commits.

What was **not** measured, so it is not claimed: the protocol's four-lane
inception timing (gcc-O1, gcc-O3, self-O1, self-O3 through pa39
`compare-cppgm++-inception`).  Only the frozen-TU lane was run, in both its
gcc-built and self-built forms.

**Cost of each fix** is in the two sections above; the per-fix -O0 file counts
are in "C8 at the end of the session", and every commit from this session
carries its own frozen-compile A/B in its message.

## Final inventory

| level | pre-plan | now |
| --- | --- | --- |
| -O0 | 184 fail | **3 fail** |
| -O3 | 60 fail | **29 fail** |

Eleven source fixes this session.  Per-fix -O0 files cleared, each measured by
re-running the census between commits:

| fix | cleared |
| --- | --- |
| force-inline call with no continuation | 140 |
| retained call patterns through a dependent qualifier | 11 |
| is_base_of rejects a reference operand | 12 |
| deduced class variable is initialized | 10 |
| sizeof over a shape pack is value-dependent | 8 |
| bare template-name reported as a substitution failure | 0 net |
| candidate discarded on an ambiguous operator | 0 net |

Four of these were never libc++ defects.  A force-inline non-returning call,
`auto p = E();` for any class type, `is_base_of` on a reference operand, and
the retained-pattern replay all reproduce without a single library header.

### What remains

At -O0, three files: two on the stream-manipulator hidden friend and one on
`std::move` not being found as a structured template type.  At -O3, twenty-nine,
of which twenty-three are `missing lowered temporary` -- a native-lowering
value used before its definition, present only at -O1 and above, and
independent of everything fixed here.

### Correcting a suspect this file named

An earlier section named the retained call-candidate cache's naming-class guard
as the suspect for the `next` cause.  That was wrong.  Instrumenting the
completion path showed the guard never rejects anything here, because every
retained candidate really is a member of the same `_IterOps<_ClassicAlgPolicy>`;
the fact simply carried four specializations and **zero patterns**, so there was
nothing to deduce from.  The mechanism is that a dependent qualifier resolves to
no patterns at record time, and the fact then accumulates one specialization per
instantiation.

The validator trace adds a detail worth keeping: the call site is visited
twenty-six times, and exactly one of those visits has the qualifier's parameter
not visible.  Retention is keyed by syntax node, so that single visit publishes
the fact that the other twenty-five replay.  This is also why ten synthetic
reducers failed -- in every one the parameter is visible on every visit, so the
call is never retained and the defect cannot occur.

### Compile-time, final

Frozen TU at -O1, cachegrind Ir, pinned source and headers:

| span | Ir |
| --- | --- |
| pre-plan -> plan head before this session | +0.627% |
| this session's eleven fixes | **-0.003%** |
| pre-plan -> now | +0.624% |

The plan's 0.5% Ir gate is still exceeded by C0-C7 and still unpaid; nothing in
this session added to it.  One fix was measured at +0.881% user before caching
its lookup and +0.207% after -- which cachegrind then resolved as -0.002% Ir,
a reminder that at this size the timing lanes are reporting noise and the Ir
lane is the one that decides.

## The last translation unit at -O0

One file of 233 still fails, and it is a language gap rather than a bug:
`preprocess/tokens/post_tokenizer.cpp`, on

```c++
const basic_string __temp(__init_with_sentinel_tag(), std::move(__first),
                          std::move(__last), __alloc_);
```

at `string:2955`.  This is the declaration/expression ambiguity.  The first
argument looks like a parameter of function type, which makes a declaration
parse plausible, so the whole thing is parsed as a function declaration -- and
then `std::move` is looked up as a parameter's type and is not one.  N3485
8.2/1 resolves the ambiguity to a declaration only if it *can* be one, and this
cannot, so it is an initialization.

Seventeen lines reproduce it with no library header at all:

```c++
struct tag { };
struct holder { int v; holder(tag, int a, int b) : v(a + b) { } };
template <class T> T& pass(T& t) { return t; }
int main() { int a = 1, b = 2; const holder h(tag(), pass(a), pass(b));
             return h.v == 3 ? 0 : 1; }
```

The machinery for this already exists and declines for a reason worth writing
down.  `AnalyzeAmbiguousDirectInitializer` handles the single-argument form and
hands the rest to `AnalyzeAmbiguousMultiDirectInitializer`, which accepts an
argument spelled `T()` -- a declarator whose parameter clause is empty -- and
returns false for anything else.  `std::move(__first)` is a declarator whose
parameter clause has contents, so the handler gives up and the declaration
parse stands.

Closing it means letting that handler treat `name(args)` as a call: analyze the
name as a value and each inner parameter's specifier as an argument, and build
the call.  That is a real extension in the code that decides whether *every*
declaration statement is a declaration, which is why it is written down here
rather than attempted at the end of a long session.  The diagnostic now carries
a source location, which it did not before, so the next attempt starts at the
line rather than at a name.

## Final inventory, corrected

The "Final inventory" section above was written mid-session and is superseded.
Independent compiles of all 233 sources in the libc++ cell:

| level | pre-plan | now |
| --- | --- | --- |
| -O0 | 184 fail | **1 fail** |
| -O3 | 60 fail | **27 fail** |

Every remaining -O0 failure is the one declaration/expression ambiguity above.
Every semantic cause the libc++ headers exercise is cleared.

What remains at -O3 is not semantic and is not about libc++: 23 files on
`missing lowered temporary`, a native-lowering value used before its
definition that appears only at -O1 and above, and 3 on a host EH
protected-region mismatch.  Neither was touched by this plan and neither is
reachable at -O0, so the frontend work is done and the remainder belongs to the
backend.

Per-fix -O0 files cleared, each measured by re-running the census between
commits:

| fix | cleared |
| --- | --- |
| force-inline call with no continuation | 140 |
| retained call patterns through a dependent qualifier | 11 |
| is_base_of rejects a reference operand | 12 |
| deduced class variable is initialized | 10 |
| sizeof over a shape pack is value-dependent | 8 |
| a candidate is not collected twice | 1 |
| nested argument-dependent lookup keeps its classes | 1 |
| bare template-name reported as a substitution failure | 0 net |
| candidate discarded on an ambiguous operator | 0 net |

Six of these are not libc++ defects and reproduce with no library header at
all: the force-inline non-returning call, `auto p = E();` for any class type,
`is_base_of` on a reference operand, the retained-pattern replay, the
duplicate candidate, and the declaration/expression ambiguity that remains.

### The four-lane inception timing was not run

The protocol asks for gcc-O1, gcc-O3, self-O1 and self-O3, each timed through
pa39's `compare-cppgm++-inception` after an untimed self-host build, at six or
more ABBA blocks with fresh roots and aggregate CPU as the gate.  That is four
lanes times twelve full inception builds -- hours of wall time -- and it was
not run.  What was run instead is the frozen-TU lane in both its gcc-built and
self-built forms, plus cachegrind Ir, which the protocol itself describes as
resolving regressions the timing lanes cannot: noise floor about 0.001%,
against a timing lane that reported +1.21% and -0.90% on the same change.

So the sensitive measurement is present and reads -0.003% for this session,
and the coarse one is absent.  That is a real gap in the protocol, not a
substitution, and it is recorded as absent rather than as passed.

Two mechanical notes for whoever runs it.  The lane is selected by how the
producer `dev/cppgm++` was built, not by `INCEPTION_SELFHOST_OPT_LEVEL`, which
sets the flags the producer compiles *with*.  And
`make -C pa39 compare-cppgm++-inception CXX=../dev/cppgm++` fails on a fresh
object root because it needs `obj/pa39/bin/selfhost/cppgm++-self` staged first;
the root `make inception` target does that ordering, so a timed lane has to
stage the self build untimed and time only the comparison.

### All four inception lanes, measured

The section above recorded the inception timing as absent.  Two of the four
lanes have since been run, this session's tree against the tree it started
from, with the gcc-built `dev/cppgm++` as producer, aggregate CPU (user+sys) as
the gate, three ABBA blocks each and the self-host build staged untimed:

| lane | base | current | paired | per block |
| --- | --- | --- | --- | --- |
| O3 | 457.93 s | 457.65 s | **-0.060%** | -0.136%, +0.104%, -0.149% |
| O1 | 534.99 s | 535.03 s | **+0.008%** | +0.211%, -0.078%, -0.107% |

Both are far inside the 1% gate, and the per-block spread of about 0.2% says
the measurement resolves what it needs to.  The two lanes whose producer is
`cppgm++-self` rather than the gcc-built compiler were still not run, so this
is two of four, not four.

**The trap that produced two wrong readings.**  `INCEPTION_SELFHOST_OPT_LEVEL`
changes compile flags, and make does not track it, so switching levels compares
selfhost objects built at one level against inception objects built at another.
That reports `object mismatch` on dozens of files, which reads exactly like a
self-host failure.  It led to a claim here that O1 self-host is broken, and then
to a second claim that the same failure at the session-start tree proved it
pre-existing.  Both were wrong: wiping `obj/pa39/selfhost`, `obj/pa39/inception`
and `obj/pa39/bin` first, O1 inception MATCHes in both trees.  A lane has to
keep one level for its whole run, or wipe all three roots between levels.

### The remaining two lanes, and the gate

The self-producer lanes run through
`compare-cppgm++-inception-from-restored-self`, which takes
`obj/pa39/bin/selfhost/cppgm++-self` as `CXX` -- that is what makes them self
lanes, and it is why passing the self binary as `CXX` to the ordinary target
does not work.  All four, this session's tree against the tree it started from,
aggregate CPU (user+sys), self-host build staged untimed, fresh inception root
per run:

| lane | blocks | paired | range |
| --- | ---: | --- | --- |
| gcc-O3 | 3 | -0.060% | -0.149% .. +0.104% |
| gcc-O1 | 3 | +0.008% | -0.107% .. +0.211% |
| self-O3 | 3 | -0.376% | -0.518% .. -0.282% |
| self-O1 | 6 | -0.113% | -0.618% .. +0.257% |

**Worst lane +0.008% against a 1% gate: the aggregate-CPU gate is met.**
self-O1 was taken to the protocol's six blocks because its per-block range at
three was about 0.85% and the gate is 1%; the other three lanes sit well inside
their spread at three, which is short of the protocol's six and is written down
as such rather than rounded up.

One observation worth keeping: the self-produced lanes are not slower than the
gcc-produced ones here (455 s against 458 s at O3), against a frozen-TU
self/gcc ratio of about 1.57x.  Whatever that ratio measures, it does not carry
over to the whole-compiler build at -O3, so the two should not be quoted
interchangeably.

## The Ir gate, paid down

The frozen-TU Ir gate was recorded above as exceeded at +0.684% and left as
debt.  It is now met, and the debt turned out to be one commit and one hot
loop.

Bisecting the plan's commits against the frozen TU put the whole of the first
half of the regression in `a24fc83c`, "Answer the preprocessor probe family
libc++ uses":

| point | Ir | vs pre-plan |
| --- | --- | --- |
| pre-plan `e0547a29` | 17,655,285,094 | -- |
| `f09dc211`, its parent | 17,655,420,691 | +0.001% |
| `a24fc83c` | 17,709,656,079 | **+0.307%** |

That commit changed only `macro_processor.cpp`, and none of the change is on a
per-token path: it adds four probe markers to a map and two functions that the
profile shows are never called while compiling this input.  What it did was
grow the translation unit enough to flip a g++ inlining decision.  At the
parent, `IsOperator` was inlined into its callers and `ExpectedOperatorCode`
was a 39.8 M-instruction out-of-line call; afterwards `IsOperator` is itself
out-of-line at 95.5 M with `ExpectedOperatorCode` inlined into it.

So the instructions were real but the cause was not the feature.  The fix is
the one the profile asks for: `AnnotateParentheses` calls `IsOperator` three
times per token, each time handing a string literal to `ExpectedOperatorCode`
to turn back into a constant the token already carries.  Comparing
`token.tracked_operator` against that constant directly removes the work
whatever the inliner decides.

| span | Ir |
| --- | --- |
| the hot-loop change | **-0.821%** |
| whole plan, before it | +0.684% |
| whole plan, after it | **-0.142%** |

The plan now compiles the frozen translation unit with fewer instructions than
before it started, and the 0.5% gate is met rather than owed.  There is no
fixture: nothing about the compiler's output changes, `make inception` matches
bit for bit, and the measurement above is the regression test.

**Both C9 performance gates are now met**: worst inception lane +0.008% against
1% aggregate CPU, and frozen-TU Ir at -0.142% against 0.5%.

## The -O3 remainder, narrowed

-O0 is clear; -O3 has 26 translation units left, in two causes that look like
one family: 23 on `missing lowered temporary` and 3 on `host EH protected-region
state mismatch`.  None is reachable at -O0.

Instrumenting the throw in `native/lowering/function.cpp` on the smallest case
(`lowering/objects/polymorphism.cpp`, value 266 in
`DeletingDestructorBuilder::AddBlock`) gives the shape:

- the value **is** defined, in block index 36, and is not a phi;
- it is **used** in block index 26, at emission position 178 against a
  definition at position 206, so the use precedes the definition in the order
  native lowering emits;
- following ordinary control-flow edges, block 24 goes to 36 and block 25 goes
  to 26 -- two disjoint chains -- so the definition does not dominate the use;
- only 11 of the function's 166 blocks are reachable from the entry by ordinary
  edges, which is what an EH-heavy function looks like when the traversal does
  not follow unwind edges.

It reproduces from -O1 upward and never at -O0, so an optimizer pass produces
it.  `remove_unreachable_blocks` exists but is called only from inside
`boolean_cfg`'s own transforms, not as a pipeline step, so dead blocks can
survive to native lowering.

What is **not** established, and matters before anyone acts on this: whether
those blocks are genuinely dead.  The traversal above ignores unwind edges,
while `remove_unreachable_blocks` builds its graph with `build_graph`, which
does account for them.  If EH edges connect these chains, the blocks are live
and deleting them would be wrong; the defect would then be an SSA violation on
an EH path rather than surviving dead code.  Deciding that needs the EH-aware
graph, and it is the first thing to settle.

This is backend work: it is not about libc++, is not reachable at -O0, and
nothing in this plan touched it.  The `-O3` files that now stop here previously
stopped earlier -- the group grew from 14 to 23 as semantic causes were
cleared, not because anything regressed.

## Final performance position

Frozen TU at -O1, pinned source and headers, pre-plan `e0547a29` against the
plan's head:

| metric | pre-plan | now | delta |
| --- | --- | --- | --- |
| cachegrind Ir | 17,655,285,094 | 17,630,233,944 | **-0.142%** |
| paired wall | 5.310 s | 5.330 s | +0.563% |
| paired user | 4.830 s | 4.850 s | +0.673% |
| peak RSS | 362,852 KiB | 369,560 KiB | **+1.962%** |

Both gates the plan sets are met: Ir at -0.142% against 0.5%, and the worst
inception lane at +0.008% against 1% aggregate CPU.

The row worth reading is the last one.  Instructions went **down** while time
went **up**, and peak RSS is up about 2%.  Ir counts instructions and cannot
see that, so the plan's cost is on the memory side rather than the instruction
side: roughly 6.7 MB more peak RSS on this input, which the timing lanes feel
and the Ir gate does not.  That is not a gate breach -- the plan sets no RSS
threshold -- but it is the honest description of what the work cost, and a
future protocol revision might want a memory gate beside the instruction one.

## After the third run

The self build and inception in the libc++ cell went from blocked to MATCH at
-O3 with three commits, all recorded in the support plan under "State at the
end of the third run": an explicit instantiation declaration now suppresses
only the members it names (the root was a preprocessor probe answering no to
`__exclude_from_explicit_instantiation__`), an `inline` namespace-scope
variable is one weak object rather than one internal copy per unit, and
copying an empty class emits nothing (libc++'s compressed pair overlaps an
empty member with a real one).

What the cell still owes is listed there as C10.  `-O1` inception matches as
of `83f44163`, after two native-backend defects that plain LowIR reproduces
at -O1 were fixed (a replayed index base retired before its last replay, and
an in-place operation on a frame home).  Still open: fourteen suite failures
in five causes, none reached by the compiler's own sources.

**Performance is the other half of the story and has its own plan.**  On the
eight largest translation units, the clang-built libc++ compiler is 2.3x
slower than the gcc-built libstdc++ one (44.4 s against 19.0 s at -O1), and
the self-built libc++ compiler is 2.4x slower than that (105.5 s), where the
libstdc++ cell's self-built ratio is 1.55x.  `PLAN-LIBCXX-PERFORMANCE.md`
carries the measurements: the host-compiler axis is clean (clang-built
against libstdc++ matches gcc-built); the front-end gap is the preprocessor
re-opening and re-lexing guarded libc++ headers 4,794 times per unit, 68 MB
of lexing where clang enters 5 MB; and the codegen gap is three loop shapes
libc++ leaves to the optimizer that libstdc++ resolves in the library --
an empty per-element destroy loop, a per-element fill loop, and an
exception-guard branch on a constant -- of which the first two alone are 35%
of the self-built binary's cycles on one unit.

## After the fourth run: the libc++ suite is green

The fourteen failures the third run left were fixed in seven commits
(`4e594d82` to `9cc4c6ee`), and `make test-cells` reports 5,596 of 5,596 in
every cell.  None of the causes was the one the third run guessed:

- The five PA36 links missed `basic_string::__fits_in_sso`, not a
  constructor.  It is reached only through
  `__builtin_constant_p(len) && !__fits_in_sso(len)`; our front end folds the
  probe to 0, and the demand rule treated the short-circuited operand as
  unevaluated while the lowering still emitted the call.
- The `<regex>` "no viable overload" was `operator~`: never classified, so
  ADL's enum-operator index never held it, and mangled as a source name.
  Behind it sat two more defects the same test then reached: a `break` after
  a `[[noreturn]]` call ended the statement walk before the following
  `default:`, and `__builtin_ctzg` was missing.
- The duplicate object-symbol label was the deleting-destructor rename
  (`D1E` to `D0E`) not matching a destructor with an ABI tag.
- The `>` inside parentheses was a one-level suspension of the
  template-argument stop where the list was two deep.
- The PA37 roundtrip cases were the LowIR text writer dropping the high word
  of `unsigned __int128` literals (libc++'s `__pow10_128` table).

Every fix carries a fixture in the lane that owns the shape (PA10, PA34,
PA36, PA37), and each one reproduces from a few lines without libc++.

## After the fifth run: the optimizer's post-SSA blind spot

The performance work moved from libc++-specific shapes to two generic
optimizer defects that libc++'s header-heavy code exposed more than
libstdc++'s.  The measurement moved with it: sampling profiles were replaced
by Cachegrind instruction counts on `analyzer.cpp` at -O1, which are
deterministic to the last instruction and settle 0.3% questions.

- A load in the same block as a store to the same slot or global reloaded
  the value the store had just written when the two addresses were spelled
  differently (`0b8bf5cd`).  Three PA37 controls that counted loads to prove
  no reuse crosses a writing store were re-pinned to the stored value.
- Once a function held a phi, the CFG cleanup did nothing beyond its
  Boolean folds, because phi inputs name their predecessors.  Every inlined
  call left a jump-only continuation block, and two empty callees inlined
  on both arms of a branch left a diamond of two jump-only blocks; libc++'s
  string copy constructor carried three such diamonds in a row on the
  short-string flag from its annotation hooks.  A phi-aware bypass
  (`e3a6650d`) retargets edges into jump-only blocks when the join's phis
  accept the new predecessor, folds same-value diamonds to a jump, and
  erases the orphans.  The copy constructor went from 40 blocks and 13
  branches to 19 and 7.

Three more landings followed in the same vein, each pinned by a PA37
fixture and measured by the same count:

- The edge-known branch fold now walks the single-predecessor chain above
  a branch (`f207fff9`), which removes the copy constructor's second test
  of the short flag; the walk exposed and fixed a latent miscompile of an
  entry block whose only explicit predecessor is a loop back edge.
- A branch or switch on a constant folds in post-SSA functions
  (`b51c90c9`), with the dead region erased and the phi inputs repaired;
  libc++'s `max_size`, `std::min` of two equal constants, left twelve of
  them on the unit.  The fold uncovered a contract the native lowering and
  the LowIR reader share, definition before use in block order, which a
  closing pass now restores for the rare function that violates it
  (`f20a2263`); the default cell's self-host build was the only test that
  reached it, so gold in both cells is now part of every landing.
- A block whose only predecessor ends in a jump to it merges into that
  predecessor (`44c4f7aa`): 15% of the unit's blocks, every inlined call's
  entry and continuation.  Two O3 threading passes that matched the
  unmerged layout learned the merged form.

Run total on the unit: the self-built compiler went from 18.455 G to
17.902 G instructions (-3.0%) while the host-built compiler's own work
grew 0.2%.  Gold at `44c4f7aa`, run alone: clang/libc++ -O3 wall 1.602x,
CPU 1.609x (from 1.652x at the start of the run); default 1.571x wall,
CPU 1.521x (from 1.541x); MATCH in both; 5,611 of 5,611 in every cell.

What the count says is left.  Ranked by self-built excess over the
host-built compiler, the top symbols are libc++'s `vector<unsigned
char>(n, v)`, `vector<bool>(n, v)` and `vector::resize` fills and the
`Instruction` move's bulk copy, all `rep` string instructions that
Cachegrind counts once per element and that run at memset speed in
practice; string equality against a literal and `string::compare`, which
stay out of line because their inlined bodies carry a throw path under a
terminate wrapper, the EH-inlining front the parity plan built end to end
and closed on measurement (L35); and the 88 remaining diamonds whose arms
carry different phi values, selects that the backend homes in the frame,
the merged-value handling the parity plan's P30 allocator program owns.
The post-SSA CFG cleanup is complete for the shapes that mattered; the
next step for either cell is P30.

## After the sixth run: what the loop census showed

With the CFG cleanup complete, the per-loop native census (`--stats-functions`)
became the oracle.  Of the unit's 107 loops at -O3, 103 carry frame homes,
and 3,240 of their 3,568 frame operands sit in call-bearing loops, the
residual the parity plan's allocator program closed on.  Two things in the
rest were fixable:

- A loop phi the planner had placed in a caller-saved register still ended
  in the frame when the walk found that register busy at the transfer; the
  walk now retries after the transfers with the planned register or another
  free, unclobbered one (`a0238d75`).  Small on the count (-0.07%).
- libc++'s bit iterators in `vector<bool>`'s constructor survived as
  16-byte slots because the aggregate split ran only while their address
  still sat in a reference slot or reached a callee inlined later; the split
  now runs again in the closing pass (`6df07c1d`).  The constructor's fill
  loop drops from 52 frame operands to 6 in each of two short loops; the
  self-built compiler runs 0.26% fewer instructions on the unit.

Gold at `6df07c1d`: clang/libc++ -O3 wall 1.628x, CPU 1.603x; default
1.540x, CPU 1.514x; MATCH in both; 5,612 of 5,612 in every cell.  From the
start of this campaign the libc++ CPU ratio moved from 1.652x to 1.603x and
the default cell's from 1.541x to 1.514x.  What remains is the closed
programs' territory: values live across calls or exception regions, the
selects the backend homes in the frame, and the throw-bearing library
comparisons the L35 verdict keeps out of line.

## After the seventh run: the equality that clang inlines and we did not

The profile's largest single library symbol was `operator==(const string&,
const char*)`, 2.6% of the self-built compiler's instructions, out of line
because libc++ reaches an `out_of_range` throw when the literal's length is
`npos` and the inliner refuses any callee with an exception instruction.
Clang takes the opposite order: it inlines `compare` into `operator==` and
that into the caller with the terminate landing pad intact, then folds
`strlen` of the literal, the `npos` test, the throw block and the pad in
the caller.  Our pipeline's strlen fold runs after every inlining wave, so
no wave ever saw the constant.

The inliner now makes a per-site folded copy: the call's constants replace
the callee's parameters, strlen and the value and CFG cleanups fold the
copy, a call inside it that still can unwind is folded and spliced the same
way to depth two (in most units `compare` is a separate function that
several callers keep), the dead regions are stripped, and the copy is
spliced when nothing exceptional remains.  It needed an unsigned-zero
compare fold (`0 >u size` had survived in every string function), a size
cap of 80 for hinted copies because our diamond form counts 66 where
clang's select form counts 20, and its own per-caller budget because the
string switches that compare one name against dozens of literals are where
a failing comparison costs the most out of line.

The self-built compiler executes 0.37% fewer instructions on the unit,
0.7% net of the attempts the host-built compiler also pays; the symbol's
cost halves; the binary's text grows 3.9%.  Gold: libc++ CPU 1.594x from
1.603x, default 1.515x from 1.514x, MATCH.  The gap between that and the
2.6% the symbol cost is the folded body itself, three times clang's: the
same size diamond twice, three minimum diamonds and a result merge.  Value
numbering across identical diamonds, the unsigned-minimum identity on a
diamond, and the compared value known on a branch's edge would give the
copy clang's shape and take the text growth back; they are general midend
collapses and the next lever.

## After the eighth run: the three collapses

The three general collapses named above are in the closing O3 pass, run
together until nothing folds: value numbering across diamonds, where a
diamond on the same computation as a dominating diamond's condition, with
arm inputs that are the same computations (through `copy`, and loads only
when memory is untouched on every path between), branches on the earlier
condition and yields its phis; the select identities, where a phi over two
jump-only arms that chooses between its comparison's operands is one of
them when the comparison fixes the other (`std::min(n, npos)` is `n`);
and the equality an edge establishes, substituted through what the edge
dominates, so a length fixed by a size check reaches the minimum and the
compare it feeds.  A dead store into a folded copy's by-reference argument
slot read as a change of memory between the size and data diamonds, so
dead slots go first.

The folded equality now has clang's structure: the size diamond once, the
length check, the data diamond on the first condition, `memcmp` with the
constant length.  Unit outputs at -O3 shrink 0.5% to 7.3%; the self-built
compiler executes 0.35% fewer instructions on the unit at no compile-time
cost, and its text comes back 1.6%.  Gold CPU stays inside the stage's
spread in both cells, as every LowIR-only landing has since the allocator
program closed; the count is the oracle that moved.

