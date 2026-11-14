## CPPGM Programming Assignment 38 (`cppgm++ --emit-lowir -x c`)

### Overview

This optional assignment adds C input to the existing compiler. It uses the
same LowIR boundary as PA10 through PA23: a `.t` source file is parsed and
analyzed once, then lowered to the PA8 LowIR model. Both output backends use
those typed facts. A C input must not be translated into C++ or passed to an
external C compiler.

PA38 is experimental and is excluded from the normal C++ test report. It does
not change the required C++ assignments or their language rules.

### Prerequisites

Complete the source frontend, typed LowIR lowering and hosted-header work
through PA30. PA37 is needed for the additional Wasm test target.

### Driver and output

The assignment's test mode is:

```sh
cppgm++ --emit-lowir -O0 -x c -std=c17 -o output.lowir input.t
```

`-std=gnu17` selects the GNU extension dialect. The input language and target
are separate choices. `-x c++` continues to select C++; a language and standard
that disagree must be diagnosed. The existing `.c` handling is unchanged unless
C is selected explicitly.

The output is ordinary [PA8 LowIR](../pa8/lowir.md). Preserve target sizes,
alignment, function signatures, object names, qualifiers and conversions. C
functions and objects use their C ABI names. A C++ `extern "C"` declaration
continues to have C++ syntax and semantics; language linkage does not select C.

### Initial C profile

The first fixture set covers these parts of the C17 input profile:

- C keywords, preprocessing tokens, feature macros and C++ keywords used as
  ordinary C identifiers; `_Bool`, `_Static_assert`, `_Alignas`, `_Alignof`
  and `_Noreturn` use their C meanings.
- Procedural scalar and pointer expressions, control flow, prototype-based
  function declarations, calls, callbacks and variadic argument promotions.
- Implicit conversions between object pointers and qualified `void*`, without
  discarding pointee qualifiers; integral initialization of enumeration objects.
- C character and string literal types. Comparisons and logical operators
  produce `int`. Assignment, comma, conditional and prefix increment results
  do not become C++ lvalues.
- Struct and enum tags, typedefs, aggregate initialization, member access,
  function parameters and results, C object linkage and tentative definitions.
- Hosted C header declarations and their builtin operations through the
  existing hosted environment.

This is an initial profile, not a claim of complete C17 conformance. Function
declarations without prototypes are explicitly unsupported; write `(void)`
for no parameters. K&R definitions, variable-length arrays, `_Generic`, and
general compound-literal and designated-initializer support are outside this
initial fixture set. Extend the profile and its tests together. Do not silently
apply a C++ rule where a supported C construct has a different meaning.

Share preprocessing, declarator parsing, type storage, ABI classification,
optimization and backend machinery with the C++ path. Record the source
language explicitly. Retain semantic facts through lowering rather than
reclassifying rendered source or LowIR text.

### Tests and reference tools

The directory and test commands follow the other LowIR frontend assignments:

```sh
make
make test
make check TEST=tests/general/100-language-facts.t
make run INPUT=tests/general/100-language-facts.t
make ref-test
```

`tests/general/*.t` inputs produce LowIR and are compared with the checked-in
reference outputs. Rejected inputs have the usual failure-status reference.
An adjacent `.t.1` adds another translation unit, as in the earlier source
assignments. `tests/controls/*.t` state an expected program result and run the
generated LowIR through the supplied native backend. There is no second C
compiler or compiler-to-compiler comparison in this assignment's test lane.

`make test-wasm` runs the same source inputs through PA37's Wasm checker and
their `.wasm-expect` result sidecars. It requires the pinned Wasm tools and SDK.
From the repository root, `make test-pa38` selects this assignment explicitly;
`make test-c` and `make test-c-wasm` are convenience aliases.
