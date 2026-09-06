# Assignment combination plan

The course is 38 lessons that grew by accretion.  This plan combines and
re-axes them into a coherent 39-lesson arc: fold a lesson that no longer
teaches anything, refocus one onto the half that survived, make the LowIR
band contiguous, split templating into five lessons, and rebuild the hosted
area around what can actually be verified.

It is the last phase of `docs/PLAN-CPPGM-EXTENDED-V4.md` and does not start
until that plan's phases are closed and `main` carries this implementation.

## Strategy: content first, numbering last

The combination changes two independent things: **what each lesson teaches**
and **what each lesson is called**.  Doing them together would mean every
content review happening in a tree where the paths are also moving, and every
reference regeneration racing a rename.

So the work runs in two stages, and the order is not negotiable:

**Stage A rewrites content in place, at the numbers the assignments already
have.**  A folded lesson's tests move to their new owner while both keep
their current numbers.  The freed PA24 slot is reused in place for the
templating integration lesson.  The hosted area is re-axed inside PA34 and
PA35.  Nothing is renamed, no README is renumbered, and after every step the
full gate set is green.

**Stage B renumbers, once, when the content is settled.**  One mechanical
pass moves the directories, rewrites the cross-references, and updates the
handouts and `ROADMAP.md` to match.  Because Stage A left the content
correct, Stage B has no judgement in it: if a test moves in Stage B, that is
a bug in Stage A.

The benefit is that Stage A is reviewable lesson by lesson and revertible
lesson by lesson, and Stage B is a single reviewable diff whose correctness
is checkable by construction (the same tests, the same references, new
paths).

## Target arc (39 lessons)

- **1–9 front end** — unchanged; PA9 stays as the stepping stone to PA13.
- **10–13 semantics and the LowIR bridge** — unchanged.
- **LowIR generation, consolidated and contiguous** — today's PA14–22 plus
  PA26–29, basics to advanced, with **five templating lessons** in band:
  three basics (the PA18/19/21 shapes), one advanced single-feature (a
  re-scoped PA22), and one **integration** lesson (new).
- **`lowir2native`** — after the whole LowIR band rather than mid-stream.
- *(`cpplink` folded out.)*
- **`-c` and link driver** (today's PA30) → **abimangle** (today's PA31).
- **Host exceptions, three lessons** — EH metadata (the refocused PA25, in a
  new home) → EH interactions (PA32) → EH core (PA33).
- **Hosted, three lessons** — compat features → front-end conformance
  (`static_assert`, perf-gated) → back-end ABI interop (link, run, assert).
- **`lowiropt` → `lowir2native -O` → inception** — unchanged.

## Decisions of record

Settled 2026-05-30/31; Stage A implements them rather than reopening them.

1. **Fold `cpplink` (PA24).**  Its native `machine_linker` is bypassed by the
   host-target link path cppgm++ actually uses, so it teaches nothing the
   course still needs.  Seven of its nine tests already duplicate PA30
   coverage and are dropped; two are salvaged into PA30, recast as
   cppgm++ `-c` plus a host link so they verify object emission rather than a
   native linker: `startup-hooks-across-objects` (static-init and startup
   hooks across translation units) and `data-reloc-indirect-call` (a
   cross-object data relocation feeding an indirect call).
2. **Refocus PA25 onto host `.gcc_except_table` emission** and move it next
   to the host-EH core.  Keep the EH-table half, drop the native EH-link
   half.  Its boundary with PA33: PA25 is correct metadata (basic
   throw/catch/cleanup plus a structural check that the table is emitted and
   call-site complete), PA33 is full semantics (cross-TU, nested, cleanup
   chains, RTTI matching).  The oracle is host-linked execution.
3. **Keep PA9 and PA30.**  PA9 is the stepping stone to PA13; PA30 is the
   first executable, separate compilation and the driver.
4. **Consolidate the LowIR band.**  PA26–29 are LowIR-text lessons with no
   dependency on PA23–25 — their separation was a style choice — so PA14–22
   and PA26–29 become contiguous and `lowir2native` follows the whole band.
5. **Templating becomes five lessons.**  PA18/19/21 stay as isolated basics,
   PA22 is re-scoped to advanced single-feature, and one integration lesson
   is added for multi-feature interaction.  No extra basics split.
6. **Re-axis the hosted area on what can be verified.**  The old
   compile-versus-link split does not hold: the Itanium ABI lessons made
   linking cheap and host-based, and compile-only is largely cheatable, since
   only `static_assert` is checkable without running.  The axis is instead
   what a test can prove and how.

## Stage A — content in place, at today's numbers

Each phase ends green on the full gate set below.  Phases are independent
except where noted, so they can land in any order that keeps the tree green.

### A1. Fold `cpplink`

Drop the seven duplicated tests, recast the two salvaged ones into PA30 as
`-c` plus host link, and retire the PA24 contract from its handout.  PA24
keeps its number for now and holds nothing; A4 refills it.

### A2. Refocus the exceptions metadata lesson

Reduce PA25 to `.gcc_except_table` emission from LowIR EH regions: the
call-site table covering the whole function range including the head, the
action table and the type-info table.  Tests are host-linked runs —
throw to catch and cleanup, uncaught to `terminate`, nested-frame unwind, no
spurious terminate — plus the structural completeness check.  PA25 stays at
its number; Stage B moves it next to PA32 and PA33.

### A3. Make the LowIR band contiguous in content

Nothing physically moves here.  Confirm with the placement auditor that no
PA26–29 test depends on PA23–25, and record any test whose content assumes
the old ordering so Stage B's move is mechanical.

### A4. Split templating into five lessons

The size and membership of the integration lesson come from data, not
taste.  `scripts/audit_pa_feature_placement.py` already detects per-test
feature usage over PA14–22 and PA26–29; extend it to carry the template
sub-features (function template, member template, alias template, partial
specialization, variable template, dependent name, explicit specialization,
non-type parameter, pack expansion, SFINAE and `void_t`, forwarding
reference, non-deduced context), then:

1. run it over PA22's tests and record, per test, the **set** of template
   features it uses;
2. classify by arity — one feature stays in the re-scoped advanced-single
   PA22 or moves back to its owning basics lesson; two or more interacting
   features go to the integration lesson, clustered by the combination
   (member template with pack expansion, dependent alias with partial
   ordering, forwarding reference with non-deduced context);
3. treat the dependent-name bucket as the swing factor: dependent-name
   resolution interacts with everything, so a `dependent-*` test that
   combines with another feature is usually an integration test.

The integration lesson is built in the freed PA24 slot, in place.  Its
handout is written here; Stage B only renumbers it.

### A5. Re-axis the hosted area

Scope first.  Of the ad-hoc PA34/PA35 tests, the ones that pull no library
header are not hosted tests at all and leave the area:

- genuine compat-layer features (`__builtin_*`, GNU asm symbol labels) join
  the hosted compat-features lesson;
- tests whose only purpose is to assert a symbol name — hand-spelled `std`
  templates stressing Itanium substitution and compression, or `abi_tag` —
  move to PA31 abimangle, where the cross-TU link is the mangling oracle
  rather than the point;
- generic core C++ (templates, vtables and multiple inheritance, separate
  compilation, backend relocation and TLS) goes back to its owning lesson or
  is dropped as a near-duplicate.

The header-bearing remainder becomes three lessons:

- **Compat features**, the first hosted lesson: the directed no-include
  tests plus the preprocessing tests plus the no-header compat reproducers.
  It implements the host-compatibility layer — builtins, macros, parse
  extensions, asm labels — verified directly, and is required before any
  header compiles.
- **Front-end conformance**, perf-gated: per header, one compile-time
  property asserted with `static_assert` — a trait result, a SFINAE outcome,
  an `integral_constant` or ratio value, an alias or `decltype` identity.
  Compile-only but cheat-proof, because a passing `static_assert` requires
  having resolved the template.  A hard wall-clock budget is itself a pass
  criterion: slow template resolution fails.  The header set is fixed, not
  student-chosen, so the bar is comparable.
- **Back-end ABI interop**: build a minimal program per capability —
  container storage, `iostream`, exception propagation through the host
  personality, allocator behaviour, vtable dispatch, cross-TU symbol
  resolution — run it, and assert the runtime result against g++ and clang.
  Running defeats the no-output cheat, and the cross-TU tests genuinely need
  the host link.

Weak or cheatable tests (calls with no result, declaration-only) are triaged
per test: upgraded into one of the two verified lessons, or dropped as near
duplicates.  The per-test disposition of all 185 is in
`docs/implemented/v3/pa34-35-test-disposition.md`.

Open before A5 can finish: which headers the front-end conformance lesson
fixes on, and how many.  The constexpr pool clusters on `<type_traits>`,
`<chrono>`, `<functional>`, `<ratio>` and `<utility>`; pick the set once a
per-test pass confirms which computations actually fold to a constant
expression, since some candidates will fall back to run-and-assert.

## Stage B — renumbering and handouts

One pass, after Stage A is green and reviewed.

1. Move the assignment directories to their new numbers, using the tooling
   that performed the PA31 ABI insertion
   (`docs/assignment-numbering-migration-2026-08.md` records how that went).
2. Rewrite every cross-reference: assignment READMEs, `ROADMAP.md`, the
   grammar files and explorers, the harness paths, the export's copied-path
   lists, `doc/`'s manifests, and the feature-ownership table the placement
   auditor reads.
3. Regenerate references only where a path is embedded in them; a test's
   content does not change in Stage B, so a diff in generated output that is
   not a path is a Stage A escape and gets fixed there, not here.
4. Re-run the export and diff the student tree against the pre-renumber
   export: the only differences should be paths and the numbers inside
   handouts.

## Gates

Every Stage A phase and Stage B itself must leave all of these green.  They
are the same gates `docs/PLAN-CPPGM-EXTENDED-V4.md` closes on.

- `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-report-nobuild`, byte exact.
- `make test-debuginfo-nobuild`, `make inception`, `make test-variants`,
  `make -C pa38 test-perf`.
- `make -C pa39 test-through-pa10 CXX=../dev/cppgm++` in all four CI flavors.
- Every `make audit-*`, `perl scripts/cppgm_file_audit.pl --paths dev`,
  `make test-harness`, and
  `python3 scripts/audit_pa_feature_placement.py --fail-on-early`.
- `scripts/export_student_repo.sh --force <dest>`, which is the only gate
  that compares a changed failed-case diagnostic, and now runs on every pull
  request.

Two notes for anyone picking this up from the older draft: the strict
witness lane was purged in the v4 migration, so there are no witness
references to preserve; and each assignment now carries its own regression
lane under `paN/tests/regression/`, which moves with the assignment.
