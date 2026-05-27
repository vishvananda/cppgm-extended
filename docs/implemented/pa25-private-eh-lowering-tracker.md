# PA25 Private EH Lowering Tracker

This tracker records the explicit case-by-case review for the current `PA25`
owner surface.

The review oracle is intentionally private-runtime-shaped:

- linked runtime result
- linked map shape
- object disassembly around throw/catch/cleanup/resume paths
- normalized private EH object facts from
  [`dump_private_eh_object_facts.py`](/Users/vishvananda/cppgm/scripts/dump_private_eh_object_facts.py)

It is not a direct Clang host-EH comparison. That later comparison belongs in
the separate `PA33` host-EH review lane.

## Common Private EH Object Surface

For all currently reviewed `PA25` owners on the macOS host target, the compiled
objects share the same private EH support shape:

```text
object_format mach-o
undef cppgm_priv_exc_top
undef cppgm_priv_exc_unhandled
undef cppgm_priv_exc_value
section compact_unwind
section data
section text
reloc compact_unwind abs64 section:text
reloc text branch32 cppgm_priv_exc_unhandled
reloc text pcrel32 cppgm_priv_exc_top
reloc text pcrel32 cppgm_priv_exc_value
```

That is the owned metadata surface for the current private EH path:

- user objects import the private exception-top slot
- user objects import the private exception-value slot
- throw paths branch to the private unhandled helper when there is no handler
- unwind/support metadata stays private and compact (`__compact_unwind` here)

No reviewed owner emitted an extra synthetic private support symbol beyond the
expected imported role family.

## Reviewed Owners

### 100 same-function catch

Status: `reviewed / fine`

Runtime:

```text
program exit status: 7
```

Link map:

```text
link_map x86_64 macos
startup_size 22
code_size 285
data_size 320
object 0 code_offset 64 data_offset 320
symbol @__cppgm_eh_top data 288
symbol @__cppgm_eh_type data 304
symbol @__cppgm_eh_unhandled code 32
symbol @__cppgm_eh_value data 296
symbol @main code 64
```

Disassembly excerpt:

```text
5e: movabsq $0x7, %rax
68: leaq    (%rip), %r11
6f: movq    %rax, (%r11)
72: leaq    (%rip), %r11
79: movq    (%r11), %rcx
7c: testq   %rcx, %rcx
7f: jne     0x96
8f: callq   0x94
94: ud2
96: movq    (%rcx), %rdx
99: movq    %rdx, (%r11)
bc: jmpq    *%rax
```

Review note:

- the same-function owner uses one direct throw sequence and one catch-transfer
  sequence
- the control shape is already direct and private-runtime-specific
- no extra helper materialization beyond the private top/value/unhandled roles

### 110 cross-function catch

Status: `reviewed / fine`

Runtime:

```text
program exit status: 5
```

Link map:

```text
link_map x86_64 macos
startup_size 22
code_size 348
data_size 384
object 0 code_offset 64 data_offset 384
symbol @__cppgm_eh_top data 352
symbol @__cppgm_eh_type data 368
symbol @__cppgm_eh_unhandled code 32
symbol @__cppgm_eh_value data 360
symbol @main code 176
symbol @thrower code 64
```

Object facts:

```text
define _cppgm_406d61696e
define _cppgm_407468726f776572
undef cppgm_priv_exc_top
undef cppgm_priv_exc_unhandled
undef cppgm_priv_exc_value
```

Disassembly excerpt:

```text
0000000000000000 <__cppgm_407468726f776572>:
  e: movabsq $0x5, %rax
 1f: movq    %rax, (%r11)
 29: movq    (%r11), %rcx
 2c: testq   %rcx, %rcx
 2f: jne     0x46
 3f: callq   0x44
 44: ud2
 46: movq    (%rcx), %rdx
 49: movq    %rdx, (%r11)
 6c: jmpq    *%rax
```

Review note:

- cross-function ownership does not introduce a second private EH metadata
  shape
- the thrower stays direct, and `main` simply calls it and restores the saved
  top-of-stack frame

### 120 cross-object catch

Status: `reviewed / fine`

Runtime:

```text
program exit status: 6
```

Link map:

```text
link_map x86_64 macos
startup_size 22
code_size 348
data_size 384
object 0 code_offset 64 data_offset 384
object 1 code_offset 176 data_offset 384
symbol @__cppgm_eh_top data 352
symbol @__cppgm_eh_type data 368
symbol @__cppgm_eh_unhandled code 32
symbol @__cppgm_eh_value data 360
symbol @main code 176
symbol @thrower code 64
```

Object facts for the throwing object:

```text
define _cppgm_407468726f776572
undef cppgm_priv_exc_top
undef cppgm_priv_exc_unhandled
undef cppgm_priv_exc_value
```

Disassembly excerpt:

```text
0000000000000000 <__cppgm_407468726f776572>:
  e: movabsq $0x6, %rax
 1f: movq    %rax, (%r11)
 29: movq    (%r11), %rcx
 2c: testq   %rcx, %rcx
 2f: jne     0x46
 3f: callq   0x44
 44: ud2
 46: movq    (%rcx), %rdx
 49: movq    %rdx, (%r11)
 6c: jmpq    *%rax
```

Review note:

- the cross-object lane preserves the same private relocation surface
- the link map shows the expected two-object ownership split without any extra
  user-visible helper object

### 130 cleanup resume

Status: `reviewed / fine`

Runtime:

```text
program exit status: 10
```

Link map:

```text
link_map x86_64 macos
startup_size 22
code_size 537
data_size 584
object 0 code_offset 64 data_offset 576
symbol @__cppgm_eh_top data 544
symbol @__cppgm_eh_type data 560
symbol @__cppgm_eh_unhandled code 32
symbol @__cppgm_eh_value data 552
symbol @f code 64
symbol @g data 576
symbol @main code 352
```

Object facts:

```text
define _cppgm_4066
define _cppgm_4067
define _cppgm_406d61696e
undef cppgm_priv_exc_top
undef cppgm_priv_exc_unhandled
undef cppgm_priv_exc_value
```

Disassembly excerpt:

```text
0000000000000000 <__cppgm_4066>:
 5e: movabsq $0x9, %rax
 6f: movq    %rax, (%r11)
 79: movq    (%r11), %rcx
 7c: testq   %rcx, %rcx
 7f: jne     0x96
 8f: callq   0x94
 94: ud2
 96: movq    (%rcx), %rdx
 99: movq    %rdx, (%r11)
 bc: jmpq    *%rax
 be: movabsq $0x1, %rax
 cf: movq    %rax, (%r11)
 d9: movq    (%r11), %rcx
 dc: testq   %rcx, %rcx
 df: jne     0xf6
 ef: callq   0xf4
 f4: ud2
 f6: movq    (%rcx), %rdx
 f9: movq    %rdx, (%r11)
11c: jmpq    *%rax
```

Review note:

- cleanup + resume currently lowers as two explicit throw/resume sequences
- that shape is repetitive but not structurally wrong for the current private
  EH contract
- the owner keeps the private metadata surface minimal while still proving that
  cleanup-side control transfer works

### 140 unhandled throw

Status: `reviewed / fine`

Runtime:

```text
program exit status: 11
```

Link map:

```text
link_map x86_64 macos
startup_size 22
code_size 174
data_size 208
object 0 code_offset 64 data_offset 208
symbol @__cppgm_eh_top data 176
symbol @__cppgm_eh_type data 192
symbol @__cppgm_eh_unhandled code 32
symbol @__cppgm_eh_value data 184
symbol @main code 64
```

Disassembly excerpt:

```text
0000000000000000 <__cppgm_406d61696e>:
  e: movabsq $0xb, %rax
 1f: movq    %rax, (%r11)
 29: movq    (%r11), %rcx
 2c: testq   %rcx, %rcx
 2f: jne     0x46
 3f: callq   0x44
 44: ud2
 46: movq    (%rcx), %rdx
 49: movq    %rdx, (%r11)
 6c: jmpq    *%rax
```

Review note:

- the unhandled owner is the cleanest proof that the private unhandled branch
  is wired directly and deterministically
- no extra catch-side state restoration is emitted because the owner has no
  handler

## Result

Current conclusion: `PA25`'s present public owner surface is fully reviewed and
does not show another simple worthwhile lowering fix at this boundary.

Cross-platform validation note:

- the full `pa25` suite passes on the macOS host target
- the full `pa25` suite also passes in Linux Docker with `clang++-22`
- the normalized private object facts on Linux reduce to the same semantic
  shape (`text`/`data` plus the expected private EH role imports and
  relocations), even though the backing object format is ELF instead of Mach-O

The remaining higher-value EH comparison work is later and different:

- direct host-EH comparison belongs in `PA33`
- RTTI/vtable/thunk interaction belongs in the separate `PA33` ABI review lane
