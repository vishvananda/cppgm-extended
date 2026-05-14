# Clang Sema Interpose Probe

This is an experiment to test whether dynamic-library interposition can capture
useful Clang template-resolution activity without patching Clang source.

The probe lives at
[dev/clang_sema_interpose_probe.cpp](/private/tmp/cppgm-template-kernel-20260416/dev/clang_sema_interpose_probe.cpp).

Current hooks target exported Sema symbols in `libclang-cpp`:

- `clang::Sema::AddTemplateOverloadCandidate`
- `clang::Sema::buildOverloadedCallSet`
- `clang::Sema::BuildOverloadedCallExpr`

The probe emits one line per intercepted event to `stderr`:

```txt
[clang.interpose] add-template-candidate ...
[clang.interpose] build-overloaded-call-set ...
[clang.interpose] build-overloaded-call-expr ...
```

## Build

From `dev/`:

```bash
make clang-sema-interpose-probe
```

## Demo

From `dev/`:

```bash
make clang-sema-interpose-demo
```

That runs the probe against:

- [pa22/tests/spec/479-dependent-variable-template-empty-pack-enable-if-selection.t](/Users/vishvananda/cppgm/pa22/tests/spec/479-dependent-variable-template-empty-pack-enable-if-selection.t)

with these filters:

- `CLANG_SEMA_INTERPOSE_FILE=479-dependent-variable-template-empty-pack-enable-if-selection.t`
- `CLANG_SEMA_INTERPOSE_SYMBOL=construct`

The important question is not the exact text shape. It is whether calls inside
`libclang-cpp` are interposable at all in this environment and whether the
resulting signal is strong enough to justify going further.

## Current Result

The current probe establishes three things:

- `DYLD_INSERT_LIBRARIES` works in this environment: the probe constructor logs
  `[clang.interpose] loaded`.
- Exporting replacement functions under the same mangled C++ symbol names does
  not intercept the target Clang calls here, even with
  `DYLD_FORCE_FLAT_NAMESPACE=1`.
- The specific Sema functions we care about are defined inside
  `libclang-cpp.dylib`, while the `clang++` driver binary does not import them
  directly.

As a control, the probe also exports a replacement for
`clang::ExecuteCompilerInvocation`, which *is* imported by `clang++`. That
override still does not fire with the current same-name-export approach, so the
probe does not yet demonstrate a working C++ interposition path for Clang
symbols on this setup.

## Practical Conclusion

For this toolchain, dylib injection is easy to prove, but reliable interception
of Clang C++ frontend calls is not. Even if Mach-O `__DATA,__interpose` were
made to work for an imported high-level symbol, it would still not reach the
internal Sema hot paths that stay within `libclang-cpp`.

That makes source-level Clang instrumentation the practical path for template
metrics, with dynamic interposition remaining at best a narrow experiment for
top-level imported entry points.
