# CPPGM Part 2 Roadmap: PA10–PA39

This document outlines the proposed programming assignments for Part 2 of the `cppgm`
course.

Part 1 (`PA1`–`PA9`) reached a working preprocessing, semantic, and backend foundation. The
goal of Part 2 is a practical self-hosting C++ compiler. This revision explicitly optimizes
for assignment-sized chunks closer to the original "one solid week of work" cadence.

Two policy changes drive the structure below:

- self-hosting comes before "full independence"
- bootstrap may use the host platform's existing standard library/runtime

That means a custom stdlib, full RTTI, `dynamic_cast`, and multiple inheritance are no longer
on the critical path to self-hosting.

This revision also inserts four new milestone numbers to split previously oversized
assignments before bootstrap:

- `PA22 templatecomplete`, separating template specialization ownership from the later
  deduction/substitution/SFINAE closure
- `PA31 abimangle`, separating Itanium C++ ABI name construction from host object
  emission
- `PA33 hostabi`, separating ordinary host object interop from host C++ ABI/runtime
  behavior
- `PA35 hostedheaders`, separating heavy hosted-header compilation from smaller
  hosted intrinsics/source compatibility
- `PA36 hostedlink`, separating hosted header compilation from hosted emitted-code
  link/runtime behavior

---

## PA10: `cppast` — Real C++ AST Parser

**Goal**: Upgrade the PA6 recognizer into a real recursive-descent parser that builds an
Abstract Syntax Tree (AST).

**Topics**:
- AST node types for declarations, statements, expressions, and types
- Grammar-driven tree construction using the PA6 parser structure
- Pretty-printer / dumper for AST nodes
- Ambiguity handling: declaration vs. expression, function vs. object
- Parser treatment of `template-id` vs. `<`
- Structured `type-id` parsing for the common contexts needed by later semantic passes
- Structured template-parameter-clause parsing for later template-parameter scopes
- Anonymous class / union / enum specifiers when the same declaration immediately introduces
  the usable name
- Pack-expansion expressions in ordinary argument-list and paren-initializer syntax when the
  surrounding grammar is otherwise part of the PA10 subset

**Notes**:
- `template-id` vs. `<` cannot be fully resolved at pure parse time because the symbol table
  does not exist yet.
- Acceptable strategies include tentative parsing/backtracking or preserving an ambiguous form
  for later semantic disambiguation in PA11.
- PA10 should preserve later-needed syntax as structured nodes even when semantic
  classification is deferred.
- In particular, `type-id` and template parameter syntax belong in PA10 because PA11 depends
  on them directly.

**Input**: preprocessed C++ source
**Output**: AST dump or parse diagnostics

**Dependencies**: PA6

---

## PA11: `types` — Types, Scopes, and Lookup

**Goal**: Build the semantic foundation without taking on full overload resolution yet.

**Topics**:
- Type representation: fundamental, pointer/reference, array, function, cv-qualification
- Declarator analysis and type construction
- Scope stack: block, function, namespace, class, and template-parameter scopes
- Unqualified and qualified lookup
- `using` declarations and `using` directives
- Semantic support for `sizeof`, `alignof`, and basic `decltype`, including practical
  library-facing forms such as `decltype(sizeof(type-id))` and `decltype(nullptr)`
- Namespace-scope anonymous class / union / enum specifiers when the same declaration
  introduces the usable type name through a typedef or declarator-id
- Free-function trailing `noexcept` declarators as part of ordinary type/declarator analysis

**Out of scope for this PA**:
- full overload resolution
- conversion ranking
- template deduction

**Input**: AST from PA10
**Output**: semantic graph / annotated AST with resolved names and types

**Dependencies**: PA10

---

## PA12: `calls` — Conversions, Initialization, and Overload Resolution

**Goal**: Implement the call semantics needed to make typed C++ executable.

**Topics**:
- Value categories and lvalue/rvalue handling
- Standard conversions and reference binding
- Initialization rules needed for expressions, parameters, and returns
- Overload sets and basic overload resolution
- Non-template function/operator calls
- Block-scope `using` declarations / directives and local anonymous-union declarations inside
  function bodies
- Ordinary pointer conversions, arithmetic, comparisons, and `nullptr` handling in the first
  procedural semantic slice
- Explicit casts and compound assignment over the supported integral / enum / pointer subset
- Constructor selection at the basic level needed by upcoming object support
- Procedural control-flow statement analysis for the ordinary loop subset, including `do`

**Notes**:
- This is where overload resolution moves; it is large enough to deserve its own milestone.
- User-defined operator overloading moves later with the class/object model rather than landing in
  the first procedural slice.
- Template-aware overload machinery comes later with templates.

**Input**: typed AST from PA11
**Output**: call-resolved annotated AST

**Dependencies**: PA11

---

## PA13: `lowir2cy86` — LowIR Execution Scaffold

**Goal**: Define the compiler-owned LowIR boundary and build the runnable validation backend
for it by translating LowIR into PA9 CY86.

**Topics**:
- The full intended LowIR specification, even though PA13 only implements a procedural
  subset of it
- Parsing and validation of LowIR text
- A mechanical `LowIR -> CY86` adapter for the supported subset
- Deterministic CY86 output suitable for PA9 execution and regression testing

**Notes**:
- CY86 is explicitly a scaffold backend here, not the long-term compiler IR.
- Defining LowIR before the C++ lowering assignments avoids forcing PA14+ to lower directly
  into CY86.
- The LowIR chosen here should be friendly both to later language growth and to later
  optimization passes, without requiring SSA in PA13.

**Input**: LowIR text
**Output**: CY86 text for the supported LowIR subset

**Dependencies**: PA9

---

## PA14: `lowir` — Procedural C++ to LowIR

**Goal**: Establish the first real C++ lowering stage by translating the procedural subset
from PA12 into LowIR, with PA13 used as the runnable validation path. The frontend binary for
this milestone is `cpplowir`.

**Topics**:
- Deterministic textual LowIR as the primary PA14 output contract
- Procedural lowering of expressions, calls, returns, locals, globals, and control flow
- Reuse of the PA12 resolved semantic layer as the source of truth for lowering
- Validation of the lowered subset through `LowIR -> CY86 -> executable`

**Notes**:
- PA14 should target the same LowIR defined in PA13 rather than inventing a separate
  procedural backend format.
- This is the point where lowering knowledge starts to accumulate, so it is important that
  the boundary already be the long-term compiler IR rather than direct CY86 emission.

**Input**: call-resolved AST from PA12
**Output**: LowIR text for the supported procedural subset

**Dependencies**: PA12, PA13

---

## PA15: `classes` — Basic Object Model

**Goal**: Implement the non-polymorphic class machinery needed by ordinary C++ code and RAII.

**Topics**:
- Struct/class member layout, padding, and alignment
- Access control and member lookup
- `this`, member access expressions, and method calls
- Ordinary non-template operator overloading over the supported object-model subset, including
  member operators, non-member operators found through ordinary lookup / ADL, and chained
  reference-returning operators such as `operator<<`
- Constructors and destructors
- Semantic classification of PA10 `base-clause` and `ctor-initializer` nodes during class
  analysis
- Object lifetime and storage for locals, globals, and temporaries
- Single inheritance without virtual dispatch

**Out of scope for this PA**:
- virtual functions
- vtables
- advanced copy/move corner cases
- RTTI and `dynamic_cast`
- operator overloads that require later value semantics, especially by-value class transfer and
  copy/move assignment operators
- template-backed operator overloads

**Input**: class-containing AST and semantic graph
**Output**: extended LowIR with object layout and non-virtual method support

**Dependencies**: PA14

---

## PA16: `valuesem` — Value Semantics and Assignment Operators

**Goal**: Finish the non-polymorphic class model so ordinary user-defined value types work
cleanly before adding virtual dispatch.

**Topics**:
- Copy/move construction and assignment in the common cases used by `dev/`
- Pass-by-value and return-by-value of class objects
- Temporary materialization and the common lifetime paths needed by value semantics
- Out-of-class constructor/destructor definitions
- Delegating constructors and similar class-completion features where needed
- User-defined assignment operators in the common non-template class cases
  - especially `operator=`
- Class ABI cleanup needed for user-defined value types

**Input**: object-model-capable compiler from PA15
**Output**: extended LowIR and value-capable lowering for non-polymorphic classes

**Dependencies**: PA15

---

## PA17: `virt` — Virtual Dispatch

**Goal**: Add the polymorphic machinery on top of the completed non-polymorphic object model.

**Topics**:
- Virtual functions, vpointers, and vtables
- Virtual destructors
- Override/final behavior as needed
- Dynamic dispatch code generation
- Class ABI cleanup needed for polymorphic types

**Input**: value-semantics-capable compiler from PA16
**Output**: polymorphism-capable class semantics and dispatch-capable LowIR lowering

**Dependencies**: PA16

---

## PA18: `templates` — Basic Templates

**Goal**: Implement the first usable tier of C++ templates.

**Topics**:
- Function templates
- Class templates
- Type template parameters, type parameter packs, and template-template type parameters
- Default template arguments over the supported type/template-parameter forms, including
  defaults that refer to earlier parameters in the same template head
- Template argument deduction
- Template instantiation
- Member templates and templated member operators when they stay within the already supported
  class/value/object-model subset
- Ordinary function-template declarator forms, including trailing return types
- Template-aware overload participation at the level needed for ordinary generic code,
  including function-template operator overloads and templated member operators where the
  non-template machinery already exists

**Out of scope for this PA**:
- non-type template parameters and non-type template arguments
- explicit specialization
- partial specialization matching complexity
- two-phase lookup
- SFINAE-heavy metaprogramming
- full `constexpr` evaluation

**Input**: compiler from PA17
**Output**: instantiated template AST ready for LowIR lowering

**Dependencies**: PA17

---

## PA19: `meta` — Specialization and Compile-Time Evaluation

**Goal**: Add the first practical metaprogramming layer on top of basic templates without
letting the hardest template corners consume the entire assignment.

**Topics**:
- Non-type template parameters / arguments, including integral non-type parameter packs
- Explicit specialization for supported class/function templates
- Integral constant expressions for template arguments, including ordinary character literals,
  `sizeof...(parameter-pack)`, and supported cast expressions that fold to integral values
- `static_assert` over the supported integral constant subset, including template-dependent
  conditions that are deferred until instantiation
- Constant-valued template bindings that feed ordinary instantiation/lowering, including
  class-scope `static const` / `static constexpr` helper bindings
- Dependent vs. non-dependent names at a practical level for the supported subset, including
  dependent qualified type/value lookup needed by ordinary metaprogramming code

**Notes**:
- Full standard-conforming two-phase lookup is intentionally not required here.
- A documented single-phase approximation with known limitations is acceptable if it supports
  the self-hosting target programs.
- Partial specialization, SFINAE-heavy metaprogramming, and `constexpr` function evaluation
  remain intentionally deferred.

**Input**: template-capable compiler from PA18
**Output**: instantiated and compile-time-folded program representation ready for LowIR
lowering

**Dependencies**: PA18

---

## PA20: `constexpr` — Full Constant Evaluation

**Goal**: Complete the language-level constant-evaluation model before retargeting the
backend.

**Topics**:
- Full `constexpr` function evaluation over the intended pre-bootstrap language surface
- `constexpr` constructors, member functions, variables, and constant initialization
- Reference, pointer, object, and aggregate values in constant evaluation
- Core constant-expression rules reused by:
  - template arguments
  - `static_assert`
  - array bounds
  - enum initializers
  - ordinary constant initializers
- Constant-evaluation support for the remaining standard expression forms that earlier
  milestones left partial or pragmatic

**Notes**:
- This is a front-end semantic milestone, not a backend milestone.
- The goal here is the compiler's own constant-evaluation model, not host/vendor builtin
  probes from the bootstrap environment.
- Hosted compiler extensions and standard-library compatibility still come later.

**Input**: metaprogramming-capable compiler from PA19
**Output**: compiler with a full constant-evaluation semantic layer feeding the existing
lowering path

**Dependencies**: PA19

---

## PA21: `templatespec` — Template Entities and Specialization Model

**Goal**: Complete the template declaration and specialization model once the full constant
evaluator is available.

**Topics**:
- Alias templates and variable templates
- Explicit specialization declarations and definitions
- Class and function template partial specialization
- Partial ordering and specialization selection
- Explicit-instantiation declarations and definitions across the supported surface
- The dependent-name and instantiation behavior strictly required to make the specialization
  model work

**Notes**:
- This milestone should make template ownership and specialization selection deterministic.
- It should not yet own the full tail of function-template deduction,
  substitution-failure behavior, or the full SFINAE surface.

**Input**: constexpr-capable compiler from PA20
**Output**: compiler with a complete specialization/entity model for templates

**Dependencies**: PA20

---

## PA22: `templatecomplete` — Deduction, Substitution, and SFINAE Completion

**Goal**: Finish the remainder of the standard template language once the specialization
model is stable.

**Topics**:
- Full function-template deduction over the intended C++11 surface
- Non-deduced contexts and remaining array-bound / conversion-corner deduction cases
- Substitution-failure candidate dropping
- `enable_if` / `void_t` / detected-idiom style SFINAE behavior
- Remaining dependent-call, dependent-alias, and no-eager-instantiation behavior needed for
  ordinary generic code
- Template instantiation/lowering cleanup so generic code no longer depends on a pragmatic
  subset

**Notes**:
- This milestone owns the remaining standard template language, not just the hosted subset.
- Hosted/vendor extensions that happen to use templates still belong later in hosted
  compatibility if they are not standard-language requirements.

**Input**: specialization-complete compiler from PA21
**Output**: compiler with the intended full pre-bootstrap template surface

**Dependencies**: PA21

---

## PA23: `nativebackend` — LowIR to Native Code

**Goal**: Replace CY86 as the primary backend path by lowering LowIR directly to native
target code and native program data.

**Topics**:
- Deterministic machine-IR output as the structural backend contract
- Instruction selection from LowIR to x86-64
- Native lowering of control flow, calls, globals, constants, and stack layout
- Target-specific code/data emission structures suitable for later linking
- Retain CY86 as an optional validation/debug backend, not the primary compiler boundary
- A stable machine-IR layer that later optimization passes can target

**Notes**:
- This milestone is about retargeting the compiler from the temporary CY86 scaffold to a
  real native backend.
- It does not need to solve separate compilation and linking yet.
- Behavioral testing alone is not enough here; the machine-IR dump is the structural proof
  that lowering is happening from LowIR into a native-oriented backend representation.

**Input**: LowIR produced by PA14–PA22
**Output**: machine-IR dump plus native code/data representation suitable for final
executable emission or later object-file packaging

**Dependencies**: PA22

---

## PA24: `link` — Separate Compilation and Linking

**Goal**: Move from single-translation-unit execution to a real multi-file compiler toolchain.

**Topics**:
- Relocatable object format with code/data/rodata/bss, symbols, and relocations
- External and internal linkage across translation units
- Multi-file linking
- Static archives if practical
- Deterministic output suitable for regression testing

**Notes**:
- This PA is about linker infrastructure, not exception handling.
- The exact object format is less important than getting separate compilation and relocation
  working reliably.

**Input**: LowIR plus the native backend knowledge from PA23
**Output**: machine-object emission plus linked executable

**Dependencies**: PA23

---

## PA25: `exceptrt` — Private Exception Runtime ABI

**Goal**: Add the compiler-private exception/runtime ABI needed by the `cppeh`
compile-and-link path.

**Topics**:
- `throw`, `try`, and `catch` lowering for the internal LowIR/object pipeline
- Cleanup behavior during exceptional control flow
- Private runtime metadata and support code required by the chosen exception strategy
- ABI integration between generated objects and the `cppeh` linker/runtime path

**Notes**:
- A fully zero-cost unwinding implementation is not required for this PA.
- A documented simplification is acceptable if it is sufficient for self-hosting and does not
  make the assignment unmanageably large.
- If zero-cost unwinding is deferred, it belongs in the post-bootstrap capstone phase.
- This PA owns the compiler-private EH/runtime surface.
  In the current compiler that means symbols and support objects in the `cppgm_eh_*` family,
  plus whatever helper runtime object/archive is needed to make those symbols link and run.
- This PA does not require migration to the host C++ EH ABI.
  Keeping a private EH/runtime contract is acceptable here as long as its behavior is
  documented and deterministic.
- Ordinary host ABI/runtime interop for `cppgm++` objects is intentionally deferred to PA33.

**Input**: linked programs from PA24
**Output**: exception-capable executables on the private `cppeh` runtime path

**Dependencies**: PA24

---

## PA26: `langcore` — Core Language Closure

**Goal**: Finish the remaining mainstream C++11 language features needed before a
general-purpose self-hosting compiler is realistic.

**Topics**:
- `auto`
- captureless lambdas
- range-for
- direct braced initialization for the common scalar and array cases
- Direct braced-init expressions and direct aggregate construction over the already supported
  object/value subset
- Supported non-class functional casts and pointer/integer `reinterpret_cast`
- remaining parser/semantic/codegen gaps for ordinary C++11 source programs
- any deferred "common language" features that would otherwise make bootstrap source
  selection artificially constrained

**Notes**:
- Unlike the earlier roadmap version, this PA is not limited to "whatever the current
  `dev/` tree happens to use". The goal is to finish the ordinary C++11 core language that
  users reasonably expect to write and that we can test directly.
- Lambda syntax is already expected to exist by the end of PA10; the remaining work here is
  first-tier semantic analysis and lowering.
- Capturing lambdas and real `std::initializer_list` interoperation are intentionally
  deferred to PA27 so PA26 stays week-sized.
- Host `<initializer_list>` support is still intentionally allowed during bootstrap.

**Input**: compiler from PA25
**Output**: compiler with broad ordinary C++11 source coverage

**Dependencies**: PA25

---

## PA27: `langcomplete` — Advanced Language Closure

**Goal**: Finish the deferred language and object-model features needed so bootstrap does not
depend on carefully choosing a "safe" subset of C++.

**Topics**:
- capturing lambdas and closure objects
- `std::initializer_list` language/library interoperation
- RTTI and `typeid`
- `dynamic_cast`
- the remaining deferred advanced-language semantics that still fit over the existing
  single-inheritance object model

**Notes**:
- This PA exists because "all language features needed before bootstrap" is too large for
  one milestone in practice.
- Multiple inheritance and virtual inheritance are intentionally deferred one more step so
  PA27 can stay focused on language closure rather than ABI redesign.

**Input**: compiler from PA26
**Output**: compiler with the deferred advanced-language slice over the current object model

**Dependencies**: PA26

---

## PA28: `objectcomplete` — Non-Virtual Multi-Base Object Closure

**Goal**: Finish the remaining non-virtual object-model work needed before the full
multi-vptr / virtual-inheritance ABI stage.

**Topics**:
- non-virtual multiple inheritance
- member lookup and access across multiple base subobjects
- constructor, copy, and destructor generation across multiple non-virtual bases
- the remaining `dynamic_cast` / RTTI case that still fits the current single-vptr ABI:
  `dynamic_cast<void*>`

**Notes**:
- This milestone exists so PA27 can stay week-sized while still avoiding a permanent
  single-base-only object model.
- Virtual inheritance and polymorphic multiple inheritance are intentionally deferred one
  more step, because they require a broader ABI redesign than the rest of PA28.

**Input**: compiler from PA27
**Output**: compiler with the full intended non-virtual multi-base object-model surface

**Dependencies**: PA27

---

## PA29: `multivirt` — Full Virtual / RTTI ABI Completion

**Goal**: Finish the remaining object-model and RTTI work that depends on virtual
inheritance or polymorphic multiple inheritance.

**Topics**:
- virtual inheritance
- polymorphic multiple inheritance
- multi-vptr and virtual-base layout
- the remaining `dynamic_cast` and RTTI cases that depend on that ABI
- any remaining object-model lowering that cannot be expressed cleanly over the PA28
  non-virtual multi-base model

**Notes**:
- This milestone owns the broader ABI redesign that PA27 and PA28 intentionally deferred.
- Keeping it separate prevents the toolchain milestone from becoming an ABI/runtime catch-all.

**Input**: compiler from PA28
**Output**: compiler with the full intended pre-bootstrap object-model and RTTI surface

**Dependencies**: PA28

---

## PA30: `toolchain` — `cppgm++` Driver and Internal Toolchain Integration

**Goal**: Turn the compiler into a usable standalone toolchain entrypoint rather than a
collection of assignment-specific binaries.

**Topics**:
- A `cppgm++`-style driver binary
- Common compile/link command-line flags needed for the internal toolchain (`-c`, `-o`,
  include paths, target selection, separate compile vs link, etc.)
- Integration with the PA23 native backend and PA24 separate-compilation pipeline
- Source-level separate compilation using ordinary declarations such as `extern int g;`
- Ability to compile and link larger ordinary C++ projects through the internal object and
  linker pipeline, not just assignment fixtures

**Notes**:
- This is the milestone where the compiler stops being "the PA binaries" and starts looking
  like a practical compiler toolchain for its own ecosystem.
- Itanium C++ ABI name construction is intentionally deferred to PA31.
- Host-linker-compatible object output and explicit external-library interop are
  intentionally deferred to PA32.
- Standard-library header ingestion and hosted/vendor compatibility are intentionally
  deferred to PA34.

**Input**: compiler from PA29
**Output**: practical `cppgm++` compiler driver and toolchain

**Dependencies**: PA29

---

## PA31: `abimangle` — Standalone Itanium ABI Naming

**Goal**: Build a reusable Itanium C++ ABI name encoder before host object output makes raw
symbol spelling part of the object-file contract.

**Topics**:
- A standalone `abimangle` tool
- Normalized ABI fact files as input
- Typed representation of ABI entities, types, template arguments, dependent expressions,
  local contexts, ABI tags, special names, thunks, TLS wrappers, vtables, typeinfo, and VTTs
- Itanium substitution-table behavior in host-compatible order
- Checked-in fact-file references rather than live host compiler or object-tool oracles

**Notes**:
- This PA does not require C++ source parsing, semantic analysis, LowIR generation, object
  emission, linking, or runtime behavior.
- The point is to make the naming layer testable in isolation. PA32 then feeds real compiler
  semantic entities into this layer when it emits host-linker-compatible objects.
- Earlier LowIR-producing assignments may carry `object=...` metadata, but relaxed LowIR
  comparison omits those late object spellings. PA31 is where the ABI spelling itself becomes
  a focused public assignment contract.

**Input**: compiler/toolchain foundation from PA30
**Output**: standalone ABI name encoder ready for host object emission

**Dependencies**: PA30

---

## PA32: `hostinterop` — Host Toolchain Interoperability

**Goal**: Make `cppgm++ -c` produce objects that the real host linker accepts, and validate
the ordinary host object/toolchain interoperability surface.

**Topics**:
- Host-linker-compatible object output as a required public contract
- Hosted entry/runtime conventions such as `main(argc, argv)` under the host CRT
- Practical `extern "C"` function interop with host-built code
- Validation against host-built objects, static archives, and shared libraries
- Weak/coalesced duplicate-definition semantics for inline/template output where the main
  question is still ordinary object/link behavior
- A two-step compile-then-host-link workflow that keeps the host final link explicit

**Notes**:
- This milestone intentionally narrows the object-format contract compared with PA24/PA30:
  host-compatible objects are required here.
- It is acceptable for the implementation to keep the internal linker path for earlier
  milestones while validating host interoperability through an external host-link step.
- Full hosted-header and vendor-compatibility work is intentionally deferred to PA34.
- Ordinary hosted runtime references needed by generated objects should resolve through the
  real host toolchain here rather than through per-test helper runtime source files.
  In practice this includes host allocation/deallocation, libc/libm-style builtins, and the
  normal host CRT entry surface.
- The practical host C++ ABI/runtime surface for imported/exported vtables, RTTI,
  `dynamic_cast` / `typeid`, and ordinary host-linked EH is intentionally deferred one more
  step to PA33.
- The remaining compiler-private EH/runtime surface is still owned by PA25, not PA32.

**Input**: compiler from PA31
**Output**: `cppgm++` objects that participate in the host toolchain, plus host-linked
executables built from those objects under the ordinary host toolchain path

**Dependencies**: PA31

---

## PA33: `hostabi` — Host C++ ABI and Runtime Interoperability

**Goal**: Keep the host-linked object path from PA32, but raise the contract from "host
linker accepts these objects" to "the ordinary host C++ ABI/runtime surface actually works."

**Topics**:
- Imported/exported vtable and RTTI ownership through the host ABI
- `dynamic_cast` / `typeid` over the supported host-linked surface
- Covariant return adjustment and related thunk behavior
- Ordinary host-linked exception behavior in the tested subset
- Foreign/host ABI interaction cases whose main question is runtime ABI behavior rather than
  plain symbol import

**Notes**:
- This is distinct from PA25 private `exceptrt`, which still owns the private
  `cppgm_eh_*` internal runtime path.
- This is also distinct from PA34-PA36 hosted compatibility; PA33 is about the ordinary
  host C++ ABI/runtime path once host link succeeds at all.

**Input**: host-linkable compiler from PA32
**Output**: host-linked objects that participate correctly in the ordinary host C++ ABI and
runtime surface

**Dependencies**: PA32

---

## PA34: `hostedcompat` — Hosted Intrinsics and Source Compatibility

**Goal**: Make the compiler source-compatible with the hosted standard-library and vendor
extension environment needed for bootstrap.

**Topics**:
- Hosted preprocessor compatibility such as:
  - practical predefined macro import
  - `_Pragma`
  - `__has_*`
  - `#include_next`
  - ignored unknown pragmas and similar compatibility behavior
- GNU/Clang parser concessions and declaration forms commonly used in hosted headers
- Hosted builtin traits, transforms, intrinsics, and compatibility helpers used by the
  selected bootstrap standard library and CRT headers
- Semantic and lowering compatibility for the hosted source patterns exercised by that
  environment
- Header-free hosted compile anchors and escalating hosted source-compilation tests

**Notes**:
- This milestone is about hosted compatibility, not the standard C++ language contract of
  the earlier milestones. Standard-language bugs found here should still move backward to
  their real owners whenever possible.
- The public test surface should stay centered on `cppgm++`, even when some tests are
  logically "preprocessor" or "header compatibility" oriented.
- PA34 should not become a second runtime-ABI assignment.
  New hosted-compatibility coverage should normally compile and link through the plain host
  toolchain surface already established by PA32/PA33.
- If a PA34-hosted smoke still needs a helper runtime object, that helper should be limited
  to the explicit PA25-owned private EH/runtime surface rather than reintroducing general
  host-builtin shims.

**Input**: host-ABI-compatible compiler from PA33
**Output**: compiler that can consume the hosted headers and source patterns needed for
bootstrap

**Dependencies**: PA33

---

## PA35: `hostedheaders` — Heavy Hosted Header Compilation

**Goal**: Compile the heaviest real hosted standard-library headers end to end without
requiring link/runtime correctness yet.

**Topics**:
- Heavy standard-library header parsing and semantic analysis
- Template, trait, overload-resolution, and dependent-name depth exercised by headers such
  as `<vector>`, `<unordered_map>`, `<tuple>`, `<random>`, and `<functional>`
- Compile-only anchors that prove the header was genuinely consumed
- Performance discipline for large hosted-header translation units

**Notes**:
- PA35 is still a compile-only assignment. Hosted header-emitted code that compiles but does
  not link or run belongs to PA36.
- Intrinsics, parser concessions, and header-free hosted anchors remain in PA34.

**Input**: hosted source-compatible compiler from PA34
**Output**: compiler that can compile heavy hosted headers to host objects

**Dependencies**: PA34

---

## PA36: `hostedlink` — Hosted Header Emission and Link Compatibility

**Goal**: Once hosted headers preprocess and compile, make their emitted inline/template
code link and run correctly through the existing host toolchain path.

**Topics**:
- Emitted definitions from hosted headers and templates
- Hosted standard-library code that now compiles but still has to link and run correctly
- Symbol ownership, ABI spelling, and runtime behavior of hosted header-generated code
- Hosted link smokes where the main question is emitted-code correctness rather than source
  acceptance

**Notes**:
- This is still not a second general runtime-ABI assignment.
- It remains scoped to hosted header-emission and hosted library compatibility on top of the
  ordinary host ABI path already owned by PA33.
- Current `tests/frontier`-style bootstrap reduction remains an internal maintainer tool,
  not a public assignment surface.

**Input**: heavy hosted-header compile-compatible compiler from PA35
**Output**: hosted header-emitted code that links and runs through the plain host toolchain

**Dependencies**: PA35

---

## PA37: `optimize` — Optimization Passes

**Goal**: Add the first optimization layer over the compiler-owned backend representation.

**Topics**:
- Deterministic LowIR optimization as a first-class stage, with a standalone `lowiropt`
  oracle and shared `cppgm++ -O*` integration
- Canonical cleanup opportunities such as constant folding, copy/constant propagation,
  CFG cleanup, safe dead-code elimination, and simple non-escaping slot promotion
- Structural before/after oracles that prove the optimizer is operating on the intended IR
- Validation that optimized output preserves behavior

**Notes**:
- This milestone should improve the compiler-owned LowIR path rather than introducing a new
  backend representation or moving the main contract down to target-specific machine IR.
- The important point is establishing a clean and testable optimization stage with public
  `-O0`, `-O1`, and `-O2` levels.

**Input**: hosted source/header compatible compiler from PA36
**Output**: optimized LowIR feeding the native backend

**Dependencies**: PA36

---

## PA38: `machineopt` — Machine Backend Optimization

**Goal**: Add local and whole-function optimization over the lowered machine-IR/native
backend representation.

**Topics**:
- `lowir2native -O1` local machine-IR cleanup
- `lowir2native -O2` whole-function machine-IR cleanup
- Preservation of generated program behavior
- Machine-IR shape oracles that prove optimization is happening below the LowIR boundary

**Notes**:
- PA37 optimizes LowIR before backend lowering. PA38 starts after that boundary and improves
  the target-facing machine path.
- `-O0` remains the PA23 baseline.

**Input**: optimized LowIR and native backend from PA37
**Output**: optimized machine IR and native executables

**Dependencies**: PA37

---

## PA39: `inception` — Self-Hosting Inception

**Goal**: Build `cppgm++` with `cppgm++`, then build it again and compare the two compiler
outputs byte for byte.

**Topics**:
- Self-built `cppgm++-self`
- Inception-built `cppgm++-inception`
- Preservation checks through the earlier assignment test ladder
- Reproducible output and deterministic build behavior

**Notes**:
- PA39 does not add a new language feature, command-line mode, object format, or runtime ABI.
- Failures discovered here should usually move backward to the earlier assignment surface that
  owns the missing language, lowering, runtime, linking, optimization, or determinism behavior.

**Input**: optimized compiler implementation from PA38
**Output**: reproducible self-built compiler

**Dependencies**: PA38

---

## Dependency Graph

```text
PA6 (recog)
  └─ PA10 (cppast)
       └─ PA11 (types)
            └─ PA12 (calls)
                 └─ PA13 (lowir2cy86) ──── PA9 (cy86 execution backend)
                      └─ PA14 (lowir)
                           └─ PA15 (classes)
                                └─ PA16 (valuesem)
                                     └─ PA17 (virt)
                                          └─ PA18 (templates)
                                               └─ PA19 (meta)
                                                    └─ PA20 (constexpr)
                                                         └─ PA21 (templatespec)
                                                              └─ PA22 (templatecomplete)
                                                                   └─ PA23 (nativebackend)
                                                                        └─ PA24 (link)
                                                                             └─ PA25 (exceptrt)
                                                                                  └─ PA26 (langcore)
                                                                                       └─ PA27 (langcomplete)
                                                                                            └─ PA28 (objectcomplete)
                                                                                                 └─ PA29 (multivirt)
                                                                                                      └─ PA30 (toolchain)
                                                                                                           └─ PA31 (abimangle)
                                                                                                                └─ PA32 (hostinterop)
                                                                                                                     └─ PA33 (hostabi)
                                                                                                                          └─ PA34 (hostedcompat)
                                                                                                                               └─ PA35 (hostedheaders)
                                                                                                                                    └─ PA36 (hostedlink)
                                                                                                                                         └─ PA37 (lowiropt)
                                                                                                                                              └─ PA38 (machineopt)
                                                                                                                                                   └─ PA39 (inception)
```
