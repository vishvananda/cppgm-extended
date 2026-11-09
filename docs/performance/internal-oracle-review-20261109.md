# Runtime oracle provenance review

The [exclusion JSON](preview/internal-exclusions.json) describes a historical
comparison, not a list of proven GCC or Clang bugs. Its original IDs, reasons
and measurements are retained. Each entry now has a review with its introducing
commit, source hash, standards basis, disposition and fresh compiler outcomes.
The published cohort and speedups have not been recalculated.

The recheck uses the frozen b7e1f8d5f compiler, GCC 15.2.0, Clang 22.1.2 and the
official GCC 16.2.0 container. All 29 normal-exit disputed controls were compiled
and executed at O0/O3 in GNU C++11; GCC 16.2 also ran in GNU C++20. GCC 16.2
retains all 22 GCC C++11 disagreements. Language mode, adopted defect resolutions
and some optimization-sensitive behavior matter here, not just compiler age.
Two intentional termination checks remain outside normal-exit timing.

## The check that disagrees with both native compilers

`pa21/tests/controls/100-potential-result-default-copy-original-value.t` requires
an observable copy during implicit `[=]` capture, followed by a read of the
original constexpr object's value. The copy constructor increments a counter
and changes the copied member from 17 to 34. Both vendors read 17 but make zero
copies; cppgm makes the copy and reads 17.

This requirement combines two adopted resolutions:

- [P0588R1](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0588r1.html)
  makes a potentially evaluated reference trigger implicit capture even without
  an odr-use. [N4709, CWG motion4](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/n4709.pdf)
  explicitly adopted it as a defect report in November2017.
- [CWG2083](https://cplusplus.github.io/CWG/issues/2083.html), accepted as a DR
  in February2019, makes this non-odr-use read the original object. It does not
  cancel the capture initialization required by P0588R1.

Captured members are directly initialized when the lambda is evaluated.
The closure-representation permission in C++11 5.1.2p3 preserves observable
behavior; the ordinary copy-elision cases do not elide copying this lvalue into
its captured member. The paper's scalar example can optimize an unused capture
away because that scalar copy has no observable effect. This constructor does.
The mandatory oracle is retained as a documented course DR exception.
[LLVM's P0588R1 issue](https://github.com/llvm/llvm-project/issues/61426) remains
open; the vendor observations are also recorded directly rather than inferred
from a broad implementation-status claim. The explicit-capture neighbor passes
all three compilers and separately checks the same copy/read distinction.

## Other dispositions

| Group | Basis and decision |
|---|---|
| Parameter defaults, PA10/14 | CWG2082 plus parameter point of declaration; retain prefix-sensitive lookup. |
| Unrelated member in `decltype`, PA11 | Original C++11 9.3.1p3; no implicit const object for an unrelated class. Retain. |
| Bit-field rvalue-reference cast, PA11 | CWG1447, adopted October2012, requires a temporary; retain the unchanged original bit-field check. |
| Implicit moves, PA12/21 | P1825R0 is deliberately applied in C++11. GCC documents support in C++20 mode; Clang backports it. Retain, with CWG2549's final February2025 scope rule where exercised. Two fixtures still disagree with GCC16.2 in C++20. |
| Constant array scalar reads, PA16/22 | CWG1926/2083 potential-result rules; retain. C++20 link success can merely reflect inline-variable storage and does not establish that GCC implements the non-odr-use rule. |
| Static initialization and temporary destruction, PA21 | C++11 6.7p4 and12.2p3 separate completed initialization from subsequent full-expression temporary cleanup; retain. |
| Construction-prefix and array-tail cleanup, PA21 | C++11 12.8p15,12.2p4 and15.2p2; retain completed-subobject and default-argument temporary cleanup. |
| Conditional initializer-list cleanup, PA21 | The outer reference does not re-extend the backing array through a copied list; retain. GCC differs only with optimization. |
| Five allocation probes, PA12/21 | Rewrite size probes to use class-specific hooks/null failure rather than insufficient storage. Direct global replacement calls retain observable routing checks; new-expression probes check balanced cleanup without insisting that native optimizers retain every global allocation. |
| Two terminating programs, PA21 | Course exit134 and host SIGABRT are alternative reporting mechanisms; exclude from normal-exit timing, not correctness. |

The allocation changes pass cppgm, GCC15.2 and Clang22 at O0/O3. They do not
introduce a requirement to implement C++14 allocation elision. Student-facing
PA21 guidance now states the adopted capture semantics explicitly; the internal
provenance review is not exported to students.
