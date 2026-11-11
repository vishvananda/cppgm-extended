# LLVM producer feature and option audit

This is a maintainer investigation of LLVM 22.1.2 on Linux x86-64. The request
is to repair incorrect feature reporting, rebuild LLVM, and measure the result
against the same frozen native producer. It is not a request to impersonate
Clang or advertise unsupported language extensions.

## Scope and evidence

The inventory covers 7,500 configured source/dependency paths and 18,076
conditional-line candidates. It includes the native build's dependency graph,
the cppgm build's available dependencies, configured C++ sources, libstdc++ 15,
glibc, and LLVM's generated configuration headers. Textual candidates include
documentation and other targets; each relevant difference must be checked in
its enclosing conditions before attributing an effect to it.

The executable probe matrix compares 163 predefined/library macros and 202
builtin, attribute, and language-feature queries in 14 configurations:
C++11/14 at O1; C++17 at O0–O3; PIC disabled; SSE4.2; POPCNT; AVX2;
`-fno-inline`; fast math; Os; Oz. Wrapped LLVM/glibc/libstdc++ probes are
included, along with the C++ feature names in Clang's `Features.def`.

Raw scripts, commands, logs, outputs, inventory, and receipts are under
`/mnt/vizier/cppgm-llvm-evidence/gap-feature-paths-r48/`. The matrix runner is
`audit_gap_feature_matrix_r51.py` in the evidence root. The initial non-PIC
probe encountered the host linker's default PIE requirement; the completed
matrix links its small probe executables with `-no-pie`. This changes no
producer preprocessing options. Failed setup logs are retained.

Native and cppgm CMake configuration headers match byte for byte. Their
configured C++ option sets match apart from warnings. Both builds use O1,
NDEBUG, C++17, libstdc++, RTTI, disabled exceptions, and disabled PGO/LTO.

## Findings and repair status

| Surface | Observed difference | Consequence and action |
| --- | --- | --- |
| `__cpp_if_constexpr` | Missing despite implemented syntax/semantics | libstdc++ `std::find` on bytes selects a scalar loop instead of `memchr`; advertise by language mode and test the enabled path. |
| Other C++14/17 feature macros | Many absent; `constexpr`, range-for, static-assert retain C++11 dates | Repair implemented feature dates and standard selection together. Newer Clang C++20/23/26 extensions are not equivalent capabilities. |
| `__has_feature` | Several implemented C++14 forms return zero | Add mode-aware answers for aggregates with member initializers, contextual conversions, decltype(auto), generic/init-capture lambdas, relaxed constexpr, return deduction. |
| Allocation builtin version | `1` instead of `201802` | libstdc++ selects ordinary operator calls. Validate the complete replaceable overload contract, including rejection of arbitrary placement forms, then publish the version. Optimizations newly permitted by the builtin remain a separate code-generation opportunity. |
| Standard/GNU attribute queries | `no_unique_address` returns Boolean rather than a date; implemented scoped GNU attributes report absent | Publish the standard date, implemented noreturn, scoped GNU support, and implemented naked/vector attributes. Unknown namespaces remain unsupported. |
| `__NO_INLINE__` | Inherited from the host's O0 macro probe even at O1–O3 | Suppresses glibc's extern-inline header paths. Make normal optimization reporting describe the invocation. Explicit `-fno-inline` now disables optional copies, preserves always-inline, and reports the mode; `-finline` restores optional inlining. |
| `__OPTIMIZE_SIZE__` | Absent for Os/Oz | Enable the size-sensitive header selection. Existing optimizer maps these modes to O2; this alone does not implement a distinct size cost model. |
| PIC/PIE macros | Host PIE state leaks into explicit PIC and non-PIC invocations | Derive PIC/PIE macro values from selected flags and actual target behavior. Source views and object compilation share the selected values; target controls cover command order and explicit macro overrides. |
| `__CET__` | Host GCC's value 3 is inherited without backend CET implementation | Remove the unsupported claim. LLVM's X86 indirect-branch pass otherwise enables an extra JIT behavior. This is not a measured explanation for the normal compilation gap. |
| `__has_extension(gnu_asm)` | Reports zero despite implemented GNU assembly | Report implemented support. LLVM's split-stack conditional is Apple AArch64, so this does not activate that path on this machine. |
| Visibility attributes and flags | Unsupported/ignored | Potentially important remaining implementation work; publishing a feature define alone is insufficient. |
| `minsize`, cold, nonnull/alloc-size/leaf attributes | Some present in headers but not represented by the compiler | Review and implement their actual contracts where relevant. These cannot be repaired by claiming attribute support alone. |
| Rotate builtins | Absent; xxhash uses shift/or fallback | Backend already recognizes rotations at O1+. No large impact established; check generated code before adding support for performance reasons. |
| AVX2, fast-math and other generic accepted flags | Some accepted without the requested implementation or matching macros | Remaining driver/implementation issues, distinct from the flags used in this LLVM baseline. Do not manufacture capability macros without providing the requested behavior. |

The GNU/Clang identity macros intentionally differ. Numeric-literal spelling
differences and equivalent fundamental-type spellings are not behavioral
mismatches. Native-only diagnostic, sanitizer, Objective-C, Swift, Windows,
and post-C++17 feature probes do not justify enabling those unsupported modes.

The following important probes already agree: always-inline, noinline, pure,
const, returns-nonnull, malloc, expect, assume-aligned, unreachable, bit-cast,
overflow arithmetic, byte swapping, leading/trailing zero counts, exceptions,
and RTTI. There is no implementation use of `trivial_abi` in this LLVM version.

## Controlled bounds

Defining only `__cpp_if_constexpr=201606L` changes 3 of the same 43 profiled
provider objects: Clang Lexer, LLVM InstructionCombining, and X86MCTargetDesc.
All 325 frozen cppgm objects and the linked executable still match native.
Against the same private compiler/provider baseline, the two deterministic
profiles execute 0.0915% and 0.0625% fewer main-binary instructions. Executed
code footprint is effectively unchanged. This is a real header-path gap,
but it does not establish a multi-percent runtime improvement.

Clearing only `__NO_INLINE__` changes none of those 43 objects or the relinked
binary. Existing output checks and profiles are reused by hash; identical
binaries are not reprofiled. In particular, the simple `std::isalpha` control
still calls libc in both native and cppgm builds. An enclosing library guard
can defeat a plausible conditional-path explanation.

The O1 linked cppgm-produced LLVM exports 151,945 dynamic definitions versus
76,921 for native. Its dynamic string table is about 13.3 MiB versus 6.2 MiB.
Inlining also changes the number of emitted definitions, so this does not
isolate the visibility effect. `-ffunction-sections`/`-fdata-sections` are
accepted but not honored for ordinary strong definitions. The current link
does not use section garbage collection, so dead-section elimination is not
an established explanation for this run.

The normal compilation profiles execute only 258/215 instructions in the CET
pass itself. Out-of-line xxhash helpers are also tiny in these samples; that
does not bound inlined hash code. The `llvm::deallocate_buffer` wrapper itself
accounts for approximately 0.045–0.049% of main-binary instructions, excluding
allocator work and other deallocation paths. These are bounds on specific
observed bodies, not whole-mechanism percentages.

## Baseline before feature repairs

The full private cost-model LLVM was built with compiler r42. All four full
profiles reduce executed code footprint about 2.0–2.3% and calls about 5.4–6.2%
against accepted r24. The hardware screen reduces cycles about 2.2–2.7%, code
L2 misses 4.6%, and fetch-stall cycles 4.8% on its input; every output matches.

The subsequent 32-observation, 325-file paired trial is
`gap-C-cost-model-full-workload-r47/summary.json`. Every object/executable,
input, and immutable producer check passes.

| Ratio | Elapsed | Cycles | Instructions |
| --- | ---: | ---: | ---: |
| accepted r24 / native | 1.1263 | 1.0982 | 1.0507 |
| private cost model / native | 1.0911 | 1.0746 | 1.0439 |
| private cost model / accepted r24 | 0.9700 | 0.9788 | 0.9935 |

The candidate/native 95% intervals are 1.0384–1.1438 elapsed and
1.0578–1.0913 cycles. The paired candidate/baseline cycle interval crosses 1;
the trial does not resolve the incremental gain under the observed load.
It does not satisfy the 5% target. Raw observations, including interference,
are retained. These results precede all feature-define repairs.

## Implemented batch and qualification

The production batch emits implemented C++14/17 feature macros by language
mode, updates feature/attribute version probes, and checks the allocation
builtin's complete replaceable-overload contract before reporting version
201802. It derives optimization, size, PIC/PIE, and no-inline macros from the
invocation, removes the inherited unsupported CET claim, and honors
`-fno-inline` while retaining mandatory always-inline copies. SSE4.2 implies
POPCNT unless explicitly overridden, matching both host compilers.

Regression ownership is PA30 for hosted feature and target probes, PA31 for
the `<new>` overload compile case, and PA33 for size/no-inline optimization
controls. The PA31 single-test shell runner now accepts the same C++17 sidecar
already supported by the batch runner. References are generated through the
normal reference wrappers.

Validation so far:

- strict report: 10,634/10,634;
- target-feature driver routes: 228/228;
- debug info, PA36 smoke, isolated self-host through PA5, architecture audits,
  file audit, and the full harness pass; placement scans 9,974 tests with no
  placement or hygiene findings;
- 163 macros and 202 queries in all 14 mode configurations compile and run;
  all producer hashes remain unchanged. The new matrix is
  `gap-feature-paths-r59/matrix-receipt.json`, with every changed answer in
  `changes.json`;
- fixed-baseline compile instructions versus `8f76ebb0d`: O0 +0.4335%,
  O1 -0.6672%, O2 -1.4357%, O3 -1.6584%. All levels meet the existing gate,
  memory is effectively unchanged, and repeat outputs match. The optimized
  baseline/candidate objects need not match across compiler revisions;
  correctness is checked separately.

The first harness run inherited strict-comparison mode from the compiler
validation launcher. Ten harness tests intentionally exercise relaxed
comparison, so that setup was invalid. Its log is retained; the full harness
passes when rerun without the forced comparison environment. This is distinct
from the successful strict compiler report. The final receipt is `gap-feature-validation-final-r59.json`.

The LLVM rebuild uses an immutable compiler containing these production
fixes plus the same private r42 inlining policy used by the before binary.
Both private translation units were rebuilt against current headers. This
isolates the feature fixes without silently changing the inlining experiment.
The production source batch and private optimizer experiment remain distinct.

Artifacts are `compiler-feature-production-r59/`,
`compiler-feature-cost-model-r59/`, and `build-cppgm-gap-feature-O1-r59/` in
the evidence root. The full LLVM build succeeds (3,365 configured C++ units;
2,577 in the Clang target, no C++ fallback). Its 407.2-second elapsed build
measurement is an unpaired diagnostic under concurrent validation load,
not a build-speed comparison. LLVM timing and all-325 output verification
are in progress.

The private preprocessing-only diagnostic captures all 43 provider token
streams against native (`gap-feature-ppdiff-r59/`). Three X86 captures were
started before generated register tables existed; all three pass after those
build dependencies complete, and the first logs are retained. The captures
still contain compiler-identity differences, Clang versus GCC intrinsic
headers, diagnostic attributes, unsupported visibility/size attributes, and
newer-language extensions. These are not fixed by inventing feature defines.
The classifier is a triage aid, not proof of semantic equivalence for every
remaining token hunk.
The user requested completion of this validation/timing batch, a commit,
and then a pause; no subsequent optimization batch is authorized for this run.
