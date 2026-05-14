## Goal

Build a bootstrap/frontier debugging tool that turns broad link failures and
verbose targeted traces into short, high-signal reports.

The current problem is not lack of tracing. It is that the useful trace points
already exist, but the workflow is still:

1. hit a broad bootstrap link failure or a later hosted compile failure
2. turn on one or more verbose trace modes
3. read hundreds or thousands of lines by hand
4. infer where the lifecycle went wrong

The tool should keep the existing tracing, but make it usable for the current
link-heavy frontier.

## Current Status

Implemented now:

- Phase 1 offline link triage
- direct consumer/provider symbol diff
- focused trace reruns with presets
- `--json` structured output mode
- `--write-prefix` persistent text/detail/json outputs
- persisted sidecar metadata for safe reuse on repeated analysis
- structured trace categories:
  - `symbol.linkage`
  - `output.require`
  - `output.export`
  - `template.scope`
- concise summaries plus automatic detailed temp reports
- Phase 4 frontier integration:
  - best unresolved candidate selection from a live frontier JSON
  - suggested source file and trace preset
  - ready-to-copy provider-diff / trace rerun commands
  - automatic persisted analysis sidecars from `report_bootstrap_frontier.py`
  - automatic sidecar reuse when the frontier JSON and trace tool are unchanged

## Why This Is The Right Tool Now

The recent bootstrap fixes are all in a narrow family:

- missing emitted definitions
- wrong hosted symbol names / mangling
- missing copied out-of-class member definitions
- missing helper emission
- missing vtable / runtime / ABI ownership
- template/member selection succeeded, but output/emission did not follow

Recent commits show that pattern directly:

- `e83b4ffc` hosted enum-class / nested libc++ link mangling
- `e2387724` bootstrap host-link compatibility
- `d51321f9` helper emission
- `8412b583` operator-name host symbol mangling
- `72f462eb` parameter alias recovery in emitted function output
- `e2b9a698` hosted std vtable objects
- `fb7d10ee` deferred member definitions for empty-pack instantiations
- `71710d62` member-template link frontier
- `6a8fc02e` nested out-of-class member template definitions

The current frontier tracker also says the live frontier is real `host-link`
with broad undefined-symbol sets, not just compile-time template rejection.

The example cluster file `/tmp/bootstrap-selfhost-frontier-cluster.json` shows
the same thing:

- full `dev/src` compile succeeds
- final result is `link-failed`
- the current undefined set contains both:
  - compiler-internal symbols like `make_dump_node(...)`
  - emitted/runtime/ABI-facing symbols
  - template-scope / helper / backend symbols

That means we need a tool that starts from the link failure, narrows to a
small set of candidate symbols, and only then turns on focused tracing.

## Existing Pieces We Should Reuse

We already have the right low-level building blocks:

- `scripts/report_bootstrap_frontier.py`
  - produces stage-by-stage frontier JSON
  - already clusters failures
- `parser_trace`
  - category filters via `CPPGM_TRACE`
  - file filter via `CPPGM_TRACE_FILE`
  - symbol substring filter via `CPPGM_TRACE_SYMBOL`
  - dump-on-error and live modes
- rich trace coverage in:
  - `template.resolve`
  - `class.collect`
  - `output.class`

So the new tool should sit on top of this machinery. It should not replace it.

## Proposed Tool

Add a new script, tentatively:

- `scripts/bootstrap_trace_report.py`

It should support three modes.

### 1. Link Triage Mode

Input:

- bootstrap frontier JSON
- or raw linker stderr

Output:

- grouped undefined-symbol families
- a ranked shortlist of the most likely owning issue families
- suggested next focused trace command(s)

The first report should classify each unresolved symbol into one of these
buckets:

- missing ordinary compiler object / link input
- missing emitted definition
- mangling / symbol-linkage mismatch
- hosted runtime / ABI ownership mismatch
- vtable / RTTI / special object ownership mismatch
- helper emission / template-upgrade miss

This mode should also have a direct consumer/provider ABI diff path for the
exact workflow we are doing by hand today:

- compile one consumer object with `cppgm++`
- compile one likely provider object with `cppgm++`
- run `nm` on both
- demangle both
- compare the unresolved consumer signature against the provided definition

That is especially valuable for cases like:

- same logical function name, but different mangled type spelling
- caller/provider parameter drift
- nested-template substitution drift
- namespace / operator-name mangling drift
- constructor / special-member spelling drift

This mode should also answer simple, high-value questions automatically:

- Is the symbol defined anywhere in the repo source?
- Is there an object in the frontier batch that should define it?
- Does the symbol look internal, hosted, runtime, or synthesized?
- Which object files first reference it?

For broad failures, this is often enough to say:

- this is a build-batch omission, not a semantic trace problem
- this is a mangling family, go inspect `symbol_linkage`
- this is an output/definition lifecycle miss, go inspect template/output flow

And for the targeted manual workflow, it should be able to say:

- consumer unresolved symbol matches no provider definition
- nearest provider candidate has same basename but different signature
- first signature drift appears in parameter `N`
- likely root cause family is:
  - mangling bug
  - substituted-type reconstruction bug
  - caller/provider declaration drift
  - template-scope binding corruption

### 2. Focused Trace Mode

Input:

- a single source file or reduced reproducer
- one or more focus identifiers
  - function name
  - class name
  - mangled symbol
  - demangled substring
- a trace preset

Output:

- rerun command with the right trace categories and filters
- raw trace capture
- concise summarized report

The tool should own the “how do I trace this?” part, instead of requiring
manual env setup each time.

Example presets:

- `template-lifecycle`
  - `template.resolve`
  - `class.collect`
- `output-lifecycle`
  - `output.class`
  - `template.resolve`
- `linkage`
  - `template.resolve`
  - new `symbol.linkage`
- `full-link-root-cause`
  - `template.resolve`
  - `class.collect`
  - `output.class`
  - new `symbol.linkage`
  - new `output.require`
  - new `output.export`

The tool should also support a focused `provider-diff` preset that skips broad
tracing and instead:

- builds the consumer object
- builds one or more likely provider objects
- extracts undefined and defined symbols
- demangles and normalizes them
- reports:
  - exact matches
  - same-name different-signature matches
  - likely first drift point

This is the direct scripted version of the current manual `nm` / `c++filt`
workflow.

### 3. Lifecycle Summary Mode

This is the main value-add over raw logs.

The report should reconstruct the entity lifecycle for the focused item:

- selected
- instantiated
- upgraded / copied from out-of-class source
- definition required
- output-required reason
- emitted symbol
- exported / retained
- pruned / rejected

Then it should highlight the first suspicious gap.

Example shape:

```text
Focus: std::__1::basic_stringbuf<char>::str

Observed lifecycle
- function-instantiation-new at template_instantiation.cpp:2691
- apply-out-of-class-member-function at template_instantiation.cpp:1189
- require_function_definition reason=DirectCall
- emitted symbol=_ZNSt3__115basic_stringbuf...
- exported symbol pruned as unowned

Likely issue
- backend export pruning disagrees with semantic required-definition closure

Next place to inspect
- lowirgensemantic.cpp:6999 prune_dead_unowned_exported_symbols()
```

For template-scope-heavy failures, the tool should collapse repetitive scope
operations into summaries instead of printing every bind/overlay line:

- total pack bindings
- unique pack names touched
- scope overlay count
- first and last binding state for the focused entity
- first unresolved name that survived the final overlay

For provider-diff cases, the summary should also produce a compact signature
comparison, for example:

```text
Focus: template_angle::parse_template_id_suffix_ranges

Consumer unresolved
- arg5 = std::__1::vector<unsigned long::pair<...>, ...>&

Nearest provider definition
- arg5 = std::__1::vector<std::__1::pair<unsigned long, unsigned long>, ...>&

Likely issue
- consumer-side substituted type reconstruction corrupted nested template name

Next place to inspect
- caller-side type reconstruction and symbol-linkage naming
```

## Required Trace Improvements

The current trace coverage is good, but the message shapes are too free-form for
reliable summarization. The tool should start with current traces, then add a
small number of structured high-value events.

Add these new categories first:

- `symbol.linkage`
- `output.require`
- `output.export`
- `template.scope`

And standardize their payloads as stable key/value text:

- `action=...`
- `entity=...`
- `symbol=...`
- `reason=...`
- `owner=...`
- `source=...`
- `binding=...`

Important rule:

- do not try to make every old trace site structured
- only add structured events at the lifecycle boundaries that help explain link
  failures

The highest-value new trace points are:

1. symbol-linkage naming decisions
2. `require_function_definition(...)`
3. output/export insertion and pruning
4. deferred member-definition copy / upgrade decisions
5. final backend validation failures around exported/internal closure

## Suggested MVP

### Phase 1: Offline Link Triage

Build the script first in offline mode using existing cluster JSON and linker
stderr.

Deliverables:

- parse unresolved symbols
- group by family
- rank by likely owner
- show probable owning source/object locations
- support direct consumer/provider symbol comparison from object files

This phase does not require any compiler changes.

### Phase 2: Preset-Driven Focused Trace Reruns

Add scripted reruns for:

- direct hosted source compile
- reducer compile
- single source from `dev/src`

Deliverables:

- `--focus`
- `--preset`
- `--file`
- concise markdown/text summary

This phase can still work mostly with current trace output.

### Phase 3: Structured Lifecycle Events

Add the new structured categories and boundary events.

Deliverables:

- `symbol.linkage`
- `output.require`
- `output.export`
- `template.scope`
- better lifecycle summary accuracy

### Phase 4: Frontier Integration

Teach the tool to accept a frontier JSON and immediately propose:

- top candidate unresolved symbols
- best source file to rerun
- best trace preset
- ready-to-copy rerun command

This should integrate naturally with `report_bootstrap_frontier.py` but does
not need to be merged into that script immediately.

## What This Tool Should Not Try To Do

Do not try to prove semantic correctness.

Do not try to replace reducers.

Do not try to dump every scope operation in prettier colors.

The real goal is:

- turn a 1000-line trace into a short lifecycle report
- point at the first missing transition
- tell us which file and subsystem to inspect next

## Initial Success Criteria

The tool is successful when it can shorten the common recent bootstrap cases:

1. Missing emitted definition
   - reports selection/instantiation happened
   - reports required-definition or export stage missing

2. Mangling mismatch
   - reports both expected unresolved symbol and actual emitted symbol family

3. Out-of-class member/template copy miss
   - reports source member found, copy/upgrade path absent or rejected

4. Vtable/runtime ownership mismatch
   - reports symbol classified as ABI/runtime ownership issue, not generic
     output failure

5. Broad host-link undefined set
   - reduces the set to a few likely root-cause families instead of dozens of
     flat unresolved names

## Recommended Implementation Order

1. `bootstrap_trace_report.py` offline link triage
2. focused rerun presets around existing traces
3. structured `symbol.linkage` / `output.require` / `output.export` events
4. lifecycle summarization for template/output/linkage paths
5. frontier JSON integration

This order keeps the first version useful immediately, without waiting for a
larger trace refactor.
