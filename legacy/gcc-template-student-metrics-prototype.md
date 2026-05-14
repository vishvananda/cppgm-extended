# GCC Template Student Metrics Prototype

This prototype lives in
[dev/gcc_template_student_metrics_plugin.cpp](/private/tmp/cppgm-template-kernel-20260416/dev/gcc_template_student_metrics_plugin.cpp)
and builds as
`dev/gcctmplmetrics.dylib` via the target in
[dev/Makefile](/private/tmp/cppgm-template-kernel-20260416/dev/Makefile).

It uses GCC's supported plugin mechanism with these callbacks:

- `PLUGIN_START_UNIT`
- `PLUGIN_FINISH_DECL`
- `PLUGIN_FINISH_TYPE`
- `PLUGIN_PRE_GENERICIZE`

## Goal

The goal is not timing. It is to see whether stock GCC plugin hooks can emit
the student-facing template facts we care about:

- selected template callee/specialization
- final instantiated template arguments
- rough binding information
- enough structure to compare against the kernel surface

## Current Stable Surface

The only surface that was stable enough on local tests was:

- `PLUGIN_PRE_GENERICIZE`
- walking the function body and looking for plain `CALL_EXPR` nodes
- extracting the resolved callee from `CALL_EXPR_FN`

That is enough to recover:

- resolved function-template winner
- instantiated function signature
- final template argument lists
- a partial binding dump

It is **not** enough to recover:

- losing candidates
- SFINAE/drop reasons
- a reliable explicit-vs-deduced split for function arguments
- class/variable template selections in a stable way from this prototype

## Example: PA22 Test 479

Command:

```bash
cd /private/tmp/cppgm-template-kernel-20260416/dev
GCC_TMPL_METRICS_FILE_SUBSTR=479-dependent-variable-template-empty-pack-enable-if-selection.t \
g++-15 -std=c++17 -x c++ -fsyntax-only \
  -fplugin=./gcctmplmetrics.dylib \
  ../pa22/tests/spec/479-dependent-variable-template-empty-pack-enable-if-selection.t
```

Observed output:

```json
{"kind":"function_call","location":"../pa22/tests/spec/479-dependent-variable-template-empty-pack-enable-if-selection.t:35:32","selected":"int traits<Alloc>::construct(Alloc&, Tp*, Args&& ...) [with Tp = int; Args = {}; typename enable_if<has_construct_v<Alloc, Tp*, Args ...>, int>::type <anonymous> = 0; Alloc = int]","selection":"primary","template":"template<class Alloc> template<class Tp, class ... Args, typename enable_if<has_construct_v<Alloc, Tp*, Args ...>, int>::type <anonymous> > static int traits<Alloc>::construct(Alloc&, Tp*, Args&& ...)","args":"[[int], [int, , 0]]","bindings":[{"param":"","arg":"int","source":"non_default"}]}
```

What this proves:

- GCC plugin hooks can see the winning function-template specialization at a
  real source callsite.
- GCC exposes the final instantiated callee signature and final template
  arguments.

What is still missing compared with the kernel:

- no losing overload
- no "failed because enable-if condition was false" explanation
- binding quality is poor for nested/member templates

## Example: PA33 Test 542

Test:

- [pa33/tests/compile/542-local-functor-std-function-assignment.t](/private/tmp/cppgm-template-kernel-20260416/pa33/tests/compile/542-local-functor-std-function-assignment.t)

Two important observations:

1. Registering `PLUGIN_PRE_GENERICIZE` alone is safe.
   Command:

   ```bash
   cd /private/tmp/cppgm-template-kernel-20260416/dev
   GCC_TMPL_METRICS_FILE_SUBSTR=542-local-functor-std-function-assignment.t \
   GCC_TMPL_METRICS_ENABLE_DECLS=0 \
   GCC_TMPL_METRICS_WALK_BODIES=0 \
   g++-15 -std=c++17 -x c++ -fsyntax-only \
     -fplugin=./gcctmplmetrics.dylib \
     ../pa33/tests/compile/542-local-functor-std-function-assignment.t
   ```

   This succeeds.

2. Walking the `PRE_GENERICIZE` body with the current naive tree walker is not
   stable on this case.

   With body walking enabled, local Homebrew GCC 15.2.0 crashes while
   processing `S::g()` in this test.

This does **not** prove GCC plugins cannot expose useful data for `542`.
It proves this prototype's direct tree walk is too naive for that body shape.

## Practical Conclusion

The GCC plugin route is viable, but only for a narrower baseline than the
kernel.

What looks promising:

- function-template winner tracing from `PRE_GENERICIZE`
- final instantiated template arguments
- some assignment-friendly normalization for resolved calls

What does not look practical from a stock plugin alone:

- full `.tkq` generation
- loser candidate enumeration
- deduction/drop-reason tracing
- stable coverage of all complex hosted/header-heavy cases without either:
  - much deeper GCC-frontend-specific handling, or
  - a GCC source patch

## Next Logical Experiments

1. Keep the GCC plugin focused on `function_call` winner extraction only.
2. Move from raw tree walking to a narrower walk strategy that only inspects
   well-understood call-bearing nodes.
3. Compare GCC plugin `function_call` output against the Clang plugin and the
   kernel reduction on shared seed cases.
4. If class/variable selection is required, prefer source-patched GCC/Clang
   tracing over trying to reconstruct it entirely from stock plugin callbacks.
