## PA22 / PA29 Runtime Validation Plan

### Goal

Add earlier execution validation for the backend without coupling LowIR to CY86 or to a
host-specific syscall ABI.

### Decisions

1. LowIR should grow the missing semantic features in the actual LowIR handout.

   The important gaps are:

   - explicit signed vs unsigned integer semantics where those differ materially:
     - ordered comparisons
     - right shift
     - division
     - modulus
   - explicit numeric conversion operations:
     - integer to floating
     - floating to integer
     - `f32`/`f64` to `f80`
     - `f80` to `f32`/`f64`

2. LowIR should not gain a raw `syscall` instruction.

   CY86 syscalls are Linux/x86-64 specific and are not the right long-term backend boundary
   for LowIR.

3. PA22 should own native execution validation for backend-level features.

   These tests should remain LowIR-native and should focus on execution behavior that belongs
   to the backend regardless of the later source-driver/toolchain surface. In particular:

   - signed/unsigned integer arithmetic behavior
   - signed/unsigned comparisons and shifts
   - float arithmetic and comparisons
   - integer/float conversion behavior
   - structured global data and memory behavior

4. PA29 should own source-driven end-to-end runtime programs.

   PA9-style complete programs should be rewritten as small C++ programs and validated through
   `cppgm++ -c` and link mode. These tests should avoid host libc and hosted headers. Where
   runtime support is needed, the harness should provide a tiny object-style support library
   that is linked through `-L` / `-l` and declared in source with `extern "C"`. The first
   landing set is `210-runtime-hello-world`, `220-runtime-duplicator`,
   `230-runtime-reverser`, and `240-runtime-hexdump`.

### Assignment Fit

- PA13:
  - document the stable LowIR family and the planned arithmetic/conversion additions
- PA22:
  - add targeted `LowIR -> native` execution tests for the backend-owned behavior above
- PA29:
  - add C++ source tests that exercise simple complete-program runtime behavior end to end
    without depending on host libc

### Practical Consequences

- Do not mechanically port PA9 CY86 tests into LowIR.
- Do port their semantic intent:
  - pure compute kernels move earlier into PA22 as LowIR-native tests
  - complete source-level programs move later into PA29 as C++ tests
- Keep the runtime boundary portable and compiler-owned rather than Linux-syscall-shaped.
