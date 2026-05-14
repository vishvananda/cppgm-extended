**Goal**
Break `dev/src/callsemantic.cpp` into smaller implementation files by functionality without changing behavior. Once the split is stable and `make test-report` is green again, use the smaller translation units to profile and reduce the remaining bootstrap hotspot in the specific file that still dominates.

**Constraints**
- Preserve the current public API in [`dev/src/callsemantic.h`](/Users/vishvananda/cppgm/dev/src/callsemantic.h).
- Do not mix structural refactor and semantic fixes in the same slice unless the refactor itself exposes a compile break.
- Keep the split platform-neutral and build-neutral. No Makefile tricks beyond adding the new `.cpp` files to normal compilation.
- Keep the pre-existing semantic behavior identical until the split is complete and validated.

**Target Layout**
- [`dev/src/callsemantic.cpp`](/Users/vishvananda/cppgm/dev/src/callsemantic.cpp)
  Entry points only:
  - `analyze_calls_translation_unit(...)`
  - `describe_calls_translation_unit(...)`
  - shared diagnostic wrapper logic
- [`dev/src/callsemantic_internal.h`](/Users/vishvananda/cppgm/dev/src/callsemantic_internal.h)
  Shared internal declarations:
  - internal namespace replacing the current single-TU anonymous namespace
  - `Analyzer` class declaration
  - shared helper structs/types used across split files
  - declarations for non-member helper functions that must cross translation units
- [`dev/src/callsemantic_text.cpp`](/Users/vishvananda/cppgm/dev/src/callsemantic_text.cpp)
  Text/token/spelling helpers:
  - type spelling and reparseable text helpers
  - identifier-token extraction
  - `replace_*_identifier_token_text`
  - bound-name rewrite helpers
  - fragment parsing cache helpers
- [`dev/src/callsemantic_lookup.cpp`](/Users/vishvananda/cppgm/dev/src/callsemantic_lookup.cpp)
  Type/value/scope lookup:
  - `lookup_type_impl`
  - qualified owner resolution
  - direct/qualified function and value lookup wrappers
  - dependent named-type resolution helpers
- [`dev/src/callsemantic_templates.cpp`](/Users/vishvananda/cppgm/dev/src/callsemantic_templates.cpp)
  Template parsing and instantiation:
  - template-id parsing
  - template argument resolution/binding
  - alias/class/function/variable template instantiation hooks
  - current-specialization matching helpers
- [`dev/src/callsemantic_class.cpp`](/Users/vishvananda/cppgm/dev/src/callsemantic_class.cpp)
  Class collection and registration glue:
  - class/member registration wrappers
  - class reference-member and full-member collection wrappers
  - implicit special-member plumbing wrappers
  - enum/class declaration collection entry points
- [`dev/src/callsemantic_decls.cpp`](/Users/vishvananda/cppgm/dev/src/callsemantic_decls.cpp)
  Declaration parsing/registration:
  - declarator/decl-spec parsing wrappers
  - function registration/indexing
  - out-of-class binding resolution
  - simple declarations, function definitions, template declarations
- [`dev/src/callsemantic_symbols.cpp`](/Users/vishvananda/cppgm/dev/src/callsemantic_symbols.cpp)
  Output and linkage plumbing:
  - output requirement tracking
  - object/internal/exported symbol allocation
  - symbol linkage decisions
  - late required method tracking
- [`dev/src/callsemantic_analysis.cpp`](/Users/vishvananda/cppgm/dev/src/callsemantic_analysis.cpp)
  Top-level semantic analysis glue:
  - constructors
  - `Analyzer::analyze`
  - top-level declaration walk
  - expression/statement/lifetime wrapper calls
  - remaining local glue that does not belong elsewhere

**Slice Order**
1. Introduce `callsemantic_internal.h` and move the current anonymous-namespace declarations into a named internal namespace.
2. Move the low-risk text/token helper band into `callsemantic_text.cpp`.
3. Move lookup helpers and dependent-resolution helpers into `callsemantic_lookup.cpp`.
4. Move template parsing/instantiation helpers into `callsemantic_templates.cpp`.
5. Move symbol/output helpers into `callsemantic_symbols.cpp`.
6. Move declaration-registration helpers into `callsemantic_decls.cpp`.
7. Move class collection wrappers into `callsemantic_class.cpp`.
8. Leave only entrypoints and thin orchestration in `callsemantic.cpp` / `callsemantic_analysis.cpp`.

**Validation Strategy**
- After each slice:
  - `make build`
- After slices 2, 4, 6, and final:
  - root `make test-report`
- If a slice causes a regression:
  - reduce it to the earliest affected `pa*` test
  - add a missing earlier regression test if the failure only surfaced through hosted/bootstrap coverage
  - fix the regression before proceeding to the next slice

**Hotspot Follow-Up**
Once the split is complete and `make test-report` is green:
1. Benchmark the self-host compile again using the documented bootstrap process.
2. Profile the specific `callsemantic_*.cpp` file that dominates.
3. Reduce that file with targeted algorithmic fixes rather than continuing to edit a monolith.

**Non-Goals For This Refactor**
- No semantic cleanup beyond what is required to preserve behavior during the split.
- No reference refresh unless the split accidentally changes output and the change is proven to be semantically correct.
- No bootstrap/perf micro-fixes mixed into the structural slices unless needed to keep the split compiling.
