# PA23 Clang `-O0` Lowering Tracker

This tracker is an explicit case-by-case review of the full `PA23` test surface against
Clang `-O0` x86-64 output on macOS. It replaces the earlier family summary that closed the
review too early.

Every `PA23` test case is listed below. For each codegen case, the tracker now records:

- the test bucket (`strict` or `structural`)
- the analogue source used for the Clang comparison
- the current review status
- the actual Clang `-O0` disassembly captured during this pass

The goal is not bit-for-bit parity with Clang. The goal is to catch places where our
backend is still obviously heavier than a real non-optimizing compiler for ordinary x86-64
code shapes.

## Review Summary

- Total `PA23` cases reviewed explicitly: `61`
- Cases with generated Clang `-O0` disassembly: `60`
- `reviewed / fine`: `53`
- `simplified here`: `6`
- `not meaningfully comparable`: `1`
- `no codegen analogue`: `1`

## Status Labels

- `reviewed / fine`: no clearly worthwhile PA23 lowering simplification was found in this pass
- `simplified here`: this explicit Clang review exposed a simpler lowering and the branch now implements it
- `not meaningfully comparable`: the case reaches code generation, but Clang is exercising a different runtime/startup boundary than PA23
- `no codegen analogue`: the case is a front-end failure owner and never reaches native code emission

## Case Ledger

### `100-ret0`

- Bucket: `strict`
- Test: `strict/100-ret0.t`
- Status: `reviewed / fine`
- Clang analogue: `100-ret0.cpp`
- Review note: Trivial entrypoint. Clang and our MIR are already at the same basic shape.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/100-ret0.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	xorl	%eax, %eax
       d:      	popq	%rbp
       e:      	retq
```

</details>

### `650-short-circuit-and-call-diamond`

- Bucket: `structural`
- Test: `structural/650-short-circuit-and-call-diamond.t`
- Status: `reviewed / fine`
- Clang analogue: `650-short-circuit-and-call-diamond.cpp`
- Review note: Clang `-O0` still materializes the final boolean more heavily than our direct branch diamond. No simpler worthwhile PA23 lowering gap was exposed here.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260409-extra/out/650-short-circuit-and-call-diamond.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	subq	$0x10, %rsp
       8:      	movl	$0x0, -0x4(%rbp)
       f:      	callq	0x14 <_main+0x14>
      14:      	movl	%eax, %ecx
      16:      	xorl	%eax, %eax
      18:      	cmpl	$0x0, %ecx
      1b:      	movb	%al, -0x5(%rbp)
      1e:      	je	0x2e <_main+0x2e>
      20:      	callq	0x25 <_main+0x25>
      25:      	cmpl	$0x0, %eax
      28:      	setne	%al
      2b:      	movb	%al, -0x5(%rbp)
      2e:      	movb	-0x5(%rbp), %dl
      31:      	movl	$0x1, %eax
      36:      	xorl	%ecx, %ecx
      38:      	testb	$0x1, %dl
      3b:      	cmovnel	%ecx, %eax
      3e:      	addq	$0x10, %rsp
      42:      	popq	%rbp
      43:      	retq
      44:      	nopw	%cs:(%rax,%rax)

0000000000000050 <__ZL8lhs_truev>:
      50:      	pushq	%rbp
      51:      	movq	%rsp, %rbp
      54:      	movl	$0x1, %eax
      59:      	popq	%rbp
      5a:      	retq
      5b:      	nopl	(%rax,%rax)

0000000000000060 <__ZL8rhs_truev>:
      60:      	pushq	%rbp
      61:      	movq	%rsp, %rbp
      64:      	movl	$0x1, %eax
      69:      	popq	%rbp
      6a:      	retq
```

</details>

### `660-short-circuit-or-call-diamond`

- Bucket: `structural`
- Test: `structural/660-short-circuit-or-call-diamond.t`
- Status: `reviewed / fine`
- Clang analogue: `660-short-circuit-or-call-diamond.cpp`
- Review note: As with the `&&` owner, our direct branch diamond is already simpler than Clang `-O0`'s boolean materialization path.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260409-extra/out/660-short-circuit-or-call-diamond.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	subq	$0x10, %rsp
       8:      	movl	$0x0, -0x4(%rbp)
       f:      	callq	0x14 <_main+0x14>
      14:      	movl	%eax, %ecx
      16:      	movb	$0x1, %al
      18:      	cmpl	$0x0, %ecx
      1b:      	movb	%al, -0x5(%rbp)
      1e:      	jne	0x2e <_main+0x2e>
      20:      	callq	0x25 <_main+0x25>
      25:      	cmpl	$0x0, %eax
      28:      	setne	%al
      2b:      	movb	%al, -0x5(%rbp)
      2e:      	movb	-0x5(%rbp), %dl
      31:      	movl	$0x1, %eax
      36:      	xorl	%ecx, %ecx
      38:      	testb	$0x1, %dl
      3b:      	cmovnel	%ecx, %eax
      3e:      	addq	$0x10, %rsp
      42:      	popq	%rbp
      43:      	retq
      44:      	nopw	%cs:(%rax,%rax)

0000000000000050 <__ZL9lhs_falsev>:
      50:      	pushq	%rbp
      51:      	movq	%rsp, %rbp
      54:      	xorl	%eax, %eax
      56:      	popq	%rbp
      57:      	retq
      58:      	nopl	(%rax,%rax)

0000000000000060 <__ZL8rhs_truev>:
      60:      	pushq	%rbp
      61:      	movq	%rsp, %rbp
      64:      	movl	$0x1, %eax
      69:      	popq	%rbp
      6a:      	retq
```

</details>

### `670-unary-not-call-branch`

- Bucket: `structural`
- Test: `structural/670-unary-not-call-branch.t`
- Status: `simplified here`
- Clang analogue: `670-unary-not-call-branch.cpp`
- Review note: This review exposed that our unary-not branch path was still materializing a boolean and then re-testing it. The backend now lowers this owner directly as a compare plus branch.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260409-extra/out/670-unary-not-call-branch.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	subq	$0x10, %rsp
       8:      	movl	$0x0, -0x4(%rbp)
       f:      	callq	0x14 <_main+0x14>
      14:      	cmpq	$0x0, %rax
      18:      	setne	%dl
      1b:      	xorb	$-0x1, %dl
      1e:      	movl	$0x1, %eax
      23:      	xorl	%ecx, %ecx
      25:      	testb	$0x1, %dl
      28:      	cmovnel	%ecx, %eax
      2b:      	addq	$0x10, %rsp
      2f:      	popq	%rbp
      30:      	retq
      31:      	nopw	%cs:(%rax,%rax)

0000000000000040 <__ZL4zerov>:
      40:      	pushq	%rbp
      41:      	movq	%rsp, %rbp
      44:      	xorl	%eax, %eax
      46:      	popq	%rbp
      47:      	retq
```

</details>

### `680-i8-signed-frame-load-widen`

- Bucket: `structural`
- Test: `structural/680-i8-signed-frame-load-widen.t`
- Status: `reviewed / fine`
- Clang analogue: `680-i8-signed-frame-load-widen.cpp`
- Review note: Once forced through a real frame reload, our signed `i8` widening path is already in-family with Clang `-O0`.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260409-extra/out/680-i8-signed-frame-load-widen.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	movb	$-0x1, -0x5(%rbp)
       f:      	movsbq	-0x5(%rbp), %rax
      14:      	movq	%rax, -0x10(%rbp)
      18:      	movq	-0x10(%rbp), %rdx
      1c:      	xorl	%eax, %eax
      1e:      	movl	$0x1, %ecx
      23:      	cmpq	$-0x1, %rdx
      27:      	cmovel	%ecx, %eax
      2a:      	popq	%rbp
      2b:      	retq
```

</details>

### `690-u16-zero-frame-load-widen`

- Bucket: `structural`
- Test: `structural/690-u16-zero-frame-load-widen.t`
- Status: `reviewed / fine`
- Clang analogue: `690-u16-zero-frame-load-widen.cpp`
- Review note: The unsigned frame reload path is also already acceptable once the owner is forced through a real load.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260409-extra/out/690-u16-zero-frame-load-widen.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	movw	$0xffff, -0x6(%rbp)     ## imm = 0xFFFF
      11:      	movzwl	-0x6(%rbp), %eax
      15:      	movq	%rax, -0x10(%rbp)
      19:      	movq	-0x10(%rbp), %rdx
      1d:      	xorl	%eax, %eax
      1f:      	movl	$0x1, %ecx
      24:      	cmpq	$0xffff, %rdx           ## imm = 0xFFFF
      2b:      	cmovel	%ecx, %eax
      2e:      	popq	%rbp
      2f:      	retq
```

</details>

### `700-i8-signed-global-load-widen`

- Bucket: `structural`
- Test: `structural/700-i8-signed-global-load-widen.t`
- Status: `reviewed / fine`
- Clang analogue: `700-i8-signed-global-load-widen.cpp`
- Review note: The ordinary signed global-load widening path is already aligned with Clang's `movsbq`-style shape.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260409-extra/out/700-i8-signed-global-load-widen.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	movsbq	(%rip), %rax            ## 0x13 <_main+0x13>
      13:      	movq	%rax, -0x10(%rbp)
      17:      	movq	-0x10(%rbp), %rdx
      1b:      	xorl	%eax, %eax
      1d:      	movl	$0x1, %ecx
      22:      	cmpq	$-0x1, %rdx
      26:      	cmovel	%ecx, %eax
      29:      	popq	%rbp
      2a:      	retq
```

</details>

### `710-u16-zero-global-load-widen`

- Bucket: `structural`
- Test: `structural/710-u16-zero-global-load-widen.t`
- Status: `reviewed / fine`
- Clang analogue: `710-u16-zero-global-load-widen.cpp`
- Review note: The unsigned global-load widening path is likewise already reasonable for PA23.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260409-extra/out/710-u16-zero-global-load-widen.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	movzwl	(%rip), %eax            ## 0x12 <_main+0x12>
      12:      	movq	%rax, -0x10(%rbp)
      16:      	movq	-0x10(%rbp), %rdx
      1a:      	xorl	%eax, %eax
      1c:      	movl	$0x1, %ecx
      21:      	cmpq	$0xffff, %rdx           ## imm = 0xFFFF
      28:      	cmovel	%ecx, %eax
      2b:      	popq	%rbp
      2c:      	retq
```

</details>
### `110-object-abi-lowered`

- Bucket: `strict`
- Test: `strict/110-object-abi-lowered.t`
- Status: `simplified here`
- Clang analogue: `110-object-abi-lowered.cpp`
- Review note: This review originally exposed the constant scaled-index case that we simplified from `imul`+`add` into a direct immediate add.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/110-object-abi-lowered.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <__Z9make_pairll>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movq	%rdi, -0x18(%rbp)
       8:      	movq	%rsi, -0x20(%rbp)
       c:      	movq	-0x18(%rbp), %rax
      10:      	movq	%rax, -0x10(%rbp)
      14:      	movq	-0x20(%rbp), %rax
      18:      	movq	%rax, -0x8(%rbp)
      1c:      	movq	-0x10(%rbp), %rax
      20:      	movq	-0x8(%rbp), %rdx
      24:      	popq	%rbp
      25:      	retq
      26:      	nopw	%cs:(%rax,%rax)

0000000000000030 <_main>:
      30:      	pushq	%rbp
      31:      	movq	%rsp, %rbp
      34:      	subq	$0x20, %rsp
      38:      	movl	$0x0, -0x4(%rbp)
      3f:      	movl	$0x4, %edi
      44:      	movl	$0x5, %esi
      49:      	callq	0x4e <_main+0x1e>
      4e:      	movq	%rax, -0x18(%rbp)
      52:      	movq	%rdx, -0x10(%rbp)
      56:      	movq	-0x18(%rbp), %rax
      5a:      	movq	%rax, (%rip)            ## 0x61 <_main+0x31>
      61:      	movq	-0x10(%rbp), %rax
      65:      	movq	%rax, 0x8(%rip)         ## 0x74 <_main+0x44>
      6c:      	movq	(%rip), %rax            ## 0x73 <_main+0x43>
      73:      	addq	0x8(%rip), %rax         ## 0x82 <_main+0x52>
      7a:      	addq	$0x20, %rsp
      7e:      	popq	%rbp
      7f:      	retq
```

</details>

### `120-copyobj`

- Bucket: `strict`
- Test: `strict/120-copyobj.t`
- Status: `reviewed / fine`
- Clang analogue: `120-copyobj.cpp`
- Review note: Our bulk-copy lowering is already reasonable for PA23; Clang `-O0` is not materially simpler here.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/120-copyobj.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	movq	(%rip), %rax            ## 0x12 <_main+0x12>
      12:      	movq	%rax, (%rip)            ## 0x19 <_main+0x19>
      19:      	movq	0x8(%rip), %rax         ## 0x28 <_main+0x28>
      20:      	movq	%rax, 0x8(%rip)         ## 0x2f <_main+0x2f>
      27:      	movq	(%rip), %rax            ## 0x2e <_main+0x2e>
      2e:      	addq	0x8(%rip), %rax         ## 0x3d <_main+0x3d>
      35:      	popq	%rbp
      36:      	retq
```

</details>

### `130-zeroinit`

- Bucket: `strict`
- Test: `strict/130-zeroinit.t`
- Status: `reviewed / fine`
- Clang analogue: `130-zeroinit.cpp`
- Review note: The current zero-init path is already in-family with ordinary non-optimizing lowering.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/130-zeroinit.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	movq	$0x0, -0x18(%rbp)
      13:      	movq	$0x0, -0x10(%rbp)
      1b:      	movq	-0x18(%rbp), %rax
      1f:      	movq	%rax, (%rip)            ## 0x26 <_main+0x26>
      26:      	movq	-0x10(%rbp), %rax
      2a:      	movq	%rax, 0x8(%rip)         ## 0x39 <_main+0x39>
      31:      	movq	(%rip), %rax            ## 0x38 <_main+0x38>
      38:      	addq	0x8(%rip), %rax         ## 0x47 <_main+0x47>
      3f:      	popq	%rbp
      40:      	retq
```

</details>

### `140-structured-global-data`

- Bucket: `strict`
- Test: `strict/140-structured-global-data.t`
- Status: `simplified here`
- Clang analogue: `140-structured-global-data.cpp`
- Review note: The same constant scaled-index simplification cleaned up the ordinary global-data indexing path.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/140-structured-global-data.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	subq	$0x10, %rsp
       8:      	movl	$0x0, -0x4(%rbp)
       f:      	callq	*(%rip)                 ## 0x15 <_main+0x15>
      15:      	addq	$0x10, %rsp
      19:      	popq	%rbp
      1a:      	retq
      1b:      	nopl	(%rax,%rax)

0000000000000020 <__ZL4ret3v>:
      20:      	pushq	%rbp
      21:      	movq	%rsp, %rbp
      24:      	movl	$0x3, %eax
      29:      	popq	%rbp
      2a:      	retq
```

</details>

### `150-startup-shutdown-hooks`

- Bucket: `strict`
- Test: `strict/150-startup-shutdown-hooks.t`
- Status: `not meaningfully comparable`
- Clang analogue: `150-startup-shutdown-hooks.cpp`
- Review note: Clang goes through CRT/loader startup and teardown. The emitted instructions are recorded here, but they are not a faithful oracle for the PA23 startup stub.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/150-startup-shutdown-hooks.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <__ZN8InitFiniC1Ev>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	subq	$0x10, %rsp
       8:      	movq	%rdi, -0x8(%rbp)
       c:      	movq	-0x8(%rbp), %rdi
      10:      	callq	0x15 <__ZN8InitFiniC1Ev+0x15>
      15:      	addq	$0x10, %rsp
      19:      	popq	%rbp
      1a:      	retq
      1b:      	nopl	(%rax,%rax)

0000000000000020 <__ZN8InitFiniD1Ev>:
      20:      	pushq	%rbp
      21:      	movq	%rsp, %rbp
      24:      	subq	$0x10, %rsp
      28:      	movq	%rdi, -0x8(%rbp)
      2c:      	movq	-0x8(%rbp), %rdi
      30:      	callq	0x35 <__ZN8InitFiniD1Ev+0x15>
      35:      	addq	$0x10, %rsp
      39:      	popq	%rbp
      3a:      	retq
      3b:      	nopl	(%rax,%rax)

0000000000000040 <_main>:
      40:      	pushq	%rbp
      41:      	movq	%rsp, %rbp
      44:      	movl	$0x0, -0x4(%rbp)
      4b:      	movq	(%rip), %rax            ## 0x52 <_main+0x12>
      52:      	popq	%rbp
      53:      	retq
      54:      	nopw	%cs:(%rax,%rax)

0000000000000060 <__ZN8InitFiniC2Ev>:
      60:      	pushq	%rbp
      61:      	movq	%rsp, %rbp
      64:      	movq	%rdi, -0x8(%rbp)
      68:      	movq	$0x5, -0x4(%rip)        ## 0x6f <__ZN8InitFiniC2Ev+0xf>
      73:      	popq	%rbp
      74:      	retq
      75:      	nopw	%cs:(%rax,%rax)

0000000000000080 <__ZN8InitFiniD2Ev>:
      80:      	pushq	%rbp
      81:      	movq	%rsp, %rbp
      84:      	movq	%rdi, -0x8(%rbp)
      88:      	movq	$0x9, -0x4(%rip)        ## 0x8f <__ZN8InitFiniD2Ev+0xf>
      93:      	popq	%rbp
      94:      	retq

Disassembly of section __TEXT,__StaticInit:

00000000000000a0 <___cxx_global_var_init>:
      a0:      	pushq	%rbp
      a1:      	movq	%rsp, %rbp
      a4:      	leaq	(%rip), %rdi            ## 0xab <___cxx_global_var_init+0xb>
      ab:      	callq	0xb0 <___cxx_global_var_init+0x10>
      b0:      	movq	(%rip), %rdi            ## 0xb7 <___cxx_global_var_init+0x17>
      b7:      	leaq	(%rip), %rsi            ## 0xbe <___cxx_global_var_init+0x1e>
      be:      	leaq	(%rip), %rdx            ## 0xc5 <___cxx_global_var_init+0x25>
      c5:      	callq	0xca <___cxx_global_var_init+0x2a>
      ca:      	popq	%rbp
      cb:      	retq
      cc:      	nopl	(%rax)

00000000000000d0 <__GLOBAL__sub_I_150_startup_shutdown_hooks.cpp>:
      d0:      	pushq	%rbp
      d1:      	movq	%rsp, %rbp
      d4:      	callq	0xd9 <__GLOBAL__sub_I_150_startup_shutdown_hooks.cpp+0x9>
      d9:      	popq	%rbp
      da:      	retq
```

</details>

### `160-direct-call-branch`

- Bucket: `strict`
- Test: `strict/160-direct-call-branch.t`
- Status: `reviewed / fine`
- Clang analogue: `160-direct-call-branch.cpp`
- Review note: Direct call plus branch shape is already fine.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/160-direct-call-branch.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	subq	$0x10, %rsp
       8:      	movl	$0x0, -0x4(%rbp)
       f:      	movl	$0x7, %edi
      14:      	callq	0x19 <_main+0x19>
      19:      	addq	$0x10, %rsp
      1d:      	popq	%rbp
      1e:      	retq
      1f:      	nop

0000000000000020 <__ZL6helperl>:
      20:      	pushq	%rbp
      21:      	movq	%rsp, %rbp
      24:      	movq	%rdi, -0x8(%rbp)
      28:      	movq	-0x8(%rbp), %rax
      2c:      	popq	%rbp
      2d:      	retq
```

</details>

### `200-class-constructor-member-init`

- Bucket: `strict`
- Test: `strict/200-class-constructor-member-init.t`
- Status: `reviewed / fine`
- Clang analogue: `200-class-constructor-member-init.cpp`
- Review note: No simpler PA23-owned lowering gap stood out in the Clang comparison.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/200-class-constructor-member-init.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	subq	$0x10, %rsp
       8:      	movl	$0x0, -0x4(%rbp)
       f:      	leaq	-0x8(%rbp), %rdi
      13:      	callq	0x18 <_main+0x18>
      18:      	movl	-0x8(%rbp), %eax
      1b:      	movl	%eax, -0x4(%rbp)
      1e:      	leaq	-0x8(%rbp), %rdi
      22:      	callq	0x27 <_main+0x27>
      27:      	movl	-0x4(%rbp), %eax
      2a:      	addq	$0x10, %rsp
      2e:      	popq	%rbp
      2f:      	retq

0000000000000030 <__ZN2YPC1Ev>:
      30:      	pushq	%rbp
      31:      	movq	%rsp, %rbp
      34:      	subq	$0x10, %rsp
      38:      	movq	%rdi, -0x8(%rbp)
      3c:      	movq	-0x8(%rbp), %rdi
      40:      	callq	0x45 <__ZN2YPC1Ev+0x15>
      45:      	addq	$0x10, %rsp
      49:      	popq	%rbp
      4a:      	retq
      4b:      	nopl	(%rax,%rax)

0000000000000050 <__ZN2YPD1Ev>:
      50:      	pushq	%rbp
      51:      	movq	%rsp, %rbp
      54:      	subq	$0x10, %rsp
      58:      	movq	%rdi, -0x8(%rbp)
      5c:      	movq	-0x8(%rbp), %rdi
      60:      	callq	0x65 <__ZN2YPD1Ev+0x15>
      65:      	addq	$0x10, %rsp
      69:      	popq	%rbp
      6a:      	retq
      6b:      	nopl	(%rax,%rax)

0000000000000070 <__ZN2YPC2Ev>:
      70:      	pushq	%rbp
      71:      	movq	%rsp, %rbp
      74:      	movq	%rdi, -0x8(%rbp)
      78:      	movq	-0x8(%rbp), %rax
      7c:      	movl	$0x3, (%rax)
      82:      	popq	%rbp
      83:      	retq
      84:      	nopw	%cs:(%rax,%rax)

0000000000000090 <__ZN2YPD2Ev>:
      90:      	pushq	%rbp
      91:      	movq	%rsp, %rbp
      94:      	movq	%rdi, -0x8(%rbp)
      98:      	popq	%rbp
      99:      	retq
```

</details>

### `210-pass-by-value-lvalue`

- Bucket: `strict`
- Test: `strict/210-pass-by-value-lvalue.t`
- Status: `reviewed / fine`
- Clang analogue: `210-pass-by-value-lvalue.cpp`
- Review note: Caller/callee aggregate traffic is already acceptable for PA23.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/210-pass-by-value-lvalue.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	subq	$0x30, %rsp
       8:      	movl	$0x0, -0x4(%rbp)
       f:      	leaq	-0x8(%rbp), %rdi
      13:      	callq	0x18 <_main+0x18>
      18:      	movl	$0x8, -0x8(%rbp)
      1f:      	movl	-0x8(%rbp), %eax
      22:      	movl	%eax, -0xc(%rbp)
      25:      	leaq	-0xc(%rbp), %rdi
      29:      	callq	0x2e <_main+0x2e>
      2e:      	movq	%rax, -0x28(%rbp)
      32:      	jmp	0x34 <_main+0x34>
      34:      	movq	-0x28(%rbp), %rax
      38:      	movl	%eax, -0x4(%rbp)
      3b:      	leaq	-0xc(%rbp), %rdi
      3f:      	callq	0x44 <_main+0x44>
      44:      	leaq	-0x8(%rbp), %rdi
      48:      	callq	0x4d <_main+0x4d>
      4d:      	movl	-0x4(%rbp), %eax
      50:      	addq	$0x30, %rsp
      54:      	popq	%rbp
      55:      	retq
      56:      	movq	%rax, %rcx
      59:      	movl	%edx, %eax
      5b:      	movq	%rcx, -0x18(%rbp)
      5f:      	movl	%eax, -0x1c(%rbp)
      62:      	leaq	-0xc(%rbp), %rdi
      66:      	callq	0x6b <_main+0x6b>
      6b:      	leaq	-0x8(%rbp), %rdi
      6f:      	callq	0x74 <_main+0x74>
      74:      	movq	-0x18(%rbp), %rdi
      78:      	callq	0x7d <_main+0x7d>
      7d:      	nopl	(%rax)

0000000000000080 <__ZN1YC1Ev>:
      80:      	pushq	%rbp
      81:      	movq	%rsp, %rbp
      84:      	subq	$0x10, %rsp
      88:      	movq	%rdi, -0x8(%rbp)
      8c:      	movq	-0x8(%rbp), %rdi
      90:      	callq	0x95 <__ZN1YC1Ev+0x15>
      95:      	addq	$0x10, %rsp
      99:      	popq	%rbp
      9a:      	retq
      9b:      	nopl	(%rax,%rax)

00000000000000a0 <__ZL2id1Y>:
      a0:      	pushq	%rbp
      a1:      	movq	%rsp, %rbp
      a4:      	movq	%rdi, -0x8(%rbp)
      a8:      	movslq	(%rdi), %rax
      ab:      	popq	%rbp
      ac:      	retq
      ad:      	nopl	(%rax)

00000000000000b0 <__ZN1YD1Ev>:
      b0:      	pushq	%rbp
      b1:      	movq	%rsp, %rbp
      b4:      	subq	$0x10, %rsp
      b8:      	movq	%rdi, -0x8(%rbp)
      bc:      	movq	-0x8(%rbp), %rdi
      c0:      	callq	0xc5 <__ZN1YD1Ev+0x15>
      c5:      	addq	$0x10, %rsp
      c9:      	popq	%rbp
      ca:      	retq
      cb:      	nopl	(%rax,%rax)

00000000000000d0 <__ZN1YC2Ev>:
      d0:      	pushq	%rbp
      d1:      	movq	%rsp, %rbp
      d4:      	movq	%rdi, -0x8(%rbp)
      d8:      	popq	%rbp
      d9:      	retq
      da:      	nopw	(%rax,%rax)

00000000000000e0 <__ZN1YD2Ev>:
      e0:      	pushq	%rbp
      e1:      	movq	%rsp, %rbp
      e4:      	movq	%rdi, -0x8(%rbp)
      e8:      	popq	%rbp
      e9:      	retq
```

</details>

### `220-virtual-base-reference`

- Bucket: `strict`
- Test: `strict/220-virtual-base-reference.t`
- Status: `reviewed / fine`
- Clang analogue: `220-virtual-base-reference.cpp`
- Review note: This is mostly object-model ABI plumbing rather than a local instruction-selection issue.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/220-virtual-base-reference.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	subq	$0x40, %rsp
       8:      	movl	$0x0, -0x4(%rbp)
       f:      	leaq	-0x10(%rbp), %rdi
      13:      	movq	%rdi, -0x38(%rbp)
      17:      	callq	0x1c <_main+0x1c>
      1c:      	movq	-0x38(%rbp), %rax
      20:      	movq	%rax, -0x18(%rbp)
      24:      	movq	-0x18(%rbp), %rdi
      28:      	movq	(%rdi), %rax
      2b:      	movq	(%rax), %rax
      2e:      	callq	*%rax
      30:      	movq	%rax, -0x30(%rbp)
      34:      	jmp	0x36 <_main+0x36>
      36:      	movq	-0x30(%rbp), %rax
      3a:      	movl	%eax, -0x4(%rbp)
      3d:      	leaq	-0x10(%rbp), %rdi
      41:      	callq	0x46 <_main+0x46>
      46:      	movl	-0x4(%rbp), %eax
      49:      	addq	$0x40, %rsp
      4d:      	popq	%rbp
      4e:      	retq
      4f:      	movq	%rax, %rcx
      52:      	movl	%edx, %eax
      54:      	movq	%rcx, -0x20(%rbp)
      58:      	movl	%eax, -0x24(%rbp)
      5b:      	leaq	-0x10(%rbp), %rdi
      5f:      	callq	0x64 <_main+0x64>
      64:      	movq	-0x20(%rbp), %rdi
      68:      	callq	0x6d <_main+0x6d>
      6d:      	nopl	(%rax)

0000000000000070 <__ZN1DC1Ev>:
      70:      	pushq	%rbp
      71:      	movq	%rsp, %rbp
      74:      	subq	$0x10, %rsp
      78:      	movq	%rdi, -0x8(%rbp)
      7c:      	movq	-0x8(%rbp), %rdi
      80:      	callq	0x85 <__ZN1DC1Ev+0x15>
      85:      	addq	$0x10, %rsp
      89:      	popq	%rbp
      8a:      	retq
      8b:      	nopl	(%rax,%rax)

0000000000000090 <__ZN1DD1Ev>:
      90:      	pushq	%rbp
      91:      	movq	%rsp, %rbp
      94:      	subq	$0x10, %rsp
      98:      	movq	%rdi, -0x8(%rbp)
      9c:      	movq	-0x8(%rbp), %rdi
      a0:      	callq	0xa5 <__ZN1DD1Ev+0x15>
      a5:      	addq	$0x10, %rsp
      a9:      	popq	%rbp
      aa:      	retq
      ab:      	nopl	(%rax,%rax)

00000000000000b0 <__ZN1DC2Ev>:
      b0:      	pushq	%rbp
      b1:      	movq	%rsp, %rbp
      b4:      	subq	$0x10, %rsp
      b8:      	movq	%rdi, -0x8(%rbp)
      bc:      	movq	-0x8(%rbp), %rdi
      c0:      	movq	%rdi, -0x10(%rbp)
      c4:      	callq	0xc9 <__ZN1DC2Ev+0x19>
      c9:      	movq	-0x10(%rbp), %rax
      cd:      	movq	(%rip), %rcx            ## 0xd4 <__ZN1DC2Ev+0x24>
      d4:      	addq	$0x10, %rcx
      d8:      	movq	%rcx, (%rax)
      db:      	addq	$0x10, %rsp
      df:      	popq	%rbp
      e0:      	retq
      e1:      	nopw	%cs:(%rax,%rax)

00000000000000f0 <__ZN1BC2Ev>:
      f0:      	pushq	%rbp
      f1:      	movq	%rsp, %rbp
      f4:      	movq	%rdi, -0x8(%rbp)
      f8:      	movq	-0x8(%rbp), %rax
      fc:      	movq	(%rip), %rcx            ## 0x103 <__ZN1BC2Ev+0x13>
     103:      	addq	$0x10, %rcx
     107:      	movq	%rcx, (%rax)
     10a:      	popq	%rbp
     10b:      	retq
     10c:      	nopl	(%rax)

0000000000000110 <__ZN1D1fEv>:
     110:      	pushq	%rbp
     111:      	movq	%rsp, %rbp
     114:      	movq	%rdi, -0x8(%rbp)
     118:      	movl	$0x2, %eax
     11d:      	popq	%rbp
     11e:      	retq
     11f:      	nop

0000000000000120 <__ZN1DD0Ev>:
     120:      	pushq	%rbp
     121:      	movq	%rsp, %rbp
     124:      	subq	$0x10, %rsp
     128:      	movq	%rdi, -0x8(%rbp)
     12c:      	movq	-0x8(%rbp), %rdi
     130:      	movq	%rdi, -0x10(%rbp)
     134:      	callq	0x139 <__ZN1DD0Ev+0x19>
     139:      	movq	-0x10(%rbp), %rdi
     13d:      	callq	0x142 <__ZN1DD0Ev+0x22>
     142:      	addq	$0x10, %rsp
     146:      	popq	%rbp
     147:      	retq
     148:      	nopl	(%rax,%rax)

0000000000000150 <__ZN1B1fEv>:
     150:      	pushq	%rbp
     151:      	movq	%rsp, %rbp
     154:      	movq	%rdi, -0x8(%rbp)
     158:      	movl	$0x1, %eax
     15d:      	popq	%rbp
     15e:      	retq
     15f:      	nop

0000000000000160 <__ZN1BD1Ev>:
     160:      	pushq	%rbp
     161:      	movq	%rsp, %rbp
     164:      	subq	$0x10, %rsp
     168:      	movq	%rdi, -0x8(%rbp)
     16c:      	movq	-0x8(%rbp), %rdi
     170:      	callq	0x175 <__ZN1BD1Ev+0x15>
     175:      	addq	$0x10, %rsp
     179:      	popq	%rbp
     17a:      	retq
     17b:      	nopl	(%rax,%rax)

0000000000000180 <__ZN1BD0Ev>:
     180:      	pushq	%rbp
     181:      	movq	%rsp, %rbp
     184:      	subq	$0x10, %rsp
     188:      	movq	%rdi, -0x8(%rbp)
     18c:      	movq	-0x8(%rbp), %rdi
     190:      	movq	%rdi, -0x10(%rbp)
     194:      	callq	0x199 <__ZN1BD0Ev+0x19>
     199:      	movq	-0x10(%rbp), %rdi
     19d:      	callq	0x1a2 <__ZN1BD0Ev+0x22>
     1a2:      	addq	$0x10, %rsp
     1a6:      	popq	%rbp
     1a7:      	retq
     1a8:      	nopl	(%rax,%rax)

00000000000001b0 <__ZN1BD2Ev>:
     1b0:      	pushq	%rbp
     1b1:      	movq	%rsp, %rbp
     1b4:      	movq	%rdi, -0x8(%rbp)
     1b8:      	popq	%rbp
     1b9:      	retq
     1ba:      	nopw	(%rax,%rax)

00000000000001c0 <__ZN1DD2Ev>:
     1c0:      	pushq	%rbp
     1c1:      	movq	%rsp, %rbp
     1c4:      	subq	$0x10, %rsp
     1c8:      	movq	%rdi, -0x8(%rbp)
     1cc:      	movq	-0x8(%rbp), %rdi
     1d0:      	callq	0x1d5 <__ZN1DD2Ev+0x15>
     1d5:      	addq	$0x10, %rsp
     1d9:      	popq	%rbp
     1da:      	retq
```

</details>

### `230-class-template-field`

- Bucket: `strict`
- Test: `strict/230-class-template-field.t`
- Status: `reviewed / fine`
- Clang analogue: `230-class-template-field.cpp`
- Review note: No actionable Clang simplicity gap found.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/230-class-template-field.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	subq	$0x10, %rsp
       8:      	movl	$0x0, -0x4(%rbp)
       f:      	leaq	-0x10(%rbp), %rdi
      13:      	movl	$0x7, %esi
      18:      	callq	0x1d <_main+0x1d>
      1d:      	movq	-0x10(%rbp), %rax
      21:      	addq	$0x10, %rsp
      25:      	popq	%rbp
      26:      	retq
      27:      	nopw	(%rax,%rax)

0000000000000030 <__ZN3BoxIlEC1El>:
      30:      	pushq	%rbp
      31:      	movq	%rsp, %rbp
      34:      	subq	$0x10, %rsp
      38:      	movq	%rdi, -0x8(%rbp)
      3c:      	movq	%rsi, -0x10(%rbp)
      40:      	movq	-0x8(%rbp), %rdi
      44:      	movq	-0x10(%rbp), %rsi
      48:      	callq	0x4d <__ZN3BoxIlEC1El+0x1d>
      4d:      	addq	$0x10, %rsp
      51:      	popq	%rbp
      52:      	retq
      53:      	nopw	%cs:(%rax,%rax)

0000000000000060 <__ZN3BoxIlEC2El>:
      60:      	pushq	%rbp
      61:      	movq	%rsp, %rbp
      64:      	movq	%rdi, -0x8(%rbp)
      68:      	movq	%rsi, -0x10(%rbp)
      6c:      	movq	-0x8(%rbp), %rax
      70:      	movq	-0x10(%rbp), %rcx
      74:      	movq	%rcx, (%rax)
      77:      	popq	%rbp
      78:      	retq
```

</details>

### `240-non-type-class-specialization`

- Bucket: `strict`
- Test: `strict/240-non-type-class-specialization.t`
- Status: `reviewed / fine`
- Clang analogue: `240-non-type-class-specialization.cpp`
- Review note: No actionable Clang simplicity gap found.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/240-non-type-class-specialization.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	subq	$0x10, %rsp
       8:      	movl	$0x0, -0x4(%rbp)
       f:      	callq	0x14 <_main+0x14>
      14:      	addq	$0x10, %rsp
      18:      	popq	%rbp
      19:      	retq
      1a:      	nopw	(%rax,%rax)

0000000000000020 <__ZN3TagILi9EE5valueEv>:
      20:      	pushq	%rbp
      21:      	movq	%rsp, %rbp
      24:      	movl	$0x9, %eax
      29:      	popq	%rbp
      2a:      	retq
```

</details>

### `260-indirect-call-six-register-args`

- Bucket: `strict`
- Test: `strict/260-indirect-call-six-register-args.t`
- Status: `reviewed / fine`
- Clang analogue: `260-indirect-call-six-register-args.cpp`
- Review note: Indirect six-register call shape is already fine.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/260-indirect-call-six-register-args.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	subq	$0x10, %rsp
       8:      	movl	$0x0, -0x4(%rbp)
       f:      	leaq	(%rip), %rax            ## 0x16 <_main+0x16>
      16:      	movq	%rax, -0x10(%rbp)
      1a:      	movl	$0x1, %edi
      1f:      	movl	$0x2, %esi
      24:      	movl	$0x3, %edx
      29:      	movl	$0x4, %ecx
      2e:      	movl	$0x5, %r8d
      34:      	movl	$0x6, %r9d
      3a:      	callq	*-0x10(%rbp)
      3d:      	subq	$0x15, %rax
      41:      	addq	$0x10, %rsp
      45:      	popq	%rbp
      46:      	retq
      47:      	nopw	(%rax,%rax)

0000000000000050 <__ZL4sum6llllll>:
      50:      	pushq	%rbp
      51:      	movq	%rsp, %rbp
      54:      	movq	%rdi, -0x8(%rbp)
      58:      	movq	%rsi, -0x10(%rbp)
      5c:      	movq	%rdx, -0x18(%rbp)
      60:      	movq	%rcx, -0x20(%rbp)
      64:      	movq	%r8, -0x28(%rbp)
      68:      	movq	%r9, -0x30(%rbp)
      6c:      	movq	-0x8(%rbp), %rax
      70:      	addq	-0x10(%rbp), %rax
      74:      	addq	-0x18(%rbp), %rax
      78:      	addq	-0x20(%rbp), %rax
      7c:      	addq	-0x28(%rbp), %rax
      80:      	addq	-0x30(%rbp), %rax
      84:      	popq	%rbp
      85:      	retq
```

</details>

### `270-f80-direct-call`

- Bucket: `strict`
- Test: `strict/270-f80-direct-call.t`
- Status: `reviewed / fine`
- Clang analogue: `270-f80-direct-call.cpp`
- Review note: The conservative x87 path is acceptable here.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/270-f80-direct-call.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	subq	$0x20, %rsp
       8:      	movl	$0x0, -0x4(%rbp)
       f:      	movq	%rsp, %rax
      12:      	flds	0x38(%rip)              ## 0x50 <__ZL2ide+0x10>
      18:      	fstpt	(%rax)
      1a:      	callq	0x1f <_main+0x1f>
      1f:      	fldz
      21:      	fxch	%st(1)
      23:      	fucompi	%st(1), %st
      25:      	fstp	%st(0)
      27:      	seta	%al
      2a:      	andb	$0x1, %al
      2c:      	movzbl	%al, %eax
      2f:      	addq	$0x20, %rsp
      33:      	popq	%rbp
      34:      	retq
      35:      	nopw	%cs:(%rax,%rax)

0000000000000040 <__ZL2ide>:
      40:      	pushq	%rbp
      41:      	movq	%rsp, %rbp
      44:      	fldt	0x10(%rbp)
      47:      	fstpt	-0x10(%rbp)
      4a:      	fldt	-0x10(%rbp)
      4d:      	popq	%rbp
      4e:      	retq
```

</details>

### `280-f80-structured-global-data`

- Bucket: `strict`
- Test: `strict/280-f80-structured-global-data.t`
- Status: `reviewed / fine`
- Clang analogue: `280-f80-structured-global-data.cpp`
- Review note: The conservative x87/global-data path is acceptable here.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/280-f80-structured-global-data.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	fldt	(%rip)                  ## 0x11 <_main+0x11>
      11:      	fldz
      13:      	fxch	%st(1)
      15:      	fucompi	%st(1), %st
      17:      	fstp	%st(0)
      19:      	seta	%al
      1c:      	andb	$0x1, %al
      1e:      	movzbl	%al, %eax
      21:      	popq	%rbp
      22:      	retq
```

</details>

### `290-bad-missing-terminator`

- Bucket: `strict`
- Test: `strict/290-bad-missing-terminator.t`
- Status: `no codegen analogue`
- Clang analogue: `none`
- Review note: This is a parse-failure owner. There is no meaningful Clang disassembly analogue because the test never reaches code generation.

No Clang disassembly was generated for this case because the owned behavior is a parse failure.

### `300-atomic-load-store`

- Bucket: `strict`
- Test: `strict/300-atomic-load-store.t`
- Status: `reviewed / fine`
- Clang analogue: `300-atomic-load-store.cpp`
- Review note: No additional simplification beyond the seq_cst-specific fixes was indicated here.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/300-atomic-load-store.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	movq	(%rip), %rax            ## 0x12 <_main+0x12>
      12:      	movq	%rax, -0x18(%rbp)
      16:      	movq	-0x18(%rbp), %rax
      1a:      	movq	%rax, -0x10(%rbp)
      1e:      	movq	-0x10(%rbp), %rax
      22:      	incq	%rax
      25:      	movq	%rax, -0x20(%rbp)
      29:      	movq	-0x20(%rbp), %rax
      2d:      	movq	%rax, (%rip)            ## 0x34 <_main+0x34>
      34:      	movq	(%rip), %rax            ## 0x3b <_main+0x3b>
      3b:      	movq	%rax, -0x30(%rbp)
      3f:      	movq	-0x30(%rbp), %rax
      43:      	movq	%rax, -0x28(%rbp)
      47:      	cmpq	$0x8, -0x28(%rbp)
      4c:      	sete	%al
      4f:      	andb	$0x1, %al
      51:      	movzbl	%al, %eax
      54:      	popq	%rbp
      55:      	retq
```

</details>

### `310-atomic-add-fetch`

- Bucket: `strict`
- Test: `strict/310-atomic-add-fetch.t`
- Status: `reviewed / fine`
- Clang analogue: `310-atomic-add-fetch.cpp`
- Review note: No additional simplification beyond the seq_cst-specific fixes was indicated here.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/310-atomic-add-fetch.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	movq	$0x5, -0x18(%rbp)
      13:      	movq	-0x18(%rbp), %rax
      17:      	lock
      18:      	xaddq	%rax, (%rip)            ## 0x20 <_main+0x20>
      20:      	movq	%rax, -0x20(%rbp)
      24:      	movq	-0x20(%rbp), %rax
      28:      	movq	%rax, -0x10(%rbp)
      2c:      	movq	(%rip), %rax            ## 0x33 <_main+0x33>
      33:      	movq	%rax, -0x30(%rbp)
      37:      	movq	-0x30(%rbp), %rax
      3b:      	movq	%rax, -0x28(%rbp)
      3f:      	xorl	%eax, %eax
      41:      	cmpq	$0x7, -0x10(%rbp)
      46:      	movb	%al, -0x31(%rbp)
      49:      	jne	0x56 <_main+0x56>
      4b:      	cmpq	$0xc, -0x28(%rbp)
      50:      	sete	%al
      53:      	movb	%al, -0x31(%rbp)
      56:      	movb	-0x31(%rbp), %al
      59:      	andb	$0x1, %al
      5b:      	movzbl	%al, %eax
      5e:      	popq	%rbp
      5f:      	retq
```

</details>

### `320-atomic-seq-cst-fence`

- Bucket: `strict`
- Test: `strict/320-atomic-seq-cst-fence.t`
- Status: `reviewed / fine`
- Clang analogue: `320-atomic-seq-cst-fence.cpp`
- Review note: The fence-only shape is already in the expected space.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/320-atomic-seq-cst-fence.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	lock
       c:      	orl	$0x0, -0x40(%rsp)
      11:      	xorl	%eax, %eax
      13:      	popq	%rbp
      14:      	retq
```

</details>

### `330-atomic-exchange`

- Bucket: `strict`
- Test: `strict/330-atomic-exchange.t`
- Status: `simplified here`
- Clang analogue: `330-atomic-exchange.cpp`
- Review note: This review led to the x86-64 seq_cst exchange simplification using direct `xchg` semantics.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/330-atomic-exchange.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	movq	$0x9, -0x18(%rbp)
      13:      	movq	-0x18(%rbp), %rax
      17:      	xchgq	%rax, (%rip)            ## 0x1e <_main+0x1e>
      1e:      	movq	%rax, -0x20(%rbp)
      22:      	movq	-0x20(%rbp), %rax
      26:      	movq	%rax, -0x10(%rbp)
      2a:      	movq	(%rip), %rax            ## 0x31 <_main+0x31>
      31:      	movq	%rax, -0x30(%rbp)
      35:      	movq	-0x30(%rbp), %rax
      39:      	movq	%rax, -0x28(%rbp)
      3d:      	xorl	%eax, %eax
      3f:      	cmpq	$0x7, -0x10(%rbp)
      44:      	movb	%al, -0x31(%rbp)
      47:      	jne	0x54 <_main+0x54>
      49:      	cmpq	$0x9, -0x28(%rbp)
      4e:      	sete	%al
      51:      	movb	%al, -0x31(%rbp)
      54:      	movb	-0x31(%rbp), %al
      57:      	andb	$0x1, %al
      59:      	movzbl	%al, %eax
      5c:      	popq	%rbp
      5d:      	retq
```

</details>

### `340-atomic-compare-exchange-success`

- Bucket: `strict`
- Test: `strict/340-atomic-compare-exchange-success.t`
- Status: `reviewed / fine`
- Clang analogue: `340-atomic-compare-exchange-success.cpp`
- Review note: No further actionable gap stood out after the seq_cst cleanup.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/340-atomic-compare-exchange-success.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	movq	$0x7, -0x10(%rbp)
      13:      	movq	$0x9, -0x20(%rbp)
      1b:      	movq	-0x10(%rbp), %rax
      1f:      	movq	-0x20(%rbp), %rcx
      23:      	lock
      24:      	cmpxchgq	%rcx, (%rip)            ## 0x2c <_main+0x2c>
      2c:      	movq	%rax, %rcx
      2f:      	sete	%al
      32:      	movb	%al, -0x41(%rbp)
      35:      	movq	%rcx, -0x40(%rbp)
      39:      	testb	$0x1, %al
      3b:      	jne	0x45 <_main+0x45>
      3d:      	movq	-0x40(%rbp), %rax
      41:      	movq	%rax, -0x10(%rbp)
      45:      	movb	-0x41(%rbp), %al
      48:      	andb	$0x1, %al
      4a:      	movb	%al, -0x21(%rbp)
      4d:      	movb	-0x21(%rbp), %al
      50:      	andb	$0x1, %al
      52:      	movb	%al, -0x11(%rbp)
      55:      	movq	(%rip), %rax            ## 0x5c <_main+0x5c>
      5c:      	movq	%rax, -0x38(%rbp)
      60:      	movq	-0x38(%rbp), %rax
      64:      	movq	%rax, -0x30(%rbp)
      68:      	xorl	%eax, %eax
      6a:      	testb	$0x1, -0x11(%rbp)
      6e:      	movb	%al, -0x42(%rbp)
      71:      	je	0x8a <_main+0x8a>
      73:      	xorl	%eax, %eax
      75:      	cmpq	$0x9, -0x30(%rbp)
      7a:      	movb	%al, -0x42(%rbp)
      7d:      	jne	0x8a <_main+0x8a>
      7f:      	cmpq	$0x7, -0x10(%rbp)
      84:      	sete	%al
      87:      	movb	%al, -0x42(%rbp)
      8a:      	movb	-0x42(%rbp), %al
      8d:      	andb	$0x1, %al
      8f:      	movzbl	%al, %eax
      92:      	popq	%rbp
      93:      	retq
```

</details>

### `350-atomic-compare-exchange-failure`

- Bucket: `strict`
- Test: `strict/350-atomic-compare-exchange-failure.t`
- Status: `reviewed / fine`
- Clang analogue: `350-atomic-compare-exchange-failure.cpp`
- Review note: No further actionable gap stood out after the seq_cst cleanup.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/350-atomic-compare-exchange-failure.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	movq	$0x5, -0x10(%rbp)
      13:      	movq	$0x9, -0x20(%rbp)
      1b:      	movq	-0x10(%rbp), %rax
      1f:      	movq	-0x20(%rbp), %rcx
      23:      	lock
      24:      	cmpxchgq	%rcx, (%rip)            ## 0x2c <_main+0x2c>
      2c:      	movq	%rax, %rcx
      2f:      	sete	%al
      32:      	movb	%al, -0x41(%rbp)
      35:      	movq	%rcx, -0x40(%rbp)
      39:      	testb	$0x1, %al
      3b:      	jne	0x45 <_main+0x45>
      3d:      	movq	-0x40(%rbp), %rax
      41:      	movq	%rax, -0x10(%rbp)
      45:      	movb	-0x41(%rbp), %al
      48:      	andb	$0x1, %al
      4a:      	movb	%al, -0x21(%rbp)
      4d:      	movb	-0x21(%rbp), %al
      50:      	andb	$0x1, %al
      52:      	movb	%al, -0x11(%rbp)
      55:      	movq	(%rip), %rax            ## 0x5c <_main+0x5c>
      5c:      	movq	%rax, -0x38(%rbp)
      60:      	movq	-0x38(%rbp), %rax
      64:      	movq	%rax, -0x30(%rbp)
      68:      	xorl	%eax, %eax
      6a:      	testb	$0x1, -0x11(%rbp)
      6e:      	movb	%al, -0x42(%rbp)
      71:      	jne	0x8a <_main+0x8a>
      73:      	xorl	%eax, %eax
      75:      	cmpq	$0x7, -0x30(%rbp)
      7a:      	movb	%al, -0x42(%rbp)
      7d:      	jne	0x8a <_main+0x8a>
      7f:      	cmpq	$0x7, -0x10(%rbp)
      84:      	sete	%al
      87:      	movb	%al, -0x42(%rbp)
      8a:      	movb	-0x42(%rbp), %al
      8d:      	andb	$0x1, %al
      8f:      	movzbl	%al, %eax
      92:      	popq	%rbp
      93:      	retq
```

</details>

### `370-unsigned-int-ops`

- Bucket: `strict`
- Test: `strict/370-unsigned-int-ops.t`
- Status: `reviewed / fine`
- Clang analogue: `370-unsigned-int-ops.cpp`
- Review note: No clear PA23-owned inefficiency relative to Clang `-O0`.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/370-unsigned-int-ops.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	movl	$0x5, -0x8(%rbp)
      12:      	movl	$0x9, -0xc(%rbp)
      19:      	movl	-0x8(%rbp), %eax
      1c:      	addl	-0xc(%rbp), %eax
      1f:      	movl	%eax, -0x10(%rbp)
      22:      	movl	-0xc(%rbp), %eax
      25:      	subl	-0x8(%rbp), %eax
      28:      	movl	%eax, -0x14(%rbp)
      2b:      	movl	-0x8(%rbp), %eax
      2e:      	imull	-0xc(%rbp), %eax
      32:      	movl	%eax, -0x18(%rbp)
      35:      	xorl	%eax, %eax
      37:      	cmpl	$0xe, -0x10(%rbp)
      3b:      	movb	%al, -0x19(%rbp)
      3e:      	jne	0x55 <_main+0x55>
      40:      	xorl	%eax, %eax
      42:      	cmpl	$0x4, -0x14(%rbp)
      46:      	movb	%al, -0x19(%rbp)
      49:      	jne	0x55 <_main+0x55>
      4b:      	cmpl	$0x2d, -0x18(%rbp)
      4f:      	sete	%al
      52:      	movb	%al, -0x19(%rbp)
      55:      	movb	-0x19(%rbp), %al
      58:      	andb	$0x1, %al
      5a:      	movzbl	%al, %eax
      5d:      	popq	%rbp
      5e:      	retq
```

</details>

### `380-unsigned-compare-predicates`

- Bucket: `strict`
- Test: `strict/380-unsigned-compare-predicates.t`
- Status: `reviewed / fine`
- Clang analogue: `380-unsigned-compare-predicates.cpp`
- Review note: The important compare/branch materialization issues are already covered by the structural compare owners.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/380-unsigned-compare-predicates.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	movl	$0x5, -0x8(%rbp)
      12:      	movl	$0x9, -0xc(%rbp)
      19:      	movl	-0x8(%rbp), %ecx
      1c:      	xorl	%eax, %eax
      1e:      	cmpl	-0xc(%rbp), %ecx
      21:      	movb	%al, -0xd(%rbp)
      24:      	jae	0x59 <_main+0x59>
      26:      	movl	-0x8(%rbp), %ecx
      29:      	xorl	%eax, %eax
      2b:      	cmpl	-0xc(%rbp), %ecx
      2e:      	movb	%al, -0xd(%rbp)
      31:      	ja	0x59 <_main+0x59>
      33:      	movl	-0xc(%rbp), %ecx
      36:      	xorl	%eax, %eax
      38:      	cmpl	-0x8(%rbp), %ecx
      3b:      	movb	%al, -0xd(%rbp)
      3e:      	jbe	0x59 <_main+0x59>
      40:      	movl	-0xc(%rbp), %ecx
      43:      	xorl	%eax, %eax
      45:      	cmpl	-0x8(%rbp), %ecx
      48:      	movb	%al, -0xd(%rbp)
      4b:      	jb	0x59 <_main+0x59>
      4d:      	movl	-0x8(%rbp), %eax
      50:      	cmpl	-0xc(%rbp), %eax
      53:      	setne	%al
      56:      	movb	%al, -0xd(%rbp)
      59:      	movb	-0xd(%rbp), %al
      5c:      	andb	$0x1, %al
      5e:      	movzbl	%al, %eax
      61:      	popq	%rbp
      62:      	retq
```

</details>

### `410-u32-bswap-and-float-conversions`

- Bucket: `strict`
- Test: `strict/410-u32-bswap-and-float-conversions.t`
- Status: `reviewed / fine`
- Clang analogue: `410-u32-bswap-and-float-conversions.cpp`
- Review note: No additional actionable lowering gap beyond the ordinary float-immediate cleanup.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/410-u32-bswap-and-float-conversions.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	subq	$0x20, %rsp
       8:      	movl	$0x0, -0x4(%rbp)
       f:      	movl	$0x11223344, %edi       ## imm = 0x11223344
      14:      	callq	0x19 <_main+0x19>
      19:      	movl	%eax, -0x8(%rbp)
      1c:      	movl	-0x8(%rbp), %eax
      1f:      	cvtsi2sd	%rax, %xmm0
      24:      	movsd	%xmm0, -0x10(%rbp)
      29:      	cvttsd2si	-0x10(%rbp), %rax
      2f:      	movl	%eax, -0x14(%rbp)
      32:      	movl	-0x8(%rbp), %eax
      35:      	cmpl	-0x14(%rbp), %eax
      38:      	sete	%al
      3b:      	andb	$0x1, %al
      3d:      	movzbl	%al, %eax
      40:      	addq	$0x20, %rsp
      44:      	popq	%rbp
      45:      	retq
      46:      	nopw	%cs:(%rax,%rax)

0000000000000050 <__ZL7bswap32j>:
      50:      	pushq	%rbp
      51:      	movq	%rsp, %rbp
      54:      	movl	%edi, -0x4(%rbp)
      57:      	movl	-0x4(%rbp), %eax
      5a:      	bswapl	%eax
      5c:      	popq	%rbp
      5d:      	retq
```

</details>

### `620-atomic-i32-exchange`

- Bucket: `strict`
- Test: `strict/620-atomic-i32-exchange.t`
- Status: `simplified here`
- Clang analogue: `620-atomic-i32-exchange.cpp`
- Review note: This review directly motivated the simpler seq_cst exchange path.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/620-atomic-i32-exchange.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	movl	$0xb, -0xc(%rbp)
      12:      	movl	-0xc(%rbp), %eax
      15:      	xchgl	%eax, (%rip)            ## 0x1b <_main+0x1b>
      1b:      	movl	%eax, -0x10(%rbp)
      1e:      	movl	-0x10(%rbp), %eax
      21:      	movl	%eax, -0x8(%rbp)
      24:      	movl	(%rip), %eax            ## 0x2a <_main+0x2a>
      2a:      	movl	%eax, -0x18(%rbp)
      2d:      	movl	-0x18(%rbp), %eax
      30:      	movl	%eax, -0x14(%rbp)
      33:      	xorl	%eax, %eax
      35:      	cmpl	$0x7, -0x8(%rbp)
      39:      	movb	%al, -0x19(%rbp)
      3c:      	jne	0x48 <_main+0x48>
      3e:      	cmpl	$0xb, -0x14(%rbp)
      42:      	sete	%al
      45:      	movb	%al, -0x19(%rbp)
      48:      	movb	-0x19(%rbp), %al
      4b:      	andb	$0x1, %al
      4d:      	movzbl	%al, %eax
      50:      	popq	%rbp
      51:      	retq
```

</details>

### `640-atomic-i32-seqcst-store`

- Bucket: `strict`
- Test: `strict/640-atomic-i32-seqcst-store.t`
- Status: `simplified here`
- Clang analogue: `640-atomic-i32-seqcst-store.cpp`
- Review note: This review directly motivated the simpler seq_cst store path using `xchg` rather than fences around a store.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/640-atomic-i32-seqcst-store.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	movl	$0xb, -0x8(%rbp)
      12:      	movl	-0x8(%rbp), %eax
      15:      	xchgl	%eax, (%rip)            ## 0x1b <_main+0x1b>
      1b:      	movl	(%rip), %eax            ## 0x21 <_main+0x21>
      21:      	movl	%eax, -0x10(%rbp)
      24:      	movl	-0x10(%rbp), %eax
      27:      	movl	%eax, -0xc(%rbp)
      2a:      	cmpl	$0xb, -0xc(%rbp)
      2e:      	sete	%al
      31:      	andb	$0x1, %al
      33:      	movzbl	%al, %eax
      36:      	popq	%rbp
      37:      	retq
```

</details>

### `250-stack-arguments-beyond-six`

- Bucket: `structural`
- Test: `structural/250-stack-arguments-beyond-six.t`
- Status: `reviewed / fine`
- Clang analogue: `250-stack-arguments-beyond-six.cpp`
- Review note: The current call-lowering shape already matches the important stack-argument behavior.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/250-stack-arguments-beyond-six.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	subq	$0x20, %rsp
       8:      	movl	$0x0, -0x4(%rbp)
       f:      	movl	$0x1, %edi
      14:      	movl	$0x2, %esi
      19:      	movl	$0x3, %edx
      1e:      	movl	$0x4, %ecx
      23:      	movl	$0x5, %r8d
      29:      	movl	$0x6, %r9d
      2f:      	movq	$0x7, (%rsp)
      37:      	movq	$0x8, 0x8(%rsp)
      40:      	callq	0x45 <_main+0x45>
      45:      	subq	$0x24, %rax
      49:      	addq	$0x20, %rsp
      4d:      	popq	%rbp
      4e:      	retq
      4f:      	nop

0000000000000050 <__ZL4sum8llllllll>:
      50:      	pushq	%rbp
      51:      	movq	%rsp, %rbp
      54:      	movq	0x18(%rbp), %rax
      58:      	movq	0x10(%rbp), %rax
      5c:      	movq	%rdi, -0x8(%rbp)
      60:      	movq	%rsi, -0x10(%rbp)
      64:      	movq	%rdx, -0x18(%rbp)
      68:      	movq	%rcx, -0x20(%rbp)
      6c:      	movq	%r8, -0x28(%rbp)
      70:      	movq	%r9, -0x30(%rbp)
      74:      	movq	-0x8(%rbp), %rax
      78:      	addq	-0x10(%rbp), %rax
      7c:      	addq	-0x18(%rbp), %rax
      80:      	addq	-0x20(%rbp), %rax
      84:      	addq	-0x28(%rbp), %rax
      88:      	addq	-0x30(%rbp), %rax
      8c:      	addq	0x10(%rbp), %rax
      90:      	addq	0x18(%rbp), %rax
      94:      	popq	%rbp
      95:      	retq
```

</details>

### `260-f64-direct-call`

- Bucket: `structural`
- Test: `structural/260-f64-direct-call.t`
- Status: `reviewed / fine`
- Clang analogue: `260-f64-direct-call.cpp`
- Review note: Ordinary float call-site literals are now loaded directly into XMM registers rather than via stack scratch.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/260-f64-direct-call.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	subq	$0x10, %rsp
       8:      	movl	$0x0, -0x4(%rbp)
       f:      	movsd	0x61(%rip), %xmm0       ## 0x78 <__ZL3adddd+0x28>
      17:      	movsd	0x61(%rip), %xmm1       ## 0x80 <__ZL3adddd+0x30>
      1f:      	callq	0x24 <_main+0x24>
      24:      	movsd	0x44(%rip), %xmm1       ## 0x70 <__ZL3adddd+0x20>
      2c:      	ucomisd	%xmm1, %xmm0
      30:      	sete	%al
      33:      	setnp	%cl
      36:      	andb	%cl, %al
      38:      	andb	$0x1, %al
      3a:      	movzbl	%al, %eax
      3d:      	addq	$0x10, %rsp
      41:      	popq	%rbp
      42:      	retq
      43:      	nopw	%cs:(%rax,%rax)

0000000000000050 <__ZL3adddd>:
      50:      	pushq	%rbp
      51:      	movq	%rsp, %rbp
      54:      	movsd	%xmm0, -0x8(%rbp)
      59:      	movsd	%xmm1, -0x10(%rbp)
      5e:      	movsd	-0x8(%rbp), %xmm0
      63:      	addsd	-0x10(%rbp), %xmm0
      68:      	popq	%rbp
      69:      	retq
```

</details>

### `360-promoted-i32-compare-adjacent-pointer`

- Bucket: `structural`
- Test: `structural/360-promoted-i32-compare-adjacent-pointer.t`
- Status: `reviewed / fine`
- Clang analogue: `360-promoted-i32-compare-adjacent-pointer.cpp`
- Review note: Direct compare-fed branch and compare-as-value work already cover the main simplicity target here.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/360-promoted-i32-compare-adjacent-pointer.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	leaq	(%rip), %rax            ## 0x12 <_main+0x12>
      12:      	movq	%rax, -0x10(%rbp)
      16:      	movq	-0x10(%rbp), %rax
      1a:      	addq	$0x4, %rax
      1e:      	movq	%rax, -0x18(%rbp)
      22:      	movq	-0x18(%rbp), %rax
      26:      	cmpq	-0x10(%rbp), %rax
      2a:      	setne	%al
      2d:      	andb	$0x1, %al
      2f:      	movzbl	%al, %eax
      32:      	popq	%rbp
      33:      	retq
```

</details>

### `390-integral-float-conversions`

- Bucket: `structural`
- Test: `structural/390-integral-float-conversions.t`
- Status: `reviewed / fine`
- Clang analogue: `390-integral-float-conversions.cpp`
- Review note: No additional actionable gap after the ordinary float-immediate cleanup.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/390-integral-float-conversions.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	movq	$0x1, -0x10(%rbp)
      13:      	movsd	0x3d(%rip), %xmm0       ## 0x58 <_main+0x58>
      1b:      	movsd	%xmm0, -0x18(%rbp)
      20:      	xorl	%eax, %eax
      22:      	cmpq	$0x1, -0x10(%rbp)
      27:      	movb	%al, -0x19(%rbp)
      2a:      	jne	0x48 <_main+0x48>
      2c:      	movsd	-0x18(%rbp), %xmm0
      31:      	movsd	0x1f(%rip), %xmm1       ## 0x58 <_main+0x58>
      39:      	ucomisd	%xmm1, %xmm0
      3d:      	sete	%al
      40:      	setnp	%cl
      43:      	andb	%cl, %al
      45:      	movb	%al, -0x19(%rbp)
      48:      	movb	-0x19(%rbp), %al
      4b:      	andb	$0x1, %al
      4d:      	movzbl	%al, %eax
      50:      	popq	%rbp
      51:      	retq
```

</details>

### `400-float-width-conversions`

- Bucket: `structural`
- Test: `structural/400-float-width-conversions.t`
- Status: `reviewed / fine`
- Clang analogue: `400-float-width-conversions.cpp`
- Review note: No additional actionable gap after the ordinary float-immediate cleanup.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/400-float-width-conversions.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	movl	$0x3fc00000, -0x8(%rbp) ## imm = 0x3FC00000
      12:      	movss	-0x8(%rbp), %xmm0
      17:      	cvtss2sd	%xmm0, %xmm0
      1b:      	movsd	%xmm0, -0x10(%rbp)
      20:      	fldl	-0x10(%rbp)
      23:      	fstpt	-0x20(%rbp)
      26:      	fldt	-0x20(%rbp)
      29:      	fstpl	-0x30(%rbp)
      2c:      	movsd	-0x30(%rbp), %xmm0
      31:      	movsd	%xmm0, -0x28(%rbp)
      36:      	movsd	-0x28(%rbp), %xmm0
      3b:      	movsd	0x15(%rip), %xmm1       ## 0x58 <_main+0x58>
      43:      	ucomisd	%xmm1, %xmm0
      47:      	seta	%al
      4a:      	andb	$0x1, %al
      4c:      	movzbl	%al, %eax
      4f:      	popq	%rbp
      50:      	retq
```

</details>

### `410-f32-ordered-compare-branch`

- Bucket: `structural`
- Test: `structural/410-f32-ordered-compare-branch.t`
- Status: `reviewed / fine`
- Clang analogue: `410-f32-ordered-compare-branch.cpp`
- Review note: Direct compare-fed branches are now in the right structural shape.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/410-f32-ordered-compare-branch.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	movss	0x3d(%rip), %xmm0       ## 0x50 <_main+0x50>
      13:      	movss	%xmm0, -0x8(%rbp)
      18:      	movss	0x2c(%rip), %xmm0       ## 0x4c <_main+0x4c>
      20:      	movss	%xmm0, -0xc(%rbp)
      25:      	movss	-0x8(%rbp), %xmm1
      2a:      	movss	-0xc(%rbp), %xmm0
      2f:      	ucomiss	%xmm1, %xmm0
      32:      	jbe	0x3d <_main+0x3d>
      34:      	movl	$0x1, -0x4(%rbp)
      3b:      	jmp	0x44 <_main+0x44>
      3d:      	movl	$0x0, -0x4(%rbp)
      44:      	movl	-0x4(%rbp), %eax
      47:      	popq	%rbp
      48:      	retq
```

</details>

### `420-i32-direct-compare-branch`

- Bucket: `structural`
- Test: `structural/420-i32-direct-compare-branch.t`
- Status: `reviewed / fine`
- Clang analogue: `420-i32-direct-compare-branch.cpp`
- Review note: Direct integer compare-to-branch shape is now in-family with Clang `-O0`.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/420-i32-direct-compare-branch.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	movl	$0x7, -0x8(%rbp)
      12:      	cmpl	$0x7, -0x8(%rbp)
      16:      	jne	0x21 <_main+0x21>
      18:      	movl	$0x1, -0x4(%rbp)
      1f:      	jmp	0x28 <_main+0x28>
      21:      	movl	$0x0, -0x4(%rbp)
      28:      	movl	-0x4(%rbp), %eax
      2b:      	popq	%rbp
      2c:      	retq
```

</details>

### `430-u32-direct-compare-branch`

- Bucket: `structural`
- Test: `structural/430-u32-direct-compare-branch.t`
- Status: `reviewed / fine`
- Clang analogue: `430-u32-direct-compare-branch.cpp`
- Review note: Direct unsigned compare-to-branch shape is now in-family with Clang `-O0`.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/430-u32-direct-compare-branch.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	movl	$0x7, -0x8(%rbp)
      12:      	cmpl	$0x9, -0x8(%rbp)
      16:      	jae	0x21 <_main+0x21>
      18:      	movl	$0x1, -0x4(%rbp)
      1f:      	jmp	0x28 <_main+0x28>
      21:      	movl	$0x0, -0x4(%rbp)
      28:      	movl	-0x4(%rbp), %eax
      2b:      	popq	%rbp
      2c:      	retq
```

</details>

### `440-f64-eq-compare-branch`

- Bucket: `structural`
- Test: `structural/440-f64-eq-compare-branch.t`
- Status: `reviewed / fine`
- Clang analogue: `440-f64-eq-compare-branch.cpp`
- Review note: Direct floating compare-to-branch shape is now in-family with Clang `-O0`.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/440-f64-eq-compare-branch.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	movsd	0x35(%rip), %xmm0       ## 0x48 <_main+0x48>
      13:      	movsd	%xmm0, -0x10(%rbp)
      18:      	movsd	0x28(%rip), %xmm0       ## 0x48 <_main+0x48>
      20:      	movsd	%xmm0, -0x18(%rbp)
      25:      	movsd	-0x10(%rbp), %xmm0
      2a:      	ucomisd	-0x18(%rbp), %xmm0
      2f:      	jne	0x3c <_main+0x3c>
      31:      	jp	0x3c <_main+0x3c>
      33:      	movl	$0x1, -0x4(%rbp)
      3a:      	jmp	0x43 <_main+0x43>
      3c:      	movl	$0x0, -0x4(%rbp)
      43:      	movl	-0x4(%rbp), %eax
      46:      	popq	%rbp
      47:      	retq
```

</details>

### `450-i64-leaf-register-chain`

- Bucket: `structural`
- Test: `structural/450-i64-leaf-register-chain.t`
- Status: `reviewed / fine`
- Clang analogue: `450-i64-leaf-register-chain.cpp`
- Review note: Our register-resident leaf lowering is already acceptable and is sometimes leaner than Clang `-O0`.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/450-i64-leaf-register-chain.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <__Z1fl>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movq	%rdi, -0x8(%rbp)
       8:      	movq	-0x8(%rbp), %rax
       c:      	addq	$0x1, %rax
      10:      	movq	%rax, -0x10(%rbp)
      14:      	movq	-0x10(%rbp), %rax
      18:      	addq	$0x2, %rax
      1c:      	movq	%rax, -0x18(%rbp)
      20:      	movq	-0x18(%rbp), %rax
      24:      	addq	$0x3, %rax
      28:      	movq	%rax, -0x20(%rbp)
      2c:      	movq	-0x20(%rbp), %rax
      30:      	addq	$0x4, %rax
      34:      	movq	%rax, -0x28(%rbp)
      38:      	movq	-0x28(%rbp), %rax
      3c:      	popq	%rbp
      3d:      	retq
      3e:      	nop

0000000000000040 <_main>:
      40:      	pushq	%rbp
      41:      	movq	%rsp, %rbp
      44:      	subq	$0x10, %rsp
      48:      	movl	$0x0, -0x4(%rbp)
      4f:      	movl	$0x1, %edi
      54:      	callq	0x59 <_main+0x19>
      59:      	cmpq	$0xb, %rax
      5d:      	sete	%al
      60:      	andb	$0x1, %al
      62:      	movzbl	%al, %eax
      65:      	addq	$0x10, %rsp
      69:      	popq	%rbp
      6a:      	retq
```

</details>

### `460-u32-compare-value-materialize`

- Bucket: `structural`
- Test: `structural/460-u32-compare-value-materialize.t`
- Status: `reviewed / fine`
- Clang analogue: `460-u32-compare-value-materialize.cpp`
- Review note: Value materialization uses a reasonable non-optimizing shape.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/460-u32-compare-value-materialize.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	movl	$0x7, -0x8(%rbp)
      12:      	movl	$0x9, -0xc(%rbp)
      19:      	movl	-0x8(%rbp), %eax
      1c:      	cmpl	-0xc(%rbp), %eax
      1f:      	setb	%al
      22:      	andb	$0x1, %al
      24:      	movzbl	%al, %eax
      27:      	movl	%eax, -0x10(%rbp)
      2a:      	movl	-0x8(%rbp), %eax
      2d:      	cmpl	-0xc(%rbp), %eax
      30:      	setne	%al
      33:      	andb	$0x1, %al
      35:      	movzbl	%al, %eax
      38:      	movl	%eax, -0x14(%rbp)
      3b:      	movl	-0x10(%rbp), %eax
      3e:      	addl	-0x14(%rbp), %eax
      41:      	cmpl	$0x2, %eax
      44:      	sete	%al
      47:      	andb	$0x1, %al
      49:      	movzbl	%al, %eax
      4c:      	popq	%rbp
      4d:      	retq
```

</details>

### `470-f32-leaf-register-chain`

- Bucket: `structural`
- Test: `structural/470-f32-leaf-register-chain.t`
- Status: `reviewed / fine`
- Clang analogue: `470-f32-leaf-register-chain.cpp`
- Review note: The ordinary `f32` XMM path is now in good shape.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/470-f32-leaf-register-chain.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <__Z1ff>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movss	%xmm0, -0x4(%rbp)
       9:      	movss	0x7f(%rip), %xmm0       ## 0x90 <_main+0x40>
      11:      	addss	-0x4(%rbp), %xmm0
      16:      	movss	%xmm0, -0x8(%rbp)
      1b:      	movss	0x69(%rip), %xmm0       ## 0x8c <_main+0x3c>
      23:      	addss	-0x8(%rbp), %xmm0
      28:      	movss	%xmm0, -0xc(%rbp)
      2d:      	movss	0x53(%rip), %xmm0       ## 0x88 <_main+0x38>
      35:      	addss	-0xc(%rbp), %xmm0
      3a:      	movss	%xmm0, -0x10(%rbp)
      3f:      	movss	-0x10(%rbp), %xmm0
      44:      	popq	%rbp
      45:      	retq
      46:      	nopw	%cs:(%rax,%rax)

0000000000000050 <_main>:
      50:      	pushq	%rbp
      51:      	movq	%rsp, %rbp
      54:      	subq	$0x10, %rsp
      58:      	movl	$0x0, -0x4(%rbp)
      5f:      	movss	0x31(%rip), %xmm0       ## 0x98 <_main+0x48>
      67:      	callq	0x6c <_main+0x1c>
      6c:      	movss	0x20(%rip), %xmm1       ## 0x94 <_main+0x44>
      74:      	ucomiss	%xmm1, %xmm0
      77:      	seta	%al
      7a:      	andb	$0x1, %al
      7c:      	movzbl	%al, %eax
      7f:      	addq	$0x10, %rsp
      83:      	popq	%rbp
      84:      	retq
```

</details>

### `480-f64-leaf-copy-chain`

- Bucket: `structural`
- Test: `structural/480-f64-leaf-copy-chain.t`
- Status: `reviewed / fine`
- Clang analogue: `480-f64-leaf-copy-chain.cpp`
- Review note: The ordinary `f64` XMM path is now in good shape.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/480-f64-leaf-copy-chain.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <__Z1fd>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movsd	%xmm0, -0x8(%rbp)
       9:      	movsd	-0x8(%rbp), %xmm0
       e:      	movsd	%xmm0, -0x10(%rbp)
      13:      	movsd	-0x10(%rbp), %xmm0
      18:      	movsd	%xmm0, -0x18(%rbp)
      1d:      	movsd	-0x18(%rbp), %xmm0
      22:      	movsd	%xmm0, -0x20(%rbp)
      27:      	movsd	-0x20(%rbp), %xmm0
      2c:      	popq	%rbp
      2d:      	retq
      2e:      	nop

0000000000000030 <_main>:
      30:      	pushq	%rbp
      31:      	movq	%rsp, %rbp
      34:      	subq	$0x10, %rsp
      38:      	movl	$0x0, -0x4(%rbp)
      3f:      	movsd	0x29(%rip), %xmm0       ## 0x70 <_main+0x40>
      47:      	callq	0x4c <_main+0x1c>
      4c:      	movsd	0x14(%rip), %xmm1       ## 0x68 <_main+0x38>
      54:      	ucomisd	%xmm1, %xmm0
      58:      	seta	%al
      5b:      	andb	$0x1, %al
      5d:      	movzbl	%al, %eax
      60:      	addq	$0x10, %rsp
      64:      	popq	%rbp
      65:      	retq
```

</details>

### `490-call-clobber-register-pressure`

- Bucket: `structural`
- Test: `structural/490-call-clobber-register-pressure.t`
- Status: `reviewed / fine`
- Clang analogue: `490-call-clobber-register-pressure.cpp`
- Review note: No further simplicity gap stood out in the mixed live-range/call-pressure case.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/490-call-clobber-register-pressure.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	subq	$0x40, %rsp
       8:      	movl	$0x0, -0x4(%rbp)
       f:      	movq	$0x1, -0x10(%rbp)
      17:      	movq	$0x2, -0x18(%rbp)
      1f:      	movq	$0x3, -0x20(%rbp)
      27:      	movq	$0x4, -0x28(%rbp)
      2f:      	movq	$0x5, -0x30(%rbp)
      37:      	movq	$0x6, -0x38(%rbp)
      3f:      	movq	-0x10(%rbp), %rdi
      43:      	movq	-0x18(%rbp), %rsi
      47:      	movq	-0x20(%rbp), %rdx
      4b:      	movq	-0x28(%rbp), %rcx
      4f:      	movq	-0x30(%rbp), %r8
      53:      	movq	-0x38(%rbp), %r9
      57:      	callq	0x5c <_main+0x5c>
      5c:      	cmpq	$0x15, %rax
      60:      	sete	%al
      63:      	andb	$0x1, %al
      65:      	movzbl	%al, %eax
      68:      	addq	$0x40, %rsp
      6c:      	popq	%rbp
      6d:      	retq
      6e:      	nop

0000000000000070 <__ZL1hllllll>:
      70:      	pushq	%rbp
      71:      	movq	%rsp, %rbp
      74:      	movq	%rdi, -0x8(%rbp)
      78:      	movq	%rsi, -0x10(%rbp)
      7c:      	movq	%rdx, -0x18(%rbp)
      80:      	movq	%rcx, -0x20(%rbp)
      84:      	movq	%r8, -0x28(%rbp)
      88:      	movq	%r9, -0x30(%rbp)
      8c:      	movq	-0x8(%rbp), %rax
      90:      	addq	-0x10(%rbp), %rax
      94:      	addq	-0x18(%rbp), %rax
      98:      	addq	-0x20(%rbp), %rax
      9c:      	addq	-0x28(%rbp), %rax
      a0:      	addq	-0x30(%rbp), %rax
      a4:      	popq	%rbp
      a5:      	retq
```

</details>

### `500-f64-compare-value-materialize`

- Bucket: `structural`
- Test: `structural/500-f64-compare-value-materialize.t`
- Status: `reviewed / fine`
- Clang analogue: `500-f64-compare-value-materialize.cpp`
- Review note: Value materialization uses a reasonable non-optimizing shape.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/500-f64-compare-value-materialize.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	movsd	0x6d(%rip), %xmm0       ## 0x80 <_main+0x80>
      13:      	movsd	%xmm0, -0x10(%rbp)
      18:      	movsd	0x60(%rip), %xmm0       ## 0x80 <_main+0x80>
      20:      	movsd	%xmm0, -0x18(%rbp)
      25:      	movsd	-0x10(%rbp), %xmm0
      2a:      	ucomisd	-0x18(%rbp), %xmm0
      2f:      	sete	%al
      32:      	setnp	%cl
      35:      	andb	%cl, %al
      37:      	andb	$0x1, %al
      39:      	movzbl	%al, %eax
      3c:      	movl	%eax, -0x1c(%rbp)
      3f:      	movsd	-0x10(%rbp), %xmm0
      44:      	movsd	0x2c(%rip), %xmm1       ## 0x78 <_main+0x78>
      4c:      	ucomisd	%xmm1, %xmm0
      50:      	setne	%al
      53:      	setp	%cl
      56:      	orb	%cl, %al
      58:      	andb	$0x1, %al
      5a:      	movzbl	%al, %eax
      5d:      	movl	%eax, -0x20(%rbp)
      60:      	movl	-0x1c(%rbp), %eax
      63:      	addl	-0x20(%rbp), %eax
      66:      	cmpl	$0x2, %eax
      69:      	sete	%al
      6c:      	andb	$0x1, %al
      6e:      	movzbl	%al, %eax
      71:      	popq	%rbp
      72:      	retq
```

</details>

### `510-u32-f64-conversion-branch`

- Bucket: `structural`
- Test: `structural/510-u32-f64-conversion-branch.t`
- Status: `reviewed / fine`
- Clang analogue: `510-u32-f64-conversion-branch.cpp`
- Review note: No additional actionable gap after the direct branch and float-immediate fixes.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/510-u32-f64-conversion-branch.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	movl	$0x7, -0x8(%rbp)
      12:      	movl	-0x8(%rbp), %eax
      15:      	cvtsi2sd	%rax, %xmm0
      1a:      	movsd	%xmm0, -0x10(%rbp)
      1f:      	cvttsd2si	-0x10(%rbp), %rax
      25:      	movl	%eax, -0x14(%rbp)
      28:      	cmpl	$0x7, -0x14(%rbp)
      2c:      	jne	0x37 <_main+0x37>
      2e:      	movl	$0x1, -0x4(%rbp)
      35:      	jmp	0x3e <_main+0x3e>
      37:      	movl	$0x0, -0x4(%rbp)
      3e:      	movl	-0x4(%rbp), %eax
      41:      	popq	%rbp
      42:      	retq
```

</details>

### `520-i8-direct-compare-branch`

- Bucket: `structural`
- Test: `structural/520-i8-direct-compare-branch.t`
- Status: `reviewed / fine`
- Clang analogue: `520-i8-direct-compare-branch.cpp`
- Review note: Narrow signed compare-to-branch shape is now acceptable.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/520-i8-direct-compare-branch.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	movb	$0x7, -0x5(%rbp)
       f:      	movsbl	-0x5(%rbp), %eax
      13:      	cmpl	$0x7, %eax
      16:      	jne	0x21 <_main+0x21>
      18:      	movl	$0x1, -0x4(%rbp)
      1f:      	jmp	0x28 <_main+0x28>
      21:      	movl	$0x0, -0x4(%rbp)
      28:      	movl	-0x4(%rbp), %eax
      2b:      	popq	%rbp
      2c:      	retq
```

</details>

### `530-u16-direct-compare-branch`

- Bucket: `structural`
- Test: `structural/530-u16-direct-compare-branch.t`
- Status: `reviewed / fine`
- Clang analogue: `530-u16-direct-compare-branch.cpp`
- Review note: Narrow unsigned compare-to-branch shape is now acceptable.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/530-u16-direct-compare-branch.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	movw	$0x7, -0x6(%rbp)
      11:      	movzwl	-0x6(%rbp), %eax
      15:      	cmpl	$0x9, %eax
      18:      	jge	0x23 <_main+0x23>
      1a:      	movl	$0x1, -0x4(%rbp)
      21:      	jmp	0x2a <_main+0x2a>
      23:      	movl	$0x0, -0x4(%rbp)
      2a:      	movl	-0x4(%rbp), %eax
      2d:      	popq	%rbp
      2e:      	retq
```

</details>

### `540-i16-leaf-normalize-chain`

- Bucket: `structural`
- Test: `structural/540-i16-leaf-normalize-chain.t`
- Status: `reviewed / fine`
- Clang analogue: `540-i16-leaf-normalize-chain.cpp`
- Review note: Narrow integer normalization is already in a reasonable PA23 shape.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/540-i16-leaf-normalize-chain.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <__Z1fs>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movw	%di, %ax
       7:      	movw	%ax, -0x2(%rbp)
       b:      	movw	-0x2(%rbp), %ax
       f:      	movw	%ax, -0x4(%rbp)
      13:      	movswl	-0x4(%rbp), %eax
      17:      	addl	$0x1, %eax
      1a:      	movw	%ax, -0x6(%rbp)
      1e:      	movswl	-0x6(%rbp), %eax
      22:      	addl	$0x1, %eax
      25:      	movw	%ax, -0x8(%rbp)
      29:      	movswl	-0x8(%rbp), %eax
      2d:      	popq	%rbp
      2e:      	retq
      2f:      	nop

0000000000000030 <_main>:
      30:      	pushq	%rbp
      31:      	movq	%rsp, %rbp
      34:      	subq	$0x10, %rsp
      38:      	movl	$0x0, -0x4(%rbp)
      3f:      	movl	$0x1, %edi
      44:      	callq	0x49 <_main+0x19>
      49:      	cmpl	$0x3, %eax
      4c:      	sete	%al
      4f:      	andb	$0x1, %al
      51:      	movzbl	%al, %eax
      54:      	addq	$0x10, %rsp
      58:      	popq	%rbp
      59:      	retq
```

</details>

### `550-i64-direct-compare-branch`

- Bucket: `structural`
- Test: `structural/550-i64-direct-compare-branch.t`
- Status: `reviewed / fine`
- Clang analogue: `550-i64-direct-compare-branch.cpp`
- Review note: Direct 64-bit compare-to-branch shape is acceptable.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/550-i64-direct-compare-branch.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	movq	$0x7, -0x10(%rbp)
      13:      	cmpq	$0x7, -0x10(%rbp)
      18:      	jne	0x23 <_main+0x23>
      1a:      	movl	$0x1, -0x4(%rbp)
      21:      	jmp	0x2a <_main+0x2a>
      23:      	movl	$0x0, -0x4(%rbp)
      2a:      	movl	-0x4(%rbp), %eax
      2d:      	popq	%rbp
      2e:      	retq
```

</details>

### `560-ptr-null-direct-compare-branch`

- Bucket: `structural`
- Test: `structural/560-ptr-null-direct-compare-branch.t`
- Status: `reviewed / fine`
- Clang analogue: `560-ptr-null-direct-compare-branch.cpp`
- Review note: Pointer/null compare-to-branch shape is acceptable.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/560-ptr-null-direct-compare-branch.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	movq	$0x0, -0x10(%rbp)
      13:      	cmpq	$0x0, -0x10(%rbp)
      18:      	jne	0x23 <_main+0x23>
      1a:      	movl	$0x1, -0x4(%rbp)
      21:      	jmp	0x2a <_main+0x2a>
      23:      	movl	$0x0, -0x4(%rbp)
      2a:      	movl	-0x4(%rbp), %eax
      2d:      	popq	%rbp
      2e:      	retq
```

</details>

### `570-ptr-index-arithmetic`

- Bucket: `structural`
- Test: `structural/570-ptr-index-arithmetic.t`
- Status: `reviewed / fine`
- Clang analogue: `570-ptr-index-arithmetic.cpp`
- Review note: This review originally exposed the constant scaled-index simplification.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/570-ptr-index-arithmetic.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	leaq	(%rip), %rax            ## 0x12 <_main+0x12>
      12:      	movq	%rax, -0x10(%rbp)
      16:      	movq	-0x10(%rbp), %rax
      1a:      	addq	$0x8, %rax
      1e:      	movq	%rax, -0x18(%rbp)
      22:      	movq	-0x18(%rbp), %rax
      26:      	movq	-0x10(%rbp), %rcx
      2a:      	subq	%rcx, %rax
      2d:      	sarq	$0x3, %rax
      31:      	movq	%rax, -0x20(%rbp)
      35:      	xorl	%eax, %eax
      37:      	cmpq	$0x1, -0x20(%rbp)
      3c:      	movb	%al, -0x21(%rbp)
      3f:      	jne	0x4f <_main+0x4f>
      41:      	movq	-0x18(%rbp), %rax
      45:      	cmpq	$0xd, (%rax)
      49:      	sete	%al
      4c:      	movb	%al, -0x21(%rbp)
      4f:      	movb	-0x21(%rbp), %al
      52:      	andb	$0x1, %al
      54:      	movzbl	%al, %eax
      57:      	popq	%rbp
      58:      	retq
```

</details>

### `580-mixed-gpr-xmm-call-abi`

- Bucket: `structural`
- Test: `structural/580-mixed-gpr-xmm-call-abi.t`
- Status: `reviewed / fine`
- Clang analogue: `580-mixed-gpr-xmm-call-abi.cpp`
- Review note: The mixed direct-call ABI shape is now fine.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/580-mixed-gpr-xmm-call-abi.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <__Z3mixldld>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movq	%rdi, -0x8(%rbp)
       8:      	movsd	%xmm0, -0x10(%rbp)
       d:      	movq	%rsi, -0x18(%rbp)
      11:      	movsd	%xmm1, -0x20(%rbp)
      16:      	movq	-0x8(%rbp), %rax
      1a:      	cvttsd2si	-0x10(%rbp), %rcx
      20:      	addq	%rcx, %rax
      23:      	addq	-0x18(%rbp), %rax
      27:      	cvttsd2si	-0x20(%rbp), %rcx
      2d:      	addq	%rcx, %rax
      30:      	popq	%rbp
      31:      	retq
      32:      	nopw	%cs:(%rax,%rax)

0000000000000040 <_main>:
      40:      	pushq	%rbp
      41:      	movq	%rsp, %rbp
      44:      	subq	$0x10, %rsp
      48:      	movl	$0x0, -0x4(%rbp)
      4f:      	movl	$0xa, %edi
      54:      	movsd	0x24(%rip), %xmm0       ## 0x80 <_main+0x40>
      5c:      	movl	$0x14, %esi
      61:      	movsd	0x1f(%rip), %xmm1       ## 0x88 <_main+0x48>
      69:      	callq	0x6e <_main+0x2e>
      6e:      	cmpq	$0x21, %rax
      72:      	sete	%al
      75:      	andb	$0x1, %al
      77:      	movzbl	%al, %eax
      7a:      	addq	$0x10, %rsp
      7e:      	popq	%rbp
      7f:      	retq
```

</details>

### `590-f80-arithmetic-compare-owner`

- Bucket: `structural`
- Test: `structural/590-f80-arithmetic-compare-owner.t`
- Status: `reviewed / fine`
- Clang analogue: `590-f80-arithmetic-compare-owner.cpp`
- Review note: The conservative x87 path remains acceptable here.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/590-f80-arithmetic-compare-owner.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	fld1
       d:      	fstpt	-0x20(%rbp)
      10:      	flds	0x2a(%rip)              ## 0x40 <_main+0x40>
      16:      	fstpt	-0x30(%rbp)
      19:      	fldt	-0x20(%rbp)
      1c:      	fldt	-0x30(%rbp)
      1f:      	faddp	%st, %st(1)
      21:      	fstpt	-0x40(%rbp)
      24:      	fldt	-0x40(%rbp)
      27:      	flds	0x17(%rip)              ## 0x44 <_main+0x44>
      2d:      	fucompi	%st(1), %st
      2f:      	fstp	%st(0)
      31:      	setnp	%cl
      34:      	sete	%al
      37:      	andb	%cl, %al
      39:      	andb	$0x1, %al
      3b:      	movzbl	%al, %eax
      3e:      	popq	%rbp
      3f:      	retq
```

</details>

### `600-atomic-i8-load-store`

- Bucket: `structural`
- Test: `structural/600-atomic-i8-load-store.t`
- Status: `reviewed / fine`
- Clang analogue: `600-atomic-i8-load-store.cpp`
- Review note: No additional simplification beyond the seq_cst-specific fixes was indicated here.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/600-atomic-i8-load-store.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	movb	(%rip), %al             ## 0x11 <_main+0x11>
      11:      	movb	%al, -0x6(%rbp)
      14:      	movb	-0x6(%rbp), %al
      17:      	movb	%al, -0x5(%rbp)
      1a:      	movb	-0x5(%rbp), %al
      1d:      	incb	%al
      1f:      	movb	%al, -0x7(%rbp)
      22:      	movb	-0x7(%rbp), %al
      25:      	movb	%al, (%rip)             ## 0x2b <_main+0x2b>
      2b:      	movb	(%rip), %al             ## 0x31 <_main+0x31>
      31:      	movb	%al, -0x9(%rbp)
      34:      	movb	-0x9(%rbp), %al
      37:      	movb	%al, -0x8(%rbp)
      3a:      	movsbl	-0x8(%rbp), %eax
      3e:      	cmpl	$0x8, %eax
      41:      	sete	%al
      44:      	andb	$0x1, %al
      46:      	movzbl	%al, %eax
      49:      	popq	%rbp
      4a:      	retq
```

</details>

### `610-indirect-mixed-gpr-xmm-call-abi`

- Bucket: `structural`
- Test: `structural/610-indirect-mixed-gpr-xmm-call-abi.t`
- Status: `reviewed / fine`
- Clang analogue: `610-indirect-mixed-gpr-xmm-call-abi.cpp`
- Review note: The correctness fix is landed and the resulting shape is acceptable.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/610-indirect-mixed-gpr-xmm-call-abi.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <__Z3mixldld>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movq	%rdi, -0x8(%rbp)
       8:      	movsd	%xmm0, -0x10(%rbp)
       d:      	movq	%rsi, -0x18(%rbp)
      11:      	movsd	%xmm1, -0x20(%rbp)
      16:      	movq	-0x8(%rbp), %rax
      1a:      	cvttsd2si	-0x10(%rbp), %rcx
      20:      	addq	%rcx, %rax
      23:      	addq	-0x18(%rbp), %rax
      27:      	cvttsd2si	-0x20(%rbp), %rcx
      2d:      	addq	%rcx, %rax
      30:      	popq	%rbp
      31:      	retq
      32:      	nopw	%cs:(%rax,%rax)

0000000000000040 <_main>:
      40:      	pushq	%rbp
      41:      	movq	%rsp, %rbp
      44:      	subq	$0x10, %rsp
      48:      	movl	$0x0, -0x4(%rbp)
      4f:      	leaq	(%rip), %rax            ## 0x56 <_main+0x16>
      56:      	movq	%rax, -0x10(%rbp)
      5a:      	movl	$0xa, %edi
      5f:      	movsd	0x29(%rip), %xmm0       ## 0x90 <_main+0x50>
      67:      	movl	$0x14, %esi
      6c:      	movsd	0x24(%rip), %xmm1       ## 0x98 <_main+0x58>
      74:      	callq	*-0x10(%rbp)
      77:      	cmpq	$0x21, %rax
      7b:      	sete	%al
      7e:      	andb	$0x1, %al
      80:      	movzbl	%al, %eax
      83:      	addq	$0x10, %rsp
      87:      	popq	%rbp
      88:      	retq
```

</details>

### `630-ptr-compare-value-materialize`

- Bucket: `structural`
- Test: `structural/630-ptr-compare-value-materialize.t`
- Status: `reviewed / fine`
- Clang analogue: `630-ptr-compare-value-materialize.cpp`
- Review note: Pointer compare-as-value materialization is acceptable.

<details>
<summary>Clang <code>-O0</code> disassembly</summary>

```asm

/tmp/pa23-disasm-review-20260408/out/630-ptr-compare-value-materialize.o:	file format mach-o 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 <_main>:
       0:      	pushq	%rbp
       1:      	movq	%rsp, %rbp
       4:      	movl	$0x0, -0x4(%rbp)
       b:      	leaq	(%rip), %rax            ## 0x12 <_main+0x12>
      12:      	movq	%rax, -0x10(%rbp)
      16:      	movq	-0x10(%rbp), %rax
      1a:      	addq	$0x8, %rax
      1e:      	movq	%rax, -0x18(%rbp)
      22:      	movq	-0x18(%rbp), %rax
      26:      	cmpq	-0x18(%rbp), %rax
      2a:      	sete	%al
      2d:      	andb	$0x1, %al
      2f:      	movzbl	%al, %eax
      32:      	movl	%eax, -0x1c(%rbp)
      35:      	movq	-0x18(%rbp), %rax
      39:      	cmpq	-0x10(%rbp), %rax
      3d:      	setne	%al
      40:      	andb	$0x1, %al
      42:      	movzbl	%al, %eax
      45:      	movl	%eax, -0x20(%rbp)
      48:      	movl	-0x1c(%rbp), %eax
      4b:      	addl	-0x20(%rbp), %eax
      4e:      	cmpl	$0x2, %eax
      51:      	sete	%al
      54:      	andb	$0x1, %al
      56:      	movzbl	%al, %eax
      59:      	popq	%rbp
      5a:      	retq
```

</details>
