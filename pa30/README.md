## CPPGM Programming Assignment 30 (`abimangle`)

### Overview

Write one C++ application called `abimangle`.

`abimangle` takes normalized ABI fact files as input and writes Itanium C++
ABI mangled names. Each input case describes the semantic facts for one ABI
name: the entity being named, owner scopes, type structure, template
parameters, template arguments, dependent expressions, local contexts, ABI
tags, and special ABI-name forms.

The input is not C++ source. This assignment is about ABI name construction
only. It does not require C++ parsing, semantic analysis, LowIR generation,
object emission, linking, or runtime behavior.

### Prerequisites

Complete PA29 before starting this assignment.

You will want to reuse:

- the PA29 build and tool-driver structure
- the PA18+ template and dependent-type concepts as design background
- the PA24-PA27 namespace, class, member, lambda, and special-member concepts
  as design background
- the Itanium C++ ABI mangling rules in `../doc/itanium-mangling.txt`

The tests assume a POSIX-like shell environment with `make`, `bash`, `perl`,
and a working host C++ compiler for building the test executable.

### Starter Kit

The starter kit provides:

- `dev/abimangle.cpp`, populated with command-line handling for `abimangle`
- `pa30/abimangle.cpp`, a wrapper that builds the editable tool source from
  `../dev/abimangle.cpp`
- `pa30/Makefile`
- `pa30/scripts/`, the ABI fact test harness
- `pa30/tests/abi/`, the checked-in ABI fact tests and reference files
- shared support sources and headers under `dev/src/`
- an optional ABI fact scaffold in `dev/src/abi_mangle.h`

Put code changes in `dev/`, especially `dev/abimangle.cpp` and reusable
helpers under `dev/src/`. Do not edit generated `.my` files. Test inputs and
references are part of the handout unless your instructor asks you to add or
update tests.

The assignment-facing scaffold is the fact data model and the declared
parse/serialize/mangle API in `dev/src/abi_mangle.h`. Encoding tables,
Itanium terminal spelling, compiler semantic lowering, and other implementation
logic are intentionally outside the PA30 wrapper and test harness.

There is no separate reference binary in the starter kit. The checked-in
`.ref.*` files are the oracle.

### Command-Line Contract

Required form:

```sh
abimangle -o <outfile> <abi-facts-file>...
```

`abimangle` shall read all input fact files in command-line order and write one
mangled name for each input case to `<outfile>`.

Each output name is written on its own line:

```text
_ZN2ns1fEiPc
```

If an input file contains multiple cases, the output preserves the case order
from that file before moving to the next input file.

### ABI Fact Files

ABI fact files are line-oriented. The checked-in tests use normalized facts of
the forms described here.

Simple cases can be one line:

```text
function f
function path ns::f
variable ns::g
type ptr:const:int
typeinfo ns::C
vtable ns::C
```

The normalized builtin word `float128` denotes GNU `__float128` and uses the
Itanium builtin type code `g`. This is an ABI fact spelling, not a requirement
to parse `__float128` as C++ source in PA30.

Structured cases introduce reusable facts before the final target:

```text
let-type Char template-param 0
let-arg Char_arg type Char
let-type Traits template std::char_traits Char_arg
let-arg Traits_arg type Traits
let-type Alloc template std::allocator Char_arg
let-arg Alloc_arg type Alloc
let-type String template std::__cxx11::basic_string Char_arg Traits_arg Alloc_arg
function path std::getline Char_arg
param ref String
```

Definition forms:

- `let-type <id> ...`: a type fact
- `let-arg <id> ...`: a template-argument fact
- `let-expr <id> ...`: a dependent-expression fact
- `let-context <id> function ...`: a local or lambda context named by a
  function target
- `let-context <id> raw <context-fragment>`: a local or lambda context already
  normalized as an Itanium local-name context fragment
- `let-entity <id> ...`: an entity fact used by entity-valued template
  arguments and dependent expressions

Target forms:

- `type ...`
- `function ...` with optional following terminal and `param ...` lines
- `variable ...`
- `typeinfo ...`
- `vtable ...`
- `vtt ...`
- `construction-vtable ...`
- `tls-wrapper variable ...`
- `thunk ... function ...`
- `virtual-base-thunk ... function ...`

Function operator terminals use semantic names, not raw Itanium terminal
fragments:

```text
function path C::operator
operator-terminal plus
param int

function path operator
operator-terminal literal _digits
param ulonglong

function path C::operator
conversion-terminal int
```

Complex function encodings may also be written as a `function encoding` target
followed by normalized component lines. Template-id components use
`name-template ... <arg-ref>...`; function-template arguments use
`function-template-arg <arg-ref>`, with `function-template-prefix <key>` when
the function-template prefix is substitutable; local entities use
`local-context ...` or `lambda-context ...` followed by the same terminal,
qualifier, result, and parameter lines as ordinary functions.

Namespace-scope lambda closure types use
`type namespace-lambda <source-name> [namespace-qualifier...]`. Their call
operators may be written either as
`function namespace-lambda <source-name> <terminal> [namespace-qualifier...]`
or, in a `function encoding` case, as
`namespace-lambda-context <source-name> [namespace-qualifier...]` followed by
ordinary terminal, qualifier, result, and parameter lines. The source name is
the ABI source-name component, such as `$_0`.

`operator-terminal <name>` names the C++ operator semantically. Supported names
include `plus`, `minus`, `address-of`, `deref`, `new`, `new-array`,
`delete`, `delete-array`, `multiply`, `divide`, `remainder`, `bit-or`,
`bit-xor`, assignment operators, shifts, comparisons, logical operators,
`increment`, `decrement`, `comma`, `member-pointer`, `arrow`, `call`, and
`index`. For operators whose Itanium terminal depends on unary versus binary
use, the encoder chooses from the parameter count and member/non-member shape;
explicit names such as `unary-plus`, `binary-plus`, `unary-minus`,
`binary-minus`, `bit-and`, and `multiply` may be used when the shape should be
unambiguous.

Literal operators are written as `operator-terminal literal <suffix>`, where
`<suffix>` is the unencoded suffix source name such as `_digits`. Conversion
operators remain separate `conversion-terminal <type>` facts. Local and lambda
call-operator contexts continue to use `operator-call` as a semantic terminal
marker, not as an Itanium code.

Thunks, wrappers, typeinfo, and vtable names are described as ABI facts instead
of already-mangled names.

Raw external symbols may be carried with `let-entity <id> symbol <mangled-name>`
when a template argument or dependent expression names an entity that is already
known by ABI symbol rather than by a source-level qualified name.

Template-template arguments may name either a namespace-scope template with
`let-arg <id> template-entity <qualified-name>` or a member template of an
already-structured owner type with
`let-arg <id> member-template-entity <owner-type> <member-name> <substitution>`.
Type facts may also spell a class-template specialization whose template name
is a template-template parameter using `type template-param-template <index> <arg-ref>...`.
Member type facts use the same structured owner rule, so `type member <owner>
<name>` may be rooted in a dependent template specialization or builtin
transform type such as `__remove_const<T>`.

The fact format is deliberately small, but it is still an ABI entity graph. It
should not become a second C++ parser.

### Required ABI Coverage

The checked-in tests are numbered from simpler names toward more complete ABI
situations:

- `100-*`: basic functions, variables, named types, builtin types, pointers,
  arrays, member pointers, typeinfo, vtables, VTTs, and variadic forms
- `200-*`: ABI tags, local entities, local and namespace-scope lambdas,
  operators, conversion terminals, TLS wrappers, and thunks
- `300-*`: entity-valued template arguments, template-template arguments,
  standard substitutions, construction vtables, and dependent integral values
- `400-*`: dependent aliases and dependent member/owner types
- `500-*`: dependent expressions, casts, calls, type traits, `sizeof(type)`,
  packs, and substitution of equivalent dependent expressions
- `600-*`: nested owner contexts and standard-library-adjacent inline namespace
  cases

An implementation should handle Itanium substitution ordering, nested names,
local-name contexts, template parameter references, template arguments,
dependent expressions, ABI tags, special names, and every target form covered
by the tests.

Reference:

- Local copy of Itanium C++ ABI, Chapter 5.1 "External Names (a.k.a.
  Mangling)": [`../doc/itanium-mangling.txt`](../doc/itanium-mangling.txt)

### Output Format

The output file contains one mangled name per target, followed by a newline.

For successful test cases, standard output and standard error are ignored. You
may use them for diagnostics.

### Error Handling

If command-line parsing, input reading, fact parsing, or name construction
fails, `abimangle` shall exit with failure.

For negative tests, exact diagnostics are not the grading contract. The harness
compares exit status first. If the reference path fails, stdout and stderr are
diagnostic side effects rather than required output.

### Testing

Run the ABI naming suite with:

```sh
make test
```

To run one test through the shared check target:

```sh
make check TEST=tests/abi/100-global-function.t
```

For each test case `x.t`:

- `abimangle` is executed to produce `x.my`
- the exit status is recorded in `x.my.exit_status`
- `x.my` is compared against `x.ref`
- `x.my.exit_status` is compared against `x.ref.exit_status`

The checked-in references are the oracle. Your tests should not invoke the host
compiler, `nm`, `readelf`, `objdump`, or a demangler as a live ABI-name oracle,
because host compiler and standard-library version differences can create
noise around ABI tags, inline namespaces, and local entity numbering.

### Assignment Boundary

This assignment owns standalone ABI name construction from normalized ABI fact
files.

To complete this assignment, implement this behavior:

1. Parse normalized ABI fact files.
2. Represent the ABI facts with enough typed structure to apply the Itanium C++
   ABI mangling grammar.
3. Encode the supported fact records into deterministic mangled names.
4. Implement substitution-table behavior in host-compatible order for the
   tested cases.

If `abimangle` accepts a fact file and writes a different ABI name from the
checked-in reference, the issue belongs in this assignment.

### Out Of Scope

The following are out of scope for this assignment:

- C++ source input
- C++ source parsing or semantic analysis
- LowIR generation
- relocatable object generation or host linking
- ELF, Mach-O, COFF, archives, shared libraries, or relocation records
- vtable layout, RTTI object layout, exception handling, unwind metadata, or
  host runtime behavior beyond naming the corresponding ABI entities
- demangling
- using host object tools or host compiler output as compiler input

### Design Notes (Non-Normative)

A simple implementation strategy is to keep three concerns separate:

- fact-file parsing into typed records
- ABI name encoding from those typed records
- substitution-table state for one mangled name

The optional `abi_mangle.h` scaffold follows that shape. You may use it,
adjust it, or replace it. The tests require the behavior of `abimangle`, not a
specific internal representation.

Substitution is part of the ABI grammar, not just text de-duplication. The
encoder should record substitutions in the order required by the Itanium ABI
and should compare structured facts when deciding whether a component can reuse
an existing slot.

Avoid building names by assembling large ad hoc strings that are later
reparsed. Some ABI facts contain source spellings, but type structure,
template arguments, dependent expressions, and local contexts should remain
structured until the encoder emits the final mangled name.

### Stage Handoff

The next stage is PA31, where `cppgm++ -c` starts using host-object facts for
basic exception-handling metadata before broader host object and ABI behavior
in PA32 and PA33.
