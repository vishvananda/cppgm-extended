# Native Format Hook Plan

## Purpose

This plan finishes the remaining lightweight format split that still leaked into
common x86-adjacent code after the earlier object-boundary refactor.

The target is not to clone whole backends per format. The target is to move the
small set of native-format policy decisions behind one shared interface:

- base virtual address
- exit syscall number
- final x86-64 executable writer

## Current Remaining Mixing

The earlier boundary work removed the important Mach-O/ELF object-section and
relocation leakage from the x86 lowering path, but three places still open-code
format selection:

- `lowir_object_backend.cpp`
- `machine_linker.cpp`
- `cy86_native_backend.cpp`

Those branches are smaller than the ones already removed, but they still
duplicate the same native-format policy and make a third format harder to add.

## Intended Split

Introduce a shared native-format hook layer that owns:

- target text / enum selection
- base image virtual address
- exit syscall number
- final x86-64 executable writer dispatch

Keep everything else where it already belongs:

- x86 lowering remains in the existing lowering code
- Mach-O/ELF object parsing and writing remains in the object layer
- linker layout remains in the common linker

## Implementation Phases

1. Add a tiny `native_format` hook module for native-format policy.
2. Switch `machine_linker.cpp` to the hook module for final executable writing.
3. Switch `lowir_object_backend.cpp` to the same hook module for OS/runtime
   exit policy.
4. Switch `cy86_native_backend.cpp` too so the abstraction is not partial.

## Validation

Use the same reduced validation surface as the recent boundary work:

- host: `pa24`, `pa25`, `pa32`
- host: targeted `pa34` hosted-EH owners `668`, `675`, `683`, `684`, `690`,
  `709`
- host: a light `pa9` slice so the shared native writer path is exercised from
  `cy86`
- Linux Clang 22 Docker: the same subset with an isolated object root

If this lands cleanly, archive the plan immediately; it is a narrow seam fix,
not a long-running lane.
