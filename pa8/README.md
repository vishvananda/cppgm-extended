## CPPGM Programming Assignment 8 (`lowir`)

### Overview

Build the LowIR representation that your compiler will use in later
assignments. Write a C++ program named `lowir` with two interfaces:

```sh
lowir -o <outfile> <lowirfile1> [<lowirfile2> ...]
lowir --exercise <sum|swap|call> -o <outfile>
```

The first reads LowIR, validates its structure, and writes the represented
program back as LowIR. The second constructs a small program using your model
and writes it through the same serializer. Both parts are required.

This assignment introduces LowIR before C++ lowering. PA10 will generate it
from C++, and PA24 will compile it to native code. Keep the model, reader,
validator and writer in reusable modules under `dev/src/` so those assignments
extend your work.

### Prerequisites and starter kit

Complete the earlier syntax and semantic assignments. The useful prior ideas
are tokenization, parsing, names, types and structured representations; no
machine-code generation is assumed.

The starter kit provides:

- `dev/lowir.cpp`, the editable command-line scaffold, and the assignment's
  `lowir.cpp` symlink to it;
- `dev/src/lowir_model.h`, a simple example model you may adapt or replace;
- `dev/src/ir_symbol_model.h`, the shared metadata vocabulary;
- [the format reference](lowir.md), [the grammar](pa8.gram), and its HTML
  explorer under `grammar/`;
- the tests, reference outcomes and harnesses under `pa8/`;
- downloaded `lowir-ref` and `lowir2native-ref` tools for reference observation
  and the grading support described below.

Your choice of classes, containers, names and construction helpers is your own.
The course solution's interned IDs and storage layout are not a required design.
Implement compiler work in `dev/`; leave the assignment fixtures and harnesses
unchanged during normal student work.

### Implementation order

1. Represent types, literals, named symbols, functions, parameters, slots,
   basic blocks and instructions. Start with `100-ret0` and `100-local-arith`.
2. Read those structures and serialize them. Preserve program facts through
   a roundtrip, including declarations that have no use in the current unit.
3. Add globals and structured data, addresses, loads, stores and calls.
   `100-simple-call.t` and its `.t2` companion exercise multiple input files.
4. Validate block targets, terminators, operand types and phi predecessors.
5. Represent the remaining instruction and metadata forms in the specification
   tests. A field can describe work that a later backend performs: recording
   an exception operation or an atomic ordering does not require implementing
   exception handling or atomic machine instructions here.
6. Construct the three behavioral programs below through your model and writer.
   Run each exercise independently while developing it.

The ordinary fixture groups and the behavioral exercises are parts of one
assignment, not additional milestones or separately preserved products.

### Reading, validation and serialization

The input files form one LowIR unit in command-line order. Names are shared
across those files; they are not separate C++ translation units. A symbol may
be defined in a later input file. Do not introduce a duplicate declaration for
an already defined symbol merely because a file boundary occurs between uses.

Accept helper-only units, including declarations and globals without an entry
function. An entry is needed when the supplied backend builds an executable,
not when a reader writes a LowIR unit. Legacy entry/init/fini names have the
roles specified by `lowir.md`; the reference writer makes those inferred roles
explicit in its output.

The required representation includes the forms exercised by `tests/spec/`:

- scalar and object-storage types, integer/floating literals, and pointers;
- declarations, scalar and structured globals, zero/data/address initializers,
  functions, parameters, slots, blocks, and object aliases;
- constants and copies, addresses and indexed addresses, memory operations,
  arithmetic/comparisons/conversions, direct and explicitly typed indirect calls;
- jumps, branches, switches, phi values and terminators;
- the atomic and exception instruction records used by the fixtures;
- the symbol, parameter, function, storage, projection and debug metadata used
  by those fixtures, including the required placement and value constraints.

Use `lowir.md` for each field's type and meaning, and `pa8.gram` for syntax.
The full format reference also documents later additions. A feature appearing
there alone does not add a new PA8 requirement: this handout and the owning
specification fixtures define the introductory interface. Later assignments
extend that interface when their producers and consumers need additional facts.

Preserve the represented types, names, operands, literal values, declarations,
instruction order, control-flow edges and metadata. Do not optimize away unused
declarations or instructions during a roundtrip. Whitespace and metadata-field
ordering do not have to match the reference writer. Normal grading reads the
two outputs with the supplied reference reader before comparing their canonical
forms when their text differs.

Use the same writer for values created in memory: constructing the exercise
programs cannot depend on having an original LowIR spelling for every node.
You do not have to preserve comments or input whitespace.

Reject the structural errors covered by the specification tests, including
conflicting symbol identities, duplicate parameter/slot/block names, missing
terminators, instructions after a terminator, invalid targets, incompatible
operand/call types, malformed phi predecessor/type relationships, and invalid
metadata placement or values within the required subset. For a comparison,
the written type describes its operands; the result is an integer truth value.
No code generation or instruction execution is performed by this interface.

On success, write the LowIR output file and exit successfully. On a required
input failure, exit with `EXIT_FAILURE`; diagnostic text and the output file's
contents are not graded for that failed invocation. `--help` and `-h` describe
both interfaces.

### Required program-construction exercises

Construct these functions as LowIR and serialize them with `--exercise`.
They are helper-only units. The harness supplies a separate calling program,
combines it with your output, and executes the result using `lowir2native-ref`.
Do not emit the harness's entry function or redefine its callback functions.

| Exercise | Required LowIR function | Behavior |
| --- | --- | --- |
| `sum` | `@sum_to(%n : i64) -> i64` | Return the sum of the integers from 1 through `n`, including zero when `n` is zero. Inputs are in the range 0 through 1,000,000. |
| `swap` | `@swap_values(%left : ptr, %right : ptr) -> void` | Exchange the two addressed `i64` values. Each pointer names a valid aligned `i64` object; the pointers may name the same object. |
| `call` | `@call_twice(%fn : ptr, %x : i64) -> i64` | Invoke the supplied `i64(i64)` function with `x`, invoke it again with its first result, and return the second result. Preserve both calls' effects and order. |

The function symbols, parameter types/order and result types are the callable
interface. Parameter names, block names, temporary names and instruction
sequences are free choices. A loop and an arithmetic expression can both
implement `sum`; behavioral grading does not demand one lowering strategy.
The fixtures exercise several arguments, aliased storage, return values and
callback effects. They contain callers, not the function bodies you must build.

Build the exercise programs through your LowIR representation and writer.
Inspecting reference output is useful for learning; copying a reference body
or invoking the reference implementation to perform your work bypasses the
exercise. No C++-to-LowIR frontend is needed for these programs.

### Testing

From this directory:

```sh
make
make test
make check TEST=tests/spec/100-ret0.t
make check TEST=tests/behavior/100-sum.t
make check TEST='tests/spec/100-simple-call.t tests/behavior/*.t'
```

`make` builds the student `lowir` tool. `make test` runs the roundtrip suite and
all three construction exercises. `check` accepts files, globs and mixed lists
and routes each case to its corresponding check.

For a roundtrip case `x.t`, the harness invokes:

```sh
lowir -o x.my x.t [x.t2 ...]
```

It compares exit status, and on success checks the represented output against
`x.ref`. The supplied `lowir-ref` reader normalizes differing spellings for
this comparison; it is a grader, not an implementation dependency of your tool.

For a construction case, `x.exercise` selects the exercise and `x.t` supplies
the calling program:

```sh
lowir --exercise <name> -o x.my.lowir
lowir2native-ref -O0 -o x.my.program x.my.lowir x.t
x.my.program
```

The construction check requires successful LowIR generation and backend
compilation, then compares the executable's stdout and exit status. It never
compares your LowIR or a machine-IR dump against a reference. The `.my.lowir`
file remains available to inspect after a failure. No `.ref.lowir` or `.ref.mir`
file is required for behavioral grading.

`NATIVE_REFERENCE_APP` selects the supplied backend independently of the tool
under test and defaults to `../dev/lowir2native-ref`. The harness does not build
the unfinished student native backend. The downloaded wrapper fetches the
reference bundle when needed. At PA24 the native assignment will instead test
your implementation of `lowir2native`.

Finish the assignment with the root cumulative `make test-report-through-pa8`.
DWARF, debugger integration, optimization, object linking and native code
emission belong to later assignments.
