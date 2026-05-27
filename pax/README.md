## CPPGM Programming Assignment X (`abimangle` and ABI names)

### Status

This is a draft assignment staged in `pax/` until we choose its final numbered
position. The intended insertion point is between PA30 and PA31, so PA31 can
focus on host object interoperability instead of introducing the Itanium
mangling model at the same time as host-linkable object emission.

### Overview

PAX is the ABI naming assignment. It requires a typed Itanium C++ ABI name
encoder and a source-integration path that feeds that encoder from the
compiler's semantic model.

The assignment deliberately splits the work into two observable contracts:

- a standalone ABI fact input that describes already-resolved ABI entities,
  types, template parameters, template arguments, and declaration contexts
- a source integration mode that emits the ABI names the compiler would use for
  declarations in ordinary C++ translation units

The standalone input isolates the mangler core from parsing, semantic analysis,
object emission, and runtime behavior. The source integration mode prevents
students from passing a standalone DSL while their compiler still lacks a
usable semantic-to-ABI-name bridge.

The checked-in standalone tests use a line-oriented fact form. Simple tests are
one line, such as `function ::ns::f int` or `type ptr:const:int`. Structured
tests introduce named facts before the result:

```text
let-type T template-param 0
let-type U template-param 1
let-arg U_arg type U
let-type Rebind member-template T rebind U_arg
type member Rebind other
```

`let-type` names canonical ABI types, `let-arg` names template arguments, and
`let-expr` names dependent expressions. `let-context` names a typed enclosing
function context for local classes and lambdas, and `let-entity` names an ABI
entity that can be used by entity-valued template arguments. Result lines are
`type ...` or `function ...`, with optional `param ...` lines for function
parameter types. Operator terminals use semantic names such as `operator-call`,
not Itanium terminal spellings. This keeps the standalone tests as normalized
ABI facts instead of source code or direct calls into an implementation-specific
mangler API.

The scaffold exposes the text format as typed semantic ABI facts in
`abi_mangle.h`. `AbiFactFile` contains `AbiFactCase` records; each case has an
ordered list of `AbiFact` definitions and one `AbiMangleTarget`. Single-case
files do not need an explicit label. The facts use typed records such as
`AbiType`, `AbiTemplateArg`, `AbiDependentExpr`, `AbiFunctionPath`, and
`AbiEntity`, not raw token lines. `parse_fact_text` reads the line format into
those records, and `serialize_fact_file` writes the same normalized format back
out. The reference tool round-trips parsed facts through the serializer before
mangling, then encodes names only from the typed records. If a required
maintainer mangling case cannot be expressed through these records, the ABI fact
surface is missing semantic data and should be extended.

The standalone ABI tests are numbered from simpler names toward more complete
ABI situations. Each checked-in test file covers exactly one mangled name:
`100-*` covers basic names and types, `200-*` local entities, `300-*`
entity-valued template arguments, `400-*` dependent member types, `500-*`
dependent expressions, and `600-*` nested template owner contexts.

PAX does not require producing relocatable objects, linking, or executing
programs.

### Prerequisites

Complete PA30 before starting this assignment.

You will reuse:

- the full C++ language pipeline through PA30
- the PA18+ template and dependent-type model
- the PA27-PA29 class, member, namespace, lambda, and special-member semantic
  model
- the PA30 `cppgm++` driver shell

You will add a reusable ABI naming layer that later assignments can use for
host object emission, vtables, RTTI, exception objects, thunks, template
instantiations, and standard-library-adjacent symbols.

### Assignment Surface

PAX has two required modes.

#### Standalone ABI Fact Mode

Add a tool named `abimangle`:

```sh
abimangle -o <outfile> <abi-facts-file>...
```

Each input is a small ABI fact file. The format is not C++ source. It is a
normalized entity graph that already encodes the semantic facts required by the
Itanium mangling grammar:

- declaration context and source-visible entity name
- entity kind: function, variable, typeinfo, vtable, VTT, thunk, constructor,
  destructor, operator, conversion function, or lambda-related name
- language linkage and symbol binding when it affects the name table
- canonical type structure, including cv-qualification, references, pointers,
  arrays, functions, member pointers, and class/enum names
- template parameter depth/index/pack identity
- template arguments, including type arguments, non-type template arguments,
  template-template arguments, packs, dependent expressions, and defaulted
  arguments that participate in the ABI spelling
- alias targets, dependent names, inline namespace ownership, anonymous
  namespace ownership, local entity context, and ABI tags where required

`abimangle` writes one deterministic ABI name per input case. A draft output
shape is:

```text
_ZN2ns1fEiPc
```

The final fact-file syntax can be line-oriented or S-expression-like, but it
must remain an ABI entity graph rather than a second C++ frontend.

#### Source Integration Mode

Add a `cppgm++` mode:

```sh
cppgm++ --emit-abi-names -o <outfile> <srcfile>...
```

The output is a deterministic table of selected ABI-visible names discovered
from the compiler's semantic model. A draft output shape is:

```text
defined strong function ::ns::f(int) _ZN2ns1fEi
defined weak function-template ::algo::id<int>(int) _ZN4algo2idIiEET_S1_
undefined external function ::host::take(int) _ZN4host4takeEi
```

The source mode must feed the same typed ABI entity model as `abimangle`.
Formatting a source declaration, reparsing it, demangling it, or reading object
tool output is not an acceptable bridge.

### Required ABI Coverage

The standalone fact tests should cover these areas before PA31 depends on
them:

- ordinary external names for functions, variables, namespaces, nested names,
  local names, anonymous namespaces, inline namespaces, and C linkage
- builtin, qualified, pointer, reference, array, function, member-pointer, enum,
  class, and dependent types
- substitution table behavior and substitution ordering
- Itanium standard substitutions such as `St`, `Sa`, `Sb`, and standard-library
  inline namespace interactions used by the course tests
- function templates with type parameters, non-type template parameters,
  default arguments, template-template parameters, and parameter packs
- dependent return types, dependent aliases, dependent expressions, `decltype`,
  and owner-template references
- constructors, destructors, conversion functions, overloaded operators, special
  member entry points, vtables, typeinfo, VTTs, and thunks
- lambda and local-class naming where later assignments need host ABI names
- ABI tags and weak/ODR-mergeable emitted template names

The source integration tests should be smaller. They should verify that real
semantic declarations feed the same ABI model for representative examples from
the standalone matrix, especially dependent template and owner-context cases.

### Architecture Requirements

The implementation should introduce a typed ABI naming model, not a collection
of string helpers. Expected core records include:

- `AbiEntity`, with entity kind, context, language linkage, ABI tags, special
  name kind, and optional local/lambda discriminator
- `AbiType`, with canonical type structure and source spelling only where the
  ABI grammar requires source identity
- `AbiTemplateParam`, with depth, index, pack identity, and kind
- `AbiTemplateArg`, with typed value, type, template, pack, and dependent
  expression variants
- `AbiDependentExpr`, with template/function parameters, literals, operations,
  member expressions, and entity references
- `AbiFunctionPath`, with declaration ownership, template arguments, and
  function parameter types for context-sensitive names
- `AbiMangleContext`, owning the substitution table and current template
  parameter environment

Both `abimangle` and `cppgm++ --emit-abi-names` must build these records and
call the same Itanium encoder. Later PA31/PA32 object emission should consume
the resulting object symbols instead of reimplementing mangling rules in the
object backend.

### Testing

The eventual test layout should be:

- `tests/abi/`: standalone ABI fact files and checked-in `.ref` outputs
- `tests/source/`: C++ source integration tests and checked-in `.ref` outputs
- optional instructor-only oracle scripts that compare generated names against
  host compiler output when regenerating references

Student tests should compare checked-in references. They should not invoke the
host compiler as a live mangling oracle, because compiler version differences
can create noise around ABI tags, standard-library inline namespaces, and local
entity numbering.

### Assignment Boundary

PAX owns ABI name construction.

To complete PAX, an implementation must:

1. Parse normalized ABI fact files into typed ABI records.
2. Encode those records using the Itanium C++ ABI mangling grammar.
3. Produce deterministic source integration output from real semantic entities.
4. Use one shared ABI naming model for both inputs.
5. Preserve earlier PA30 behavior.

If a later host object test fails only because the raw symbol spelling is wrong,
the missing behavior should either be covered by PAX or intentionally deferred
from PAX in this README.

### Out Of Scope

PAX does not require:

- relocatable object generation or host linking
- ELF, Mach-O, COFF, archives, shared libraries, or relocation records
- vtable layout, RTTI object layout, exception handling, unwind metadata, or
  host runtime behavior beyond naming the corresponding ABI entities
- demangling
- using `nm`, `readelf`, `objdump`, or host compiler output as compiler input
- a full second C++ frontend for the standalone ABI fact format

### Stage Handoff

After PAX, PA31 should treat ABI names as available compiler facts. PA31 should
verify that host-linkable objects preserve those facts through symbol tables,
bindings, weak/ODR coalescing, and host linker behavior. PA32 should then add
host C++ ABI/runtime object semantics such as vtable ownership, RTTI layout,
exception handling, and thunks while continuing to use the PAX ABI naming
layer.
