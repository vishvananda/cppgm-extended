# PA21 lifetime acceptance

Lifetime regression programs should check completed construction prefixes,
destruction order, exception selection, live objects and static-storage lifetime.
Their complete module layout is not an extra language requirement. The 26
self-checking construction-prefix, inner-handler, guarded-static and backing-array
regressions now use the existing `control_t` lane. The remaining 172 LowIR/fact
fixtures retain their normal comparator. With the new static-completion regression,
PA21 has 233 tests: 172 text/fact fixtures and 61 behavior controls.

Fourteen moved programs already assert their runtime outcomes. Twelve previously
printed construction/destruction traces without checking the trace. They now use
unsigned trace digests that also include the selected path,
injected failure point, result and lifetime counters. The expected traces were
cross-checked against independent GCC and Clang executions before conversion.
No `printf` runtime is required. These controls use the same public reference
backend and normal result reporting as the existing controls. PA8 structural
rules and the focused IR/fact checks remain requirements.

Function-local static initialization completes before destruction of its
initializer's unextended temporaries. Record that boundary in lowering, register
exit destruction and retire the host guard-abort cleanup before destroying those
temporaries. Preserve constructor-failure cleanup and retry. The new control
covers class, scalar and reference statics, constructor failure, a temporary
destructor that throws after successful initialization, repeat entry, and exactly
one exit destruction of the completed class object.

The old compiler fails this control in both course LowIR execution and host-object
mode. The corrected compiler and GCC pass both applicable paths; the independent
Rust student compiler also passes through the supplied backend. The installed
Clang reconstructs the static after the temporary destructor throws, so it is not
used as the oracle for this boundary. The course follows the initialization and
full-expression distinction in C++11 6.7p4, 12.2p3 and 3.8p1; GCC's
[static-local implementation rationale](https://gcc.gnu.org/legacy-ml/gcc-patches/2004-08/msg02293.html)
also explicitly distinguishes these two failures.

Performance validation uses immutable before/after compiler binaries on PA35's
frozen workload with three user-mode hardware-counter samples per level. At
`-O0` the after/before instruction ratio is 0.999956; at `-O3` it is 1.000002.
Both output objects are byte-identical. There is no measured instruction-count
regression on this workload; this does not establish every input's cost.

Validation also passes the full strict report (7,041/7,041), debug-info tests,
self-host through PA5, inception, PA35 smoke, architecture/placement/file audits
and the harness tests. Both the solution and the independent Rust student
compiler pass all 233 PA21 checks. A deliberately corrupted destruction trace
fails the behavior checker. The student export regenerates and verifies 20,285
portable reference output files.
