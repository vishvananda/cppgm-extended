# PA24 design example

The [assignment README](README.md) defines the requirements. These notes
explain one possible implementation.

### Design Notes (Non-Normative)

The cleanest PA24 structure is:

- parse LowIR into a structured internal representation
- lower that representation into a structured machine-IR program
- dump that machine-IR program deterministically for testing
- lower that machine-IR program into target-specific code/data
- write the final executable image from that lowered form

Reuse PA8's LowIR machinery and build the encoding, layout, and fixup
components introduced in [the native encoding lesson](native-encoding.md).

For the compact MIR shapes used by the checked fixtures, useful implementation
strategies include:

- keep incoming parameters and call results in their ABI registers until an
  emitted instruction invalidates that location
- reserve every allocator-managed incoming register that still carries a live
  parameter, including on a wide scalar boundary, and release it through the
  ordinary typed use count after its final selected consumer
- represent each instruction's fixed-register writes as a compact register
  mask and keep a scalar in an incoming register only when its live interval
  crosses none of those writes
- when a full-width scalar call result also needs a stable later home, let an
  earlier GPR call-argument use read its intact `rax` carrier directly
- omit parameter homes and setup transfers when slot selection removes every
  use that would have consumed them
- omit a transfer to a stable parameter home when every selected consumer can
  read the still-intact incoming ABI register
- let a promoted or forwarded parameter-slot load continue to name the
  parameter's stable selected home; its consumer can apply any required
  register constraint directly, after accounting for clobbers between the
  eliminated store and load
- use the same selected parameter home when constructing ordinary and extended
  call-argument move sets
- let a representation-preserving scalar copy or decay share an intact parameter
  location when the copied result's interval crosses no clobber
- record each block's sole predecessor and successor in dense CFG facts so a
  compiler-created scalar can retain its selected register across one exact
  adjacent edge without constructing per-block live-value sets
- lower `phi` values to parallel edge transfers; split a critical edge before
  MIR selection so copies for an untaken successor never execute
- retain a compact address-value bit on a parallel phi source so location
  equality and cycle scheduling do not confuse a frame address with a scalar
  stored at the same frame location; rematerialize that source with `lea`
- select the signed or unsigned extending memory form directly for a typed
  narrow integer load instead of emitting a partial load followed by a
  register-only normalization
- route address-setup/load folding through the same typed-load encoder so the
  compact address form cannot discard narrow-value normalization
- use the sole-use next instruction to recognize a narrow call result consumed
  by a same-width store, return, or explicit integer conversion, while
  retaining explicit normalization for wider consumers
- carry a typed immediate's signed range and the result fact of Boolean or
  integer-extension instructions into the adjacent normalization decision
- retain integer constants as typed MIR immediates, letting native emission
  materialize a scratch only when the concrete x86 encoding requires it, and
  place division or variable-shift operands directly in their required
  registers while keeping a fixed shift count on the shift instruction
- when an `i64` bitwise AND has a constant mask that clears every upper
  32-bit result bit, select the equivalent 32-bit x86 operation so the
  architectural zero extension supplies the complete `i64` value; retain the
  64-bit operation when any upper result bit may survive
- admit direct division-to-return setup only when its dividend is already a
  register or immediate; otherwise reuse the ordinary typed materialization
  path before assigning `rax` and `rdx`
- encode immediate memory stores directly at 8, 16, and 32 bits and for
  sign-extended 32-bit values at 64 bits; choose an encoder scratch that does
  not overlap a dereference base or index for other 64-bit values
- compare the exact target-byte cost of a fixed small `zero_bytes` with its
  `rep stosb` setup and use direct zero stores only when they are smaller;
  encode the 16-byte case with a cleared reserved vector scratch and one
  unaligned store, without consuming allocatable floating-point capacity
- encode a 1-, 2-, 4-, or 8-byte `copy_bytes` as one complete scalar load and
  store when that is cheaper than string-instruction setup; choose the scratch
  from the MIR instruction's declared clobber set and keep both logical
  address registers intact until their last use
- encode a fixed `copy_bytes` through 32 bytes with reserved-scratch vector
  chunks and a scalar tail, extending that direct form through 64 bytes for
  operations with at least eight-byte declared alignment; keep larger or more
  weakly aligned copies on the compact string-operation path
- lower a direct canonical three-argument builtin `memcpy` with an unused
  returned pointer to a dynamic `copy_bytes` operation after normal ABI
  argument staging, while retaining calls for used results and unmarked or
  incompatible callees
- carry a sole-use load's typed frame, global, dereference, or indexed address
  into an immediately following legal integer right operand, keeping its
  address inputs live until the consuming instruction
- keep an immediately returned quotient in `rax` and an immediately returned
  remainder in `rdx`; the return instruction may name that selected result
  carrier directly
- lower a sole-use `i128` comparison and branch as high-word decisions plus an
  unsigned low-word tie-break, and give a comparison used as a value a frame
  fallback instead of requiring a free GPR
- give an atomic load result a typed temporary frame home when its live range
  requires a register class with no free member
- prefer an available caller-saved register to adding a callee-saved register
  to the frame's `preserve` list
- reuse compatible compiler-created temporary frame locations when their value
  lifetimes do not overlap, while keeping source slots and parameter slots
  distinct
- fold a one-use `index` into the following memory operand, omit work for an
  unused or zero-displacement index, and use one `lea` when an indexed address
  must remain as a value
- retain a one-use frame address through a scalar memory consumer so the
  selected memory operand names the frame location directly
- classify address-only pointer results once in dense per-value facts and
  retain a typed base/index/displacement value while its carriers are stable,
  instead of repeatedly rescanning uses or keying hot-path state by text
- load frame-resident object chunks directly into their ABI argument registers
  instead of materializing a temporary object address
- transfer direct-object return chunks between their frame locations and ABI
  result registers without materializing a temporary object address
- keep an indirect-call target in its selected register when argument setup
  does not overwrite that register
- use the address result's recorded final consumer to keep a nonadjacent
  direct-object call-result destination frame-shaped without rescanning the
  intervening instructions
- count returns once during final native layout and share a restore/teardown
  sequence only when its encoded bytes exceed the added branch bytes; keep
  every semantic return and its result carrier in MIR

These are suggestions, not required internal data structures or algorithms.
Any implementation is acceptable if it preserves program behavior and produces
the checked strict or structural MIR output.
