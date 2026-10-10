# Testing and references

This guide is for the compiler solution repository. The student export has
[its own guide](docs/student-export-root/TESTING_AND_REFERENCES.md) and pinned
downloadable reference tools. Here, every `*-ref` wrapper invokes the matching
tool built from `dev/`.

## Run tests

Run from the repository root:

```sh
make
make test-pa8
make test-report-through-pa10
CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-report
make test-debuginfo
make test-harness
make inception
```

The report covers PA1–PA33; PA34's final check is inception. Narrow the report
with `ACTIVE_TEST_REPORT_PAS='pa8 pa10'`. Control parallelism with
`TEST_REPORT_ASSIGNMENT_JOBS` and `TEST_REPORT_SUBTEST_JOBS`; `ORDERED=false`
streams completed jobs. Use `CPPGM_TEST_RUNNER=0` to diagnose ordinary versus
batch-runner behavior. Architecture audits and self-host checks are listed in
[AGENTS.md](AGENTS.md).

For quick iteration:

```sh
make -C pa8 check TEST='tests/spec/100-*.t'
```

`TEST=` accepts a path, quoted glob or quoted list. Mixed-input assignments
route each selected fixture to its owning runner.

## Fixture ownership

Every required fixture lives under its owning `paN/tests/`. From PA5 onward,
the three-digit prefix identifies a feature cluster (`100-`, `200-`, ...).
PA1–PA4 use finer subgroups. The placement auditor checks the latest feature
used; put a reducer in the earliest assignment that owns all its requirements.
`controls/` contains focused property checks. Personal inputs and scratch
harnesses belong in `student.tests/` and are run explicitly.

PA24, PA32 and PA33 also have `tests/regression/`, which pins this solution's
particular output shapes. Maintainer `make test` runs both `test-course` and
`test-regression`; student-export `make test` runs the course contract only.
PA33's export omits its regression fixtures and `maintainer/` tooling entirely.
The source-only `pa33/maintainer/Makefile` retains census and Cachegrind
targets; these are not PA33 student requirements.
Keep solution-specific expectations out of graded course fixtures.

## Reference policy

Choose the oracle required by the handout: output, exit status, a focused
property, or a code-quality envelope. Diagnostic text is not graded. PA8's
three construction exercises grade program outcomes through `lowir2native-ref`,
allowing alternative valid LowIR. Informational IR/MIR sidecars are not oracles.

Regenerate references from the built solution:

```sh
make ref-test-pa8
make ref-test
make ref-test-debuginfo
```

Before changing a LowIR reference, distinguish behavior from presentation;
[pa8/lowir.md](pa8/lowir.md) defines what normal comparison absorbs. Maintainer
CI enables byte-exact LowIR comparison. Regeneration on an unchanged compiler
must leave tracked references unchanged. Never change a fixture or reference
merely to hide an implementation failure.

Failed-case stdout is an informational diagnostic example, generated on Linux
by the export and untracked here. Successful stdout and required exit-status
sidecars remain tracked. Export validation regenerates and verifies every
portable reference before packaging the student repository.
CI also installs the bundle in a fresh client and exercises every exported
reference wrapper. Publication repeats this check against the uploaded release
URL before pushing the student repository.

## Symbol names

The course names C++ entities as clang names them: clang is the oracle for
every mangled name, internal ones included, and GCC only a cross-check.
LowIR comparison relaxes `object=` spellings, so a wrong name fails no LowIR
fixture; check them against clang after changing ABI naming:

```sh
python3 scripts/check_clang_names.py --jobs 8
python3 scripts/check_clang_names.py --verbose pa20 pa21
```

The maintainer script compiles each C++ LowIR fixture's source with clang
(`-std=c++11 -O0`, `--clang` or `CLANG` selects the compiler) and compares
every symbol of the reference with clang's: a name spelled differently from
clang's name for the same entity is reported (an internal-linkage marker, a
closure or unnamed type, a local entity, an external name, or a LowIR
spelling standing for a C++ entity). An internal symbol whose name is not
mangled is compared with clang's local symbol for the same entity: a
temporary that a static-storage variable extends is compared with clang's
`_ZGR` name for that variable and position, and every clang `_ZGR`
temporary of a variable the reference defines must appear in it. Which
constructor and destructor entries a unit emits (C1/C2, CI1/CI2,
D0/D1/D2) is an emission choice and is not judged, nor is a symbol clang
does not emit. A signature whose clang spelling mangles an
instantiation-dependent expression (decltype, an expression template
argument or array bound) is compared like any other: the course retains
those expressions. The script exits 1 when a mismatch is not listed with
its reason in the script's `DOCUMENTED` table, and `--verbose` lists every
mismatch of a fixture.
